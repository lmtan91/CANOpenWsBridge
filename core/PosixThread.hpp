#ifndef ARISTA_POSIX_THREAD_H
#define ARISTA_POSIX_THREAD_H

#include "Core.hpp"
#include "Error.hpp"
#include "Thread.hpp"

// #include <gsl/span>

#include <optional>
#include <thread>

namespace arista {

class PosixThread {
public:
    static outcome::result<void, Error> setPriority(std::thread& thread, Thread::Priority priority);
private:

};

}

#endif // ARISTA_POSIX_THREAD_H