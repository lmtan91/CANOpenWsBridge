
#ifndef SUBJECT_H
#define SUBJECT_H

#include "Observable.hpp"
#include "Observer.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <mutex>

namespace arista {

template <class T> class Subject : public Observable<T>, public Observer<T> {
  public:
    Subject() = default;
    Subject(Subject&) = delete;
    virtual ~Subject() = default;

    void next(const T& value)
    {
        std::vector<std::shared_ptr<Observer<T>>> observers;
        {
            std::scoped_lock lock(m_mutex);
            observers = m_observers;
        }

        for (auto o : observers) {
            o->next(value);
        }
    }

    std::unique_ptr<Subscription> subscribe(const std::shared_ptr<Observer<T>>& observer)
    {
        std::scoped_lock lock(m_mutex);
        m_observers.push_back(observer);

        return std::unique_ptr<Subscription>(
            new FuncSubscription([this, observer]() { this->unsubscribe(observer); }));
    }

    void unsubscribe(const std::shared_ptr<Observer<T>>& observer)
    {
        std::scoped_lock lock(m_mutex);
        auto it = std::find(m_observers.begin(), m_observers.end(), observer);
        if (it != m_observers.end()) {
            m_observers.erase(it);
        }
    }

    void clear() {
        std::scoped_lock lock(m_mutex);
        m_observers.clear();
    }

  private:
    std::mutex m_mutex;
    std::vector<std::shared_ptr<Observer<T>>> m_observers;
};

template<>
class Subject<void> : public Observable<void>, public Observer<void> {
  public:
    Subject() = default;
    Subject(Subject&) = delete;
    ~Subject() override = default;

    void next() override
    {
        std::vector<std::shared_ptr<Observer<void>>> observers;
        {
            std::scoped_lock lock(m_mutex);
            observers = m_observers;
        }

        for (const auto& o : observers) {
            o->next();
        }
    }

    std::unique_ptr<Subscription> subscribe(const std::shared_ptr<Observer<void>>& observer) override
    {
        std::scoped_lock lock(m_mutex);
        m_observers.push_back(observer);

        return std::unique_ptr<Subscription>(
            new FuncSubscription([this, observer]() { 
                this->unsubscribe(observer);
        }));
    }

    void unsubscribe(const std::shared_ptr<Observer<void>>& observer)
    {
        std::scoped_lock lock(m_mutex);

        auto it = std::find(m_observers.begin(), m_observers.end(), observer);
        if (it != m_observers.end()) {
            m_observers.erase(it);
        }
    }
    void clear() {
        std::scoped_lock lock(m_mutex);
        m_observers.clear();
    }

  private:
    std::mutex m_mutex;
    std::vector<std::shared_ptr<Observer<void>>> m_observers;
};


}

#endif