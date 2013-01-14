#pragma once

namespace throf
{
    class StackElement
    {
    public:
        enum ElementType
        {
            Uninitialized,
            Variable,
            String,
            Number,
            Boolean,
            WordReference,
            Quotation
        };

        struct BooleanType
        {
            bool value;
            operator bool() const { return value; }

            BooleanType() { }
            BooleanType(bool val) : value(val) { }
            BooleanType(const BooleanType& right) { this->value = right.value; }

            std::string toString() { return (value ? "true" : "false"); }
        };

    private:
        WORD_IDX _dataWordRefIdx;
        std::string _wordName;
        long _dataNumber;
        std::string _dataString;
        std::vector<StackElement> _dataQuotation;
        BooleanType _dataBoolean;
        ElementType _type;

        // helper func
        void CopyObj(const StackElement& other)
        {
            this->_dataNumber = other._dataNumber;
            this->_dataString = other._dataString;
            this->_dataQuotation = other._dataQuotation;
            this->_dataBoolean = other._dataBoolean;
            this->_dataWordRefIdx = other._dataWordRefIdx;
            this->_wordName = other._wordName;
            this->_type = other._type;
        }

    public:        
        const std::string& stringData() const
        {
            return _dataString;
        }

        const long& numberData() const
        {
            return _dataNumber;
        }

        const std::vector<StackElement>& quotationData() const
        {
            return _dataQuotation;
        }

        BooleanType booleanData() const
        {
            return _dataBoolean;
        }

        const WORD_IDX wordRefIdx() const
        {
            return _dataWordRefIdx;
        }

        const std::string& wordName() const
        {
            if (_type == WordReference)
            {
                return _wordName;
            }

            stringstream strBuilder;
            strBuilder << "StackElement is not of type WordReference, type = " << _type << ".";
            throw ThrofException("StackElement", strBuilder.str());
        }

        const ElementType type() const
        {
            return _type;
        }

        StackElement() :
            _type(ElementType::Uninitialized),
            _dataNumber(0xdeadbeef),
            _dataString(""),
            _dataWordRefIdx(0xdeadbeef),
            _dataBoolean(false),
            _wordName("")
        { }

        explicit StackElement(const ElementType type, long val) :
            _type(type),
            _dataNumber(val),
            _dataString(""),
            _dataBoolean(val != 0),
            _dataWordRefIdx(0xdeadbeef),
            _wordName("")
        { }

        explicit StackElement(const ElementType type, std::string val) :
            _type(type),
            _dataNumber(0xdeadbeef),
            _dataString(val),
            _dataBoolean(val != ""),
            _dataWordRefIdx(0xdeadbeef),
            _wordName("")
        { }

        explicit StackElement(const ElementType type, std::vector<StackElement> val) :
            _type(type),
            _dataNumber(0xdeadbeef),
            _dataString(""),
            _dataBoolean(val.size() != 0),
            _dataWordRefIdx(0xdeadbeef),
            _dataQuotation(val),
            _wordName("")
        { }

        explicit StackElement(const ElementType type, BooleanType val) :
            _type(type),
            _dataNumber(0xdeadbeef),
            _dataString(""),
            _dataBoolean(val), 
            _dataWordRefIdx(0xdeadbeef),
            _wordName("")
        { }

        explicit StackElement(const ElementType type, WORD_IDX val, const std::string wordName) :
            _type(type),
            _dataNumber(0xdeadbeef),
            _dataString(""),
            _dataBoolean(true),
            _dataWordRefIdx(val),
            _wordName(wordName)
        { }

        StackElement(const StackElement& other)
        {
            CopyObj(other);
        }

        StackElement& operator=(const StackElement& right)
        {
            CopyObj(right);
            return *this;
        }
    };
}