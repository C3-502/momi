#ifndef MOMI_H
#define MOMI_H
#include <string>
#include <vector>
#include <pthread.h>

#include "define.h"
#include "util.h"
#include "connection.h"

class Momi
{
public:
    Momi(int conn_num, std::string output_path, std::string filename, std::string url, int protocol);

    Momi(const Momi&);

    /**
     * @brief extract_file_info 提取远程文件信息
     */
    void extract_file_info();

    /**
     * @brief local_check 本地文件检测，创建本地文件，以及下载恢复检测
     */
    void local_check();

    /**
     * @brief generate_conns 生成连接对象
     */
    void generate_conns();

    /**
     * @brief one_curl_download 线程处理函数
     */
    static void * one_curl_download(void *conn);

    /**
     * @brief process_data
     * @param buffer
     * @param size
     * @param nmemb
     * @param user_p
     * @return
     */
    static size_t process_data(void * buffer, size_t size, size_t nmemb, void *user_p);

    /**
     * @brief run 开始运行下载
     */
    void run();

private:
    int conn_num;
    std::string output_path;
    std::string filename;
    std::string url;
    int protocol;
    long long filesize=0;
    std::vector<Connection*> conns;
    std::string filepath;
};

/**
 * @brief The transStruct struct
 * 每个连接每次处理接受到的数据时带过去的参数
 */
struct transStruct {
  Connection *conn;
  std::ofstream *fp;
};

#endif // MOMI_H
