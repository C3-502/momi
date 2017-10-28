#ifndef __LOADER_H__
#define __LOADER_H__

#include <thread>
#include <string>
#include <vector>
#include <curl/curl.h>

namespace momi {

class MomiTask;

class Loader
{
public:
    enum LoaderStatus
    {
        New, Downloading, Complete, Stop
    };

public:
    explicit Loader(MomiTask* task, uint64_t start, uint64_t end);
	virtual ~Loader() {}

	uint32_t start() const { return start_; }
	void set_start(uint32_t start) { start_ = start; }
	uint32_t end() const { return end_; }
	void set_end(uint32_t end) { end_ = end; }
	void pause() {}
	void resume() {}
	void run();

    void set_status(LoaderStatus status) { status_ = status; }
    LoaderStatus status() { return status_; }

    MomiTask* task() { return task_; }
    void save_meta_info(const std::string& buf);
protected:
	virtual void loader_work_func() {}


protected:
    uint64_t start_;
    uint64_t end_;
    MomiTask* task_;
    std::vector<void*> workers_;
    std::thread thread_;
    LoaderStatus status_;
};


class HttpLoader: public Loader
{
public:
    HttpLoader(MomiTask* task, uint64_t start, uint64_t end)
        : Loader(task, start, end)
    {}
	virtual ~HttpLoader() {}

protected:
	virtual void loader_work_func();
};

}


#endif
