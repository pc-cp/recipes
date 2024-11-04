#include "../Mutex.h"
#include "../Thread.h"
#include <set>
#include <stdio.h>

class Request;

class Inventory {
    public:
        void add(Request *req) {
            muduo::MutexLockGuard lock(mutex_);
            requests_.insert(req);
        } 

        /*
            用于防止编译器将函数内联。这意味着，即使编译器认为内联可以优化性能，也不会将 remove 函数内联。
        */
        void remove(Request *req) __attribute__ ((noinline)) {
            muduo::MutexLockGuard lock(mutex_);
            requests_.erase(req);
        }

        void printAll() const;

    private:
        mutable muduo::MutexLock mutex_;
        std::set<Request *> requests_;
};

Inventory g_inventory;

class Request {
    public:
        void process() {
            muduo::MutexLockGuard lock(mutex_);
            g_inventory.add(this);
            printf("Request:process() finished\n");
        }

        ~Request() __attribute__ ((noinline)) {
            // request'lock -> Inventory'lock
            muduo::MutexLockGuard lock(mutex_);
            // sleep(1);
            g_inventory.remove(this);
        }

        void print() const __attribute__ ((noinline)) {
            muduo::MutexLockGuard lock(mutex_);
        }
    private:
        mutable muduo::MutexLock mutex_;
};

void Inventory::printAll() const {
    // Inventory'lock -> Request'lock
    muduo::MutexLockGuard lock(mutex_);
    sleep(1);
    printf("Request:print() will execute\n");
    for(auto &request : requests_) {
        request->print();
    }

    printf("Inventory::printAll() unlocked\n");
}

void threadFunc() {
    Request *req = new Request;
    req->process();
    sleep(1);
    delete req;
}

int main() {
    muduo::Thread thread(threadFunc);
    thread.start();
    usleep(500 * 1000); // sleep(0.5)
    g_inventory.printAll();
    thread.join();
}