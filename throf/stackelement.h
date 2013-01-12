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
        long _dataNumber;
        std::string _dataString;
        std::vector<Token> _dataQuotation;
        BooleanType _dataBoolean;
        ElementType _type;

        // helper func
        void CopyObj(const StackElement& other)
        {
            this->_dataNumber = other._dataNumber;
            this->_dataString = other._dataString;
            this->_dataQuotation = other._dataQuotation;
            this->_dataBoolean = other._dataBoolean;
            this->_type = other._type ;
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

        const std::vector<Token>& quotationData() const
        {
            return _dataQuotation;
        }

        BooleanType booleanData() const
        {
            return _dataBoolean;
        }

        const ElementType type() const
        {
            return _type;
        }

        StackElement() :
            _type(ElementType::Uninitialized), _dataNumber(0xdeadbeef), _dataString("")
        { }

        StackElement(ElementType type, long val) :
            _type(type), _dataNumber(val), _dataString("")
        { }

        StackElement(ElementType type, std::string val) :
            _type(type), _dataNumber(0xdeadbeef), _dataString(val)
        { }

        StackElement(ElementType type, std::vector<Token> val) :
            _type(type), _dataNumber(0xdeadbeef), _dataString(""), _dataQuotation(val)
        { }

        StackElement(ElementType type, BooleanType val) :
            _type(type), _dataNumber(0xdeadbeef), _dataString(""), _dataBoolean(val)
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