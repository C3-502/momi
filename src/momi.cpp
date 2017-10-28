#include<iostream>
#include <curl/curl.h>
#include <thread>

#include "momi.h"
#include "loader.h"

namespace momi
{

Momi::Momi(int thread_num, std::string output_path, std::string filename, std::string url, int protocol)
    :thread_num(thread_num), tasks_(0)
{
    MomiTask* task = new MomiTask(url, output_path, filename, protocol);
    task->init();
    tasks_.push_back(task);
}

Momi::~Momi()
{

}

void Momi::init()
{

}

void Momi::run()
{
    curl_global_init(CURL_GLOBAL_ALL);
    // how to start loader?
    curl_global_cleanup();
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

    if (momi::is_dir_exist(path) == -1) {
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
    u_int64_t filesize = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, this->url_.c_str());
        //SSL
        if (0 == (this->protocol_ % 2)) {
            if (SKIP_SSL_VERIFY) {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
            } else{
                //todo
                //curl_easy_setopt(curl, CURLOPT_CAINFO, "");
                //curl_easy_setopt(curl, CURLOPT_CAPATH, "");
            }
        }

        //HTTP(S)分块支持检测
        if (PROTOCOLS::P_HTTP == this->protocol_ || PROTOCOLS::P_HTTPS == this->protocol_) {
            std::string header;
            struct curl_slist *list = NULL;
            list = curl_slist_append(list, "range: bytes=0-");
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        }

        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cout<<"remote_check failed: "<<curl_easy_strerror(res);
            return false;
        }

        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                          &filesize);
        if (filesize>0) {
            std::cout<<"filesize "<<this->filename_<<":"<<(unsigned long long)filesize<<" bytes"<<std::endl;
            this->filesize_ = filesize;
        }
        curl_easy_cleanup(curl);
    }
    return true;
}

void MomiTask::rename()
{

}

void MomiTask::save(char *buf, uint64_t start, uint64_t count)
{
    ::lseek(fd_, start, SEEK_SET);
    ::write(fd_, buf, count);
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

}
