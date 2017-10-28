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
#include "saver.h"

namespace momi {

class Loader;
class MomiTask;
class Momi
{
public:
    Momi() {}
    Momi(int conn_num, std::string output_path, std::string filename, std::string url, int protocol);

    ~Momi();

    void init();

    void run();

    void save(const std::string& str, uint64_t pos, size_t count, uint32_t timestamp, MomiTask* task);

    //下载类型，新下载or恢复下载
    DOWNLOAD_TYPE d_type;

    //传输类型，支持多线程or单线程
    TRANSFER_TYPE t_type;

private:
    void start_loader(MomiTask* task, uint64_t start, uint64_t end);

private:
    int thread_num;
    std::vector<MomiTask*> tasks_;
    Saver* saver_;
};

class MomiTask
{
public:
    enum MomiTaskStatus
    {
        New, Downloading, Complete, Stop
    };

public:
    MomiTask(const std::string& url, const std::string& output_path,
             const std::string& filename, int protocol, Momi* momi)
        : url_(url), output_path_(output_path), filename_(filename),
          protocol_(protocol), filepath_(output_path + filename), filesize_(0),
          fd_(-1), status_(New), momi_(momi)
    {

    }

    void init();
    bool local_check();
    bool remote_check();

    const std::string& url() { return url_; }
    uint64_t file_size() { return filesize_; }
    const std::string& tmp_file_path() { return tmpfilepath_; }
    const std::string& file_path() { return filepath_; }

    MomiTaskStatus status() { return status_; }
    void set_status(MomiTaskStatus status) { status_ = status; }
    void async_save(const std::string& buf, uint64_t start, uint64_t count);
    void save(char *buf, uint64_t start, uint64_t count);
    void save_meta_info();

    void rename();

private:
    std::string output_path_;
    std::string filename_;
    std::string url_;
    int protocol_;
    std::string filepath_;
    std::string tmpfilepath_;
    uint64_t filesize_;
    int fd_;
    MomiTaskStatus status_;
    std::vector<Loader*> loaders_;
    Momi* momi_;
};

typedef struct {
  Connection *conn;
  std::ofstream *fp;
} Trans_Struct;

typedef struct {
    Momi* momi;
    Connection* conn;
    int index;
} Args_Struct;

}

#endif // MOMI_H
