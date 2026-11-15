#pragma once
#include <chrono>
#include <string>

#include "Version.h"
#include "VersionHistory.h"

namespace branchdocs::domain {

class Document
{
    public:
        explicit Document(std::string title);
        Document(std::string id, std::string title);

        const std::string &id() const noexcept { return m_id; }
        const std::string &title() const noexcept { return m_title; }
        const std::string &content() const noexcept { return m_content; }
        bool isModified() const noexcept { return m_isModified; }

        void edit(std::string newContent);
        Version createSnapshot(const std::string &comment);
        void restoreFrom(const Version &v);
        void restoreState(std::string content, bool wasModified);

        VersionHistory &history() noexcept { return m_history; }
        const VersionHistory &history() const noexcept { return m_history; }

    private:
        std::string m_id;
        std::string m_title;
        std::string m_content;
        std::chrono::system_clock::time_point m_createdAt;
        bool m_isModified = false;
        VersionHistory m_history;
};

} // namespace branchdocs::domain