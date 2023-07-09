#ifndef ARISTA_CORE_ERRORS_H
#define ARISTA_CORE_ERRORS_H

#include <system_error>
#include <string>

namespace arista {

    enum class CoreErrorCode {
        InitFailed,
        NotInitialized,
        AlreadyInitialized,
        CloseFailed,
        StartFailed,
        StopFailed,
        OperationFailed,
        InvalidSignal
    };

    // Define a custom error code category derived from std::error_category
    class CoreErrorCategory : public std::error_category
    {
    public:
        // Return a short descriptive name for the category
        virtual const char *name() const noexcept override final { return "basic error"; }
        // Return what each enum means in text
        virtual std::string message(int c) const override final;

        virtual std::error_condition default_error_condition(int c) const noexcept override final;
    };

    const CoreErrorCategory& coreErrorCategory();

    inline std::error_code make_error_code(arista::CoreErrorCode e) {
        return {static_cast<int>(e), arista::coreErrorCategory()};
    }

    inline std::error_code lastSystemError() {
        return std::make_error_code(static_cast<std::errc>(errno));
    }
}

#endif //ARISTA_CORE_ERRORS_H
