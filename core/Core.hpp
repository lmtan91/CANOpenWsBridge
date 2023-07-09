#ifndef ARISTA_CORE_H
#define ARISTA_CORE_H

#include "outcome/outcome.hpp"
namespace outcome = OUTCOME_V2_NAMESPACE;

#include "fmt/format.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string_view>

#ifdef _MSC_VER
    #ifdef ARISTA_CORE_INDLL
    # define ARISTA_CORE_DECLSPEC __declspec(dllexport)
    #else
    # define ARISTA_CORE_DECLSPEC __declspec(dllimport)
    #endif
#else
    #define ARISTA_CORE_DECLSPEC
#endif

namespace gsl {

template <typename T>
class span {
public:
    constexpr span() noexcept : data_(nullptr), size_(0) {}
    constexpr span(T* data, std::size_t size) noexcept : data_(data), size_(size) {}

    template <std::size_t N>
    constexpr span(T (&arr)[N]) noexcept : data_(arr), size_(N) {}

    constexpr T* data() const noexcept { return data_; }
    constexpr std::size_t size() const noexcept { return size_; }
    constexpr bool empty() const noexcept { return size_ == 0; }

    constexpr T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("span index out of range");
        }
        return data_[index];
    }

    constexpr T* begin() const noexcept { return data_; }
    constexpr T* end() const noexcept { return data_ + size_; }

private:
    T* data_;
    std::size_t size_;
};

}  // namespace gsl

namespace arista {

    template <typename T, std::size_t Extent>
    class span {
    public:
        constexpr span() noexcept : data_(nullptr), size_(0) {}
        constexpr span(T* data, std::size_t size) noexcept : data_(data), size_(size) {}

        constexpr T* data() const noexcept { return data_; }
        constexpr std::size_t size() const noexcept { return size_; }

        constexpr T& operator[](std::size_t index) const noexcept { return data_[index]; }

    private:
        T* data_;
        std::size_t size_;
    };

    template<class T, std::size_t Extent>
    using span = arista::span<T, Extent>;

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

#endif // ARISTA_CORE_H
