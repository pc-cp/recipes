#include <algorithm>
#include <vector>
#include <stdio.h>
#include "../Mutex.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Observable;

class Observer : public boost::enable_shared_from_this<Observer> {
    public:
        virtual ~Observer();
        virtual void update() = 0;

        void observe(Observable *s);

    protected:
        Observable* subject_;
};

class Observable {
    public:
        void register_(boost::weak_ptr<Observer> x) {
            observers_.push_back(x);
        }
        // void unregister(boost::weak_ptr<Observer> x);
        void notifyObservers() {
            muduo::MutexLockGuard lock(mutex_);
            Iterator it = observers_.begin();
            while(it != observers_.end()) {
                boost::shared_ptr<Observer>obj(it->lock());
                if(obj) {
                    obj->update();
                    ++it;
                }
                else {
                    printf("notifyObservers() erase\n");
                    it = observers_.erase(it);
                }
            }
        }
    private:
        mutable muduo::MutexLock mutex_;
        std::vector<boost::weak_ptr<Observer> > observers_;
        typedef std::vector<boost::weak_ptr<Observer> >::iterator Iterator;
};

Observer::~Observer() {
    // subject_->unregister(this);
}

void Observer::observe(Observable *s) {
    // Returns a std::shared_ptr<T> that shares ownership
    // of *this with all existing std::shared_ptr that refer to *this
    s->register_(shared_from_this());
    subject_ = s;
}

class Foo : public Observer {
    virtual void update() {
        printf("Foo::update() %p\n", this);
    }
};

int main() {

    Observable subject;
    {
        boost::shared_ptr<Foo> p(new Foo);
        p->observe(&subject);
        subject.notifyObservers();
    }
    subject.notifyObservers();
}