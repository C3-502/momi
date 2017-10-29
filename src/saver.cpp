/*************************************************************************
	> File Name: saver.cpp
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月28日 星期六 15时48分11秒
 ************************************************************************/
#include "saver.h"

namespace momi
{

void Saver::run()
{
    while (true)
    {
        std::unique_lock<std::mutex> lk(push_mutex_);
        push_cond_.wait(lk, [this]{ return !queue_.empty(); });

        SaveNode* node = queue_.front();
        queue_.pop_front();
        MomiTask* task = node->task();
        if (node->msg_type() == SaveNode::Write)
        {
            task->save(node->str(), node->pos(), node->count());
        }
        else if (node->msg_type() == SaveNode::Finish)
        {
            task->rename();
            break;
        }
        else
        {
            std::cerr << "error, invalid msg_type"<< std::endl;
        }

    }
    std::cout << "download finish" << std::endl;
}

void Saver::save(const std::string& str, uint64_t pos, size_t count, uint32_t timestamp, MomiTask* task,  MsgType msg_type)
{
    std::lock_guard<std::mutex> guard(push_mutex_);
    SaveNode* node = new SaveNode(str, pos, count, timestamp, task, msg_type);
    queue_.push_back(node);
    push_cond_.notify_one();
}

}
