#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../Thread.h"

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <stdio.h>

class Test {
    public: 
        Test(int numThreads) : /*mutex_(), */latch_(numThreads), threads_(numThreads) {
            for(int i = 0; i < numThreads; ++i) {
                char name[32];
                snprintf(name, sizeof name, "work thread %d", i);
                threads_.push_back(new muduo::Thread(
                    /*
                        将成员函数 threadFunc 绑定到当前对象，
                        使其可以作为独立的函数对象传递给新线程，
                        以在该线程中运行 Test::threadFunc()。
                    */
                    boost::bind(&Test::threadFunc, this), std::string(name)
                ));
            }
            // 对 threads_ 容器中的每一个线程对象调用其 start 方法，从而启动所有线程。
            // 等价于每个 Thread 对象执行 thread->start()
            for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::start, _1));
        }

        void run(int times) {
            printf("waiting for count down latch\n");
            latch_.wait();
            printf("all threads started\n");
            for(int i = 0; i < times; ++i) {
                // muduo::MutexLockGuard lock(mutex_);
                char buf[32];
                snprintf(buf, sizeof buf, "hello %d", i);
                queue_.put(buf);
                printf("tid = %d, put data = %s, size = %zd\n", muduo::CurrentThread::tid(), buf, queue_.size());
            }
        }

        void joinAll() {
            for(size_t i = 0; i < threads_.size(); ++i) {
                queue_.put("stop");
            }

            for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::join, _1));
        }

    private:
        void threadFunc() {
            printf("tid = %d, %s started\n", 
                                            muduo::CurrentThread::tid(),
                                            muduo::CurrentThread::name());

            latch_.countDown();
            bool running = true;
            while(running) {
                // muduo::MutexLockGuard lock(mutex_);
                std::string d(queue_.take());
                printf("tid = %d, get data = %s, size = %zd\n", muduo::CurrentThread::tid(), d.c_str(), queue_.size());
                running = (d != "stop");
            }

            printf("tid = %d, %s stopped\n", muduo::CurrentThread::tid(), muduo::CurrentThread::name());
        }

        // mutable muduo::MutexLock mutex_;
        muduo::BlockingQueue<std::string> queue_;
        muduo::CountDownLatch latch_;
        boost::ptr_vector<muduo::Thread> threads_;
};

int main() {
    printf("pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());
    Test t(5);
    t.run(100);
    t.joinAll();

    printf("number of created threads %d\n", muduo::Thread::numCreated());
}