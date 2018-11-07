#include "lang-lib.hpp"

int main()
{
    std::shared_ptr<std::ifstream> f = std::make_shared<std::ifstream>("lang.ion");
    lang::state state;
    state.lex(f);
    state.ast();
    // std::cout << lang::walknode(state.root) << std::endl;
    state.comp();
    state.opcodes.pop_back();
    uint64_t pl = 0;
    for (lang::opcode o: state.opcodes)
    {
        // std::cout << pl << "\t" << o.type << "\t" << o.helper << std::endl;
        pl ++;
    }
    // std::cout << std::endl;
    state.run();
}