#pragma once

namespace throf
{
    class Interpreter
    {
    public:
        Interpreter();
        ~Interpreter();

        void repl();
        void loadFile(Tokenizer& tokenizer);
        void dumpInterpreterState();
    
    // helper funcs
    private:
        void dispatchWord(const std::string& s);
        void processToken(const Token& tok);
        void processDirective(Token& directive, Token& arg);
        void initialize();
        void addWordToDictionary(const std::string& s, std::vector<Token> toks);
        std::string stackToString();

        // convenience throwers
        void throwIfTypeUnexpected(const StackElement& element,
            StackElement::ElementType expected, const string msg) const;

        // block assignment
        Interpreter& operator=(Interpreter& right) { return right; }

    // member vars
    private:
        typedef unordered_map<string, WORD_IDX> StringToWORDDictionary;
        typedef unordered_map<WORD_IDX, std::vector<Token>> Dictionary;
        typedef unordered_map<string, StackElement> VariableDictionary;
        Dictionary _dictionary;
        StringToWORDDictionary _stringToWordDict;
        VariableDictionary _variableDictionary;
        std::vector<StackElement> _stack;
        std::string _filename;
    };
}