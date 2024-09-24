#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    /*explicit：
    *主要用于防止隐式类型转换.当构造函数被声明为explicit时，
    *编译器将禁止使用该构造函数进行隐式的类型转换。
    *作用是防止编译器在某些情况下进行自动类型转换，导致不可预期的行为。
    */
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0); // 确保线程数目大于0

            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {
                    /*该线程的互斥锁*/
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        if(!pool->tasks.empty()) {/*池子不为空时*/
                            /*从队列中获取一个任务然后执行
                            * 执行时可以加锁
                            * 获取任务时采用move可以节省资源
                            */
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } /*线程池关闭时跳出*/
                        else if(pool->isClosed) break;
                        /*线程池为空时，利用条件变量等待*/
                        else pool->cond.wait(locker);
                    }
                }).detach();/*线程类的一个成员函数，用于将线程和创建的thread对象分离*/
            }
    }

    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;   
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                /*c++11引入的RALL资源获取即初始化风格的做机制*/
                /*当前作用域自动加锁，作用域结束自动释放锁*/
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true; // 关闭线程池
            }
            pool_->cond.notify_all(); // 唤醒所有线程退出
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            /*完美转发添加任务*/
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        } // 唤醒一个线程
        pool_->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx; // 互斥锁，保护共享数据
        std::condition_variable cond; // 条件变量，管理任务的等待和通知机制
        bool isClosed; // 标志线程池是否关闭
        std::queue<std::function<void()>> tasks; // 任务队列，存放待执行的任务
    };
    std::shared_ptr<Pool> pool_;
};


#endif //THREADPOOL_H