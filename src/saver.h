/*************************************************************************
	> File Name: saver.h
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月28日 星期六 15时48分01秒
 ************************************************************************/
#include<iostream>
#include<mutex>
#include "momi.h"
#ifndef _SAVER_H
#define _SAVER_H

namespace momi
{

class SaveNode
{
public:
    SaveNode(char *str, uint64_t pos, size_t count, uint32_t timestamp, MomiTask* task) 
        : str_(str), pos_(pos), count_(count), timestamp_(timestamp), task_(task), next_(NULL){};

    ~SaveNode(){}

    char * str() { return str_; }
    uint64_t pos() { return pos_; }
    size_t count() { return count_; }
    uint32_t timestamp() { return timestamp_; }
    MomiTask* task() { return task_; }
    SaveNode* next() { return next_; }

private:
    char *str_;
    uint64_t pos_;
    size_t count_;
    uint32_t timestamp_;
    MomiTask *task_;
    SaveNode *next_;
};

class SaveQueue
{
public:
    SaveQueue(){}
    ~SaveQueue(){}

    void push(SaveNode* node);
    void unshift();
    bool empty();
    SaveNode* head() { return head_; }

private:
    SaveNode* head_=NULL;
    SaveNode* tail_=NULL;
    std::mutex push_mutex;
};

class Saver
{
public:
    Saver() {}
    ~Saver() {}
    void run();
    void save(char *str, uint64_t pos, size_t count, uint32_t timestamp, MomiTask* task);
private:
    SaveQueue queue;
};

}
#endif
