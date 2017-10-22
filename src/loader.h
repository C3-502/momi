#ifndef __LOADER_H__
#define __LOADER_H__

// #include <thread>
#include <string>
#include <curl/curl.h>

namespace momi {

class Loader
{
public:
	Loader()
		: start_(0), current_pos_(0), end_(0)
	{}
	virtual ~Loader() {}
	Loader(uint32_t start, uint32_t end)
		: start_(start), current_pos_(start), end_(end)
	{}

	uint32_t start() const { return start_; }
	void set_start(uint32_t start) { start_ = start; }
	uint32_t end() const { return end_; }
	void set_end(uint32_t end) { end_ = end; }
	void pause() {}
	void resume() {}
	void run();
protected:
	virtual void loader_work_func() {}


protected:
	uint32_t start_;
	uint32_t current_pos_;
	uint32_t end_;
	std::string url_;
	// std::thread::id tid_;
	// std::thread thread_;
};


class HttpLoader: public Loader
{
public:
	HttpLoader(uint32_t start, uint32_t end)
		: Loader(start, end) 
	{}
	virtual ~HttpLoader() {}

protected:
	virtual void loader_work_func();
};

}


#endif
