all: mitscript mitscriptc

#################################################################################################################################
#### The commands below make the Interpeter with its main in bc-parser-driver.cpp

mitscript: bc-lexer.cpp bc-parser.cpp parser.cpp lexer.cpp AST.o Compiler.o Interpreter.o
	g++ -g -std=c++1y bc-parser-driver.cpp bc-parser.cpp bc-lexer.cpp ./sourceScanner/AST.o ./binaries/Compiler.o ./binaries/Interpreter.o ./sourceScanner/lexer.cpp ./sourceScanner/parser.cpp -o ./binaries/mitscript

Interpreter.o: Interpreter.cpp Interpreter.h
	g++ -g -c -std=c++1y Interpreter.cpp -o ./binaries/Interpreter.o

bc-parser.cpp: bc-parser.yy
	bison -Dapi.prefix={bc} --output=bc-parser.cpp --defines=bc-parser.h bc-parser.yy

bc-lexer.cpp: bc-lexer.lex
	flex -P bc --outfile=bc-lexer.cpp --header-file=bc-lexer.h bc-lexer.lex


#############################################################################################################################
#### The commands below make the compiler and AST.o - they also make an mitscriptc binary with its main in parser-driver.cpp

mitscriptc: AST.o Compiler.o lexer.cpp parser.cpp
	g++ -g -std=c++1y ./sourceScanner/lexer.cpp ./sourceScanner/parser.cpp ./sourceScanner/AST.o ./binaries/Compiler.o parser-driver.cpp -o ./binaries/mitscriptc
 
Compiler.o: Compiler.cpp Compiler.h
	g++ -g -c -std=c++1y Compiler.cpp -o ./binaries/Compiler.o

lexer.cpp: ./sourceScanner/lexer.lex
	flex  --outfile=./sourceScanner/lexer.cpp --header-file=./sourceScanner/lexer.h ./sourceScanner/lexer.lex

parser.cpp: ./sourceScanner/parser.yy
	bison --output=./sourceScanner/parser.cpp --defines=./sourceScanner/parser.h -v ./sourceScanner/parser.yy

AST.o: ./sourceScanner/AST.h ./sourceScanner/AST.cpp ./sourceScanner/Visitor.h
	g++ -g -c -std=gnu++1y ./sourceScanner/AST.cpp -o ./sourceScanner/AST.o

clean: 
	rm -f bc-lexer.cpp bc-lexer.h bc-parser.cpp bc-parser.h bc-parser.output bc-parser *.exe
	rm -f ./binaries/* 
	rm -f ./sourceScanner/AST.o ./sourceScanner/lexer.cpp ./sourceScanner/lexer.h ./sourceScanner/parser.cpp ./sourceScanner/parser.h ./sourceScanner/parser.output
