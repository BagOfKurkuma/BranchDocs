#pragma once
#include <chrono>
#include <string>

namespace branchdocs::domain {

class Version {
public:
    Version(std::string content, std::string comment);

    const std::string& id()      const noexcept { return m_id; }
    const std::string& content() const noexcept { return m_content; }
    const std::string& comment() const noexcept { return m_comment; }
    std::chrono::system_clock::time_point timestamp() const noexcept { return m_timestamp; }

    std::string formattedTimestamp() const;

private:
    const std::string m_id;
    const std::string m_content;
    const std::string m_comment;
    const std::chrono::system_clock::time_point m_timestamp;
};

} // namespace branchdocs::domain
