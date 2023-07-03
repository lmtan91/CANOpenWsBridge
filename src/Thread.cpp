
#include "Thread.hpp"
#include "PosixThread.hpp"

namespace arista {

    outcome::result<void, Error> Thread::setPriority(std::thread& thread, Thread::Priority priority)
    {
        return arista::PosixThread::setPriority(thread, priority);
    }

}