#pragma once
#include <string>

namespace branchdocs::domain {

class Command
{
    public:
        virtual ~Command() = default;
        virtual void execute() = 0;
        virtual void undo() = 0;
        virtual std::string description() const = 0;
};

} // namespace branchdocs::domain