#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "./Thread.h"

pthread_key_t key;

void des(void *arg);

void *child1(void *arg) {
    int *p = new int(100);
    pthread_setspecific(key, p);
    printf("%p\n", p);
    printf("%d: pthread_getspecific(key): %p\n", muduo::CurrentThread::tid(), pthread_getspecific(key));
    auto q = (int *)pthread_getspecific(key);
    printf("%d\n", *q);
    return NULL;
}

void *child2(void *arg) {
    int *p = new int(10);
    pthread_setspecific(key, p);
    printf("%p\n", p);
    printf("%d: pthread_getspecific(key): %p\n", muduo::CurrentThread::tid(), pthread_getspecific(key));
    auto q = (int *)pthread_getspecific(key);
    printf("%d\n", *q);
    return NULL;
}

void des(void *arg) {
    printf("%d in des\n", muduo::CurrentThread::tid());
    int* p = static_cast<int *>(arg);
    delete p;
}

int main() {
    pthread_t tid1, tid2;
    pthread_key_create(&key, des);
    pthread_create(&tid1, NULL, child1, NULL);
    pthread_create(&tid2, NULL, child2, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_key_delete(key);
}