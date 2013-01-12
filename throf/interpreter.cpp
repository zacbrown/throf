#include "stdafx.h"
#include <iostream>

namespace throf
{
    Interpreter::Interpreter() : _filename("")
    {
        initialize();
    }

    Interpreter::~Interpreter()
    { }

    void Interpreter::repl()
    {

    }

    void Interpreter::initialize()
    {
        for (auto itr = STR_TO_PRIM_WORD_MAP.cbegin(); itr != STR_TO_PRIM_WORD_MAP.cend(); itr++)
        {
            string str = (*itr).first;
            PRIMITIVE_WORD prim = (*itr).second;
            _stringToWordDict[str] = prim;
            _dictionary[prim] = vector<Token>();
        }
    }

    void Interpreter::throwIfTypeUnexpected(const StackElement& element,
        StackElement::ElementType expected, const string msg) const
    {
        if (element.type() != expected)
        {
            stringstream errBuilder;
            errBuilder << msg;
            switch (element.type())
            {
            case StackElement::Boolean:
                errBuilder << "'" << (element.booleanData() ? "true" : "false") << "' (boolean)";
            case StackElement::Number:
                errBuilder << "'" << element.numberData() << "' (number)";
                break;
            case StackElement::String:
            case StackElement::Variable:
                errBuilder << "\"" << element.stringData() << "\" (string literal)";
                break;
            case StackElement::Uninitialized:
            default:
                errBuilder << "uninitialized (??)";
                break;
            }

            throw ThrofException("Interpreter", errBuilder.str(), _filename);
        }
    }

    void Interpreter::dispatchWord(const string& s)
    {
        WORD_IDX idx = _stringToWordDict[s];
        string& filename = _filename;

        auto dispatch_arithmetic = [](WORD_IDX operation, const StackElement& top, const StackElement& bottom)
        {
            long ret = 0;
            switch (operation)
            {
            case PRIM_ADD:
                ret = bottom.numberData() + top.numberData();
                break;
            case PRIM_SUB:
                ret = bottom.numberData() - top.numberData();
                break;
            case PRIM_MUL:
                ret = bottom.numberData() * top.numberData();
                break;
            case PRIM_DIV:
                ret = bottom.numberData() / top.numberData();
                break;
            case PRIM_MOD:
                ret = bottom.numberData() % top.numberData();
            }

            return ret;
        };

        auto dispatch_comparison = [filename](WORD_IDX operation, const StackElement& top, const StackElement& bottom)
        {
            switch(operation)
            {
            case PRIM_LT:
                return bottom.numberData() < top.numberData();
            case PRIM_GT:
                return bottom.numberData() > top.numberData();
            case PRIM_LTE:
                return bottom.numberData() <= top.numberData();
            case PRIM_GTE:
                return bottom.numberData() >= top.numberData();
            }

            stringstream strBuilder;
            strBuilder << "unexpected word passed as comparison word : '" << operation << "'";
            throw ThrofException("Interpreter", strBuilder.str(), filename);
        };

        switch(idx)
        {
        case PRIM_STACK:
            printf("%s", stackToString().c_str());
            break;
        case PRIM_IF:
            {
                StackElement falseQuotation = _stack.back(); _stack.pop_back();
                StackElement trueQuotation = _stack.back(); _stack.pop_back();
                StackElement boolOutcome = _stack.back(); _stack.pop_back();

                if (boolOutcome.booleanData())
                {
                    for (auto tok : trueQuotation.quotationData())
                    {
                        processToken(tok);
                    }
                }
                else
                {
                    for (auto tok : falseQuotation.quotationData())
                    {
                        processToken(tok);
                    }
                }
            }
            break;
        case PRIM_DROP:
            _stack.pop_back();
            break;
        case PRIM_SWAP:
            {
                StackElement topOrig = _stack.back(); _stack.pop_back();
                StackElement bottomOrig = _stack.back(); _stack.pop_back();
                _stack.emplace_back(topOrig);
                _stack.emplace_back(bottomOrig);
            }
            break;
        case PRIM_TWOSWAP:
            {
                StackElement idx4 = _stack.back(); _stack.pop_back();
                StackElement idx3 = _stack.back(); _stack.pop_back();
                auto itr = _stack.end();
                itr -= 2;
                _stack.emplace(itr, idx4);
                _stack.emplace(itr, idx3);
            }
            break;
        case PRIM_SET:
            {
                StackElement variableName = _stack.back(); _stack.pop_back();
                StackElement value = _stack.back(); _stack.pop_back();

                throwIfTypeUnexpected(variableName, StackElement::Variable, "unexpected variable name ");
                _variableDictionary[variableName.stringData()] = value;
            }
            break;
        case PRIM_GET:
            {
                StackElement variableName = _stack.back(); _stack.pop_back();
                throwIfTypeUnexpected(variableName, StackElement::Variable, "unexpected variable name ");

                _stack.emplace_back(_variableDictionary[variableName.stringData()]);
            }
            break;
        case PRIM_ROT:
            {
                StackElement elem = _stack.back(); _stack.pop_back();
                auto itr = _stack.end();
                // decrement to point to first element, once to point to second element
                // Iterators puzzlingly point just before first/behind last element when
                // initialized. Perhaps by convenience?
                itr -= 2;
                _stack.insert(itr, elem);
            }
            break;
        case PRIM_PICK:
            {
                StackElement elemIndex = _stack.back(); _stack.pop_back();
                throwIfTypeUnexpected(elemIndex, StackElement::Number, "expected number, got : ");

                if (elemIndex.numberData() < 0)
                {
                    throw ThrofException("Interpreter", "must provide non-negative number (>0) to PICK", _filename);
                }

                auto itr = _stack.cend(); itr--;
                int idx = 0;
                while (idx < elemIndex.numberData() && itr != _stack.cbegin())
                {
                    itr--;
                    idx++;
                }
                const StackElement& elem = *itr;
                _stack.emplace_back(elem);
            }
            break;
        case PRIM_ADD:
        case PRIM_SUB:
        case PRIM_MUL:
        case PRIM_DIV:
        case PRIM_MOD:
            {
                StackElement top = _stack.back(); _stack.pop_back();
                StackElement bottom = _stack.back(); _stack.pop_back();
                throwIfTypeUnexpected(top, StackElement::Number, "expected number, got : ");
                throwIfTypeUnexpected(bottom, StackElement::Number, "expected number, got : ");
                _stack.emplace_back(StackElement(StackElement::Number, dispatch_arithmetic(idx, top, bottom)));
            }
            break;
        case PRIM_LT:
        case PRIM_GT:
        case PRIM_LTE:
        case PRIM_GTE:
            {
                StackElement top = _stack.back(); _stack.pop_back();
                StackElement bottom = _stack.back(); _stack.pop_back();
                throwIfTypeUnexpected(top, StackElement::Number, "expected number, got : ");
                throwIfTypeUnexpected(bottom, StackElement::Number, "expected number, got : ");
                _stack.emplace_back(StackElement(StackElement::Boolean,
                    StackElement::BooleanType(dispatch_comparison(idx, top, bottom))));
                
            }
            break;
        case PRIM_EQ:
        case PRIM_NEQ:
            {
                StackElement top = _stack.back(); _stack.pop_back();
                StackElement bottom = _stack.back(); _stack.pop_back();

                if (top.type() != bottom.type())
                {
                    stringstream strBuilder;
                    strBuilder << "unexpected mismatch of types on stack when excuting " << PRIM_WORD_TO_STR_MAP.at(idx);
                    strBuilder << " : " << top.type() << " <> " << bottom.type();
                    throw ThrofException("Interpreter", strBuilder.str(), _filename);
                }

                bool ret = false;
                switch (top.type())
                {
                case StackElement::String:
                    ret = 0 == top.stringData().compare(bottom.stringData());
                    break;
                case StackElement::Number:
                    ret = top.numberData() == bottom.numberData();
                    break;
                case StackElement::Boolean:
                    ret = top.booleanData() == bottom.booleanData();
                    break;
                default:
                    {
                        stringstream strBuilder;
                        strBuilder << "unsupported type for comparison : " << top.type();
                        throw ThrofException("Interpreter", strBuilder.str(), _filename);
                    }
                }
                printInfo("top: %s, bottom: %s, ret: %d", top.stringData().c_str(), bottom.stringData().c_str(), ret);
                // change the return value if necessary
                if (PRIM_NEQ == idx)
                {
                    ret = !ret;
                }

                _stack.emplace_back(StackElement(StackElement::Boolean, StackElement::BooleanType(ret)));
            }
            break;
        case PRIM_NOT:
            {
                StackElement elem = _stack.back(); _stack.pop_back();
                throwIfTypeUnexpected(elem, StackElement::Boolean, "expected boolean, got : ");
                _stack.emplace_back(StackElement(StackElement::Boolean,
                    StackElement::BooleanType(!elem.booleanData())));
            }
            break;
        case PRIM_AND:
        case PRIM_OR:
        case PRIM_XOR:
            {
                StackElement top = _stack.back(); _stack.pop_back();
                StackElement bottom = _stack.back(); _stack.pop_back();
                throwIfTypeUnexpected(top, StackElement::Boolean, "expected boolean, got : ");
                throwIfTypeUnexpected(bottom, StackElement::Boolean, "expected boolean, got : ");

                bool ret = false;
                switch (idx)
                {
                case PRIM_AND:
                    ret = top.booleanData() && bottom.booleanData();
                    break;
                case PRIM_OR:
                    ret = top.booleanData() || bottom.booleanData();
                    break;
                case PRIM_XOR:
                    ret = top.booleanData() != bottom.booleanData();
                    break;
                }

                _stack.emplace_back(StackElement(StackElement::Boolean,
                    StackElement::BooleanType(ret)));
            }
            break;
        default:
            vector<Token> toks = _dictionary[idx];
            for (auto itr = toks.cbegin(); itr != toks.cend(); itr++)
            {
                Token tok = (*itr);
                if (tok.getType() == Token::TokenType::QuotationOpen)
                {
                    vector<Token> quotation;
                    while (itr != toks.cend())
                    {
                        tok = *(++itr);

                        if (tok.getType() == Token::TokenType::QuotationClose)
                        {
                            break;
                        }

                        quotation.emplace_back(tok);
                    }

                    if (tok.getType() != Token::TokenType::QuotationClose)
                    {
                        throw ThrofException("Interpreter", "unexpected end of quotation without closing marker ']'", _filename);
                    }

                    _stack.emplace_back(StackElement(StackElement::Quotation, quotation));
                }
                else
                {
                    processToken(tok);
                }
            }
            break;
        }
    }

    void Interpreter::addWordToDictionary(const string& s, vector<Token> toks)
    {
        WORD_IDX idx = _stringToWordDict[s] = _stringToWordDict.size() + 1;
        _dictionary[idx] = toks;
    }

    void Interpreter::processToken(const Token& tok)
    {
        bool isTrueToken = (0 == tok.getData().compare("true"));
        if (isTrueToken || 0 == tok.getData().compare("false"))
        {
            _stack.emplace_back(StackElement(StackElement::ElementType::Boolean,
                StackElement::BooleanType(isTrueToken)));
        }
        else if (contains(_stringToWordDict, tok.getData()))
        {
            dispatchWord(tok.getData());
        }
        else if (contains(_variableDictionary, tok.getData()))
        {
            _stack.emplace_back(StackElement(StackElement::ElementType::Variable,
                tok.getData()));
        }
        else if (is_number(tok.getData()))
        {
            // ohai, it's a number
            _stack.emplace_back(StackElement(StackElement::ElementType::Number,
                parse_number(tok.getData())));
        }
        else if (tok.getType() == Token::TokenType::StringLiteral)
        {
            // ohai, it's a string literal
            //
            // escaped quotations are not really supported yet...
            _stack.emplace_back(StackElement(StackElement::ElementType::String,
                tok.getData()));
        }
        else
        {
            stringstream strBuilder;
            strBuilder << "'" << tok.getData() << "' (type: " << tok.getType() << ") is not a defined word or valid data type";
            throw ThrofException("Interpreter", strBuilder.str().c_str(), _filename);
        }
    }

    void Interpreter::processDirective(Token& directive, Token& arg)
    {
        WORD_IDX directiveIdx = _stringToWordDict[directive.getData()];
        switch(directiveIdx)
        {
        case PRIM_INCLUDE:
            {
                FileReader reader(arg.getData());
                Tokenizer tokenizer = Tokenizer::tokenize(reader);
                loadFile(tokenizer);
            }
            break;
        case PRIM_VARIABLE:
            _variableDictionary[arg.getData()] = StackElement();
            break;
        default:
            stringstream strBuilder;
            strBuilder << "ERROR: '" << directive.getData() << "' is not a defined word or valid data type";
            throw ThrofException("Interpreter", strBuilder.str().c_str());
        }
    }

    // build the dictionary and script context
    void Interpreter::loadFile(Tokenizer& tokenizer)
    {
        _filename = tokenizer.filename();
        auto fetch_words_till_terminator = [](const string& wordName, Tokenizer& tokenizer)
        {
            vector<Token> tokens;
            Token tok = tokenizer.getNextToken();
            while (tok.getType() != Token::TokenType::DefinitionTerminator)
            {
                tokens.emplace_back(tok);
                if (tokenizer.hasNextToken())
                {
                    tok = tokenizer.getNextToken();
                }
                else
                {
                    stringstream errBuilder;
                    errBuilder << "word definition terminator (' ; ') expected at end of word '";
                    errBuilder << wordName << "'";
                    throw ThrofException("Interpreter", errBuilder.str(), tokenizer.filename());
                }
            }

            return tokens;
        };

        while (tokenizer.hasNextToken())
        {
            Token tok = tokenizer.getNextToken();
            switch (tok.getType())
            {
            case Token::TokenType::WordDefinition:
                addWordToDictionary(tok.getData(), fetch_words_till_terminator(tok.getData(), tokenizer));
                break;
            case Token::TokenType::WordOrData:
            case Token::TokenType::StringLiteral:
                processToken(tok);
                break;
            case Token::TokenType::Directive:
                {
                    Token directiveArg = tokenizer.getNextToken();
                    processDirective(tok, directiveArg);
                }
                break;
            case Token::TokenType::QuotationOpen:
                {
                    vector<Token> quotation;
                    while (tokenizer.hasNextToken())
                    {
                        tok = tokenizer.getNextToken();

                        if (tok.getType() == Token::TokenType::QuotationClose)
                        {
                            break;
                        }

                        quotation.emplace_back(Token(tok));
                    }

                    if (tok.getType() != Token::TokenType::QuotationClose)
                    {
                        throw ThrofException("Interpreter", "unexpected end of quotation without closing marker ']'", _filename);
                    }

                    _stack.emplace_back(StackElement(StackElement::ElementType::Quotation, quotation));
                }
                break;
            case Token::TokenType::QuotationClose:
            case Token::TokenType::DefinitionTerminator:
            default:
                stringstream strBuilder;
                strBuilder << "unexpected definition terminator (';'), quotation close (']') or token type (";
                strBuilder << tok.getType() << ")";
                throw ThrofException("Interpreter", strBuilder.str().c_str(), "");
            }
        }
    }

    static void prettyFormatQuotationStackElement(StackElement& elem, stringstream& strBuilder)
    {
        vector<Token> toks = elem.quotationData();
        for (size_t ii = 0; ii < toks.size(); ii++)
        {
            if (toks[ii].getType() == Token::TokenType::StringLiteral)
            {
                strBuilder << "\"" << toks[ii].getData() << "\" ";
            }
            else
            {
                strBuilder << toks[ii].getData() << " ";
            }
        }
    }

    string Interpreter::stackToString()
    {
        stringstream strBuilder;
        strBuilder << "Stack (size: " << _stack.size() << "): " << endl << endl;
        strBuilder << "\t  Top" << endl << "\t---------" << endl;

        for (auto itr = _stack.crbegin(); itr != _stack.crend(); itr++)
        {
            StackElement elem = *itr;
            
            switch (elem.type())
            {
            case StackElement::Number:
                strBuilder << "\t   " << elem.numberData() << endl;
                break;
            case StackElement::String:
                strBuilder << "\t   \"" << elem.stringData() << "\"" << endl;
                break;
            case StackElement::Quotation:
                strBuilder << "\t   ";
                prettyFormatQuotationStackElement(elem, strBuilder);
                strBuilder << endl;
                break;
            case StackElement::Boolean:
                strBuilder << "\t   " << (elem.booleanData()).toString() << endl;
                break;
            default:
                stringstream errBuilder;
                errBuilder << "unexpected stack element type (" << elem.type() << ")" << endl;
                throw ThrofException("Interpreter", errBuilder.str().c_str());
            }
        }

        strBuilder << endl;

        return strBuilder.str();
    }

    void Interpreter::dumpInterpreterState()
    {
        stringstream strBuilder;
        size_t numCompiledWords = _stringToWordDict.size() - STR_TO_PRIM_WORD_MAP.size();

        strBuilder << "Dictionary (compiled words: " << numCompiledWords << ") : " << endl << endl;
        for (auto itr = _stringToWordDict.cbegin(); itr != _stringToWordDict.cend(); itr++)
        {
            vector<Token>& toks = _dictionary[(*itr).second];
            strBuilder << "\t" << (*itr).first << " : ";

            for (auto jtr = toks.cbegin(); jtr != toks.cend(); jtr++)
            {
                if ((*jtr).getType() == Token::TokenType::StringLiteral)
                {
                    strBuilder << "\"" << (*jtr).getData() << "\" ";
                }
                else
                {
                    strBuilder << (*jtr).getData() << " ";
                }
            }

            strBuilder << endl;
        }

        strBuilder << endl << "Variables (size: " << _variableDictionary.size() << "):" << endl << endl;
        for (auto itr = _variableDictionary.cbegin(); itr != _variableDictionary.cend(); itr++) 
        {
            strBuilder << "\t" << (*itr).first << " : ";
            const StackElement& elem = (*itr).second;
            switch (elem.type())
            {
            case StackElement::Uninitialized:
                strBuilder << "uninitialized" << endl;
                break;
            case StackElement::Number:
                strBuilder << elem.numberData() << endl;
                break;
            case StackElement::String:
                strBuilder << elem.stringData() << endl;
                break;
            case StackElement::Boolean:
                strBuilder << "\t   " << (elem.booleanData()).toString() << endl;
                break;
            default:
                {
                    stringstream errBuilder;
                    errBuilder << "unexpected StackElement type : " << elem.type();
                    throw ThrofException("Interpreter", errBuilder.str(), _filename);
                }
            }
        }

        strBuilder << endl << stackToString();

        cout << strBuilder.str();
    }
}