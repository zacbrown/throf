#include "stdafx.h"
#include "interpreter.h"

namespace throf
{
    using namespace std;

    Tokenizer::Tokenizer(const string& filename, const vector<Token>& tokens) : 
        _tokens(tokens), _tokensIndex(0), _filename(filename)
    {
    }

    Tokenizer::~Tokenizer()
    {
    }

    const string& Tokenizer::filename() const
    {
        return _filename;
    }

    // static
    Tokenizer Tokenizer::tokenize(FileReader& reader)
    {
        vector<Token> tokens;

        auto fetch_next_token = [](FileReader& reader)
        {
            stringstream strBuilder;
            int c = -1;
            
            while (reader.peek(c) && !std::isspace(c))
            {
                if (!reader.getc(c))
                {
                    break;
                }
                strBuilder << static_cast<char>(c);
            }
            return strBuilder.str();
        };

        auto fetch_definition = [fetch_next_token](FileReader& reader)
        {
            int colon = -1;

            if (!reader.getc(colon) || colon != ':')
            {
                throw ThrofException("Tokenizer", "expected colon (':') at beginning of word definition.", reader.filename());
            }

            // strip whitespace between ':' and word definition
            // example: ':    \t herp'
            int c = -1;

            while (reader.peek(c) && std::isspace(c)) 
            {
                // make sure at least a whitespace char exists between the ':' and word name:
                // example: ': foo', not ':foo'
                if (!reader.getc(c))
                {
                    throw ThrofException("Tokenizer", "unexpected end of buffer", reader.filename());
                }
            }

            return fetch_next_token(reader);
        };

        auto get_token_type = [](string tok)
        {
            return (0 == tok.compare(";")) ? Token::TokenType::DefinitionTerminator : Token::TokenType::WordOrData;
        };

        auto check_if_marker = [](FileReader& reader, int checkChar)
        {
            auto throw_if_getc_failed = [reader, checkChar]()
            {
                stringstream strBuilder;
                strBuilder << "unexpected end of stream while parsing '" << static_cast<char>(checkChar) << "'";
                throw ThrofException("Tokenizer", strBuilder.str(),
                    reader.filename());
            };

            int before = -1, marker = -1, after = -1;

            if (!reader.ungetc())
            {
                // beginning of file, no character to fetch "before" the marker
                if (!reader.getc(marker) || !reader.getc(after))
                {
                    throw_if_getc_failed();
                }

                reader.ungetc();
                return marker == checkChar && std::isspace(after);
            }

            if (!reader.getc(before) || !reader.getc(marker))
            {
                throw_if_getc_failed();
            }
            reader.getc(after);
            reader.ungetc();

            return marker == checkChar && std::isspace(before) && (after == -1 || std::isspace(after));
        };

        auto check_if_definition_marker = [](FileReader& reader)
        {
            int marker = -1, space = -1;
            if (!reader.getc(marker) || !reader.getc(space))
            {
                throw ThrofException("Tokenizer", "unexpected end of stream while parsing word definition",
                    reader.filename());
            }
            reader.ungetc();
            reader.ungetc();
            
            return std::isspace(space) && marker == ':';
        };

        auto check_if_directive_marker = [fetch_next_token](FileReader& reader)
        {
            int marker = -1; int alphanumeric = -1;

            if (!reader.getc(marker) || !reader.getc(alphanumeric))
            {
                throw ThrofException("Tokenizer", "unexpected end of stream while parsing directive",
                    reader.filename());
            }
            reader.ungetc();
            reader.ungetc();
            return std::isalnum(alphanumeric) && marker == ':';
        };

        auto check_if_string_literal_end = [](FileReader& reader)
        {
            int quote = -1, space = -1;
            if (reader.getc(quote))
            {
                if (reader.peek(space))
                {
                    reader.ungetc();
                    return std::isspace(space) && quote == '"';
                }
                else if (-1 == space)
                {
                    // This is the case where the string literal is the last token in the file.
                    // I.e. there are no more characters following the string literal as it's loaded.
                    reader.ungetc();
                    return quote == '"';
                }
            }

            return false;
        };

        bool isSkipLine = false;
        bool isStringLiteral = false;
        stringstream stringLitBuilder;
        while (!reader.eof())
        {
            int c = -1;
            reader.peek(c);
            string strToken;

            if (isSkipLine)
            {
                if (c == '\n')
                {
                    isSkipLine = false;
                }
                reader.getc(c);
                continue;
            }

            if (isStringLiteral)
            {
                if (check_if_string_literal_end(reader))
                {
                    reader.getc(c); // pull the '"' out
                    isStringLiteral = false;
                    tokens.push_back(Token(Token::TokenType::StringLiteral, stringLitBuilder.str()));
                    stringLitBuilder.str("");
                    stringLitBuilder.clear();
                    continue;
                }
                reader.getc(c);
                stringLitBuilder << static_cast<char>(c);
                continue;
            }

            // # Denotes skip parsing the remainder of this line
            if (c == '#')
            {
                isSkipLine = true;
                continue;
            }
            else if (c == '"')
            {
                isStringLiteral = true;
                reader.getc(c); // strip the quote
                continue;
            }
            // ( ... ) denotes skip the chunk between the parantheses.
            // There must be a space on either side of the '(' and ')' to parse.
            else if (c == '(' && check_if_marker(reader, '('))
            {
                while (c != ')' && !check_if_marker(reader, ')'))
                {
                    reader.getc(c);
                }
            }
            // Process the whitespace
            else if (std::isspace(c))
            {
                reader.getc(c);
                continue;
            }
            // Word definition
            else if (c == ':' && check_if_definition_marker(reader))
            {                
                strToken = fetch_definition(reader);
                tokens.push_back(Token(Token::TokenType::WordDefinition, strToken));
                continue;
            }
            else if (c == ':' && check_if_directive_marker(reader))
            {
                strToken = fetch_next_token(reader);
                tokens.push_back(Token(Token::TokenType::Directive, strToken));
                continue;
            }
            else if (c == '[' && check_if_marker(reader, '['))
            {
                reader.getc(c);
                tokens.push_back(Token(Token::TokenType::QuotationOpen, "["));
                continue;
            }
            else if (c == ']' && check_if_marker(reader, ']'))
            {
                reader.getc(c);
                tokens.push_back(Token(Token::TokenType::QuotationClose, "]"));
                continue;
            }
            // A primitive word, system word, user-defined word, or data.
            else if (std::isalnum(c) || std::ispunct(c))
            {
                strToken = fetch_next_token(reader);
                tokens.push_back(Token(get_token_type(strToken), strToken));
                continue;
            }
        }

        if (isStringLiteral)
        {
            // we never closed our string literal
            // probably should wire in line numbers so we can report an appropriate one.
            throw ThrofException("Tokenizer", "string literal was not closed", reader.filename());
        }

        return Tokenizer(reader.filename(), tokens);
    }

    const Token Tokenizer::getNextToken()
    {
        return _tokens[_tokensIndex++];
    }

    const vector<Token> Tokenizer::getQuotation()
    {
        if (_tokens[_tokensIndex].getType() == Token::TokenType::QuotationOpen)
        {
            vector<Token> ret;

            while (_tokens[_tokensIndex].getType() != Token::TokenType::QuotationClose)
            {
                ret.emplace_back(_tokens[_tokensIndex++]);
            }
            ret.emplace_back(_tokens[_tokensIndex]); // add the QuotationClose

            return ret;
        }
        
        stringstream strBuilder;
        strBuilder << "cannot fetch quotation at this point in tokenization, current token: '";
        strBuilder << _tokens[_tokensIndex].getData() << "'";
        throw ThrofException("Tokenizer", strBuilder.str(), _filename);
    }

    bool Tokenizer::hasNextToken()
    {        
        return _tokensIndex < _tokens.size() ? true : false;
    }

    void Tokenizer::reset()
    {
        _tokensIndex = 0;
    }
}