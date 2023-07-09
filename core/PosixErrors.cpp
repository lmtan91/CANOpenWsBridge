#include "PosixErrors.hpp"

std::string arista::PosixErrorCategory::message(int c) const {
    switch (static_cast<PosixErrorCodes>(c))
    {
        case PosixErrorCodes::InvalidDomain:
            return "Invalid protocol domain";
        case PosixErrorCodes::InvalidType:
            return "Invalid protocol type";
        case PosixErrorCodes::Invalid:
            return "Invalid operation";
        case PosixErrorCodes::OperationNotSupported:
            return "Operation not supported";
        case PosixErrorCodes::UnixSocketPathTooLong:
            return "UnixSocket path too long";
        case PosixErrorCodes::PermissionDenied:
            return "Permission denied";
        case PosixErrorCodes::NotFound:
            return "Not found";
    }

    return "unknown";
}

std::error_condition arista::PosixErrorCategory::default_error_condition(int c) const noexcept {
    switch (static_cast<PosixErrorCodes>(c))
    {
        case PosixErrorCodes::InvalidDomain:
            return make_error_condition(std::errc::invalid_argument);
        case PosixErrorCodes::InvalidType:
            return make_error_condition(std::errc::invalid_argument);
        case PosixErrorCodes::OperationNotSupported:
            return make_error_condition(std::errc::operation_not_supported);
        case PosixErrorCodes::UnixSocketPathTooLong:
            return make_error_condition(std::errc::invalid_argument);
    }

    return std::error_condition(c, *this);
}

const arista::PosixErrorCategory &arista::posixSocketErrorCategory() {
    static arista::PosixErrorCategory c;
    return c;
}


