
#ifndef OBSERVER_H
#define OBSERVER_H

#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <utility>

namespace arista {

template <class T> class Observer {
  public:
    virtual ~Observer() = default;
    virtual void next(const T& value) = 0;
};

template <>
class Observer<void> {
  public:
    virtual ~Observer() = default;
    virtual void next() = 0;
};

template <class T> class FuncObserver : public Observer<T> {
  public:
    explicit FuncObserver(std::function<void(const T&)> func) { _func = func; }

    void next(const T& value)
    {
        if (_func) {
            _func(value);
        }
    }

  private:
    std::function<void(const T&)> _func;
};

template <>
class FuncObserver<void> : public Observer<void> {
  public:
    explicit FuncObserver(std::function<void(void)> func) { _func = std::move(func); }
    ~FuncObserver() override = default;

    void next() override
    {
        if (_func) {
            _func();
        }
    }

  private:
    std::function<void(void)> _func;
};



}

#endif