#include "Document.h"
#include <stdexcept>

#include "Uuid.h"

namespace branchdocs::domain {

Document::Document(std::string title)
    : m_id(generateUuid())
    , m_title(std::move(title))
    , m_createdAt(std::chrono::system_clock::now())
{
}

void Document::edit(std::string newContent)
{
    if (newContent == m_content)
        return;
    m_content = std::move(newContent);
    m_isModified = true;
}

Version Document::createSnapshot(const std::string &comment)
{
    if (!m_isModified) {
        throw std::logic_error("createSnapshot: no changes since last version");
    }
    Version v(m_content, comment);
    m_history.addVersion(v);
    m_isModified = false;
    return v;
}

} // namespace branchdocs::domain
