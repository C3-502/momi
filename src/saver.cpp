/*************************************************************************
	> File Name: saver.cpp
	> Author: kongxun 
	> Mail: kongxun.yb@gmail.com
	> Created Time: 2017年10月28日 星期六 15时48分11秒
 ************************************************************************/

#include "saver.h"

namespace momi
{

void SaveQueue::push(SaveNode* node)
{
    std::lock_guard<std::mutex> guard(push_mutex);
    if (empty()) {
        tail_ = head_ = node;
    } else {
        tail_->set_next(node);
    }
}

void SaveQueue::unshift()
{
    if (!empty()) {
        SaveNode* del_ = head_;
        head_=head_->next();
        if (NULL==head_) {
            tail_=head_;
        }
        delete head_;
    }
}

bool SaveQueue::empty()
{
    if (head_==tail_ && NULL==head_) {
        return true;
    }
    return false;
}

void Saver::run()
{
    while (true)
    {
        if (queue_.empty())
        {
            continue;
        }
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
    SaveNode* node = new SaveNode(str, pos, count, timestamp, task, msg_type);
    queue_.push_back(node);
}

}
