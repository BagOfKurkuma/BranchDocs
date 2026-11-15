#include "Version.h"
#include "Uuid.h"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace branchdocs::domain {

Version::Version(std::string content, std::string comment)
    : m_id(generateUuid())
    , m_content(std::move(content))
    , m_comment(std::move(comment))
    , m_timestamp(std::chrono::system_clock::now())
{}

std::string Version::formattedTimestamp() const {
    const std::time_t t = std::chrono::system_clock::to_time_t(m_timestamp);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace branchdocs::domain
