#include "CoreErrors.hpp"

namespace arista {

std::string CoreErrorCategory::message(int c) const
{
    switch (static_cast<CoreErrorCode>(c)) {
        case CoreErrorCode::InitFailed:
            return "Init failed";
        case CoreErrorCode::NotInitialized:
            return "Not Initialized";
        case CoreErrorCode::AlreadyInitialized:
            return "Already Initialized";
        case CoreErrorCode::CloseFailed:
            return "Close failed";
        case CoreErrorCode::StartFailed:
            return "Start failed";
        case CoreErrorCode::StopFailed:
            return "Stop failed";
        case CoreErrorCode::OperationFailed:
            return "Operation failed";
        case CoreErrorCode::InvalidSignal:
            return "Invalid signal";
    }

    return "Unknown error";
}

std::error_condition CoreErrorCategory::default_error_condition(int c) const noexcept
{
    return std::error_condition(c, *this);
}


const CoreErrorCategory& coreErrorCategory()
{
    static arista::CoreErrorCategory c;
    return c;
}

}
