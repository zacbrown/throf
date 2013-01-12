#pragma once

namespace throf
{
    class FileReader
    {
        std::vector<int> _buffer;
        size_t _index;
        std::string _filename;

        void slurpFile(FILE* fileHandle)
        {
            int c = fgetc(fileHandle);
            while (c != EOF)
            {
                _buffer.push_back(c);
                c = fgetc(fileHandle);
            }

            fclose(fileHandle); // best effort
        }

    public:
        FileReader(std::string filename) : _index(0), _filename(filename)
        {
            FILE* fileHandle = fopen(filename.c_str(), "r");
            if (nullptr == fileHandle)
            {
                stringstream strBuilder;
                strBuilder << "fopen failed, errno = " << errno;
                throw ThrofException("FileReader", strBuilder.str().c_str());
            }            
            slurpFile(fileHandle);
        }

        ~FileReader()
        {
        }

        bool getc(int& c)
        {
            if (_index < _buffer.size())
            {
                c = _buffer[_index++];
                return true;
            }
            return false;
        }

        bool ungetc()
        {
            if (_index > 0)
            {
                _index--;
                return true;
            }
            return false;
        }

        bool peek(int &c)
        {
            return getc(c) && ungetc();
        }

        const bool eof() const
        {
            return _index >= _buffer.size();
        }

        const size_t getFileIndex()
        {
            return _index;
        }

        const size_t getBufferSize() const
        {
            return _buffer.size();
        }

        const std::string& filename() const
        {
            return _filename;
        }
    };

    class Tokenizer;

    class Token
    {
    public:
        enum TokenType
        {
            Directive,              // "e.g. - :variable, :include"
            WordDefinition,         // ": "
            DefinitionTerminator,   // ";"
            WordOrData,
            StringLiteral,
            QuotationOpen,          // "["
            QuotationClose          // "]"
        };

        // comparison operators
        bool operator== (const Token& right) const
        {
            bool dataMatch = (0 == this->_data.compare(right._data));
            bool tokentypeMatch = (this->_type == right._type);
            return dataMatch && tokentypeMatch;
        }

        const TokenType getType() const
        {
            return _type;
        }

        const std::string getData() const
        {
            return _data;
        }

        // ctor/dtor
        Token(TokenType type, std::string data) : _type(type), _data(data) { }

        ~Token() { }

        Token(const Token& other)
        {
            this->_data = other._data;
            this->_type = other._type;
        }

    private:
        TokenType _type;
        std::string _data;

        friend class Tokenizer;
    };

    class Tokenizer
    {
    public:
        const Token getNextToken();
        const std::vector<Token> getQuotation();
        bool hasNextToken();
        void reset();
        const std::string& filename() const;

        static Tokenizer tokenize(FileReader& reader);

        ~Tokenizer();

    private:
        std::vector<Token> _tokens;
        size_t _tokensIndex;
        std::string _filename;

        explicit Tokenizer(const std::string& filename, const std::vector<Token>& tokens);
    };
}