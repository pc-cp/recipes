#include "../Mutex.h"

class Request {
    public:
        void process() {
            muduo::MutexLockGuard lock(mutex_);
            print();
        }

        void print() {
            muduo::MutexLockGuard lock(mutex_);
        }

    private:
        mutable muduo::MutexLock mutex_;
};

int main() {
    Request req;
    req.process();
}