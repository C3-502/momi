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

        std::shared_ptr<SaverMsg> msg = queue_.front();
        queue_.pop_front();
        MomiTask* task = msg->task();
        lk.unlock();
        if (msg->msg_type() == SaverMsg::Write)
        {
            SaveNode* node = msg->save_node();
            task->save(node->data(), node->pos(), node->count());
        }
        else if (msg->msg_type() == SaverMsg::Finish)
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

void Saver::save_task(MomiTask *task, void *data, uint64_t start, uint64_t count)
{
    time_t ts = time(NULL);
    SaveNode* node = new SaveNode(data, start, count);
    std::shared_ptr<SaverMsg> msg(new SaverMsg(task, SaverMsg::Write, ts, node));
    std::lock_guard<std::mutex> lk(push_mutex_);
    queue_.push_back(msg);
    push_cond_.notify_one();
}

void Saver::finish_task(MomiTask *task)
{
    time_t ts = time(NULL);
    std::shared_ptr<SaverMsg> msg(new SaverMsg(task, SaverMsg::Finish, ts, nullptr));
    std::lock_guard<std::mutex> lk(push_mutex_);
    queue_.push_back(msg);
    push_cond_.notify_one();
}

}
