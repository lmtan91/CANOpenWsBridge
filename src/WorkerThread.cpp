#include "WorkerThread.hpp"

#include "Util.hpp"
#include "Error.hpp"
#include "CoreErrors.hpp"

#include <chrono>
#include <mutex>
#include <utility>

namespace arista {

using namespace std::literals::chrono_literals;

template <typename F, typename... Args>
decltype(auto) invoke(F&& f, Args&&... args)
{
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

WorkerThread::WorkerThread(std::string name, arista::Thread::Priority priority)
    : m_name(std::move(name)), m_priority(priority)
{}

WorkerThread::~WorkerThread() { join(); }

outcome::result<void, Error> WorkerThread::init(std::function<void(void)> repeatableWork,
                                                std::chrono::milliseconds interval)
{
    m_interval = interval;
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_thread) {
            return Error(arista::make_error_code(arista::CoreErrorCode::AlreadyInitialized));
        }

        m_stop = false;
        m_wakeUp = false;
        m_thread = std::make_unique<std::thread>(&WorkerThread::exec, this, repeatableWork);
    }

    return outcome::success();
}

void WorkerThread::wakeup() 
{
    m_wakeUp = true;
    m_condition.notify_all(); 
}

void WorkerThread::stop()
{
    m_stop = true;
    wakeup();
}

void WorkerThread::join()
{
    if (m_thread) {
        m_thread->join();
        m_thread.reset();
    }
}

void WorkerThread::exec(const std::function<void(void)>& repeatableWork)
{
    bool stop = false;
    bool noWait = false;

    {
        // need lock otherwise m_thread is not always assigned yet
        std::unique_lock<std::mutex> lock(m_mutex);

        auto setPriorityResult = arista::Thread::setPriority(*m_thread, m_priority);
        if (!setPriorityResult) {
            arista::printError("Failed setting thread priority: {}", setPriorityResult.error().message());
        }
    }

    while (!stop) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            if (!noWait) {
                m_condition.wait_for(lock, m_interval);
            }

            if (m_stop) {
                stop = true;
                continue;
            }

            m_wakeUp = false;
        }

        invoke(repeatableWork);

        noWait = m_skipWaitForNextCycle || m_wakeUp;
        m_skipWaitForNextCycle = false;
    }

    arista::printInfo("Closing thread {}!", m_name);
}

} // namespace arista
