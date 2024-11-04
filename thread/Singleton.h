#ifndef MUDUO_BASE_SINGLETON_H
#define MUDUO_BASE_SINGLETON_H

#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <stdlib.h>     // atexit

namespace muduo {
    template <typename T> 
    class Singleton : boost::noncopyable {
        public:
            static T& instance() {
                /*
                    The first call to pthread_once() by any thread in a process, 
                    with a given once_control, will call the init_routine() with no arguments. 
                    
                    Subsequent calls of pthread_once() with the same once_control will not call the init_routine().
                */
                pthread_once(&ponce_, &Singleton::init);
                return *value_;
            }

        
        private:
            Singleton();
            ~Singleton();
            static void init() {
                value_ = new T();
                // 注册一个函数 destroy，以便在程序退出时自动调用它。
                ::atexit(destroy);
            }

            static void destroy() {
                delete value_;
            }

        private:
            static pthread_once_t   ponce_;
            static T*               value_;
    };


    /*
    普通静态成员必须在类外定义和初始化。//也可放在.cpp文件
    */
    template <typename T>
    pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

    template <typename T>
    T* Singleton<T>::value_ = NULL;

}
#endif // MUDUO_BASE_SINGLETON_H