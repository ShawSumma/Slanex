#pragma once
#include "lang-defs.hpp"
namespace lang
{
    namespace lib
    {
        bool anything_compare(anything, anything);
        bool table_compare(table_type, table_type);
        bool list_compare(std::vector<anything>, std::vector<anything>);
        fn_ret fn_print(state *, aty2);
        fn_ret fn_add(state *, aty2);
        fn_ret fn_mult(state *, aty2);
        fn_ret fn_do(state *, aty2);
        fn_ret fn_to_str(state *, aty2);
        fn_ret fn_type_of(state *, aty2);
        fn_ret fn_def_str(state *, aty2);
        fn_ret fn_list(state *, aty2);
        fn_ret fn_to_str(state *, aty2);
        table_type generate();
    }
}