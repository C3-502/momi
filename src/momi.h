#ifndef MOMI_H
#define MOMI_H
#include <string>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "define.h"
#include "util.h"
#include "connection.h"

namespace momi {

class Momi
{
public:
    Momi(int conn_num, std::string output_path, std::string filename, std::string url, int protocol);

    ~Momi();

    void init();

    void run();

    void pause();

    void resume();

    bool local_check();

    bool remote_check();

    void generate_conns();

    bool generate_tempfile_info();

    bool load_tempfile_info();
   
    //所有线程结束后，程序结束前的处理函数
    bool before_finish();

    static void *thread_func(void *trans_p);

    static size_t process_data(void * buffer, size_t size, size_t nmemb, void *user_p);

    //下载类型，新下载or恢复下载
    DOWNLOAD_TYPE d_type;

    //传输类型，支持多线程or单线程
    TRANSFER_TYPE t_type;

private:
    int thread_num;
    std::string output_path;
    std::string filename;
    std::string url;
    int protocol;
    u_int64_t filesize;
    std::vector<Connection*> conns;
    std::string filepath;
    std::string tmpfilepath;
    int tmpfile_fd;
};

typedef struct {
  Connection *conn;
  std::ofstream *fp;
} Trans_Struct;

typedef struct {
    Connection* conn;
    int index;
} Args_Struct;

}

#endif // MOMI_H
