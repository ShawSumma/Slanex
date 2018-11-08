#include "lang-lib.hpp"

int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        std::cout << "atleast 1 argument is needed" << std::endl;
        exit(1); 
    }
    std::shared_ptr<std::ifstream> f = std::make_shared<std::ifstream>(argv[1]);
    lang::state state;
    state.lex(f);
    state.ast();
    // std::cout << lang::walknode(state.root) << std::endl;
    state.comp();
    state.opcodes.pop_back();
    uint64_t pl = 0;
    // for (lang::opcode o: state.opcodes)
    // {
    //    std::cout << pl << ": " << o.type << "\t" << o.helper << std::endl;
    //     pl ++;
    // }
    // std::cout << std::endl;
    state.run();
}