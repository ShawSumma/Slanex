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
// #include <boost/optional.hpp>
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

    using table_type = std::vector<std::shared_ptr<std::pair<anything, anything>>>;

    enum any_type
    {
        ANY_TYPE_INT,
        ANY_TYPE_RAT,
        ANY_TYPE_STR,
        ANY_TYPE_LIST,
        ANY_TYPE_TABLE,
        ANY_TYPE_BOOL,
        ANY_TYPE_FUNC,
        ANY_TYPE_ERROR,
        ANY_TYPE_NONE,
        ANY_TYPE_DATA,
    };

    template<typename T>
    T any_fast(anything &);
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
}