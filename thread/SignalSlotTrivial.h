#ifndef MUDUO_BASE_SIGNALSLOTTRIVIAL_H
#define MUDUO_BASE_SIGNALSLOTTRIVIAL_H

#include <memory>
#include <vector>

template <typename Signature>
class SignalTrivial;

template <typename RET, typename... ARGS>
class SignalTrivial<RET(ARGS...)> {
    public:
        typedef std::function<void (ARGS...)> Functor;

        void connect(Functor &&func) {
            functors_.push_back(std::forward<Functor>(func));
        }  

        void call(ARGS&&... args) {
            for(const Functor & f : functors_) {
                f(args...);
            }
        }

    private:
        std::vector<Functor> functor_;
};

#endif // MUDUO_BASE_SIGNALSLOTTRIVIAL_H