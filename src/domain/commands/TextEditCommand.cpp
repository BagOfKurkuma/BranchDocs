#include "TextEditCommand.h"

namespace branchdocs::domain {

TextEditCommand::TextEditCommand(Document &doc, std::string newContent) : m_document(doc), m_newContent(std::move(newContent)) {}

void TextEditCommand::execute()
{
    if (!m_executed) {
        m_oldContent = m_document.content();
        m_preModified = m_document.isModified();
        m_executed = true;
    }
    m_document.restoreState(m_newContent, true);
}

void TextEditCommand::undo() { m_document.restoreState(m_oldContent, m_preModified); }

std::string TextEditCommand::description() const { return "Edit content"; }

} // namespace branchdocs::domain