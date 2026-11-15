#pragma once
#include <cstddef>
#include <memory>
#include <vector>

#include "Command.h"

namespace branchdocs::domain {

class CommandStack
{
    public:
        void execute(std::unique_ptr<Command> cmd);
        void undo();
        void redo();
        bool canUndo() const noexcept { return !m_undoStack.empty(); }
        bool canRedo() const noexcept { return !m_redoStack.empty(); }
        void clear();
        std::size_t undoSize() const noexcept { return m_undoStack.size(); }
        std::size_t redoSize() const noexcept { return m_redoStack.size(); }

    private:
        std::vector<std::unique_ptr<Command>> m_undoStack;
        std::vector<std::unique_ptr<Command>> m_redoStack;
};

} // namespace branchdocs::domain