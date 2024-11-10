#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace moenew
{
    class ThreadPool
    {
    public:
        ThreadPool()
        {
            init();
        };
        ~ThreadPool()
        {
            stop = true;
            condition.notify_all();
            for (std::thread &worker : workers)
            {
                worker.join();
            }
        }
        void init(size_t size = std::thread::hardware_concurrency())
        {
            for (size_t i = 0; i < size; ++i)
            {
                workers.emplace_back([this]
                                     { work_func(); });
            }
        }
        void enqueue(std::function<void()> task)
        {
            {
                std::lock_guard<std::mutex> lock(mtx);
                tasks.push(task);
                ++active_tasks;
            }
            condition.notify_one();
        }
        void wait()
        {
            while (active_tasks > 0)
            {
                std::this_thread::yield();
            }
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable condition;
        std::atomic<bool> stop{false};
        std::atomic<int> active_tasks{0};

        void work_func()
        {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    condition.wait(lock, [this]
                                   { return stop || !tasks.empty(); });
                    if (stop && tasks.empty())
                        return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
                --active_tasks;
            }
        }
    };
};