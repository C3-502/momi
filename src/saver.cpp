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
    while (true) {
        SaveNode* node = queue_->head();
        MomiTask* task = node->task();
        task->save(node->str(), node->pos(), node->count());
        if ((MomiTask::MomiTaskStatus::Complete == task->status())
            && queue_->empty()){
           break; 
        }
    }
}

void Saver::save(const std::string& str, uint64_t pos, size_t count, uint32_t timestamp, MomiTask* task)
{
    SaveNode* node = new SaveNode(str, pos, count, timestamp, task);
    queue_->push(node);
}

}
