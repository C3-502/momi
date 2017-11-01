/*************************************************************************
	> File Name: saver.h
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月28日 星期六 15时48分01秒
 ************************************************************************/
#ifndef _SAVER_H
#define _SAVER_H

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <list>
#include "momi.h"

namespace momi
{
class MomiTask;
class SaveNode
{
public:
    SaveNode(void* data, uint64_t pos, uint64_t count)
        : data_(data), pos_(pos), count_(count)
    {}

    ~SaveNode() {}

    void* data() { return data_; }
    uint64_t pos() { return pos_; }
    size_t count() { return count_; }

private:
    void* data_;
    uint64_t pos_;
    size_t count_;
};


class SaverMsg
{
public:
    enum MsgType
    {
        Write, Finish
    };
public:
    SaverMsg(MomiTask* task, MsgType msg_type, time_t timestamp, void* content)
        : task_(task), msg_type_(msg_type), timestamp_(timestamp), content_(content)
    {}

    MomiTask* task() { return task_; }
    time_t timestamp() { return timestamp_; }
    MsgType msg_type() { return msg_type_; }
    SaveNode* save_node() { return (SaveNode*) content_; }
private:
    MomiTask* task_;
    time_t timestamp_;
    MsgType msg_type_;
    void* content_;
};


class Saver
{
public:
    Saver() {}
    ~Saver() {}
    void run();

public:
    void save_task(MomiTask* task, void* data, uint64_t start, uint64_t count);
    void finish_task(MomiTask* task);

private:
    std::list<SaverMsg*> queue_;
    std::mutex push_mutex_;
    std::condition_variable push_cond_;
};

}
#endif
