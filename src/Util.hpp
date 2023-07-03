#ifndef ARISTA_UTIL_H
#define ARISTA_UTIL_H

#include "outcome/outcome.hpp"
namespace outcome = OUTCOME_V2_NAMESPACE;

#include "fmt/format.h"

// #include <gsl/span>

#include <cstdio>
#include <cstdlib>
#include <string_view>

#ifdef _MSC_VER
    #ifdef KATLA_CORE_INDLL
    # define KATLA_CORE_DECLSPEC __declspec(dllexport)
    #else
    # define KATLA_CORE_DECLSPEC __declspec(dllimport)
    #endif
#else
    #define KATLA_CORE_DECLSPEC
#endif

namespace arista {

    // template<class T, std::size_t Extent>
    // using span = gsl::span<T, Extent>;

    /****
     * Declare format and print here for convenience. Current implementation uses the Fmt lib,
     * the idea is to use std c++ format later. Which is the reason for the indirection.
     */
    template <typename S, typename... Args>
    inline std::string format(const S& format_str, Args&&... args) {
        return fmt::format(format_str, args...);
    }

    template <typename S, typename... Args>
    inline void print(std::FILE* f, const S& format_str, Args&&... args) {
        fmt::print(f, format_str, args...);
    }

    template <typename S, typename... Args>
    inline void printInfo(const S& message, Args&&... args) {
        print(stdout, fmt::format("{}\n", message), args...);
        fflush(stdout);
    }

    template <typename S, typename... Args>
    inline void printError(const S& message, Args&&... args) {
        print(stderr, fmt::format("{}\n", message), args...);
        fflush(stderr);
    }

    template <typename S, typename... Args>
    inline void fatal(const S& message, Args&&... args) {
        print(stderr, fmt::format("{}\n", message), args...);
        fflush(stderr);
        std::abort();
    }
}

#endif // ARISTA_UTIL_H