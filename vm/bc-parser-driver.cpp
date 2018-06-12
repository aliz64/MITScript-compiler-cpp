#include "bc-parser.h"
#include "bc-lexer.h"

#include "../gc/gc.h"

#include <iostream>
#include "./sourceScanner/Visitor.h"
#include "Compiler.h"

#include "Interpreter.h"
#include "prettyprinter.h"

using namespace std;

int main(int argc, char** argv) { 

    if (argc < 3) {
        cout << "Invalid Arguments: filename followed by flag and mem (optional 4500MB default)";
        return 1;
    }

    int mem = 0;
    if (argc == 3) {
        mem = 4500;
    } else {

        if (  (argc != 5) || strcmp(argv[3], "-mem") ) {
            cout << "Invalid Arguments: filename followed by flag and mem (optional 4500MB default)";
            return 1;
        }
        mem = atoi(argv[4]);
        if (mem < 4) { cout << "please enter at least 4 MB"; return 1; }
    }

    FILE* infile = fopen(argv[1], "r");
    if (infile == NULL) {
    cout << "Cannot open file " << argv[1] << endl;
        return 1;
    }

    // For doing SOURCE code
    if ( !strcmp(argv[2], "-s") || !strcmp(argv[2], "-sp") ) {


        CollectedHeap heap(mem);


        Compiler* comp = new Compiler();
        Statement* program = comp->parseSourceAST(infile); //parse the SOURCE code
        if (program == NULL) {
            cout << "Source Parsing Failed!";
            return 1;
        }

        try {
            program->accept(*comp);
        } catch (CompilerException e) {
            cout << e.message() << endl;
            return 1;
        }

        delete program; // deleting the AST as we dont need it anymore

        Function* f = new Function();
        comp->getOutFunc(f, comp->main);
        InstructionHeap* heapI = comp->heapI;
        delete comp;


        if ( !strcmp(argv[2], "-sp") ) {
            PrettyPrinter p;
            p.print(*f, std::cout); cout << endl;// << "------" << endl;

            delete f; // delete the funciton
            heapI->clean(); delete heapI;
            return 0;
        }


        try {
            Interpreter inter(heap);
            Value* v = inter.interpret(f);
        } catch (InterpreterException e) {
            cout << e.message() << endl;

            delete f; // delete the funciton
            heapI->clean(); delete heapI;
            return 1;
        }

        delete f; // delete the funciton
        heapI->clean(); delete heapI;
        return 0;
    }

    //For doing BYTE code
    if ( !strcmp(argv[2], "-b") ) {
        void* scanner;
        bclex_init(&scanner);
        bcset_in(infile, scanner);
        Function* output;
        int rvalue = bcparse(scanner, output);
        if(rvalue == 1) {
            cout << "Parsing failed" << endl;
            return 1;
        }

        
        Function* print = new Function(); print->isNative = true; print->nativeName = "print()";
        Function* intcast = new Function(); intcast->isNative = true; intcast->nativeName = "intcast()";
        Function* input = new Function(); input->isNative = true; input->nativeName = "input()";

        std::vector<Function*> fVec;
        fVec.push_back(print); fVec.push_back(intcast); fVec.push_back(input);
        for (Function* f : output->functions_) { fVec.push_back(f); }
        output->functions_ = fVec;


        CollectedHeap heap(mem);
        Interpreter inter(heap);
        Value* v = inter.interpret(output);

        delete output; // delete the funciton
        return 0;
    }


    cout << "Invalid Flags";
    return 1;
}
