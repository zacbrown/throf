#include "stdafx.h"

using namespace throf;

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
        FileReader reader(INIT_FILENAME);
        Tokenizer tokenizer = Tokenizer::tokenize(reader);
        interpreter.loadFile(tokenizer);

    }
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        // REPL mode
    }
    else
    {
        string filename = argv[1];
        try
        {
            FileReader reader(filename);
            Tokenizer tokenizer = Tokenizer::tokenize(reader);
            dumpTokens(tokenizer);
            Interpreter interpreter;
            loadInitFile(interpreter);
            interpreter.loadFile(tokenizer);
            interpreter.dumpInterpreterState();
        }
        catch (ThrofException& e)
        {
            printError("Error encountered while processing file '%s'", filename.c_str());
            printError("\t filename: %s", e.filename());
            printError("\t component: %s", e.component());
            printError("\t explanation: %s", e.what());
        }
    }
    return 0;
}