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
            _dictionary[prim] = vector<StackElement>();
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

    void Interpreter::dispatch(const StackElement& elem)
    {
        string& filename = _filename;
        switch (elem.type())
        {
        case StackElement::Boolean:
        case StackElement::Number:
        case StackElement::String:
        case StackElement::Quotation:
        case StackElement::Variable:
            _stack.emplace_back(elem);
            return;
        case StackElement::WordReference:
            break;
        default:
            throw ThrofException("Interpreter", "Unexpected uninitialized StackElement.", _filename);
        }

        WORD_IDX idx = elem.wordRefIdx();

        auto dispatch_arithmetic = [filename](WORD_IDX operation, const StackElement& top, const StackElement& bottom)
        {
            switch (operation)
            {
            case PRIM_ADD:
                return bottom.numberData() + top.numberData();
            case PRIM_SUB:
                return bottom.numberData() - top.numberData();
            case PRIM_MUL:
                return bottom.numberData() * top.numberData();
            case PRIM_DIV:
                return bottom.numberData() / top.numberData();
            case PRIM_MOD:
                return bottom.numberData() % top.numberData();
            }

            stringstream strBuilder;
            strBuilder << "unexpected word passed as arithmetic word : '" << operation << "'";
            throw ThrofException("Interpreter", strBuilder.str(), filename);
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
        case PRIM_CLS:
            _stack.clear();
            break;
        case PRIM_STACK:
            printf("%s", stackToString().c_str());
            break;
        case PRIM_IF:
            {
                StackElement falseQuotation = _stack.back(); _stack.pop_back();
                StackElement trueQuotation = _stack.back(); _stack.pop_back();
                StackElement boolOutcome = _stack.back(); _stack.pop_back();

                throwIfTypeUnexpected(falseQuotation, StackElement::Quotation, "Expected quotation as 3rd stack argument to 'if' word : ");
                throwIfTypeUnexpected(trueQuotation, StackElement::Quotation, "Expected quotation as 2nd stack argument to 'if' word : ");

                const vector<StackElement>& q = boolOutcome.booleanData() ? trueQuotation.quotationData() : falseQuotation.quotationData();
                for (auto elem : q)
                {
                    dispatch(elem);
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
                auto itr = _stack.end();
                itr -= 3;
                StackElement elem = *itr;
                _stack.erase(itr);
                _stack.emplace_back(elem);
            }
            break;
        case PRIM_NROT:
            {
                StackElement elem = _stack.back(); _stack.pop_back();
                auto itr = _stack.end();
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

                auto itr = _stack.end(); itr--;
                int idx = 0;
                while (idx < elemIndex.numberData() && itr != _stack.begin())
                {
                    itr--;
                    idx++;
                }
                StackElement elem = *itr;
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
            // Non-core word used
            auto def = _dictionary[idx];
            for (auto itr = def.begin(); itr != def.end(); itr++)
            {
                StackElement& elem = *itr;
                switch(elem.type())
                {
                case StackElement::Boolean:
                case StackElement::Number:
                case StackElement::String:
                case StackElement::Quotation:
                    _stack.emplace_back(StackElement(elem));
                    break;
                case StackElement::WordReference:
                    dispatch(elem);
                    break;
                }
            }
            break;
        }
    }

    StackElement Interpreter::createStackElementFromToken(Tokenizer& tokenizer, const Token& tok)
    {
        bool isTrueToken = (0 == tok.getData().compare("true"));
        if (isTrueToken || 0 == tok.getData().compare("false"))
        {
            return StackElement(StackElement::Boolean,
                StackElement::BooleanType(isTrueToken));
        }
        else if (is_number(tok.getData()))
        {
            // ohai, it's a number
            return StackElement(StackElement::Number,
                parse_number(tok.getData()));
        }
        else if (tok.getType() == Token::TokenType::StringLiteral)
        {
            // ohai, it's a string literal
            //
            // escaped quotations are not really supported yet...
            return StackElement(StackElement::String,
                tok.getData());
        }
        else if (tok.getType() == Token::TokenType::QuotationOpen)
        {
            vector<StackElement> quotation;
            while (tokenizer.hasNextToken())
            {
                Token nextTok = tokenizer.getNextToken();

                if (nextTok.getType() == Token::TokenType::QuotationClose)
                {
                    break;
                }

                quotation.emplace_back(createStackElementFromToken(tokenizer, nextTok));
            }

            return StackElement(StackElement::Quotation, quotation);
        }
        else if (contains(_stringToWordDict, tok.getData()))
        {
            WORD_IDX idx = _stringToWordDict[tok.getData()];
            return StackElement(StackElement::WordReference, idx, tok.getData());
        }
        else if (contains(_variableDictionary, tok.getData()))
        {
            return StackElement(StackElement::ElementType::Variable, tok.getData());
        }
        else
        {
            stringstream strBuilder;
            strBuilder << "'" << tok.getData() << "' (type: " << tok.getType() << ") is not a defined word or valid data type";
            throw ThrofException("Interpreter", strBuilder.str().c_str(), _filename);
        }
    }

    void Interpreter::addWordToDictionary(Tokenizer& tokenizer, const string& s)
    {
        WORD_IDX idx = _stringToWordDict[s] = _stringToWordDict.size() + 1;
        vector<StackElement> ret;

        Token tok = tokenizer.getNextToken();
        while (tok.getType() != Token::DefinitionTerminator)
        {
            ret.emplace_back(createStackElementFromToken(tokenizer, tok));

            if (tokenizer.hasNextToken())
            {
                tok = tokenizer.getNextToken();
            }
            else
            {
                stringstream errBuilder;
                errBuilder << "word definition terminator (' ; ') expected at end of word '";
                errBuilder << s << "'";
                throw ThrofException("Interpreter", errBuilder.str(), tokenizer.filename());
            }
        }

        _dictionary[idx] = ret;
    }

    void Interpreter::processToken(Tokenizer& tokenizer, const Token& tok)
    {
        StackElement elem = createStackElementFromToken(tokenizer, tok);
        switch (elem.type())
        {
        case StackElement::Boolean:
        case StackElement::Number:
        case StackElement::String:
        case StackElement::Variable:
        case StackElement::Quotation:
            _stack.emplace_back(elem);
            break;
        case StackElement::WordReference:
            dispatch(elem);
            break;
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

        while (tokenizer.hasNextToken())
        {
            Token tok = tokenizer.getNextToken();
            switch (tok.getType())
            {
            case Token::TokenType::WordDefinition:
                addWordToDictionary(tokenizer, tok.getData());
                break;
            case Token::TokenType::WordOrData:
            case Token::TokenType::StringLiteral:
                processToken(tokenizer, tok);
                break;
            case Token::TokenType::Directive:
                {
                    Token directiveArg = tokenizer.getNextToken();
                    processDirective(tok, directiveArg);
                }
                break;
            case Token::TokenType::QuotationOpen:
                {
                    vector<StackElement> quotation;
                    while (tokenizer.hasNextToken())
                    {
                        tok = tokenizer.getNextToken();

                        if (tok.getType() == Token::TokenType::QuotationClose)
                        {
                            break;
                        }

                        quotation.emplace_back(createStackElementFromToken(tokenizer, tok));
                    }

                    if (tok.getType() != Token::TokenType::QuotationClose)
                    {
                        throw ThrofException("Interpreter", "unexpected end of quotation without closing marker ']'", _filename);
                    }

                    _stack.emplace_back(StackElement(StackElement::Quotation, quotation));
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

    void Interpreter::prettyFormatStackElement(const StackElement& elem, stringstream& strBuilder)
    {
        switch (elem.type())
        {
        case StackElement::Variable:
            strBuilder << elem.stringData() << " ";
            break;
        case StackElement::String:
            strBuilder << "\"" << elem.stringData() << "\" ";
            break;
        case StackElement::Number:
            strBuilder << elem.numberData() << " ";
            break;
        case StackElement::Boolean:
            strBuilder << elem.booleanData() << " ";
            break;
        case StackElement::Quotation:
            prettyFormatQuotation(elem, strBuilder);
            break;
        case StackElement::WordReference:
            {
                vector<StackElement>& wordDef = _dictionary[elem.wordRefIdx()];
                for (auto itr = wordDef.begin(); itr != wordDef.end(); itr++)
                {
                    prettyFormatStackElement(*itr, strBuilder);
                }
            }
            break;
        default:
            stringstream errBuilder;
            errBuilder << "unexpected stack element type (" << elem.type() << ")" << endl;
            throw ThrofException("Interpreter", errBuilder.str().c_str());
            break;
        }
    }

    void Interpreter::prettyFormatQuotation(const StackElement& elem, stringstream& strBuilder)
    {
        vector<StackElement> elements = elem.quotationData();
        for (size_t ii = 0; ii < elements.size(); ii++)
        {
            prettyFormatStackElement(elements[ii], strBuilder);
        }
    }

    string Interpreter::stackToString()
    {
        stringstream strBuilder;
        strBuilder << "Stack (size: " << _stack.size() << "): " << endl << endl;
        strBuilder << "\t  Top" << endl << "\t---------" << endl;

        for (auto itr = _stack.rbegin(); itr != _stack.rend(); itr++)
        {
            StackElement elem = (*itr);
            strBuilder << "\t   ";
            prettyFormatStackElement(elem, strBuilder);
            strBuilder << endl;
        }

        strBuilder << endl;

        return strBuilder.str();
    }

    void Interpreter::dumpInterpreterState()
    {
        stringstream strBuilder;
        size_t numCompiledWords = _stringToWordDict.size() - STR_TO_PRIM_WORD_MAP.size();

        strBuilder << "Dictionary (compiled words: " << numCompiledWords << ") : " << endl << endl;
        for (auto itr = _stringToWordDict.begin(); itr != _stringToWordDict.end(); itr++)
        {
            vector<StackElement>& stackElems = _dictionary[(*itr).second];
            strBuilder << "\t" << (*itr).first << " : ";

            for (auto jtr = stackElems.cbegin(); jtr != stackElems.cend(); jtr++)
            {
                const StackElement& elem = *jtr;
                prettyFormatStackElement(elem, strBuilder);
            }

            strBuilder << endl;
        }

        strBuilder << endl << "Variables (size: " << _variableDictionary.size() << "):" << endl << endl;
        for (auto itr = _variableDictionary.begin(); itr != _variableDictionary.end(); itr++) 
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