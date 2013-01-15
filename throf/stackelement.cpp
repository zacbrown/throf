#include "stdafx.h"

namespace throf
{
    void StackElement::CopyObj(const StackElement& other)
    {
        this->_dataNumber = other._dataNumber;
        this->_dataString = other._dataString;
        this->_dataQuotation = other._dataQuotation;
        this->_dataBoolean = other._dataBoolean;
        this->_dataWordRefCurrentOffset = other._dataWordRefCurrentOffset;
        this->_dataWordRefId = other._dataWordRefId;
        this->_wordName = other._wordName;
        this->_type = other._type;
    }

    StackElement::StackElement() :
        _type(ElementType::Nil),
        _dataNumber(0xdeadbeef),
        _dataString(""),
        _dataWordRefCurrentOffset(0),
        _dataWordRefId(0xdeadbeef),
        _dataBoolean(false),
        _wordName("")
    { }

    StackElement::StackElement(const StackElement::ElementType type, long val) :
        _type(type),
        _dataNumber(val),
        _dataString(""),
        _dataBoolean(val != 0),
        _dataWordRefCurrentOffset(0),
        _dataWordRefId(0xdeadbeef),
        _wordName("")
    { }

    StackElement::StackElement(const StackElement::ElementType type, string val) :
        _type(type),
        _dataNumber(0xdeadbeef),
        _dataString(val),
        _dataBoolean(val != ""),
        _dataWordRefCurrentOffset(0),
        _dataWordRefId(0xdeadbeef),
        _wordName("")
    { }

    StackElement::StackElement(const StackElement::ElementType type, std::vector<StackElement> val) :
        _type(type),
        _dataNumber(0xdeadbeef),
        _dataString(""),
        _dataBoolean(val.size() != 0),
        _dataWordRefCurrentOffset(0),
        _dataWordRefId(0xdeadbeef),
        _dataQuotation(val),
        _wordName("")
    { }

    StackElement::StackElement(const StackElement::ElementType type, BooleanType val) :
        _type(type),
        _dataNumber(0xdeadbeef),
        _dataString(""),
        _dataBoolean(val), 
        _dataWordRefCurrentOffset(0),
        _dataWordRefId(0xdeadbeef),
        _wordName("")
    { }

    StackElement::StackElement(const ElementType type, const string wordName, WORD_ID wordIdx, size_t definitionIndex) :
        _type(type),
        _dataNumber(0xdeadbeef),
        _dataString(""),
        _dataBoolean(true),
        _dataWordRefCurrentOffset(definitionIndex),
        _dataWordRefId(wordIdx),
        _wordName(wordName)
    { }

    StackElement::StackElement(const StackElement& other)
    {
        CopyObj(other);
    }

    StackElement& StackElement::operator=(const StackElement& right)
    {
        CopyObj(right);
        return *this;
    }

    const string& StackElement::stringData() const
    {
        return _dataString;
    }

    const long& StackElement::numberData() const
    {
        return _dataNumber;
    }

    const vector<StackElement>& StackElement::quotationData() const
    {
        return _dataQuotation;
    }

    StackElement::BooleanType StackElement::booleanData() const
    {
        return _dataBoolean;
    }

    const size_t StackElement::wordRefCurrentOffset() const
    {
        if (_type == WordReference)
        {
            return _dataWordRefCurrentOffset;
        }

        stringstream strBuilder;
        strBuilder << "StackElement is not of type WordReference, type = " << _type << ".";
        throw ThrofException("StackElement", strBuilder.str());
    }

    const WORD_ID StackElement::wordRefId() const
    {
        if (_type == WordReference)
        {
            return _dataWordRefId;
        }

        stringstream strBuilder;
        strBuilder << "StackElement is not of type WordReference, type = " << _type << ".";
        throw ThrofException("StackElement", strBuilder.str());
    }

    const string& StackElement::wordName() const
    {
        if (_type == WordReference)
        {
            return _wordName;
        }

        stringstream strBuilder;
        strBuilder << "StackElement is not of type WordReference, type = " << _type << ".";
        throw ThrofException("StackElement", strBuilder.str());
    }

    const StackElement::ElementType StackElement::type() const
    {
        return _type;
    }
}