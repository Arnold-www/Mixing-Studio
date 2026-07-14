#pragma once

// Traditional Command pattern: encapsulate a user intent as an executable object.
class ICommand
{
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
};
