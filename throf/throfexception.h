#pragma once

namespace throf
{

    class ThrofException : public std::exception
    {
    private:
        // private default ctor
        ThrofException() { }

        std::string _explanation;
        std::string _filename;
        std::string _component;

    public:
        ThrofException
        (
            const std::string& component,
            const std::string& explanation
        ) : _component(component), _explanation(explanation), _filename("")
        { }

        ThrofException
        (
            const std::string& component,
            const std::string& explanation,
            const std::string& filename
        ) : _component(component), _explanation(explanation), _filename(filename)
        { }

        virtual const char* what() const throw()
        {
            return _explanation.c_str();
        }

        const char* filename() const throw()
        {
            return _filename.c_str();
        }

        const char* component() const throw()
        {
            return _component.c_str();
        }

        static void throwIfTrue(bool expr, std::string strComponent, std::string strExplanation)
        {
            if (expr)
            {
                throw ThrofException(strComponent, strExplanation);
            }
        }
    };
}