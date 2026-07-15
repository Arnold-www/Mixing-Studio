#pragma once

// Shared command contract for ViewModel / App binding (Qt slots may invoke execute()).
class ICommandBase
{
public:
    virtual ~ICommandBase() = default;
    virtual void execute() = 0;
};

// Backward-compatible alias used by existing command classes.
using ICommand = ICommandBase;
