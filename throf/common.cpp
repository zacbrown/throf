#include "stdafx.h"

namespace throf
{
    bool is_number(const std::string& s)
    {
        static std::regex is_number_regex("-?[\\d]+");
        return !s.empty() && std::regex_match(s, is_number_regex);
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