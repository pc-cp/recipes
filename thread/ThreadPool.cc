#include "ThreadPool.h"
#include "Exception.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

using namespace muduo;

ThreadPool::ThreadPool(const std::string& name) : 
                        mutex_(),
                        cond_(mutex_),
                        name_(name),
                        running_(false) { }


ThreadPool::~ThreadPool() {
    if(running_) {
        stop();
    }
}

void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for(int i = 0; i < numThreads; ++i) {
        char id[32];
        snprintf(id, sizeof id, "%d", i);
        threads_.push_back(new muduo::Thread(
            boost::bind(&ThreadPool::runInThread, this), name_+id
        ));
    }

    for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::start, _1));
}

void ThreadPool::stop() {
    running_ = false;
    cond_.notifyAll();
    for_each(threads_.begin(), 
             threads_.end(), 
             boost::bind(&muduo::Thread::join, _1
             ));
}

void ThreadPool::run(const Task& task) {
    if(threads_.empty()) {
        task();
    }
    else {
        MutexLockGuard lock(mutex_);
        queue_.push_back(task);
        cond_.notify();
    }
}

ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    while(queue_.empty() && running_) {
        cond_.wait();
    }

    Task task;
    if(!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
    }
    return task;
}

void ThreadPool::runInThread() {
    try {
        while(running_) {
            Task task(take());
            if(task) {
                task();
            }
        }
    }
    catch(const std::exception& ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch(...) {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        abort();
    }
}