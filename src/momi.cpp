#include<iostream>
#include <curl/curl.h>

#include "momi.h"

namespace momi
{

Momi::Momi(int conn_num, std::string output_path, std::string filename, std::string url, int protocol)
    :conn_num(conn_num), output_path(output_path), filename(filename), url(url), protocol(protocol)
{
}

void Momi::init()
{
    this->d_type = DOWNLOAD_TYPE::NEW_D;
    this->t_type = TRANSFER_TYPE::IS_MULTI;

    if(local_check()) {
        momi::writelog("local check success");
    }
    if(remote_check()) {
        momi::writelog("remote check success");
    }

    if ( DOWNLOAD_TYPE::NEW_D == this->d_type) {
        this->conn_num = (TRANSFER_TYPE::IS_MULTI == this->t_type) ? \
                    min(this->conn_num, momi::MAX_THREAD_NUM) : 1;
        generate_conns();
    } else {
        load_temp_info();
    }
}

/**
 * @brief Momi::local_check
 * 如果目录不存在，则创建目录，并创建文件，如果文件不存在，则创建文件；
 * other，若下载未完成，则恢复下载，若下载完成，则自增文件名。
 */
bool Momi::local_check()
{
    std::string path = this->output_path;
    std::string filename = this->filename;
    u_int64_t filesize = this->filesize;

    if (momi::is_dir_exist(path) == -1) {
        //目录不存在，创建目录
        #if _WIN32
            _mkdir(path.c_str());
        #else
            mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        #endif
    }

    std::string basefilepath = path + filename;
    std::string filepath(basefilepath);
    int file_counter = 0;
    do {
        std::ifstream fin(filepath.c_str());
        if (!fin) {
            //文件不存在,创建文件
            //std::cout<<filepath<<std::endl;
            std::ofstream fout(filepath.c_str());
            if(!fout){
                momi::writelog("文件创建失败");
                return false;
            }
            fout.close();
            break;
        } else {
            //文件存在，1）下载已完成，文件名递增；2）下载未完成，恢复下载
            #if _WIN32
                struct _stat sb;
                if (_stat(filepath.c_str(), &sb) == 0) {
            #else
                struct stat sb;
                if (stat(filepath.c_str(), &sb) == 0) {
            #endif
                    file_counter++;
                    std::cout<<sb.st_size<<std::endl<<filesize<<std::endl;
                    if (sb.st_size == filesize) {
                        //下载完成
                        filepath = basefilepath + std::string("(") + std::to_string(file_counter) + std::string(")");
                        continue;
                    } else{
                        //下载未完成
                        this->d_type = DOWNLOAD_TYPE::RESUME_D;
                        momi::writelog("恢复下载");
                        break;
                    }
                }
        }
    } while(true);

    //缓存真正的输出文件
    this->filepath = filepath;
    return true;
}

bool Momi::remote_check()
{
    CURL *curl;
    CURLcode res;
    u_int64_t filesize = 0;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());

        if (momi::SKIP_PEER_VERIFICATION) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        }

        if (momi::SKIP_HOSTNAME_VERIFICATION) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        }
//分块支持检测
//        struct curl_slist *list = NULL;
//        list = curl_slist_append(list, "");
//        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cout<<"curl_easy_perform() failed: "<<curl_easy_strerror(res);
            return false;
        }

        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                          &filesize);
        if ((CURLE_OK == res) && (filesize>0)) {
            std::cout<<"filesize "<<this->filename<<":"<<(long long)filesize<<" bytes"<<std::endl;
            this->filesize = filesize;
        }
        curl_easy_cleanup(curl);
    }
    return true;
}

void Momi::generate_conns()
{
    //划分每个连接处理的文件段
    int size = this->filesize/(this->conn_num-1);
    int last_size = 0;

    int mod = this->filesize%this->conn_num;
    if (mod == 0) {
        size = this->filesize/this->conn_num;
    } else {
        last_size = this->filesize - ((this->conn_num-1)*size);
    }

    for (int i=1; i<=this->conn_num; i++) {
        Connection *conn = new Connection(this->url, this->filepath);
        conn->set_conn_id(i);
        conn->set_start_byte(size*(i-1)+1);
        conn->set_conn_size(size);
        if (i==this->conn_num) {
            conn->set_conn_size(last_size);
        }
        this->conns.push_back(conn);
    }
}

void load_temp_info()
{

}

void * Momi::thread_func(void *args_ptr)
{
    Args_Struct *args_p = (Args_Struct *)args_ptr;
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

        if (momi::SKIP_PEER_VERIFICATION) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        }

        if (momi::SKIP_HOSTNAME_VERIFICATION) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
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
    curl_global_init(CURL_GLOBAL_DEFAULT);

    pthread_t tid[this->conn_num];
    int error;
    for(int i=0; i<this->conn_num; i++){
        Args_Struct *args_ptr = (Args_Struct *)malloc(sizeof(Args_Struct));
        args_ptr->conn = this->conns[i];
        args_ptr->index = i;

        error = pthread_create(&tid[i], NULL, thread_func, args_ptr);
        if(0 != error){
            std::cout<<"线程"<<i+1<<"创建错误"<<std::endl;
        }else{
            std::cout<<"连接"<<i+1<<"开始下载"<<std::endl;
        }
    }

    for(int i=1; i<=this->conn_num; i++){
        error = pthread_join(tid[i], NULL);
        std::cout<<"连接"<<i<<"结束"<<std::endl;
    }
    curl_global_cleanup();
}

}
