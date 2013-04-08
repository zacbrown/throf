#pragma once

#include <set>
#include <string>
#include <map>

namespace throf
{
    using namespace std;

    typedef int WORD_ID;
    typedef int PRIMITIVE_WORD;

#define op_code(e, val, str) \
    const PRIMITIVE_WORD PRIM_ ## e = val; \
    const char* const PRIM_ ## e ## _STR = str \

    op_code(WORDS, -3, "words");
    op_code(CLS, -2, "cls");
    op_code(STACK, -1, "stack");
    op_code(IF, 0, "if");
    op_code(DROP, 1, "drop");
    op_code(SWAP, 2, "swap");
    op_code(TWOSWAP, 3, "2swap");
    op_code(INCLUDE, 4, ":include");
    op_code(VARIABLE, 5, ":variable");
    op_code(SET, 6, "!");
    op_code(GET, 7, "@");
    op_code(ROT, 8, "rot");
    op_code(NROT, 9, "-rot");
    op_code(PICK, 10, "pick");
    op_code(ADD, 11, "+");
    op_code(SUB, 12, "-");
    op_code(MUL, 13, "*");
    op_code(DIV, 14, "/");
    op_code(MOD, 15, "mod");
    op_code(LT, 16, "<");
    op_code(GT, 17, ">");
    op_code(LTE, 18, "<=");
    op_code(GTE, 19, ">=");
    op_code(EQ, 20, "==");
    op_code(NEQ, 21, "<>");
    op_code(NOT, 22, "not");
    op_code(AND, 23, "and");
    op_code(OR, 24, "or");
    op_code(XOR, 25, "xor");
    op_code(DEFER, 26, ":defer");


#undef op_code
 
    static unordered_map<string, PRIMITIVE_WORD> createStrToPrimMap()
    {
        unordered_map<string, PRIMITIVE_WORD> ret;
        ret[PRIM_WORDS_STR]     = PRIM_WORDS    ;
        ret[PRIM_CLS_STR]       = PRIM_CLS      ;
        ret[PRIM_STACK_STR]     = PRIM_STACK    ;
        ret[PRIM_IF_STR]        = PRIM_IF       ;
        ret[PRIM_DROP_STR]      = PRIM_DROP     ;
        ret[PRIM_SWAP_STR]      = PRIM_SWAP     ;
        ret[PRIM_TWOSWAP_STR]   = PRIM_TWOSWAP  ;
        ret[PRIM_INCLUDE_STR]   = PRIM_INCLUDE  ;
        ret[PRIM_VARIABLE_STR]  = PRIM_VARIABLE ;
        ret[PRIM_SET_STR]       = PRIM_SET      ;
        ret[PRIM_GET_STR]       = PRIM_GET      ;
        ret[PRIM_ROT_STR]       = PRIM_ROT      ;
        ret[PRIM_NROT_STR]      = PRIM_NROT     ;
        ret[PRIM_PICK_STR]      = PRIM_PICK     ;
        ret[PRIM_ADD_STR]       = PRIM_ADD      ;
        ret[PRIM_SUB_STR]       = PRIM_SUB      ;
        ret[PRIM_MUL_STR]       = PRIM_MUL      ;
        ret[PRIM_DIV_STR]       = PRIM_DIV      ;
        ret[PRIM_MOD_STR]       = PRIM_MOD      ;
        ret[PRIM_LT_STR]        = PRIM_LT       ;
        ret[PRIM_GT_STR]        = PRIM_GT       ;
        ret[PRIM_LTE_STR]       = PRIM_LTE      ;
        ret[PRIM_GTE_STR]       = PRIM_GTE      ;
        ret[PRIM_EQ_STR]        = PRIM_EQ       ;
        ret[PRIM_NEQ_STR]       = PRIM_NEQ      ;
        ret[PRIM_NOT_STR]       = PRIM_NOT      ;
        ret[PRIM_AND_STR]       = PRIM_AND      ;
        ret[PRIM_OR_STR]        = PRIM_OR       ;
        ret[PRIM_XOR_STR]       = PRIM_XOR      ;
        ret[PRIM_DEFER_STR]     = PRIM_DEFER    ;

        return ret;
    }

    static unordered_map<PRIMITIVE_WORD, string> createPrimToStrMap()
    {
        unordered_map<PRIMITIVE_WORD, string> ret;
        ret[PRIM_WORDS]     = PRIM_WORDS_STR    ;
        ret[PRIM_CLS]       = PRIM_CLS_STR      ;
        ret[PRIM_STACK]     = PRIM_STACK_STR    ;
        ret[PRIM_IF]        = PRIM_IF_STR       ;
        ret[PRIM_DROP]      = PRIM_DROP_STR     ;
        ret[PRIM_SWAP]      = PRIM_SWAP_STR     ;
        ret[PRIM_TWOSWAP]   = PRIM_TWOSWAP_STR  ;
        ret[PRIM_INCLUDE]   = PRIM_INCLUDE_STR  ;
        ret[PRIM_VARIABLE]  = PRIM_VARIABLE_STR ;
        ret[PRIM_SET]       = PRIM_SET_STR      ;
        ret[PRIM_GET]       = PRIM_GET_STR      ;
        ret[PRIM_ROT]       = PRIM_ROT_STR      ;
        ret[PRIM_NROT]      = PRIM_NROT_STR     ;
        ret[PRIM_PICK]      = PRIM_PICK_STR     ;
        ret[PRIM_ADD]       = PRIM_ADD_STR      ;
        ret[PRIM_SUB]       = PRIM_SUB_STR      ;
        ret[PRIM_MUL]       = PRIM_MUL_STR      ;
        ret[PRIM_DIV]       = PRIM_DIV_STR      ;
        ret[PRIM_MOD]       = PRIM_MOD_STR      ;
        ret[PRIM_LT]        = PRIM_LT_STR       ;
        ret[PRIM_GT]        = PRIM_GT_STR       ;
        ret[PRIM_LTE]       = PRIM_LTE_STR      ;
        ret[PRIM_GTE]       = PRIM_GTE_STR      ;
        ret[PRIM_EQ]        = PRIM_EQ_STR       ;
        ret[PRIM_NEQ]       = PRIM_NEQ_STR      ;
        ret[PRIM_NOT]       = PRIM_NOT_STR      ;
        ret[PRIM_AND]       = PRIM_AND_STR      ;
        ret[PRIM_OR]        = PRIM_OR_STR       ;
        ret[PRIM_XOR]       = PRIM_XOR_STR      ;
        ret[PRIM_DEFER]     = PRIM_DEFER_STR    ;
        return ret;
    }

    static const unordered_map<string, PRIMITIVE_WORD> STR_TO_PRIM_WORD_MAP = createStrToPrimMap();
    static const unordered_map<PRIMITIVE_WORD, string> PRIM_WORD_TO_STR_MAP = createPrimToStrMap();

    
    template <typename TMap, typename T> bool contains(const TMap& map, const T& val)
    { 
        return map.find(val) != map.end();
    }
}