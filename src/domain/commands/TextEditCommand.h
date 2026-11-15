#pragma once
#include <string>

#include "../Document.h"
#include "Command.h"

namespace branchdocs::domain {

class TextEditCommand : public Command
{
    public:
        TextEditCommand(Document &doc, std::string newContent);

        void execute() override;
        void undo() override;
        std::string description() const override;

    private:
        Document &m_document;
        std::string m_newContent;
        std::string m_oldContent;
        bool m_preModified = false;
        bool m_executed = false;
};

} // namespace branchdocs::domain