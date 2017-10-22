#include "loader.h"

#include <sys/epoll.h>
#include <stdio.h>
#include <iostream>

namespace momi {

namespace detail {

static int timeout = -1;

struct GlobalInfo
{
	int epfd;
	CURLM* multi_curl;
};

struct SocketInfo
{
	curl_socket_t socket;
};

static size_t write_cb(void* ptr, size_t size, size_t count, void* data)
{
	char* str = (char* ) ptr;
	std::cout << str << std::endl;
	return size * count;
}

static int sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp)
{
	std::cout << "sock_cb start" << std::endl;
	detail::GlobalInfo *g = (detail::GlobalInfo*) cbp;
	detail::SocketInfo *fdp = (detail::SocketInfo*) sockp;
	struct epoll_event ev = { 0 };
	int epoll_op = EPOLL_CTL_MOD;
	if (what == CURL_POLL_REMOVE)
	{
		if (fdp)
			delete fdp;
		epoll_op = EPOLL_CTL_DEL;
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

		if (!fdp)
		{
			fdp = new SocketInfo;
			fdp->socket = s;
			epoll_op = EPOLL_CTL_ADD;
			curl_multi_assign(g->multi_curl, s, fdp);
		}
	}
	printf("epoll_ctl, op=%d, fd=%d, what=%d\n", epoll_op, s, what);
	epoll_ctl(g->epfd, epoll_op, s, &ev);
	return 0;
}

static int timer_cb(CURLM* multi, long time_out, void* u)
{
	std::cout << "timeout: " << time_out << std::endl;
	timeout = time_out;
	return 0;
}

static void set_easy_curl_opt(CURL* curl)
{
	curl_easy_setopt(curl, CURLOPT_URL, "http://www.baidu.com");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
}

static void init_easy_curl(CURLM* multi, uint32_t size)
{
	for (uint32_t i=0; i < size; ++i)
	{
		CURL* curl = curl_easy_init();
		set_easy_curl_opt(curl);
		curl_multi_add_handle(multi, curl);
	}
}

}


void Loader::run()
{
	loader_work_func();
}

void HttpLoader::loader_work_func()
{
	std::cout << "curl_version: " << curl_version() << std::endl;
	std::cout << "loader_work_func start" << std::endl;
	detail::GlobalInfo g;
	CURLM *multi = curl_multi_init();
	g.multi_curl = multi;
	int epfd = epoll_create(32);
	if (epfd < 0)
	{

	}
	g.epfd = epfd;
	curl_multi_setopt(multi, CURLMOPT_SOCKETFUNCTION, detail::sock_cb);
	curl_multi_setopt(multi, CURLMOPT_SOCKETDATA, &g);
	curl_multi_setopt(multi, CURLMOPT_TIMERFUNCTION, detail::timer_cb);

	detail::init_easy_curl(multi, 1);

	struct epoll_event events[32] = { 0 };
	int running_events;
	
	while (true)
	{
		std::cout << "epoll_wait start, timeout=" << detail::timeout << std::endl;
		int nevents = epoll_wait(epfd, events, 32, detail::timeout);

		if (nevents < 0)
		{
			printf("error\n");
		}
		else if (nevents == 0)
		{
			printf("nevents === 0\n");
			curl_multi_socket_action(multi, CURL_SOCKET_TIMEOUT, 0, &running_events);
		}
		else
		{
			//std::cout << "epoll_wait return, nevents=" << nevents << std::endl;
			for (int i = 0; i < nevents; ++i)
			{
				printf("events[%d].events=%d\n", i, events[i].events);
			// 	if (events[i].events & EPOLLIN)
			// 	{
			// 		curl_multi_socket_action(multi, events[i].data.fd, 0, &running_events);
			// 	}
			// 	else if (events[i].events & EPOLLOUT)
			// 	{
			// 		curl_multi_socket_action(multi, events[i].data.fd, 0, &running_events);
			// 	}
			// 	else
			// 	{

			// 	}
				curl_multi_socket_action(multi, events[i].data.fd, 0, &running_events);
			}

		}
		if (running_events == 0)
			break;
	}
}

}