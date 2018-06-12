#include <iostream>
#include "Compiler.h"
#include "./sourceScanner/Visitor.h"
#include "./sourceScanner/parser.h"
#include "./sourceScanner/lexer.h"
#include "prettyprinter.h"
#include "../gc/gc.h"

using namespace std;

int main(int argc, char** argv) {
    void*  scanner;
    yylex_init(&scanner);
    if (argc < 2) {
        cout << "Expecting file name as argument" << endl;
        return 1;
    }

    FILE* infile = fopen(argv[1], "r");
    if (infile == NULL) {
        cout << "Cannot open file " << argv[1] << endl;
        return 1;
    }
    yyset_in(infile, scanner);
    Statement* output;
    int rvalue = yyparse(scanner, output);
    if (rvalue == 1) {
        cout << "Parsing failed" << endl;
        return 1;
    }

    try {
        PrettyPrinter p;
        Compiler comp;
        output->accept(comp);
        //BasicBlock* currB = comp.main->cfg;
        // while (currB != NULL) {
        //     for (Instruction* i : *(currB->instructions)) {
        //         p.print(*i, std::cout);
        //         cout << endl;
        //     }
        //     currB = currB->next;
        // }
        Function* f = new Function();
        comp.getOutFunc(f, comp.main);
        p.print(*f,std::cout);

    } catch (CompilerException e) {
        cout << e.message();
        return 1;
    }
    return 0;
}