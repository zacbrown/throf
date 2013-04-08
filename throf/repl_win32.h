#pragma once

#include "stdafx.h"
#include <iostream>

using namespace std;

namespace throf
{
    class REPL
    {
    private:
        HANDLE hStdIn;
        DWORD dwOriginalConsoleSettings;
        Interpreter& interpreter;

        REPL& operator=(REPL& other) { return other; } // block assignment

    public:
        REPL(Interpreter& i) : interpreter(i)
        {
            this->hStdIn = GetStdHandle(STD_INPUT_HANDLE);
            ThrofException::throwIfTrue(INVALID_HANDLE_VALUE == this->hStdIn, "REPL", 
                string("GetStdHandle failed while setting up REPL with GetLastError()=" + to_string(GetLastError())));

            ThrofException::throwIfTrue(GetConsoleMode(this->hStdIn, &dwOriginalConsoleSettings) == FALSE, "REPL",
                string("GetConsoleMode failed while setting up REPL with GetLastError()=" + to_string(GetLastError())));

            ThrofException::throwIfTrue(SetConsoleMode(this->hStdIn, ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT) == FALSE,
                "REPL", string("SetConsoleMode failed while setting up REPL with GetLastError()=" + to_string(GetLastError())));

            startProcessingLoop();
        }

        ~REPL()
        {
            ThrofException::throwIfTrue(SetConsoleMode(this->hStdIn, dwOriginalConsoleSettings) == FALSE, "REPL",
                string("SetConsoleMode failed while restoring console settings with GetLastError()=" + to_string(GetLastError())));

            if (this->hStdIn)
            {
                CloseHandle(this->hStdIn);
            }
        }

        void startProcessingLoop()
        {
            DWORD dwNumCharsRead = 0;
            static const DWORD BUFFER_SIZE = 4096;
            char* chars = new char[BUFFER_SIZE + 1];
            ThrofException::throwIfTrue(nullptr == chars, "REPL", "Unexpected error dynamically allocating character array.");
            unique_ptr<char[]> spCharBuffer(chars);

            for(;;)
            {
                std::cout << "> ";

                ThrofException::throwIfTrue(FALSE == ReadConsole(this->hStdIn, chars, BUFFER_SIZE, &dwNumCharsRead, NULL), "REPL",
                    "ReadConsole failed while reading input with GetLastError()=" + to_string(GetLastError()));

                chars[dwNumCharsRead] = 0; // so we don't pass junk in.

                InputReader reader(std::string(chars), true);
                auto tokenizer = Tokenizer::tokenize(reader);
                interpreter.loadFile(tokenizer);
            }
        }
    };
}