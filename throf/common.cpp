#include "stdafx.h"

namespace throf
{   
    CommonGlobalStaticInitializer globalInit;

    bool is_number(const std::string& s)
    {   
#ifdef _WIN32
        static std::regex is_number_regex("-?\\d+");
        return !s.empty() && std::regex_match(s, is_number_regex);
#else
        // GCC 4.7 (latest at time of writing - January 2013) is sorely
        // broken with an incomplete and/or buggy implementation of
        // std::regex. For whatever reason, it throws exceptions with 
        // simple regular expressions. Instead, we make use of the
        // POSIX C regex library.
        return !s.empty() && regexec(&globalInit.is_number_regex, s.c_str(), 0, NULL, 0) == 0;
#endif
    }

    int parse_number(const std::string& s)
    {
        std::stringstream strBuilder;
        strBuilder << s;
        int ret;
        strBuilder >> ret;
        return ret;
    }
}