#include "Error.hpp"

namespace arista {

Error::Error(const std::error_code errorCode, std::string description, std::string info)
    : m_errorCode(errorCode), m_description(description), m_info(info)
{}

}
