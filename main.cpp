// #include "lang-lib.hpp"
#include "lang.hpp"

uint64_t feval(lang::state &state, std::istream &is, bool repl_mode, uint64_t start)
{
    bool broken = false;

    state.lex(is, repl_mode);

    if (state.toks.size() == 0)
    {
        return start;
    }

    state.ast();
    state.toks = {};
    
    // std::cout << lang::walknode(state.root) << std::endl;

    broken = state.comp();
    if (broken)
    {
        state.root = lang::node();
        return state.opcodes.size();
    }
    state.root = lang::node();
    state.opcodes.pop_back();
    broken = state.run(start, state.opcodes.size());
    state.vm_stack = {};
    if (broken)
    {
        return state.opcodes.size();
    }
    return state.opcodes.size();
}

int main(int argc, char** argv)
{
    lang::state state;     
    uint64_t start = 0;
    if (argc <= 1)
    {
        while (1)
        {
            std::cout << ">>>";
            start = feval(state, std::cin, true, start);
            std::cout << std::endl;
        }
    }
    else
    {
        std::ifstream f(argv[1]);
        start = feval(state, f, false, start); 
    }
}