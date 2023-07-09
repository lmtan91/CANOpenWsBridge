
#ifndef ARISTA_WORKER_THREAD_H
#define ARISTA_WORKER_THREAD_H

#include "Core.hpp"
#include "Thread.hpp"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace arista {

class WorkerThread {
  public:
    WorkerThread(std::string name, arista::Thread::Priority priority);
    virtual ~WorkerThread();

    outcome::result<void, Error> init(std::function<void(void)> repeatableWork, std::chrono::milliseconds interval);

    void wakeup();

    void stop();
    void join();

  protected:
    bool m_skipWaitForNextCycle {false};

  private:
    void exec(const std::function<void(void)>& repeatableWork);

    std::unique_ptr<std::thread> m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;

    std::string m_name { "WorkerThread" };

    arista::Thread::Priority m_priority { arista::Thread::Priority::Normal };

    bool m_stop {};
    bool m_wakeUp {};
    std::chrono::milliseconds m_interval { 1000 };
};

} // namespace arista

#endif
