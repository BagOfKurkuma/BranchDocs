#pragma once
#include <string>

#include "../Document.h"
#include "Command.h"

namespace branchdocs::domain {

class MergeCommitCommand : public Command
{
    public:
        MergeCommitCommand(Document &doc, std::string mergedContent, std::string comment);

        void execute() override;
        void undo() override;
        std::string description() const override;

    private:
        Document &m_document;
        std::string m_mergedContent;
        std::string m_comment;
        std::string m_previousContent;
        bool m_preModified = false;
        std::string m_createdVersionId;
        bool m_executed = false;
};

} // namespace branchdocs::domain