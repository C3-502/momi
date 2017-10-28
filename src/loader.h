#ifndef __LOADER_H__
#define __LOADER_H__

// #include <thread>

#include <string>
#include <curl/curl.h>

namespace momi {

class MomiTask;

class Loader
{
public:
    explicit Loader(MomiTask* task);
	virtual ~Loader() {}

	uint32_t start() const { return start_; }
	void set_start(uint32_t start) { start_ = start; }
	uint32_t end() const { return end_; }
	void set_end(uint32_t end) { end_ = end; }
	void pause() {}
	void resume() {}
	void run();

    MomiTask* task() { return task_; }
protected:
	virtual void loader_work_func() {}


protected:
    uint64_t start_;
    uint64_t end_;
    MomiTask* task_;
};


class HttpLoader: public Loader
{
public:
    HttpLoader(MomiTask* task)
        : Loader(task)
    {}
	virtual ~HttpLoader() {}

protected:
	virtual void loader_work_func();
};

}


#endif
