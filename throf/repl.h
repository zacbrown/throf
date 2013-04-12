#pragma once


namespace throf
{
    const std::string REPL_PROMPT = "> ";
}

#ifdef _WIN32
#include "repl_win32.h"
#else
#include "repl_posix.h"
#endif