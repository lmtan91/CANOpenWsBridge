#ifndef ARISTA_THREAD_H
#define ARISTA_THREAD_H

#include "Util.hpp"
#include "Error.hpp"

#include <optional>
#include <thread>

namespace arista {

class Thread {
public:
    enum class Priority {
        Realtime,
        Normal,
        Idle
    };

    static outcome::result<void, Error> setPriority(std::thread& thread, Thread::Priority priority);
private:

};

}

#endif