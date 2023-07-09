
#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include "Observer.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace arista {

class Subscription {
  public:
    virtual ~Subscription() = default;
    virtual void unsubscribe() = 0;
};

class FuncSubscription : public Subscription {
  public:
    explicit FuncSubscription(std::function<void(void)> unsubscribeCallback) : m_unsubscribe(std::move(unsubscribeCallback)) {}
    ~FuncSubscription() override = default;

    void unsubscribe() override { m_unsubscribe(); }

  private:
    std::function<void(void)> m_unsubscribe;
};

template <class T> class Observable {
  public:
    virtual ~Observable() = default;
    virtual std::unique_ptr<Subscription> subscribe(const std::shared_ptr<Observer<T>>& observer) = 0;
};

} // namespace arista

#endif