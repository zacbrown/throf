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

    // helper funcs
    private:
        void initialize();
        void dispatch(const StackElement elem);
        void processDirective(Token& directive, Token& arg);
        void processToken(Tokenizer& tokenizer, const Token& tok);
        StackElement createStackElementFromToken( Tokenizer& tokenizer, const Token& tok);
        void addWordToDictionary(Tokenizer& tokenizer, const std::string& s);
        std::string stackToString();
        std::string loadedWordsToString();

        // pretty printers
        void prettyFormatStackElement(const StackElement& elem, stringstream& strBuilder);
        void prettyFormatQuotation(const StackElement& elem, stringstream& strBuilder);

        // convenience throwers
        void throwIfTypeUnexpected(const StackElement& element,
            StackElement::ElementType expected, const string msg) const;

        void throwIfVariableNotDefined(const StackElement& element, const string msg) const;

        // block assignment
        Interpreter& operator=(Interpreter& right) { return right; }

    // member vars
    private:
        typedef unordered_map<string, WORD_ID> StringToWORDDictionary;
        typedef unordered_map<WORD_ID, std::vector<std::vector<StackElement>>> Dictionary;
        Dictionary _dictionary;
        StringToWORDDictionary _stringToWordDict;
        unordered_set<string> _variablesInScope;
        unordered_set<string> _deferredWords;
        std::vector<StackElement> _stack;
        std::string _filename;
    };
}