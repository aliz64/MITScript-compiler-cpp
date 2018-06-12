#include <iostream>
#include "parser.h"
#include "lexer.h"
#include "Interpreter.h"
#include "PrettyPrinter.h"

using namespace std;

/*!
The main method of the parser and interpreter (currently updated for assignment 2)
*/
int main(int argc, char** argv) {
        void* scanner;
        yylex_init(&scanner);
        if (argc < 2) {
                cout << "Expecting file name as argumeent" << endl;
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
                Interpreter interp;
                output->accept(interp);
        }
        catch (InterpreterException& exception) {
                cout << exception.message() << endl;
                return 1;
        }
        return 0;
}
