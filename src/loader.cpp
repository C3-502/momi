#include "loader.h"
#include "momi.h"

#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>

namespace momi {

namespace detail {

static int sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
static int timer_cb(CURLM* multi, long timeout, void* u);

class CurlManager;
class CurlWorker
{
public:
    CurlWorker(MomiTask* task, uint64_t start, uint64_t end)
        : curl_(curl_easy_init()), start_(start), end_(end), current_pos_(start_), task_(task)
    {
        set_easy_curl_opt(curl_);
    }

    ~CurlWorker()
    {
        curl_easy_cleanup(curl_);
    }

    CURL* curl_worker_handle() { return curl_; }
    uint64_t current_pos() { return current_pos_; }
    void add_current_pos(uint64_t count) { current_pos_ += count; }
    bool work_done() { return current_pos_ == end_; }

    MomiTask* task() { return task_; }

private:
    void set_easy_curl_opt(CURL* curl)
    {
        const std::string& url = task_->url();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWorker::write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    }

    static size_t write_cb(void* ptr, size_t size, size_t count, void* data)
    {
        CurlWorker* worker = (CurlWorker*) data;
        uint64_t pos = worker->current_pos();
        worker->add_current_pos(count);
        MomiTask* task = worker->task();
        char* str = (char* ) ptr;
        task->save(str, pos, count);
        std::cout << str << std::endl;
        return size * count;
    }

private:
    CURL* curl_;
    CurlManager* mgr_;
    MomiTask* task_;
    uint64_t start_;
    uint64_t end_;
    uint64_t current_pos_;
};

class CurlManager
{
public:
    CurlManager()
        : multi_(curl_multi_init()), epfd_(-1), connections_(0), active_connections_(0)
    {}

    void init()
    {
        set_multi_curl_opt();
    }

    void add_worker(CurlWorker* worker)
    {
        workers_.push_back(worker);
        connections_ += 1;
        CURL* curl = worker->curl_worker_handle();
        curl_multi_add_handle(multi_, curl);
    }

    void start()
    {
        int fd = epoll_create1(0);
        if (fd < 0)
        {

        }
        epfd_ = fd;
        epoll_event events[32] = { 0 };
        while (true)
        {
            int nevents = epoll_wait(epfd_, events, 32, this->timeout_);

            if (nevents < 0)
            {
                printf("error\n");
            }
            else if (nevents == 0)
            {
                printf("nevents === 0\n");
                curl_multi_socket_action(multi_, CURL_SOCKET_TIMEOUT, 0, &active_connections_);
            }
            else
            {
                //std::cout << "epoll_wait return, nevents=" << nevents << std::endl;
                for (int i = 0; i < nevents; ++i)
                {
                    printf("events[%d].events=%d\n", i, events[i].events);
                    if (events[i].events & EPOLLIN)
                    {
                        printf("1\n");
                        int ret = curl_multi_socket_action(multi_, events[i].data.fd, CURL_CSELECT_IN, &active_connections_);
                        printf("%d\n", ret);
                    }
                    else if (events[i].events & EPOLLOUT)
                    {
                        printf("2\n");
                        curl_multi_socket_action(multi_, events[i].data.fd, CURL_CSELECT_OUT, &active_connections_);
                    }
                    else
                    {
                        printf("3\n");
                    }
                //	curl_multi_socket_action(multi, events[i].data.fd, 0, &running_events);
                }

            }
            if (active_connections_ == 0)
                break;
        }
    }

    int epfd() { return epfd_; }
    CURLM* curl_multi_handle() { return multi_; }

    void set_timeout(int timeout) { timeout_ = timeout; }

private:
    void set_multi_curl_opt()
    {
        curl_multi_setopt(multi_, CURLMOPT_SOCKETFUNCTION, sock_cb);
        curl_multi_setopt(multi_, CURLMOPT_SOCKETDATA, this);
        curl_multi_setopt(multi_, CURLMOPT_TIMERFUNCTION, timer_cb);
        curl_multi_setopt(multi_, CURLMOPT_TIMERDATA, this);
    }

private:
    CURLM *multi_;
    std::vector<CurlWorker*> workers_;
    int epfd_;
    int connections_;
    int active_connections_;
    int timeout_;
};

static int sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp)
{
    std::cout << "sock_cb start" << std::endl;
    CurlManager *mgr = (CurlManager*) cbp;

    struct epoll_event ev = { 0 };
    ev.data.fd = s;
    int epoll_op = EPOLL_CTL_MOD;
    if (what == CURL_POLL_REMOVE)
    {
        epoll_ctl(mgr->epfd(), EPOLL_CTL_DEL, s, &ev);
    }
    else
    {
        if (what == CURL_POLL_IN)
        {
            ev.events |= EPOLLIN;
        }
        else if (what == CURL_POLL_OUT)
        {
            ev.events |= EPOLLOUT;
        }
        else if (what == CURL_POLL_INOUT)
        {
            ev.events |= (EPOLLOUT | EPOLLIN);
        }

        int ret = epoll_ctl(mgr->epfd(), EPOLL_CTL_ADD, s, &ev);
        if (ret < 0)
            ret = epoll_ctl(mgr->epfd(), EPOLL_CTL_MOD, s, &ev);
        if (ret < 0)
            printf("epoll_ctl error\n");
    }
    printf("epoll_ctl, op=%d, fd=%d, what=%d\n", epoll_op, s, what);

    return 0;
}

static int timer_cb(CURLM* multi, long timeout, void* u)
{
    std::cout << "timeout: " << timeout << std::endl;
    CurlManager* mgr = (CurlManager*) mgr;
    mgr->set_timeout(timeout);
    return 0;
}

}

Loader::Loader(MomiTask *task)
    : task_(task)
{

}

void Loader::run()
{
    loader_work_func();
}

void HttpLoader::loader_work_func()
{
    detail::CurlManager *curl_mgr = new detail::CurlManager;
    curl_mgr->init();
    detail::CurlWorker* worker = new detail::CurlWorker(task_, start_, end_);
    curl_mgr->add_worker(worker);
    curl_mgr->start();
}

}