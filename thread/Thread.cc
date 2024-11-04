#include "Thread.h"
#include <boost/weak_ptr.hpp>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#if __FreeBSD__
#incldue <pthread_np.h>
#else 
#include <sys/prctl.h>
#include <linux/unistd.h>
#endif

namespace muduo {
    namespace CurrentThread {
        __thread const char * t_threadName = "unnamedThread";
    }
}

/*
匿名命名空间在 C++ 中是限定文件内符号可见性的常用方式，主要价值在于防止符号冲突、改进代码组织、替代 static、并增强代码的可维护性。
*/
namespace {
    __thread pid_t t_cachedTid = 0;

    /*
        为了实现跨平台的可移植性。不同的操作系统和标准库对获取线程 
        ID（gettid）的实现有所不同，因此需要条件编译来适配不同的平台和环境。
    */
    #if __FreeBSD__
    pid_t gettid() {
        return pthread_getthreadid_np();
    }
    #else 
    #if !__GLIBC_PREREQ(2, 30) // glibc 版本小于 2.30 的情况
    pid_t gettid() {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }
    #endif
    #endif

    void afterFork() {
        t_cachedTid = gettid();
        muduo::CurrentThread::t_threadName = "main";
    }

    class ThreadNameInitializer {
        public:
            ThreadNameInitializer() {
                muduo::CurrentThread::t_threadName = "main";
                pthread_atfork(NULL, NULL, &afterFork);
            }
    };

    // 给主线程使用
    ThreadNameInitializer init;

    struct ThreadData {
        typedef muduo::Thread::ThreadFunc ThreadFunc;
        ThreadFunc func_;
        std::string name_;
        boost::weak_ptr<pid_t> wkTid_;

        ThreadData(const ThreadFunc &func,
                   const std::string &name,
                   const boost::shared_ptr<pid_t>& tid)
                    : func_(func), name_(name), wkTid_(tid) {
                   }

        void runInThread() {
            pid_t tid = muduo::CurrentThread::tid();
            boost::shared_ptr<pid_t> ptid = wkTid_.lock();
            /*
                将当前线程的 ID (tid) 传递给一个外部共享指针 wkTid_ 指向的 pid_t 对象
            */
            if(ptid) {
                *ptid = tid;
                ptid.reset();
            }

            if(!name_.empty()) {
                muduo::CurrentThread::t_threadName = name_.c_str();
            }

            #if __FreeBSD__
                pthread_setname_np(pthread_self(), muduo::CurrentThread::t_threadName);
            #else 
                ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);
            #endif 
                func_();
                muduo::CurrentThread::t_threadName = "finished";
        }

    };

    void* startThread(void *obj) {
        ThreadData *data = static_cast<ThreadData *> (obj);
        data->runInThread();
        delete data;
        return NULL;
    }
}

using namespace muduo;

pid_t CurrentThread::tid() {
    if(t_cachedTid == 0) {
        t_cachedTid = gettid();
    }
    return t_cachedTid;
}

const char * CurrentThread::name() {
    return t_threadName;
}

/*
    getpid() 返回当前进程的 ID。在主线程中，getpid() 
    和线程 ID (tid) 是相同的，因为主线程的线程 ID 就是进程 ID。
*/
bool CurrentThread::isMainThread() {
    return tid() == ::getpid();
}

AtomicInt32 Thread::numCreated_; // initial 0

Thread::Thread(const ThreadFunc &func, const std::string &n) 
            : started_(false), joined_(false), pthreadId_(0),
              tid_(new pid_t(0)), func_(func), name_(n){ 
                numCreated_.increment();
              }

Thread::~Thread() {
    if(started_ && !joined_) {
        pthread_detach(pthreadId_);
    }
}

void Thread::start() {
    assert(!started_);
    started_ = true;

    ThreadData *data = new ThreadData(func_, name_, tid_);
    /*
    a successful call to pthread_create() stores the ID of the new thread in the buffer pointed to by thread;
    */
    if(pthread_create(&pthreadId_, NULL, &startThread, data)) {
        started_ = false;
        delete data;
        abort();
    }
}

void Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    pthread_join(pthreadId_, NULL);
}




