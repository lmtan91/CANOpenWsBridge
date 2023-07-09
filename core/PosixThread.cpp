#include "PosixThread.hpp"
#include "PosixErrors.hpp"

#include <pthread.h>

namespace arista {

outcome::result<void, Error> PosixThread::setPriority(std::thread& thread, Thread::Priority priority)
{
    auto nativeHandle = thread.native_handle();

    int policy = SCHED_OTHER;

    sched_param param {};
    param.sched_priority = 0;

    switch(priority) {
        case Thread::Priority::Realtime: 
            policy = SCHED_RR;
            param.sched_priority = 10;
            break;
        case Thread::Priority::Normal:
            policy = SCHED_OTHER;
            break;
        case Thread::Priority::Idle:

            policy = SCHED_IDLE;
    }

    int result = pthread_setschedparam(nativeHandle, policy, &param);
    if (result != 0) {
        switch(result) {
            case ESRCH:
                return Error(make_error_code(PosixErrorCodes::NotFound), "Thread not found!");
            case EINVAL:
                return Error(make_error_code(PosixErrorCodes::Invalid), "Invalid policy");
            case EPERM:
                return Error(make_error_code(PosixErrorCodes::PermissionDenied), "Permission denied");
            case ENOTSUP:
            default: {
                return Error(make_error_code(PosixErrorCodes::OperationNotSupported), "Operation not supported");
            }
        }
    }

    return outcome::success();
}

}