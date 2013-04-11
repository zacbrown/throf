#pragma once

#include "stdafx.h"
#include <iostream>

using namespace std;

namespace throf
{
    class REPL
    {
    private:
        //Interpreter& interpreter;

        REPL& operator=(REPL& other) { return other; } // block assignment

    public:
        REPL(Interpreter& i)// : interpreter(i)
        {
            startProcessingLoop();
        }

        ~REPL()
        {
        }

        void startProcessingLoop()
        {
        }
    };
}