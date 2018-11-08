#pragma once
#include "lang.hpp"
#include "lang-lib-defs.hpp"
namespace lang
{
    namespace lib
    {

        bool anything_compare(anything left, anything right)
        {
            if (left.type != right.type)
            {
                return false;
            }
            if (is_a_any<ANY_TYPE_INT>(left))
            {
                return any_fast<mpz_int>(left) == any_fast<mpz_int>(right);
            }
            if (is_a_any<ANY_TYPE_RAT>(left))
            {
                return any_fast<mpq_rational>(left) == any_fast<mpq_rational>(right);
            }
            if (is_a_any<ANY_TYPE_STR>(left))
            {
                return any_fast<std::string>(left) == any_fast<std::string>(right);
            }
            if (is_a_any<ANY_TYPE_BOOL>(left))
            {
                return any_fast<bool>(left) == any_fast<bool>(right);
            }
            if (is_a_any<ANY_TYPE_BOOL>(left))
            {
                return true;
            }
            if (is_a_any<ANY_TYPE_TABLE>(left))
            {
                return table_compare(any_fast<table_type>(left), any_fast<table_type>(right));
            }
            if (is_a_any<ANY_TYPE_LIST>(left))
            {
                return list_compare(any_fast<std::vector<anything>>(left), any_fast<std::vector<anything>>(right));
            }
            return false;
        }

        bool list_compare(std::vector<anything> lhs, std::vector<anything> rhs)
        {
            uint64_t size = lhs.size();
            if (size != rhs.size())
            {
                return false;
            }
            for (uint64_t i = 0; i < size; i++)
            {
                anything left = lhs[i];
                anything right = rhs[i];
                if (!anything_compare(left, right))
                {
                    return false;
                }
            }
            return true;
        }

        bool table_compare(table_type lhs, table_type rhs)
        {
            uint64_t size = lhs.size();
            if (size != rhs.size())
            {
                return false;
            }
            for (uint64_t i = 0; i < size; i++)
            {
                std::pair<anything, anything> left = *lhs[i];
                std::pair<anything, anything> right = *rhs[i];
                if (anything_compare(left.first, right.first) || anything_compare(left.second, right.second))
                {
                    return false;
                }
            }
            return true;
        }

        fn_ret fn_print(state *st, aty2 args)
        // write to stdout
        {
            anything strin = fn_to_str(st, args);
            // converts all argumets to a string
            if (!is_a_any<ANY_TYPE_STR>(strin))
            // if it was an error
            {
                // std::cout << "io error" << std::endl;
                return strin;
                // it should be a string
            }
            std::cout << any_fast<std::string>(strin);
            // else write it
            std::cout << std::endl;
            return make_any<ANY_TYPE_NONE, none>(none());
            // print return none
        }

        fn_ret fn_sub(state *st, aty2 args)
        // subtract works on ints and rats
        {
            uint64_t size = args.size();
            if (args.size() == 0)
            // sub should take alteast one argument
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("sub", 0)
                );
                // return error
            }
            if (is_a_any<ANY_TYPE_INT>(args[0]) || is_a_any<ANY_TYPE_RAT>(args[0]))
            // it should be only a numberic type
            {
                mpz_int whole;
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_INT>(args[0]))
                // initialize with int
                {
                    whole = any_fast<mpz_int>(args[0]); 
                    rat = 0;
                }
                else
                // initialize with rational
                {
                    rat = any_fast<mpq_rational>(args[0]);
                    whole = 0;
                }
                for (uint64_t pl = 1; pl < size; pl++)
                // subtract along the rest of the arguments
                {
                    if (is_a_any<ANY_TYPE_INT>(args[pl]))
                    // ints change the whole
                    {
                        whole -=  any_fast<mpz_int>(args[pl]);
                    }
                    else if (is_a_any<ANY_TYPE_RAT>(args[pl]))
                    // rationals change the rat
                    {
                        rat -=  any_fast<mpq_rational>(args[pl]);
                    }
                    else
                    // its not a number 
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("sub", {"number", "rational"})
                        ); // it is an error
                    }
                }
                if (rat == 0)
                    // does it have a rational part                
                {
                    return make_any<ANY_TYPE_INT, mpz_int>(whole);
                    // return just the whole 
                }
                else
                {
                    return make_any<ANY_TYPE_RAT, mpq_rational>(mpq_rational(rat + whole));
                    // its the sum because the subtraction starts at 0
                }
            }
            else
            {
               return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("sub", {"number", "rational"})
                ); // it is an error
            }
        }

        fn_ret fn_fdiv(state *st, aty2 args)
        // rational point devision 
        {
            uint64_t size = args.size();
            if (args.size() == 0)
            // i need arguments
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("div", 0)
                );
                // no arguments is an error
            }
            if (is_a_any<ANY_TYPE_INT>(args[0]) || is_a_any<ANY_TYPE_RAT>(args[0]))
            // it must be a number
            {
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_RAT>(args[0]))
                // its a natural rational
                {
                    rat = any_fast<mpq_rational>(args[0]);
                }
                else
                // its a whole number
                {
                    rat = any_fast<mpz_int>(args[0]);
                }
                for (uint64_t pl = 1; pl < size; pl++)
                // devide by the rest of them
                {
                    if (is_a_any<ANY_TYPE_INT>(args[pl]))
                    {
                        rat /=  any_fast<mpz_int>(args[pl]);
                    }
                    else if (is_a_any<ANY_TYPE_RAT>(args[pl]))
                    {
                        rat /=  any_fast<mpq_rational>(args[pl]);
                    }
                    else
                    // it was not numeric
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("div", {"number", "rational"})
                        );
                        // return an error
                    }
                }
                return  make_any<ANY_TYPE_RAT, mpq_rational>(rat);
            }
            else
            // the first argument was not a number
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("div", {"number", "rational"})
                );
                // error, it must be numeric
            }
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_add(state *st, aty2 args)
        // int + int, string + string
        // not implemented: (table + table, list + list)
        // adding tables or lists merge them
        {
            uint64_t size = args.size();
            if (args.size() == 0)
            // takes more than 1 argument
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("add", 0)
                );
            }
            if (is_a_any<ANY_TYPE_STR>(args[0]))
            // strings can be added to strings
            // similar to to-string but does not auto cast to string for each
            // does not add spaces between elements
            {
                std::string ret = any_fast<std::string>(args[0]);
                for (uint64_t pl = 1; pl < size; pl++)
                {
                    if (!is_a_any<ANY_TYPE_STR>(args[pl]))
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("add", {"string"})
                        );
                    }
                    ret += any_fast<std::string>(args[pl]);
                }
                return make_any<ANY_TYPE_STR, std::string>(ret);;
            }
            else if (is_a_any<ANY_TYPE_LIST>(args[0]))
            {
                std::vector<anything> ret = any_fast<std::vector<anything>>(args[0]);
                for (uint64_t i = 1; i < size; i++)
                {
                    if (is_a_any<ANY_TYPE_LIST>(args[i]))
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("add", {"list"})
                        );
                    }
                    std::vector<anything> insert = any_fast<std::vector<anything>>(args[i]); 
                    ret.insert(ret.end(), insert.begin(), insert.end());
                }
                return make_any<ANY_TYPE_LIST, std::vector<anything>>(ret);;
            }
            else if (is_a_any<ANY_TYPE_INT>(args[0]) ||is_a_any<ANY_TYPE_RAT>(args[0]))
            // numeric addition
            {
                mpz_int whole;
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_INT>(args[0]))
                // integers are treated as a sum of their value and the ratio zero
                {
                    whole = any_fast<mpz_int>(args[0]);
                    rat = 0;
                }
                else
                // ratios are treated as integers are
                {
                    rat = any_fast<mpq_rational>(args[0]);
                    whole = 0;
                }
                for (uint64_t pl = 1; pl < size; pl++)
                // loop through the rest of the arguments
                {
                    if (is_a_any<ANY_TYPE_INT>(args[pl]))
                    {
                        whole +=  any_fast<mpz_int>(args[pl]);
                    }
                    else if (is_a_any<ANY_TYPE_RAT>(args[pl]))
                    {
                        rat +=  any_fast<mpq_rational>(args[pl]);
                    }
                    else
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("add", {"number", "rational"})
                        ); 
                    }
                }
                if (rat == 0)
                // its basically a integer, no need to use the raional part
                {
                    return make_any<ANY_TYPE_INT, mpz_int>(whole); 
                }
                else
                // sum the whole and the ratio return it as a big ratio
                {
                    return make_any<ANY_TYPE_RAT, mpq_rational>(mpq_rational(rat + whole));
                }
            }
            else
            // not a table, list, string, or int
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("add", {"list", "str", "int"})
                ); // it is an error
            }
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_mult(state *st, aty2 args)
        // copy a string n times, n is the sum of args[1] to args[args.size()-1]
        // copy a list n times, n is the sum of args[1] to args[args.size()-1]
        // multiply two 
        {
            uint64_t size = args.size();
            if (size == 0)
            {
                // (mul) always returns error
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("mul", 0)
                );
            }
            if (is_a_any<ANY_TYPE_STR>(args[0]))
            {
                if (size == 1)
                // strings need to know what to multiply by
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::need_more_args("mul", 1)
                    );
                }
                std::string ret;
                std::string ori = any_fast<std::string>(args[0]);
                uint64_t dotimes = 1;
                // do it once
                for (uint64_t i = 1; i < size; i++)
                // loop over the arguments
                {
                    if (is_a_any<ANY_TYPE_INT>(args[i]))
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("mul", {"string"})
                        ); 
                    }
                    dotimes *= uint64_t(any_fast<mpz_int>(args[i]));
                    // multiply how many times by the argument
                }
                uint64_t arrsize = ori.size();
                for (uint64_t i = 0; i < dotimes; i++)
                {
                    ret += ori;
                }
                ret.shrink_to_fit();
                // this makes the string much smaller in not alot of time
                return make_any<ANY_TYPE_STR, std::string>(ret);
            }
            else if (is_a_any<ANY_TYPE_LIST>(args[0]))
            {
                if (size == 1)
                // arrays need to know what to multiply by                
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::need_more_args("mul", 1)
                    );
                }
                std::vector<anything> ret;
                std::vector<anything> ori = any_fast<std::vector<anything>>(args[0]);
                uint64_t dotimes = 1;
                // ori will be inserted dotimes times
                for (uint64_t i = 1; i < size; i++)
                {
                    if (!is_a_any<ANY_TYPE_INT>(args[i]))
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("mul", {"number"})
                        ); 
                    }
                    dotimes *= uint64_t(any_fast<mpz_int>(args[i]));
                }
                uint64_t arrsize = ori.size();
                std::vector<anything>::iterator beg = ori.begin();
                std::vector<anything>::iterator end = ori.end();
                // the original stays the same
                for (uint64_t i = 0; i < dotimes; i++)
                {
                    ret.insert(ret.end(), beg, end);
                    // std way to merge two lists (std::vector)
                }
                ret.shrink_to_fit();
                // ret.
                // reduce memory usage
                anything got = fn_to_str(st, ret);
                return make_any<ANY_TYPE_LIST, std::vector<anything>>(ret);
            }
            else if (is_a_any<ANY_TYPE_INT>(args[0]) || is_a_any<ANY_TYPE_RAT>(args[0]))
            // number types here
            {
                mpz_int whole;
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_INT>(args[0]))
                {
                    whole = any_fast<mpz_int>(args[0]);
                    rat = 1;
                    // starts at 1 so multiplication works
                }
                else
                {
                    rat = any_fast<mpq_rational>(args[0]);
                    whole = 1;
                    // starts at 1 so multiplication works
                }
                for (uint64_t pl = 1; pl < size; pl++)
                {
                    if (is_a_any<ANY_TYPE_INT>(args[pl]))
                    {
                        whole *=  any_fast<mpz_int>(args[pl]);
                    }
                    else if (is_a_any<ANY_TYPE_RAT>(args[pl]))
                    {
                        rat *=  any_fast<mpq_rational>(args[pl]);
                    }
                    else
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("mul", {"number", "rational"})
                        ); 
                    }
                }
                if (rat == 1)
                // there is no rational part to multiply, it returns a number
                {
                    return make_any<ANY_TYPE_INT, mpz_int>(whole);
                }
                else
                // there is a rational part and it must be multiplyed 
                {
                    return make_any<ANY_TYPE_RAT, mpq_rational>(mpq_rational(rat * whole));
                }
            }
            else
            // not a supported type (number, rational, string or list)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("mul", {"number", "rational", "string", "list"})
                );
            }
            // return  make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_do(state *st, aty2 args)
        // does almost no work, the arguments are already evaluated so all it needs i the last one
        {
            return args[args.size()-1];
        }

        fn_ret fn_to_str(state *st, aty2 args)
        // if ran on a string it returns the string in quotes
        {
            uint64_t size = args.size();
            std::stringstream ss;
            for (uint64_t pl = 0; pl < size; pl++)
            {
                if (pl != 0)
                {
                    ss << " ";
                }
                anything cur = args[pl];
                if (is_a_any<ANY_TYPE_STR>(cur))
                {
                    ss << '"';
                    ss << any_fast<std::string>(cur);
                    ss << '"';
                    // give strings extra quotes
                }
                else if (is_a_any<ANY_TYPE_INT>(cur))
                {
                    ss << any_fast<mpz_int>(cur);
                }
                else if (is_a_any<ANY_TYPE_RAT>(cur))
                {
                    mpq_rational frac = any_fast<mpq_rational>(cur);
                    ss << frac;
                }
                else if (is_a_any<ANY_TYPE_FUNC>(cur))
                {
                    ss << "<function>";
                    // cannot display functions, use a name instead
                }
                else if (is_a_any<ANY_TYPE_USER_FN>(cur))
                {
                    ss << "<function>";
                }
                else if (is_a_any<ANY_TYPE_BOOL>(cur))
                {
                    ss << (any_fast<bool>(cur) ? "true" : "false");
                    // must manually use the strings "true" and "false"
                }
                else if (is_a_any<ANY_TYPE_NONE>(cur))
                {
                    ss << "none";
                    // none cannot have a value
                }
                else if (is_a_any<ANY_TYPE_LIST>(cur))
                {
                    ss << "(list";
                    // close as possible to the code that would make it
                    std::vector<anything> passme = any_fast<std::vector<anything>>(cur);
                    anything agot = fn_to_str(st, passme);
                    if (passme.size() > 0)
                    {
                        ss << " ";
                    }
                    // run str on all the arguments that were in the list
                    if (!is_a_any<ANY_TYPE_STR>(agot))
                    // to-str returned an error
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            "to-str of list could not convert it's argumets to strings"s
                        );
                    }
                    ss << any_fast<std::string>(agot);
                    ss << ")";
                    // end the list
                }
                else if (is_a_any<ANY_TYPE_TABLE>(cur))
                {
                    ss << "(table";
                    // like lists but it goes (table key1 value1 key2 value2...)
                    table_type table = any_fast<table_type>(cur);
                    std::vector<anything> passme;
                    if (table.size() > 0)
                    {
                        ss << " ";
                    }
                    // arguments to pass to to-str
                    for (std::shared_ptr<std::pair<anything, anything>> p: table)
                    {
                        passme.push_back(p->first);
                        passme.push_back(p->second);
                        // push the two parts that make up the key and value
                        // makes them into a list-like structure
                    }
                    anything agot = fn_to_str(st, passme);
                    if (!is_a_any<ANY_TYPE_STR>(agot))
                    // to-str returned an error
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            "to-str of table could not convert it's argumets to strings"s
                        ); 
                    }
                    ss << any_fast<std::string>(agot);
                    ss << ")";
                    // end the table
                }
                else if (is_a_any<ANY_TYPE_DATA>(cur))
                {
                    ss << "<data>";
                }
                else
                // cannot change type given into string
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::type_error("to-str", {"number", "rational", "string", "function", "list", "table", "boolean"})
                    );  
                }
            }
            return make_any<ANY_TYPE_STR, std::string>(ss.str());
            // everything worked, change the stream to a string
        }
        fn_ret fn_type_of(state *st, aty2 args)
        // gets the type of a variable
        {
            uint64_t size = args.size();
            if (size != 1)
            // takes exactly one argument as more would be confusing and mostly useless
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function type-of takes exactly 1 argument"s
                );
            }
            std::stringstream ss;
            anything cur = args[0];
            // take the only argument and get its type
            if (is_a_any<ANY_TYPE_STR>(cur))
            {
                ss << "string";
            }
            else if (is_a_any<ANY_TYPE_INT>(cur))
            {
                ss << "integer";
            }
            else if (is_a_any<ANY_TYPE_RAT>(cur))
            {
                ss << "rational";
            }
            else if (is_a_any<ANY_TYPE_FUNC>(cur))
            {
                ss << "function";
            }
            else if (is_a_any<ANY_TYPE_BOOL>(cur))
            {
                ss << "boolean";
            }
            else if (is_a_any<ANY_TYPE_NONE>(cur))
            {
                ss << "none";
            }
            else if (is_a_any<ANY_TYPE_LIST>(cur))
            {
                ss << "list";
            }
            else if (is_a_any<ANY_TYPE_TABLE>(cur))
            {
                ss << "table";
            }
            else if (is_a_any<ANY_TYPE_DATA>(cur))
            {
                ss << "data";
            }
            else if (is_a_any<ANY_TYPE_USER_FN>(cur))
            {
                ss << "function";
            }
            else
            // it was an unknown type
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("mul", {"number", "rational", "string", "function", "list", "table", "boolean"})
                );  
            }
            return make_any<ANY_TYPE_STR, std::string>(ss.str());
            // return the stream string
        }
        fn_ret fn_len_of(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (size != 1)
            {
                 return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function len-of takes exactly 1 argument"s
                );
            }
            mpz_int ret;
            anything cur = args[0];
            if (is_a_any<ANY_TYPE_STR>(cur))
            {
                ret = any_fast<std::string>(cur).size();
            }
            else if (is_a_any<ANY_TYPE_LIST>(cur))
            {
                // std::cout << "end" << std::endl;
                ret = any_fast<std::vector<anything>>(cur).size();
                // std::cout << "begin" << std::endl;
            }
            else if (is_a_any<ANY_TYPE_TABLE>(cur))
            {
                // std::cout << "end" << std::endl;
                ret = any_fast<table_type>(cur).size();
                // std::cout << "begin" << std::endl;
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("len-of", {"string", "list", "table"})
                ); 
            }
            return make_any<ANY_TYPE_INT, mpz_int>(ret);
        }

        fn_ret fn_def_str(state *st, aty2 args)
        // the backend of the def keyword
        {
            uint64_t size = args.size();
            if (size != 2)
            // takes a string and the anything to set it to
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function def-str takes exactly 2 arguments"s
                );
                // raise an error
            }
            anything cur = args[0];
            if (is_a_any<ANY_TYPE_STR>(cur))
            {
                std::string name = any_fast<std::string>(cur);
                // std::cout << name << std::endl;
                st->set_var(name, args[1]);
            }
            else
            // def-str takes a string, obviously
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("def-str", {"string"})
                ); 
            }
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_list(state *st, aty2 args)
        // return the arguments to make a list
        {
            return make_any<ANY_TYPE_LIST, std::vector<anything>>(args);
        }

        fn_ret fn_seconds(state *st, aty2 args)
        // unix time since epoch
        {
            uint64_t size = args.size();
            if (size != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function time-seconds takes exactly no arguments"s
                );
            }
            std::chrono::duration curtime = std::chrono::high_resolution_clock::now().time_since_epoch();
            std::chrono::seconds secs = std::chrono::duration_cast<std::chrono::seconds>(curtime);
            mpz_int ret = secs.count();
            return make_any<ANY_TYPE_INT, mpz_int>(ret);
        }

        fn_ret fn_milliseconds(state *st, aty2 args)
        // unix time since epoch
        {
            uint64_t size = args.size();
            if (size != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function time-milliseconds takes exactly no arguments"s
                );
            }
            std::chrono::duration curtime = std::chrono::high_resolution_clock::now().time_since_epoch();
            std::chrono::milliseconds secs = std::chrono::duration_cast<std::chrono::milliseconds>(curtime);
            mpz_int ret = secs.count();
            return make_any<ANY_TYPE_INT, mpz_int>(ret);
        }

        fn_ret fn_microseconds(state *st, aty2 args)
        // unix time since epoch
        {
            uint64_t size = args.size();
            if (size != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function time-microseconds takes exactly no arguments"s
                );
            }
            std::chrono::duration curtime = std::chrono::high_resolution_clock::now().time_since_epoch();
            std::chrono::microseconds secs = std::chrono::duration_cast<std::chrono::microseconds>(curtime);
            mpz_int ret = secs.count();
            return make_any<ANY_TYPE_INT, mpz_int>(ret);
        }

        fn_ret fn_nanoseconds(state *st, aty2 args)
        // unix time since epoch
        {
            uint64_t size = args.size();
            if (size != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function time-nanoseconds takes exactly no arguments"s
                );
            }
            std::chrono::duration curtime = std::chrono::high_resolution_clock::now().time_since_epoch();
            std::chrono::nanoseconds secs = std::chrono::duration_cast<std::chrono::nanoseconds>(curtime);
            mpz_int ret = secs.count();
            return make_any<ANY_TYPE_INT, mpz_int>(ret);
        }

        fn_ret fn_greater_than(state *st, aty2 args)
        {
            uint64_t size = args.size();
            mpq_rational most;
            if (size < 2)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("greater-than", 2)
                );
            }
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                most = any_fast<mpz_int>(args[0]);
            }
            else if (is_a_any<ANY_TYPE_RAT>(args[0]))
            {
                most = any_fast<mpq_rational>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("greater-than", {"number", "rational"})
                );  
            }
            for (uint64_t i = 1; i < size; i++)
            {
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_INT>(args[i]))
                {
                    rat = any_fast<mpz_int>(args[i]);
                }
                else if (is_a_any<ANY_TYPE_RAT>(args[i]))
                {
                    rat = any_fast<mpq_rational>(args[i]);
                }
                else
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::type_error("greater-than", {"number", "rational"})
                    );  
                }
                // std::cout << most << std::endl;
                if (rat < most)
                {
                    most = rat;
                }
                else
                {
                    return make_any<ANY_TYPE_BOOL, bool>(false);
                }
            }
            return make_any<ANY_TYPE_BOOL, bool>(true);
        }

        fn_ret fn_less_than(state *st, aty2 args)
        {
            uint64_t size = args.size();
            mpq_rational most;
            if (size < 2)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::need_more_args("less-than", 2)
                ); 
            }
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                most = any_fast<mpz_int>(args[0]);
            }
            else if (is_a_any<ANY_TYPE_RAT>(args[0]))
            {
                most = any_fast<mpq_rational>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("less-than", {"number", "rational"})
                );   
            }
            for (uint64_t i = 1; i < size; i++)
            {
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_INT>(args[i]))
                {
                    rat = any_fast<mpz_int>(args[i]);
                }
                else if (is_a_any<ANY_TYPE_RAT>(args[i]))
                {
                    rat = any_fast<mpq_rational>(args[i]);
                }
                else
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("less-than", {"number", "rational"})
                );   
                }
                // std::cout << most << std::endl;
                if (rat > most)
                {
                    most = rat;
                }
                else
                {
                    return make_any<ANY_TYPE_BOOL, bool>(false);
                }
            }
            return make_any<ANY_TYPE_BOOL, bool>(true);
        }

        fn_ret fn_greater_or_equ(state *st, aty2 args)
        {
            uint64_t size = args.size();
            mpq_rational most;
            if (size < 2)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("greater-than-equ", 2)
                );
            }
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                most = any_fast<mpz_int>(args[0]);
            }
            else if (is_a_any<ANY_TYPE_RAT>(args[0]))
            {
                most = any_fast<mpq_rational>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("greater-than-equ", {"number", "rational"})
                );  
            }
            for (uint64_t i = 1; i < size; i++)
            {
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_INT>(args[i]))
                {
                    rat = any_fast<mpz_int>(args[i]);
                }
                else if (is_a_any<ANY_TYPE_RAT>(args[i]))
                {
                    rat = any_fast<mpq_rational>(args[i]);
                }
                else
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::type_error("greater-than-equ", {"number", "rational"})
                    ); 
                }
                // std::cout << most << std::endl;
                if (rat <= most)
                {
                    most = rat;
                }
                else
                {
                    return make_any<ANY_TYPE_BOOL, bool>(false);
                }
            }
            return make_any<ANY_TYPE_BOOL, bool>(true);
        }

        fn_ret fn_less_or_equ(state *st, aty2 args)
        {
            uint64_t size = args.size();
            mpq_rational most;
            if (size < 2)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::need_more_args("less-than-equ", 2)
                );
            }
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                most = any_fast<mpz_int>(args[0]);
            }
            else if (is_a_any<ANY_TYPE_RAT>(args[0]))
            {
                most = any_fast<mpq_rational>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("less-than-equ", {"number", "rational"})
                );  
            }
            for (uint64_t i = 1; i < size; i++)
            {
                mpq_rational rat;
                if (is_a_any<ANY_TYPE_INT>(args[i]))
                {
                    rat = any_fast<mpz_int>(args[i]);
                }
                else if (is_a_any<ANY_TYPE_RAT>(args[i]))
                {
                    rat = any_fast<mpq_rational>(args[i]);
                }
                else
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::type_error("less-than-equ", {"number", "rational"})
                    );  
                }
                // std::cout << most << std::endl;
                if (rat >= most)
                {
                    most = rat;
                }
                else
                {
                    return make_any<ANY_TYPE_BOOL, bool>(false);
                }
            }
            return make_any<ANY_TYPE_BOOL, bool>(true);
        }

        fn_ret fn_all_equal(state *st, aty2 args)
        {
            uint64_t size = args.size();
            mpq_rational most;
            if (size < 2)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::need_more_args("equal", 2)
                ); 
            }
            anything fst = args[0];
            for (uint64_t i = 1; i < size; i++)
            {
                // std::cout << !anything_compare(fst, args[i]) << std::endl;
                if (!anything_compare(fst, args[i]))
                {
                    return make_any<ANY_TYPE_BOOL, bool>(false);
                }
                fst = args[i];
            }
            return make_any<ANY_TYPE_BOOL, bool>(true);
        }

        fn_ret fn_getvars(state *st, aty2 args)
        {
            // std::cout << "getting vars" << std::endl;
            return make_any<ANY_TYPE_TABLE, table_type>(
                st->globals[st->globals.size()-1]
            );
        }

        fn_ret fn_getindex(state *st, aty2 args)
        {
            uint64_t size = args.size(); 
            if (size != 2 && size != 3)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function index takes 2 or 3 argument"s
                );            
            }
            if (is_a_any<ANY_TYPE_LIST>(args[0]))
            {
                if (size != 2)
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        "function index takes exactly 2 argument when first argument is a list"s
                    );            
                }
                std::vector<anything> list = any_fast<std::vector<anything>>(args[0]);
                size_t listsize = list.size();
                if (is_a_any<ANY_TYPE_INT>(args[0]))
                {
                    size_t index = size_t(any_fast<mpz_int>(args[0]));
                    if (index < listsize && index >= 0)
                    {
                        return list[index];
                    }
                    else if (-index < listsize && index < 0)
                    {
                        return list[listsize+index];
                    }
                    else
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>("list index out of range"s); 
                    }
                }
                else
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::type_error("index", {"number"})
                    ); 
                }
            }
            else if (is_a_any<ANY_TYPE_TABLE>(args[0]))
            {
                table_type table = any_fast<table_type>(args[0]);
                anything got = get_table(table, args[1]);
                if (size == 2)
                {
                    return got;
                }
                else if (is_a_any<ANY_TYPE_ERROR>(got))
                {
                    return args[2];
                }
                else
                {
                    return got;
                }
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("index", {"table", "list"})
                ); 
            }
        }

        fn_ret fn_ipow(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (args.size() == 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("ipow", 0)
                ); 
            }
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                mpz_int whole = any_fast<mpz_int>(args[0]);
                mpz_int ret = 1;
                for (uint64_t pl = 1; pl < size; pl++)
                {
                    if (is_a_any<ANY_TYPE_INT>(args[pl]))
                    {
                        mpz_int dot = any_fast<mpz_int>(args[pl]);
                        for (uint64_t i = 0; i < dot; i++)
                        {
                            ret *= whole;
                        }
                        whole = ret;
                        ret = 1;
                    }
                    else
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("pow", {"number"})
                        ); 
                    }
                }
                return make_any<ANY_TYPE_INT, mpz_int>(whole); 
            }
            else if (is_a_any<ANY_TYPE_RAT>(args[0]))
            {
                mpq_rational rat = any_fast<mpq_rational>(args[0]);
                mpq_rational temp = 1;
                for (uint64_t pl = 1; pl < size; pl++)
                {
                    if (is_a_any<ANY_TYPE_INT>(args[pl]))
                    {
                        mpz_int dot = any_fast<mpz_int>(args[pl]);
                        for (uint64_t i = 0; i < dot; i++)
                        {
                            temp *= rat;
                        }
                        rat = temp;
                        temp = 1;
                    }
                    else
                    {
                        return make_any<ANY_TYPE_ERROR,errors::str_error>(
                            errors::type_error("pow", {"number"})
                        ); 
                    }
                }
                return make_any<ANY_TYPE_RAT, mpq_rational>(rat); 
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("pow", {"number", "rational"})
                ); 
            }
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_version_no(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (args.size() != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function get-version takes exactly 0 argument"s
                ); 
            }
            table_type ret;
            ret.push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                make_any<ANY_TYPE_STR, std::string>("major"),
                make_any<ANY_TYPE_INT, mpz_int>(0)
            })));
            ret.push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                make_any<ANY_TYPE_STR, std::string>("minor"),
                make_any<ANY_TYPE_INT, mpz_int>(0)
            })));
            ret.push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                make_any<ANY_TYPE_STR, std::string>("sub"),
                make_any<ANY_TYPE_INT, mpz_int>(1)
            })));
            return make_any<ANY_TYPE_TABLE, table_type>(ret);
        }

        fn_ret fn_new_table(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (args.size() % 2 == 1)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function new-table takes an even number of argument"s
                ); 
            }
            table_type ret;
            for (uint64_t i = 0; i < size; i += 2)
            {
                ret.push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                    args[i],
                    args[i+1]
                })));
            }
            return make_any<ANY_TYPE_TABLE, table_type>(ret);
        }

        fn_ret fn_sleep_seconds(state *st, aty2 args)
        // unix time since epoch
        {
            if (args.size() != 1)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function sleep-seconds takes exactly 0 argument"s
                ); 
            }
            mpz_int timefor;
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                timefor = any_fast<mpz_int>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("sleep-seconds", {"number"})
                ); 
            }
            std::this_thread::sleep_for(std::chrono::seconds(int64_t(timefor)));
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_sleep_milliseconds(state *st, aty2 args)
        // unix time since epoch
        {
            if (args.size() != 1)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function sleep-milliseconds takes exactly 0 argument"s
                ); 
            }
            mpz_int timefor;
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                timefor = any_fast<mpz_int>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("sleep-milliseconds", {"number"})
                ); 
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(timefor)));
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_sleep_microseconds(state *st, aty2 args)
        // unix time since epoch
        {
            if (args.size() != 1)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function sleep-microseconds takes exactly 0 argument"s
                ); 
            }
            mpz_int timefor;
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                timefor = any_fast<mpz_int>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("sleep-microseconds", {"number"})
                ); 
            }
            std::this_thread::sleep_for(std::chrono::microseconds(int64_t(timefor)));
            return make_any<ANY_TYPE_NONE, none>(none());
        }


        fn_ret fn_sleep_nanoseconds(state *st, aty2 args)
        // unix time since epoch
        {
            if (args.size() != 1)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function sleep-nanoseconds takes exactly 0 argument"s
                ); 
            }
            mpz_int timefor;
            if (is_a_any<ANY_TYPE_INT>(args[0]))
            {
                timefor = any_fast<mpz_int>(args[0]);
            }
            else
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("sleep-nanoseconds", {"number"})
                ); 
            }
            std::this_thread::sleep_for(std::chrono::nanoseconds(int64_t(timefor)));
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_settable(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (args.size() != 3)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::need_more_args("append", 3)
                ); 
            }
            if (!is_a_any<ANY_TYPE_TABLE>(args[0]))
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    errors::type_error("append", {"table"})
                ); 
            }
            table_type table = any_fast<table_type>(args[0]);
            anything key = args[1];
            anything value = args[2];

            size = table.size();
            for (uint64_t i = 0; i < size; i++)
            {
                std::shared_ptr<std::pair<anything, anything>> kvp = table[i];
                if (kvp->first.type != key.type)
                {
                    continue;
                }
                if (is_a_any<ANY_TYPE_STR>(kvp->first))
                {
                    std::string name = any_fast<std::string>(kvp->first);
                    std::string sval = any_fast<std::string>(key);
                    // std::cout << name << std::endl;
                    if (name == sval)
                    {
                        table[i]->second = key;
                        return make_any<ANY_TYPE_TABLE, table_type>(table);
                    }
                }
                else if (is_a_any<ANY_TYPE_INT>(kvp->first))
                {
                    mpz_int name = any_fast<mpz_int>(kvp->first);
                    mpz_int sval = any_fast<mpz_int>(key);
                    // std::cout << name << std::endl;
                    if (name == sval)
                    {
                        table[i]->second = key;
                        return make_any<ANY_TYPE_TABLE, table_type>(table);
                    }
                }
                else if (is_a_any<ANY_TYPE_RAT>(kvp->first))
                {
                    mpq_rational name = any_fast<mpq_rational>(kvp->first);
                    mpq_rational sval = any_fast<mpq_rational>(key);
                    // std::cout << name << std::endl;
                    if (name == sval)
                    {
                        table[i]->second = key;
                        return make_any<ANY_TYPE_TABLE, table_type>(table);
                    }
                }
                else if (is_a_any<ANY_TYPE_BOOL>(kvp->first))
                {
                    bool name = any_fast<bool>(kvp->first);
                    bool sval = any_fast<bool>(key);
                    // std::cout << name << std::endl;
                    if (name == sval)
                    {
                        table[i]->second = key;
                        return make_any<ANY_TYPE_TABLE, table_type>(table);
                    }
                }
                else if (is_a_any<ANY_TYPE_NONE>(kvp->first))
                {
                    table[i]->second = key;
                    return make_any<ANY_TYPE_TABLE, table_type>(table);
                }
                else if (is_a_any<ANY_TYPE_FUNC>(kvp->first))
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        "function \"append\" can not take builtin functions"s
                    );  
                }
                else
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::type_error("append", {"string", "number", "rational", "boolean", "none"})
                    );
                }
            }
            table.push_back(std::make_shared<std::pair<anything, anything>>(
                std::pair<anything, anything>(key, value)
            ));
            return  make_any<ANY_TYPE_TABLE, table_type>(table);
        }


        fn_ret fn_merge_globals(state *st, aty2 args)
        {
            uint64_t place = st->globals.size()-1;
            for (anything a: args)
            {
                if (!is_a_any<ANY_TYPE_TABLE>(a))
                {
                    return make_any<ANY_TYPE_ERROR,errors::str_error>(
                        errors::type_error("merge", {"table"})
                    );
                }
                table_type other = any_fast<table_type>(a);
                st->globals[place].insert(st->globals[place].end(), other.begin(), other.end());
            }
            return make_any<ANY_TYPE_NONE, none>(none());
        }

        fn_ret fn_get_lib_math(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (args.size() != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function get-time takes exactly 0 argument"s
                ); 
            }
            table_type ret;
            table_type *ret_ptr = &ret;
            std::function<void(std::string, fn_type)> regester = [ret_ptr](std::string name, fn_type fn) -> void
            {
                ret_ptr->push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                    make_any<ANY_TYPE_STR, std::string>(name),
                    make_any<ANY_TYPE_FUNC, fn_type>(std::function<fn_ret(state *, aty2)>(fn))
                })));
            };

            regester("add", lib::fn_add);
            regester("sub", lib::fn_sub);
            regester("mul", lib::fn_mult);
            regester("div", lib::fn_fdiv);
            regester("do", lib::fn_do);
            regester("greater-than", lib::fn_greater_than);
            regester("less-than", lib::fn_less_than);
            regester("greater-than-equ", lib::fn_greater_or_equ);
            regester("less-than-equ", lib::fn_less_or_equ);
            regester("equal", lib::fn_all_equal);
            regester("pow", lib::fn_ipow);

            return make_any<ANY_TYPE_TABLE, table_type>(ret); 
        }

        fn_ret fn_get_lib_time(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (args.size() != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function get-time takes exactly 0 argument"s
                ); 
            }
            table_type ret;
            table_type *ret_ptr = &ret;
            std::function<void(std::string, fn_type)> regester = [ret_ptr](std::string name, fn_type fn) -> void
            {
                ret_ptr->push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                    make_any<ANY_TYPE_STR, std::string>(name),
                    make_any<ANY_TYPE_FUNC, fn_type>(std::function<fn_ret(state *, aty2)>(fn))
                })));
            };

            regester("time-seconds", lib::fn_seconds);
            regester("sleep-seconds", lib::fn_sleep_seconds);
            regester("time-milliseconds", lib::fn_milliseconds);
            regester("sleep-milliseconds", lib::fn_sleep_milliseconds);
            regester("time-microseconds", lib::fn_microseconds);
            regester("sleep-microseconds", lib::fn_sleep_microseconds);
            regester("time-nanoseconds", lib::fn_nanoseconds);
            regester("sleep-nanoseconds", lib::fn_sleep_nanoseconds);

            return make_any<ANY_TYPE_TABLE, table_type>(ret); 
        }


        fn_ret fn_get_lib_table(state *st, aty2 args)
        {
            uint64_t size = args.size();
            if (args.size() != 0)
            {
                return make_any<ANY_TYPE_ERROR,errors::str_error>(
                    "function get-time takes exactly 0 argument"s
                ); 
            }
            table_type ret;
            table_type *ret_ptr = &ret;
            std::function<void(std::string, fn_type)> regester = [ret_ptr](std::string name, fn_type fn) -> void
            {
                ret_ptr->push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                    make_any<ANY_TYPE_STR, std::string>(name),
                    make_any<ANY_TYPE_FUNC, fn_type>(std::function<fn_ret(state *, aty2)>(fn))
                })));
            };

            regester("len-of", lib::fn_len_of);
            regester("get-time", lib::fn_get_lib_time);
            regester("append", lib::fn_settable);
            regester("table", lib::fn_new_table);
            regester("list", lib::fn_list);
            regester("vars", lib::fn_getvars);
            regester("index", lib::fn_getindex);

            return make_any<ANY_TYPE_TABLE, table_type>(ret); 
        }


    }

    table_type generate()
    {
        table_type ret;

        table_type *ret_ptr = &ret;
        std::function<void(std::string, fn_type)> regester = [ret_ptr](std::string name, fn_type fn) -> void
        {
            ret_ptr->push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                make_any<ANY_TYPE_STR, std::string>(name),
                make_any<ANY_TYPE_FUNC, fn_type>(std::function<fn_ret(state *, aty2)>(fn))
            })));
        };

        regester("print", lib::fn_print);
        regester("to-str", lib::fn_to_str);

        regester("merge", lib::fn_merge_globals);
        regester("lib-math", lib::fn_get_lib_math);
        regester("lib-list", lib::fn_get_lib_table);
        regester("lib-version", lib::fn_version_no);
        regester("lib-time", lib::fn_get_lib_time);

        regester("type-of", lib::fn_type_of);
        regester("def-str", lib::fn_def_str);
    
        ret_ptr->push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
                make_any<ANY_TYPE_STR, std::string>("true"),
                make_any<ANY_TYPE_BOOL, bool>(true)
        })));
        ret_ptr->push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
            make_any<ANY_TYPE_STR, std::string>("false"),
            make_any<ANY_TYPE_BOOL, bool>(false)
        })));
        // ret_ptr->push_back(std::make_shared<std::pair<anything, anything>>(std::pair<anything, anything>({
        //     make_any<ANY_TYPE_STR, std::string>("version"),
        //     make_any<ANY_TYPE_BOOL, table_type>(version_number())
        // })));

        return ret;
    }

}