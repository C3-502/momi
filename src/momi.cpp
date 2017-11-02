#include <iostream>
#include <stdio.h>
#include <curl/curl.h>
#include <thread>

#include "momi.h"
#include "loader.h"
#include "saver.h"

namespace momi
{

Momi::Momi(int thread_num, std::string output_path, std::string filename, std::string url, int protocol)
    :thread_num(thread_num), tasks_(0), saver_(new Saver)
{
    curl_global_init(CURL_GLOBAL_ALL);
    std::cout << curl_version() << std::endl;
    MomiTask* task = new MomiTask(url, output_path, filename, protocol, this);
    task->init();
    tasks_.push_back(task);
}

Momi::~Momi()
{
    curl_global_cleanup();
}

void Momi::init()
{

}

void Momi::run()
{
    MomiTask* task = tasks_[0];
    std::cout << "filesize=" << task->file_size() << std::endl;
    start_loader(task, 0, task->file_size());
    saver_->run();
}

void Momi::save(MomiTask* task, void* data, uint64_t start, uint64_t count)
{
    saver_->save_task(task, data, start, count);
}

void Momi::notify_finished(MomiTask *task)
{
    saver_->finish_task(task);
}

void Momi::start_loader(MomiTask *task, uint64_t start, uint64_t end)
{
    momi::Loader *loader = new HttpLoader(task, start, end);
    loader->run();
}

void MomiTask::init()
{
    local_check();
    remote_check();
}

bool MomiTask::local_check()
{
    std::string path = this->output_path_;
    std::string filename = this->filename_;

    if (!path.empty() && momi::is_dir_exist(path) == -1) {
        //目录不存在，创建目录
        #if _WIN32
            _mkdir(path.c_str());
        #else
            mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        #endif
    }

    this->filepath_ = path + filename;
    this->tmpfilepath_ = this->filepath_ + std::string(".mtmp");

    if (!file_exist(this->tmpfilepath_))
    {
        //文件不存在,创建临时文件
        size_t fd = open(this->tmpfilepath_.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0666);
        if(!fd){
            writelog("文件创建失败");
            return false;
        }
        this->fd_ = fd;
    }
    else
    {
        //上次下载未完成
        //this->d_type = DOWNLOAD_TYPE::RESUME_D;
        writelog("上次下载未完成");
    }
    return true;
}

bool MomiTask::remote_check()
{
    CURL *curl;
    CURLcode res;
    long filesize = 0;

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, this->url_.c_str());
        //SSL
        if (0 == (this->protocol_ % 2))
        {
            if (SKIP_SSL_VERIFY)
            {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
            }
            else
            {
                //todo
                //curl_easy_setopt(curl, CURLOPT_CAINFO, "");
                //curl_easy_setopt(curl, CURLOPT_CAPATH, "");
            }
        }

        //HTTP(S)分块支持检测
//        if (PROTOCOLS::P_HTTP == this->protocol_ || PROTOCOLS::P_HTTPS == this->protocol_)
//        {
//            std::string header;
//            struct curl_slist *list = NULL;
//            list = curl_slist_append(list, "range: bytes=0-");
//            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
//            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
//        }

        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            std::string err_msg = std::string("remote_check failed: ") + curl_easy_strerror(res);
            std::cout<< err_msg << std::endl;
            return false;
        }

        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &filesize);

        if (filesize > 0)
        {
            std::cout<<curl_version()<<std::endl;
            std::cout << "filesize: " << filesize << " bytes" <<std::endl;
            this->filesize_ = filesize;
        }
        curl_easy_cleanup(curl);
    }
    return true;
}

void MomiTask::async_save(void* buf, uint64_t start, uint64_t count)
{
    momi_->save(this, buf, start, count);
}

void MomiTask::notify_finished()
{
    momi_->notify_finished(this);
}

void MomiTask::rename()
{
    ::rename(tmpfilepath_.c_str(), filepath_.c_str());
}

void MomiTask::save(void* data, uint64_t start, uint64_t count)
{
    if (::lseek(fd_, start, SEEK_SET) < 0)
    {
        std::cerr << "lseek error!" << std::endl;
    }

    while (true)
    {
        ssize_t nwrite = ::write(fd_, data, count);
        printf("count=%d, nwrite=%d\n", count, nwrite);
        if (nwrite < 0)
        {

            std::cerr << "write error! " << strerror(errno) <<std::endl;
            continue;
        }
        else if (nwrite < count)
        {
            std::cerr << "write incomplete!" << std::endl;
            data += nwrite;
            count -= nwrite;
        }
        else
        {
            break;
        }
    }
    download_size_ += count;
    double process = double(download_size_) / double(filesize_);
    printf("process: %.2f%%\n", 100 * process);
}

void MomiTask::save_meta_info()
{
    std::string buf;
    uint32_t len = loaders_.size();
    buf += pack<uint32_t>(len);
    for (uint32_t i = 0; i < len; ++i)
    {
        Loader* loader = loaders_[i];
        loader->save_meta_info(buf);
    }
}

void MomiTask::update_status()
{
    for (Loader* loader : loaders_)
    {
        if (loader->status() != Loader::Complete)
            return;
    }
    status_ = Complete;
}

}
