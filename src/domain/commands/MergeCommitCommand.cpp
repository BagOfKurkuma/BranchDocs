#include "MergeCommitCommand.h"

namespace branchdocs::domain {

MergeCommitCommand::MergeCommitCommand(Document &doc, std::string mergedContent, std::string comment)
    : m_document(doc)
    , m_mergedContent(std::move(mergedContent))
    , m_comment(std::move(comment))
{
}

void MergeCommitCommand::execute()
{
    if (!m_executed) {
        m_previousContent = m_document.content();
        m_preModified = m_document.isModified();
        m_executed = true;
    } else {
        m_document.history().removeVersion(m_createdVersionId);
    }

    m_document.restoreState(m_mergedContent, true);
    const auto v = m_document.createSnapshot(m_comment);
    m_createdVersionId = v.id();
}

void MergeCommitCommand::undo()
{
    m_document.history().removeVersion(m_createdVersionId);
    m_document.restoreState(m_previousContent, m_preModified);
}

std::string MergeCommitCommand::description() const { return "Merge & commit: " + m_comment; }

} // namespace branchdocs::domain