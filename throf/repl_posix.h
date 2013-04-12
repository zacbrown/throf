#pragma once

#include "stdafx.h"
#include <iostream>

#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

namespace throf
{
    class REPL
    {
    private:
        Interpreter& _interpreter;

        REPL& operator=(REPL& other) { return other; } // block assignment

    public:
        REPL(Interpreter& i) : _interpreter(i)
        {
            startProcessingLoop();
        }

        ~REPL()
        {
        }

        void startProcessingLoop()
        {
            std::unique_ptr<char> buf;
            for (;;)
            {
                buf.reset(readline(REPL_PROMPT.c_str()));
                if (!buf)
                {
                    // We don't want to do any work on empty input lines, but we
                    // also don't want to bail out. 
                    continue;
                }

                add_history(buf.get());

                try
                {
                    InputReader reader(std::string(buf.get()), true);
                    auto tokenizer = Tokenizer::tokenize(reader);
                    _interpreter.loadFile(tokenizer);
                }
                catch (const ThrofException& e)
                {
                    printf("ERROR: Error encountered while processing REPL input:");
                    printError("\tcomponent: %s", e.component());
                    printError("\texplanation: %s", e.what());
                }
            }
        }
    };
}