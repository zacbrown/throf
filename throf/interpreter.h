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
        void initialize();
        void dispatch(const StackElement& s);
        void processDirective(Token& directive, Token& arg);
        void processToken(Tokenizer& tokenizer, const Token& tok);
        StackElement createStackElementFromToken( Tokenizer& tokenizer, const Token& tok);
        void addWordToDictionary(Tokenizer& tokenizer, const std::string& s);
        std::string stackToString();

        // pretty printers
        void prettyFormatStackElement(const StackElement& elem, stringstream& strBuilder);
        void prettyFormatQuotation(const StackElement& elem, stringstream& strBuilder);

        // convenience throwers
        void throwIfTypeUnexpected(const StackElement& element,
            StackElement::ElementType expected, const string msg) const;

        // block assignment
        Interpreter& operator=(Interpreter& right) { return right; }

    // member vars
    private:
        typedef unordered_map<string, WORD_IDX> StringToWORDDictionary;
        typedef unordered_map<WORD_IDX, std::vector<StackElement>> Dictionary;
        typedef unordered_map<string, StackElement> VariableDictionary;
        Dictionary _dictionary;
        StringToWORDDictionary _stringToWordDict;
        VariableDictionary _variableDictionary;
        std::vector<StackElement> _stack;
        std::string _filename;
    };
}