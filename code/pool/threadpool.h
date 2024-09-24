#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    /*explicit��
    *��Ҫ���ڷ�ֹ��ʽ����ת��.�����캯��������Ϊexplicitʱ��
    *����������ֹʹ�øù��캯��������ʽ������ת����
    *�����Ƿ�ֹ��������ĳЩ����½����Զ�����ת�������²���Ԥ�ڵ���Ϊ��
    */
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0); // ȷ���߳���Ŀ����0

            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {
                    /*���̵߳Ļ�����*/
                    std::unique_lock<std::mutex> locker(pool->mtx);
                    while(true) {
                        if(!pool->tasks.empty()) {/*���Ӳ�Ϊ��ʱ*/
                            /*�Ӷ����л�ȡһ������Ȼ��ִ��
                            * ִ��ʱ���Լ���
                            * ��ȡ����ʱ����move���Խ�ʡ��Դ
                            */
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } /*�̳߳عر�ʱ����*/
                        else if(pool->isClosed) break;
                        /*�̳߳�Ϊ��ʱ���������������ȴ�*/
                        else pool->cond.wait(locker);
                    }
                }).detach();/*�߳����һ����Ա���������ڽ��̺߳ʹ�����thread�������*/
            }
    }

    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;   
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                /*c++11�����RALL��Դ��ȡ����ʼ������������*/
                /*��ǰ�������Զ�����������������Զ��ͷ���*/
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true; // �ر��̳߳�
            }
            pool_->cond.notify_all(); // ���������߳��˳�
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            /*����ת���������*/
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        } // ����һ���߳�
        pool_->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx; // ��������������������
        std::condition_variable cond; // ������������������ĵȴ���֪ͨ����
        bool isClosed; // ��־�̳߳��Ƿ�ر�
        std::queue<std::function<void()>> tasks; // ������У���Ŵ�ִ�е�����
    };
    std::shared_ptr<Pool> pool_;
};


#endif //THREADPOOL_H