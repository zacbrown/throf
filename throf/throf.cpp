#include "stdafx.h"

using namespace throf;

extern int* TOP_OF_STACK;
const char* const INIT_FILENAME = "init.th4";

void dumpTokens(Tokenizer& tokenizer)
{
    while (tokenizer.hasNextToken())
    {
        Token tok = tokenizer.getNextToken();
        printf("token data: %s (type: %d)\n", tok.getData().c_str(), tok.getType());
    }
    tokenizer.reset();
}

void loadInitFile(Interpreter& interpreter)
{
    FILE* f = fopen(INIT_FILENAME, "r");
    if (nullptr != f)
    {
        fclose(f);
        InputReader reader(INIT_FILENAME);
        Tokenizer tokenizer = Tokenizer::tokenize(reader);
        interpreter.loadFile(tokenizer);
    }
}

int main(int argc, char* argv[])
{
    int __TOP_OF_STACK;
    TOP_OF_STACK = &__TOP_OF_STACK;
    try
    {
        Interpreter interpreter;
        loadInitFile(interpreter);

        if (argc == 1)
        {
            // REPL mode
            interpreter.repl();
        }
        else
        {
            string filename = argv[1];
            InputReader reader(filename);
            Tokenizer tokenizer = Tokenizer::tokenize(reader);
            interpreter.loadFile(tokenizer);
        }
    }
    catch (const ThrofException& e)
    {
        printf("ERROR: Error encountered while processing file:\n");
        printError("\tfilename: %s", e.filename());
        printError("\tcomponent: %s", e.component());
        printError("\texplanation: %s", e.what());
    }

    return 0;
}