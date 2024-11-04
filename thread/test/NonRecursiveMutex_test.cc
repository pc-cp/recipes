#include "../Mutex.h"
#include "../Thread.h"
#include <vector>
#include <stdio.h>
#include <iostream>
using namespace std;

class Foo {
    public:
        void doit() const;
};

muduo::MutexLock mutex;
std::vector<Foo> foos;

void post(const Foo& f) {
    muduo::MutexLockGuard lock(mutex);
    foos.push_back(f);
}

void traverse() {
    muduo::MutexLockGuard lock(mutex);
    for(auto foo : foos) {
        foo.doit(); //
    }
}

void Foo::doit() const {
    Foo f;
    post(f);
}

int main() {
    Foo f;
    post(f);
    traverse();
}