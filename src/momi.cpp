#include<iostream>
#include <curl/curl.h>

#include "momi.h"
#include "loader.h"

namespace momi
{

Momi::Momi(int thread_num, std::string output_path, std::string filename, std::string url, int protocol)
    :thread_num(thread_num), output_path(output_path), filename(filename), url(url), protocol(protocol)
{
}

Momi::~Momi()
{
    if (this->tmpfile_fd)
    {
        close(this->tmpfile_fd);
    }
}

void Momi::init()
{
    this->d_type = DOWNLOAD_TYPE::NEW_D;
    this->t_type = TRANSFER_TYPE::IS_MULTI;

    if(remote_check()) {
        writelog("remote check success");
    }

    if(local_check()) {
        writelog("local check success");
    }
    
    if ( DOWNLOAD_TYPE::NEW_D == this->d_type) {
        this->thread_num = (TRANSFER_TYPE::IS_MULTI == this->t_type) ? \
                    MIN(this->thread_num, momi::MAX_THREAD_NUM) : 1;
        generate_tempfile_info();
        generate_conns();
    } else {
        load_tempfile_info();
    }
}

bool Momi::remote_check()
{
    CURL *curl;
    CURLcode res;
    u_int64_t filesize = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
        //SSL
        if (0 == (this->protocol % 2)) {
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
        if (PROTOCOLS::P_HTTP == this->protocol || PROTOCOLS::P_HTTPS == this->protocol) {
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
            std::cout<<"filesize "<<this->filename<<":"<<(unsigned long long)filesize<<" bytes"<<std::endl;
            this->filesize = filesize;
        }
        curl_easy_cleanup(curl);
    }
    return true;
}

/**
 * @brief Momi::local_check
 * 如果目录不存在，则创建目录，并创建文件，如果文件不存在，则创建文件；
 * 否则，上次下载未完成
 */
bool Momi::local_check()
{
    std::string path = this->output_path;
    std::string filename = this->filename;

    if (momi::is_dir_exist(path) == -1) {
        //目录不存在，创建目录
        #if _WIN32
            _mkdir(path.c_str());
        #else
            mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        #endif
    }

    this->filepath = path + filename;
    this->tmpfilepath = this->filepath + std::string(".mtmp");

    std::ifstream fin(this->tmpfilepath.c_str());
    if (!fin) {
        //文件不存在,创建临时文件
        size_t fd = open(this->tmpfilepath.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0666);
        if(!fd){
            writelog("文件创建失败");
            return false;
        }
        this->tmpfile_fd = fd;
    } else { 
        //上次下载未完成
        this->d_type = DOWNLOAD_TYPE::RESUME_D;
        writelog("上次下载未完成");
    }
    return true;
}

void Momi::generate_conns()
{
    //划分每个连接处理的文件段
    int size = this->filesize/(this->thread_num-1);
    int last_size = 0;

    int mod = this->filesize%this->thread_num;
    if (mod == 0) {
        size = this->filesize/this->thread_num;
    } else {
        last_size = this->filesize - ((this->thread_num-1)*size);
    }

    for (int i=1; i<=this->thread_num; i++) {
        Connection *conn = new Connection(this->url, this->filepath);
        conn->set_conn_id(i);
        conn->set_start_byte(size*(i-1)+1);
        conn->set_conn_size(size);
        if (i==this->thread_num) {
            conn->set_conn_size(last_size);
        }
        this->conns.push_back(conn);
    }
}

bool Momi::generate_tempfile_info()
{
    u_int64_t tmpinfo_size = 0;
    u_int64_t tmpfile_size = this->filesize + tmpinfo_size;
    if (!ftruncate(this->tmpfile_fd, tmpfile_size)) {
        return false;
    }
    return true;
}

bool Momi::load_tempfile_info()
{
    return true;
}

bool Momi::before_finish()
{
    std::string dstfile = this->filepath;
    int counter = 1;
    do {
    #if _WIN32
        struct _stat sb;
        if (0 == stat(dstfile.c_str(), &sb)) {
    #else
        struct stat sb;
        if (0 == stat(dstfile.c_str(), &sb)) {
    #endif
            dstfile = dstfile + std::string("(") + std::to_string(counter) + std::string(")");
            counter++;
            continue;
        }
        break;
    } while(true);
    
    if (0 == rename(this->tmpfilepath.c_str(), dstfile.c_str())) {
        return true; 
    }
    return false;
}

void * Momi::thread_func(void *args_ptr)
{
    Args_Struct *args_p = (Args_Struct *)args_ptr;
    Momi *momi = args_p->momi;
    Connection *conn = args_p->conn;

    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, conn->get_url().c_str());

        std::string range = "range: bytes=";
        u_int64_t start_byte = conn->get_start_byte();
        u_int64_t end_byte = conn->get_end_byte();
        momi::str_append(range, start_byte);
        momi::str_append(range, "-");
        momi::str_append(range, end_byte);
        //std::cout<<conn->get_conn_id()<<","<<range<<std::endl;

        //设置HTTP头
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, range.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        if (0 == (momi->protocol % 2)) {
            if (SKIP_SSL_VERIFY) {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
            } else{
                //todo
                //curl_easy_setopt(curl, CURLOPT_CAINFO, "");
                //curl_easy_setopt(curl, CURLOPT_CAPATH, "");
            }   
        }

        curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);
        curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

        Trans_Struct trans;
        trans.conn = conn;
        std::ofstream out_fp = std::ofstream(conn->get_file_path());
        trans.fp = (std::ofstream *)&out_fp;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&trans);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cout<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;
        }
        std::cout<<"curl_easy_perform() ok"<<std::endl;
        out_fp.close();
        curl_easy_cleanup(curl);
    }
    return NULL;
}

size_t Momi::process_data(void * buffer, size_t size, size_t nmemb, void *user_p) {
    Trans_Struct *trans = (Trans_Struct *) user_p;
    size_t start_byte = trans->conn->get_start_byte();
    size_t transfer_size = size * nmemb;
    //定位本次写入位置
    trans->fp->seekp(start_byte-1, std::ios::beg);
    //更新起始位置
    trans->conn->set_start_byte(start_byte+transfer_size);
    //写入输出流
    trans->fp->write((const char *)buffer, transfer_size);
    return transfer_size;
}

void Momi::run()
{
    curl_global_init(CURL_GLOBAL_ALL);
    
    pthread_t tid[this->thread_num];
    int error;
    for(int i=0; i<this->thread_num; i++){
        Args_Struct *args_ptr = (Args_Struct *)malloc(sizeof(Args_Struct));
        args_ptr->momi = this;
        args_ptr->conn = this->conns[i];
        args_ptr->index = i;

        error = pthread_create(&tid[i], NULL, thread_func, args_ptr);
        if(0 != error){
            std::cout<<"线程"<<i+1<<"创建错误"<<std::endl;
        }else{
            std::cout<<"连接"<<i+1<<"开始下载"<<std::endl;
        }
    }

    for(int i=1; i<=this->thread_num; i++){
        error = pthread_join(tid[i], NULL);
        std::cout<<"连接"<<i<<"结束"<<std::endl;
    }
    curl_global_cleanup();
    momi::Loader *loader = new HttpLoader(0, 32);
    loader->run();
}

}
