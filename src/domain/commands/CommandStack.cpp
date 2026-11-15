#include "CommandStack.h"

namespace branchdocs::domain {

void CommandStack::execute(std::unique_ptr<Command> cmd)
{
    cmd->execute();
    m_undoStack.push_back(std::move(cmd));
    m_redoStack.clear();
}

void CommandStack::undo()
{
    if (m_undoStack.empty())
        return;
    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    cmd->undo();
    m_redoStack.push_back(std::move(cmd));
}

void CommandStack::redo()
{
    if (m_redoStack.empty())
        return;
    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    cmd->execute();
    m_undoStack.push_back(std::move(cmd));
}

void CommandStack::clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

} // namespace branchdocs::domain