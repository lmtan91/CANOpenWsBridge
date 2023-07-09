
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "Core.hpp"
#include <string>
#include <vector>

namespace arista {
namespace string {

    std::string humanFileSize(long long fileSize);

    std::string trimPrefix(std::string src, std::string prefix);

    std::string trimSuffix(const std::string& src, const std::string& suffix);

    std::string trimWhitespace(const std::string& str);

    std::string toUpper(const std::string& str);
    std::string toLower(const std::string& str);

    bool startsWith (const std::string& str, const std::string& start);
    bool endsWith (const std::string& str, const std::string& end);

    bool contains (const std::string& str, const std::string& substr);

    std::vector<std::string> split(std::string input, std::string delimiter);

} // namespace string
} // namespace arista  


#endif // ARISTA_CORE_H
