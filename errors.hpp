#pragma once
#include "lang-defs.hpp"

namespace lang
{
    namespace errors
    {
        class str_error 
        {
            std::vector<std::string> info;
        public:
            virtual void show_error();
            str_error(std::string);
        };

        void str_error::show_error()
        {
            std::cout << "error: " << info[0] << std::endl;
        }

        str_error::str_error(std::string str)
        {
            info = {
                str
            };
        }

        str_error type_error(std::string fn_name, std::vector<std::string> okay_types)
        {
            std::string okays;
            uint64_t size = okay_types.size();
            for (uint64_t i = 0; i < size; i++)
            {
                if (i != 0)
                {
                    okays += ", ";
                }
                okays += okay_types[i];
            }
            return str_error("function \""s + fn_name + "\" can only deal with " + okays);
        }

        str_error need_more_args(std::string fn_name, uint64_t count)
        {
            return str_error("function \""s + fn_name + "\" needs more arguments " + std::to_string(count));
        }

    }
}