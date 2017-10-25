#include "loader.h"

#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>

namespace momi {

namespace detail {

static int sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
static int timer_cb(CURLM* multi, long timeout, void* u);

class CurlTask
{
public:
    CurlTask(uint32_t size, const std::string& url)
        : easy_arr_(size), url_(url)
    {
        for (uint i = 0; i < size; ++i)
        {
            CURL* easy = curl_easy_init();
            set_easy_curl_opt(easy);
            easy_arr_[i] = easy;
        }
    }

    ~CurlTask()
    {
        for (CURL* easy : easy_arr_)
        {
            curl_easy_cleanup(easy);
        }
    }

    int size()
    {
        return easy_arr_.size();
    }

    const std::vector<CURL*>& easy_arr()
    {
        return easy_arr_;
    }

private:
    void set_easy_curl_opt(CURL* curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlTask::write_cb);
    }

    static size_t write_cb(void* ptr, size_t size, size_t count, void* data)
    {
        char* str = (char* ) ptr;
        std::cout << str << std::endl;
        return size * count;
    }

private:
    std::string url_;
    std::vector<CURL*> easy_arr_;
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

    void add_task(CurlTask* task)
    {
        tasks_.push_back(task);
        connections_ += tasks_.size();
        auto easy_arr = task->easy_arr();
        for (CURL* easy : easy_arr)
        {
            curl_multi_add_handle(multi_, easy);
        }
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
    std::vector<CurlTask*> tasks_;
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

void Loader::run()
{
    loader_work_func();
}

void HttpLoader::loader_work_func()
{
    detail::CurlManager *curl_mgr = new detail::CurlManager;
    curl_mgr->init();
    detail::CurlTask* task = new detail::CurlTask(4, "www.baidu.com");
    curl_mgr->add_task(task);
    curl_mgr->start();
}

}
