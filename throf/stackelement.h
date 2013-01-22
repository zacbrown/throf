#pragma once

namespace throf
{
    class StackElement
    {
    public:
        enum ElementType
        {
            Nil,
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
        int _dataWordRefCurrentOffset;
        WORD_ID _dataWordRefId;
        std::string _wordName;
        long _dataNumber;
        std::string _dataString;
        std::vector<StackElement> _dataQuotation;
        BooleanType _dataBoolean;
        ElementType _type;

        // helper func
        void CopyObj(const StackElement& other);

    public:
        const std::string& stringData() const;

        const long& numberData() const;

        const std::vector<StackElement>& quotationData() const;

        BooleanType booleanData() const;

        const int wordRefCurrentOffset() const;

        const WORD_ID wordRefId() const;

        const std::string& wordName() const;

        const ElementType type() const;

        StackElement();

        explicit StackElement(const ElementType type, long val);

        explicit StackElement(const ElementType type, std::string val);

        explicit StackElement(const ElementType type, std::vector<StackElement> val);

        explicit StackElement(const ElementType type, BooleanType val);

        explicit StackElement(const ElementType type, const std::string wordName, WORD_ID wordIdx, int definitionIndex);

        StackElement(const StackElement& other);

        StackElement& operator=(const StackElement& right);
    };
}