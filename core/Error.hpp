#ifndef ARISTA_ERROR_H
#define ARISTA_ERROR_H

#include "Core.hpp"

#include <chrono>
#include <string>
#include <memory>
#include <system_error>

namespace arista {

class Error {
  public:
    Error() = default;
    Error(const std::error_code errorCode, std::string description = "", std::string info = "");

    virtual std::error_code code(void) const { return m_errorCode; }
    virtual std::string message(void) const { return m_errorCode.message(); }

    virtual std::string description() { return m_description; }
    virtual std::string info() { return m_info; }

    virtual std::string toString() { return arista::format("{}: {} {}", message(), m_description, m_info); }

  private:
    std::error_code m_errorCode {};

    std::chrono::time_point<std::chrono::system_clock> m_timestamp;

    std::string m_description;
    std::string m_info;
};

}

#endif