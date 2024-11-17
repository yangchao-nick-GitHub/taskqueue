#include <iostream>
#include <thread>
#include <deque>
#include <unordered_map>
#include <functional>
#include <condition_variable>
#include <chrono>

class TaskQueue
{
public:
    explicit TaskQueue(int thread_num)
    {
        for (int i  = 0; i < std::max(thread_num, 1); i++)
        {
            addWorkThread();
        }
    }

    ~TaskQueue()
    {
        for (auto& item : work_map_)
        {
            item.second->join();
        }
    }

    void workImp()
    {
        while (1)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv.wait(lock, [this]() {
                return !task_queue_.empty();
            });
            
            std::function<void()> task = task_queue_.front();
            task_queue_.pop_front();
            lock.unlock();
            try
            {
                task();
            } catch (const std::exception &e) 
            {
                std::cerr << "Exception caught:" << e.what() << std::endl;
            } catch (...)
            {
                std::cerr << "Unknown exception caught" << std::endl;
            }
            
        }
    }

    void addWorkThread()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        std::shared_ptr<std::thread> thread_ptr = std::make_shared<std::thread>(&TaskQueue::workImp, this);
        work_map_.emplace(thread_ptr->get_id(), thread_ptr);
    }

    void submit(std::function<void()> task)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        task_queue_.emplace_back(task);
        cv.notify_one();
    }

private:
    std::unordered_map<std::thread::id, std::shared_ptr<std::thread>> work_map_;
    std::deque<std::function<void()>> task_queue_;
    std::mutex mtx_;
    std::condition_variable cv;
};

