#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>

namespace muduo {
    namespace detail {
        template <typename T>
        class AtomicIntegerT : boost::noncopyable {
            public:
                AtomicIntegerT() : value_(0)  {
                }

                T get() const {
                    /*
                         GCC 内置的原子操作，用于比较并交换。它会检查目标内存地址中的值是否等于预期的旧值（即第二个参数），
                         如果相等则用新值（即第三个参数）替换并返回旧值；否则，返回当前值且不做替换。
                    */
                    return __sync_val_compare_and_swap(const_cast<volatile T*>(&value_), 0, 0);
                }

                T getAndAdd(T x) {
                    /*
                        __sync_fetch_and_add 是原子性的加法操作。通过它，
                        可以在多线程环境下安全地对 value_ 进行加法，不会引起数据竞争。

                        __sync_fetch_and_add 返回的是 value_ 操作之前的旧值。
                    */
                    return __sync_fetch_and_add(&value_, x);
                }

                /*
                    目的是实现“加后获取”的功能，也就是说，先将 x 加到 value_ 中，然后返回加完之后的结果。
                */
                T addAndGet(T x) {
                    return getAndAdd(x) + x;
                }

                T incrementAndGet() {
                    return addAndGet(1);
                }

                void add(T x) {
                    getAndAdd(x);
                }

                void increment() {
                    incrementAndGet();
                }

                void decrement() {
                    getAndAdd(-1);
                }

                T getAndSet(T newValue) {
                    /*
                        type __sync_lock_test_and_set (type *ptr, type value, ...)
                        该内置程序不是传统的test-and-set操作，而是atomic exchange操作。它value写入 *ptr，并返回 *ptr 先前的内容。
                    */
                    return __sync_lock_test_and_set(&value_, newValue);
                }

            private:
                volatile T value_;
        };
    }

    typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
    typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}

#endif // MUDUO_BASE_ATOMIC_H