#ifndef ARISTA_POSIX_ERRORS_H
#define ARISTA_POSIX_ERRORS_H

#include <system_error>
#include <string>

namespace arista {

    enum class PosixErrorCodes {
        InvalidDomain,
        InvalidType,
        Invalid,
        OperationNotSupported,
        UnixSocketPathTooLong,
        PermissionDenied,
        NotFound
    };

    // Define a custom error code category derived from std::error_category
    class PosixErrorCategory : public std::error_category
    {
    public:
        // Return a short descriptive name for the category
        virtual const char *name() const noexcept override final { return "Posix error"; }
        // Return what each enum means in text
        virtual std::string message(int c) const override final;

        virtual std::error_condition default_error_condition(int c) const noexcept override final;
    };

    const PosixErrorCategory& posixSocketErrorCategory();

    inline std::error_code make_error_code(arista::PosixErrorCodes e) {
        return {static_cast<int>(e), arista::posixSocketErrorCategory()};
    }
}

#endif //ARISTA_POSIX_ERRORS_H
