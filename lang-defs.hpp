#pragma once

#include <functional>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <stack>
#include <set>
// #include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/multiprecision/gmp.hpp>

using namespace std::literals;

namespace lang
{
    using namespace boost::multiprecision;
    struct token;
    struct opcode;
    struct state;
    struct nil;
    struct node;
    struct anything;
    struct user_fn;

    using table_type = std::vector<std::pair<anything, anything>>;

    enum any_type
    {
        ANY_TYPE_INT = 0,
        ANY_TYPE_RAT = 1,
        ANY_TYPE_STR = 2,
        ANY_TYPE_LIST = 3,
        ANY_TYPE_TABLE = 4,
        ANY_TYPE_BOOL = 5,
        ANY_TYPE_FUNC = 6,
        ANY_TYPE_USER_FN = 7,
        ANY_TYPE_ERROR = 8,
        ANY_TYPE_NONE = 9,
        ANY_TYPE_DATA = 10,
    };

    template<typename T>
    T any_fast(anything);
    template<typename T>
    T any_fast_ptr(anything &);
    template<any_type Tc, typename T>
    anything make_any(T);
    using opcode_vec = std::vector<opcode>;
    using fn_ret = anything;
    using aty2 = std::vector<anything> &;
    using fn_type = std::function<fn_ret(state*, aty2)>;
    using tokens = std::vector<token>; // vector of token often often used
    mpq_rational strtorat(std::string);
    table_type generate();
    std::string walknode(node);
    anything get_table(table_type &, anything &);
    template<any_type Tc, typename T>
    anything get_table_type(table_type &, anything &);

    using var = anything;
    using integer = mpz_int;
    using rational = mpq_rational;
    using string = std::string;
    using list = std::vector<var>;
    using pair = std::pair<var, var>;
    using table = table_type;
    using func = fn_type;
    using userfn = user_fn;
    using boolean = bool;
}