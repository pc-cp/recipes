#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include "Atomic.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>

namespace muduo {
    class Thread : boost::noncopyable {
        public:
            typedef boost::function<void ()> ThreadFunc;

            explicit Thread(const ThreadFunc &, const std::string &name = std::string());
            ~Thread();

            void start();
            void join();

            bool started() const { return started_; }
            pid_t tid() const { return *tid_; }
            const std::string &name() const { return name_; }
            static int numCreated() { return numCreated_.get(); }

        private:
            bool                        started_;
            bool                        joined_;
            // Used to identify a thread.
            pthread_t                   pthreadId_;
            // The pid_t data type represents process IDs.
            boost::shared_ptr<pid_t>    tid_;
            ThreadFunc                  func_;
            std::string                 name_;

            static AtomicInt32 numCreated_;
    };

    namespace CurrentThread {
        pid_t tid();
        const char *name();
        bool isMainThread();
    }
}

#endif // MUDUO_BASE_THREAD_H