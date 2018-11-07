#include "lang-defs.hpp"
#include "errors.hpp"

namespace lang
{
    enum token_type
    {
        TOKEN_TYPE_NAME, // [a-zA-Z][a-zA-Z0-9]+
        TOKEN_TYPE_STR, // if, elif, end, while, do
        TOKEN_TYPE_INT, // [0-9]+
        TOKEN_TYPE_FLOAT, // [0-9]+\.[0-9]+
        TOKEN_TYPE_OPEN, // ( [ {
        TOKEN_TYPE_CLOSE, // ) ] }
    };

    enum comp_stack_type
    {
        COMP_STACK_TYPE_FUNC_CALL,
        COMP_STACK_TYPE_TABLE_CALL,
        COMP_STACK_TYPE_INDEX_CALL,
        COMP_STACK_TYPE_QUOTE,
        COMP_STACK_TYPE_NORMAL,
        COMP_STACK
    };
    
    enum opcode_type
    {
        OPCODE_TYPE_PUSH_VAL,
        OPCODE_TYPE_PUSH_NAME,
        OPCODE_TYPE_POP,
        OPCODE_TYPE_FUNC_CALL,
        OPCODE_TYPE_JMP_IF_NOT,
        OPCODE_TYPE_JMP_IF,
        OPCODE_TYPE_JMP,
    };

    struct anything
    {
        std::shared_ptr<void> val;
        uint64_t type;
    };

    struct none{
        bool operator ==(none);
    };

    struct opcode
    {
        opcode_type type;
        uint64_t helper;
    };

    struct token
    {
        uint64_t line; // generated is -1
        uint64_t col; // generated is -1
        std::string token; // never empty
        token_type type; // the kind of token it is
    };

    struct node
    {
        std::vector<token> tok;
        std::vector<node> children;
    };

    struct state
    {
        tokens toks;
        opcode_vec opcodes;
        std::stack<errors::str_error> errors;
        std::vector<table_type> globals = {
            generate()
        };
        std::vector<anything> helpers;
        node root;
        anything load_global(anything &);
        void set_var(std::string &, anything &);
        void run();
        void lex(std::shared_ptr<std::istream> is);
        void comp();
        void ast();
    };

    std::set<std::string> special_funcs = {
        "if",
        "def",
        "cond",
        "while",
    };

    bool none::operator ==(none n)
    {
        return true;
    }
    
    template<typename T>
    T any_fast(anything &a)
    {
        return *std::static_pointer_cast<T>(a.val);
    }

    template<typename T>
    T *any_fast_ptr(anything &a)
    {
        return std::static_pointer_cast<T>(a.val);
    }
    
    template<any_type Tc, typename T>
    anything make_any(T v)
    {
        anything a;
        a.val = std::make_shared<T>(v);
        a.type = Tc;
        return a;
    }
    template<any_type T>
    bool is_a_any(anything &a)
    {
        return a.type == T;
    }

    std::string walknode(node n)
    {
        std::string ret = "[ ";
        for (token t: n.tok)
        {
            ret += t.token;
            ret += " ";
        }
        for (node c: n.children)
        {
            ret += walknode(c);
            ret += " ";
        }
        return ret + ']';
    }

    template<any_type Tc, typename T>
    anything get_table_type(table_type &table, anything &value)
    {
        T cmp = any_fast<T>(value);
        for (std::shared_ptr<std::pair<anything, anything>> kvp: table)
        {
            if (is_a_any<Tc>(kvp->first))
            {
                T name = any_fast<T>(kvp->first);
                if (cmp == name)
                {
                    return kvp->second;
                }
            }
        }
        return make_any<ANY_TYPE_ERROR, errors::str_error>("get table type error"s);
    }

    anything get_table(table_type &table, anything &value)
    {
        uint64_t size = table.size();
        if (is_a_any<ANY_TYPE_STR>(value))
        {
            return get_table_type<ANY_TYPE_STR, std::string>(table, value);
        }
        if (is_a_any<ANY_TYPE_INT>(value))
        {
            return get_table_type<ANY_TYPE_INT, mpz_int>(table, value);
        }
        if (is_a_any<ANY_TYPE_BOOL>(value))
        {
            return get_table_type<ANY_TYPE_STR, std::string>(table, value);
        }
        if (is_a_any<ANY_TYPE_RAT>(value))
        {
            return get_table_type<ANY_TYPE_RAT, mpq_rational>(table, value);
        }
        if (is_a_any<ANY_TYPE_NONE>(value))
        {
            return get_table_type<ANY_TYPE_NONE, none>(table, value);
        }
        return make_any<ANY_TYPE_ERROR, errors::str_error>("get table error"s);
    }

    anything state::load_global(anything &value)
    {
        uint64_t size = globals.size();
        for (uint64_t i = 1; i <= size; i--)
        {
            return get_table(globals[size-i], value);
        }
        return make_any<ANY_TYPE_ERROR, errors::str_error>("load global error"s);
    }

        void state::set_var(std::string &sval, anything &value)
        {
            uint64_t gsizem = globals.size()-1;
            uint64_t size = globals[gsizem].size();
            for (uint64_t i = 0; i < size; i++)
            {
                std::shared_ptr<std::pair<anything, anything>> kvp = globals[gsizem][i];
                if (is_a_any<ANY_TYPE_STR>(kvp->first))
                {
                    std::string name = any_fast<std::string>(kvp->first);
                    // std::cout << name << std::endl;
                    if (name == sval)
                    {
                        globals[gsizem][i]->second = value;
                        return;
                    }
                }
            }
            globals[gsizem].push_back(std::make_shared<std::pair<anything, anything>>(
                std::pair<anything, anything>(make_any<ANY_TYPE_STR, std::string>(sval), value)
            ));
        }

    void state::run()
    {

        // std::vector<uint64_t> prof(opcodes.size());
        // std::vector<uint64_t> times(opcodes.size());
    
        std::stack<anything> vm_stack;
        vm_stack.push(make_any<ANY_TYPE_ERROR, errors::str_error>("fn error"s));
        uint64_t size = opcodes.size();
        uint64_t place = 0;
        while (place < size)
        {
            // prof[place] ++;
            // uint64_t time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            if (errors.size() > 0)
            {
                errors.top().show_error();
                exit(1);
            }
            // std::cout << vm_stack.top().type().name() << std::endl;
            opcode op = opcodes[place];
            switch (op.type)
            {
                case OPCODE_TYPE_PUSH_VAL:
                {
                    vm_stack.push(helpers[op.helper]);
                    break;
                }
                case OPCODE_TYPE_PUSH_NAME:
                {
                    anything value = load_global(helpers[op.helper]);
                    // std::cout << helpers[op.helper].type().name() << std::endl;
                    if (is_a_any<ANY_TYPE_ERROR>(value))
                    {
                        std::string errstr = "no variable named"s;
                        if (is_a_any<ANY_TYPE_STR>(helpers[op.helper]))
                        {
                            errstr += ": "s + any_fast<std::string>(helpers[op.helper]);
                        }
                        errors.push(errors::str_error(errstr));
                        break;
                    }
                    // std::cout << value.type << std::endl;
                    // std::cout << typeid(fn_type).hash_code() << std::endl;
                    // std::cout <<( value.type == ANY_TYPE_ERROR) << std::endl;
                    vm_stack.push(value);
                    break;
                }
                case OPCODE_TYPE_FUNC_CALL:
                {
                    std::vector<anything> args(op.helper);
                    for (uint64_t i = 1; i <= op.helper; i++)
                    {
                        args[op.helper-i] = vm_stack.top();
                        vm_stack.pop();
                    }
                    // std::cout << "fn: " << op.helper << std::endl;
                    anything fncall = vm_stack.top();
                    // vm_stack.pop();
                    if (!is_a_any<ANY_TYPE_FUNC>(fncall))
                    {
                        errors.push(errors::str_error("cannot call anything other than a function"));
                        break;
                    }
                    fn_type fn = any_fast<fn_type>(fncall);
                    // std::cout << "calling " << place << std::endl;
                    fn_ret got = fn(this, args);
                    // std::cout << "done" << std::endl;
                    if (is_a_any<ANY_TYPE_ERROR>(got))
                    {
                        // errors.push(errors::str_error("function returned error"));
                        errors.push(any_fast<errors::str_error>(got));
                        break;
                    }
                    vm_stack.top() = got;
                    break;
                }
                case OPCODE_TYPE_POP:
                {
                    vm_stack.pop();
                    // std::cout << "pop" << std::endl;
                    break;
                }
                case OPCODE_TYPE_JMP_IF:
                {
                    anything val = vm_stack.top();
                    vm_stack.pop();
                    if (is_a_any<ANY_TYPE_BOOL>(val))
                    {
                        if (any_fast<bool>(val))
                        {
                            uint64_t target = op.helper;
                            place = target;
                        }
                    }
                    else
                    {
                        uint64_t target = op.helper;
                        place = target;
                    }
                    break;
                }
                case OPCODE_TYPE_JMP_IF_NOT:
                {
                    anything val = vm_stack.top();
                    vm_stack.pop();
                    if (is_a_any<ANY_TYPE_BOOL>(val))
                    {
                        if (!any_fast<bool>(val))
                        {
                            uint64_t target = op.helper;
                            place = target;
                        }
                    }
                    else
                    {
                        uint64_t target = op.helper;
                        place = target;
                    }
                    break;
                }
                case OPCODE_TYPE_JMP:
                {
                    uint64_t target = op.helper;
                    place = target;
                    break;
                }
            }
            // times[place] += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - time;
            place ++;
        }
        if (errors.size() > 0)
        {
            errors.top().show_error();
            exit(1);
        }
    }

    void state::ast()
    {
        std::vector<node> nodes(1);
        for (token t: toks)
        {
            if (t.type == TOKEN_TYPE_OPEN)
            {
                nodes.push_back(node());
            }
            else if (t.type == TOKEN_TYPE_CLOSE)
            {
                uint64_t nodesize = nodes.size();
                nodes[nodesize-2].children.push_back(nodes[nodesize-1]);
                nodes.pop_back();
            }
            else
            {
                uint64_t nodesize = nodes.size();
                node next;
                next.tok = {t};
                // std::cout << t.token << std::endl;
                nodes[nodesize-1].children.push_back(next);
            }
        }
        root = nodes[0];
    }

    void state::comp()
    {
        uint64_t toksize = root.tok.size();
        // std::cout << walknode(root) << std::endl;
        // std::cout << root.children.size() << "\n";
        if (toksize == 0)
        {
            uint64_t size = root.children.size();
            if (size == 0)
            {
                std::cout << "cannot have empty call" << std::endl;
                exit(1);
            }
            node croot = root;
            std::string name = "";
            uint64_t i = 0;
            for (node n: croot.children)
            {
                if (i == 0 && n.tok.size() > 0 && n.tok[0].type == TOKEN_TYPE_NAME)
                {
                    if (special_funcs.count(n.tok[0].token) != 0)
                    {
                        name = n.tok[0].token;
                        // std::cout << name << std::endl;
                        break;
                    }
                }
                root = n;
                state::comp();
                i ++;
            }
            // std::cout << name << std::endl;
            if (name == "")
            {
                opcode op;
                op.type = OPCODE_TYPE_FUNC_CALL;
                op.helper = size-1;
                opcodes.push_back(op);
            }
            else if (name == "def")
            {
                if (croot.children.size() == 3)
                {
                    opcode op;
                    op.type = OPCODE_TYPE_PUSH_NAME;
                    op.helper = helpers.size();
                    opcodes.push_back(op);
                    helpers.push_back(make_any<ANY_TYPE_STR, std::string>("def-str"s));
                    
                    node ch1 = croot.children[1];
                    if (ch1.tok.size() == 0 || ch1.tok[0].type != TOKEN_TYPE_NAME)
                    {
                        std::cout << "def takes 2 arguments, the first must be a name" << std::endl;
                        exit(1);
                    }
                    
                    op.type = OPCODE_TYPE_PUSH_VAL;
                    op.helper = helpers.size();
                    opcodes.push_back(op);
                    // std::cout << ch1.tok[0].token << std::endl;
                    helpers.push_back(make_any<ANY_TYPE_STR, std::string>(ch1.tok[0].token));

                    root = croot.children[2];
                    state::comp();

                    op.type = OPCODE_TYPE_FUNC_CALL;
                    op.helper = 2;
                    opcodes.push_back(op);
                }
                else
                {
                    std::cout << "def takes 2 arguments" << std::endl;
                    exit(1);  
                }
            }
            else if (name == "while")
            {
                if (croot.children.size() != 3)
                {
                    std::cout << "def takes 2 arguments" << std::endl;
                    exit(1);  
                }

                uint64_t beginpos = opcodes.size();

                
                root = croot.children[1];
                state::comp();

                opcode op;
                op.type = OPCODE_TYPE_JMP_IF_NOT;
                // op.helper = helpers.size();
                opcodes.push_back(op);

                uint64_t contpos = opcodes.size();

                root = croot.children[2];
                state::comp();

                op.type = OPCODE_TYPE_POP;
                op.helper = 0;
                opcodes.push_back(op);

                op.type = OPCODE_TYPE_JMP;
                op.helper = beginpos-1;
                opcodes.push_back(op);

                op.type = OPCODE_TYPE_POP;
                op.helper = 0;
                opcodes.push_back(op);

                uint64_t breakpos = opcodes.size();
                // std::cout << breakpos << std::endl;
                opcodes[contpos-1].helper = breakpos-1;

            }
            else if (name == "if")
            {
                if (croot.children.size() != 3)
                {
                    std::cout << "def takes 2 arguments" << std::endl;
                    exit(1);  
                }
                root = croot.children[1];
                state::comp();

                opcode op;
                op.type = OPCODE_TYPE_JMP_IF_NOT;
                // op.helper = helpers.size();
                opcodes.push_back(op);

                uint64_t contpos = opcodes.size();

                root = croot.children[2];
                state::comp();

                op.type = OPCODE_TYPE_POP;
                op.helper = 0;
                opcodes.push_back(op);

                uint64_t breakpos = opcodes.size();
                // std::cout << breakpos << std::endl;
                opcodes[contpos-1].helper = breakpos-1;
            }
        }
        else
        {
            for (token t: root.tok)
            {
                // std::cout << t.token << std::endl;
                if (t.type == TOKEN_TYPE_NAME)
                {
                    opcode op;
                    op.type = OPCODE_TYPE_PUSH_NAME;
                    op.helper = helpers.size();
                    opcodes.push_back(op);
                    helpers.push_back(make_any<ANY_TYPE_STR, std::string>(t.token));
                }
                else if (t.type == TOKEN_TYPE_INT)
                {
                    opcode op;
                    op.type = OPCODE_TYPE_PUSH_VAL;
                    op.helper = helpers.size();
                    opcodes.push_back(op);
                    helpers.push_back(make_any<ANY_TYPE_INT, mpz_int>(mpz_int(t.token)));
                }
                else if (t.type == TOKEN_TYPE_STR)
                {
                    opcode op;
                    op.type = OPCODE_TYPE_PUSH_VAL;
                    op.helper = helpers.size();
                    opcodes.push_back(op);
                    helpers.push_back(make_any<ANY_TYPE_STR, std::string>(t.token));
                }
                else if (t.type == TOKEN_TYPE_FLOAT)
                {
                    opcode op;
                    op.type = OPCODE_TYPE_PUSH_VAL;
                    op.helper = helpers.size();
                    opcodes.push_back(op);
                    helpers.push_back(make_any<ANY_TYPE_RAT, mpq_rational>(strtorat(t.token)));
                }
                else
                {
                    std::cout << "unknown token past ast" << t.token << std::endl;
                    exit(1);
                }
            }
        }
    }

    void state::lex(std::shared_ptr<std::istream> is)
    {
        tokens *ptoks = &toks; // this is so that it can be captured by the lambda
        uint64_t line = 1; // start at line 1
        uint64_t col = 1; // col 1
        uint64_t *line_ptr = &line;
        uint64_t *col_ptr = &col;
        std::function<char()> input = [is, line_ptr, col_ptr]() mutable -> char
        {
            char ret = is->get();
            if (ret == '\n' || ret == '\r') // newlines and returns are the accepted newline charactors
            {
                *line_ptr += 1;
                *col_ptr = 1; 
            }
            else
            {
                *col_ptr += 1;
            }
            return ret;
        };
        std::function<void(std::string, token_type)> push = [ptoks, line, col](std::string tok, token_type t) -> void
        {
            token ctok; // no constructor
            ctok.line = line;
            ctok.col = col;
            ctok.token = tok;
            ctok.type = t;
            ptoks->push_back(ctok);
        };
        char got = input(); // no need for putback when each case is liable for its own input call
        while (1)
        {
            if (got == EOF) // return ret upon the end of the file
            {
                return;
            }
            if (got == ' ' || got == '\t' || got == '\n' || got == '\r' || got == ';' || got == ',')
            {
                got = input();
            }
            if (isalpha(got)) // name, do, and end
            {
                std::string str;
                while (isalnum(got)  || got == '-') // [a-zA-Z0-9]+
                {
                    str += got;
                    got = input();
                }
                push(str, TOKEN_TYPE_NAME);
            }
            if (got == '\"')
            {
                got = input();
                std::string str;
                while (got != '\"')
                {
                    str += got;
                    got = input();
                }
                got = input();
                push(str, TOKEN_TYPE_STR);
            }
            if (isdigit(got)) // int and float
            {
                std::string str;
                while (isdigit(got)) // [0-9]+
                {
                    str += got;
                    got = input();
                }
                if (got == '.')
                {
                    str += got;
                    got = input();
                    while (isdigit(got)) // [0-9]+
                    {
                        str += got;
                        got = input();
                    }
                    push(str, TOKEN_TYPE_FLOAT);
                }
                else
                {
                    push(str, TOKEN_TYPE_INT);
                }
            }
            if ("({["s.find(got) != std::string::npos)
            {
                push(""s+got, TOKEN_TYPE_OPEN);
                got = input();
            }
            if (")}]"s.find(got) != std::string::npos)
            {
                push(""s+got, TOKEN_TYPE_CLOSE);
                got = input();
            }
        }
    }

    mpq_rational strtorat(std::string str)
    {
        std::string den = "1";
        std::string num = "0";
        bool in_whole = true;
        for (char c: str)
        {
            if (c == '.')
            {
                in_whole = false;
            }
            else if (in_whole)
            {
                num += c;
            }
            else
            {
                num += c;
                den += "0";
            }
        }
        return mpq_rational(num, den);
    }
}