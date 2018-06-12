parser: main.cpp parser.cpp lexer.cpp Visitor.h PrettyPrinter.h AST.o Interpreter.o
	g++ -O2 -g -std=gnu++11 main.cpp parser.cpp lexer.cpp AST.o Interpreter.o -o mitscript

Interpreter.o: Interpreter.h Interpreter.cpp Value.h Visitor.h
	g++ -O2 -c -std=gnu++11 Interpreter.cpp

AST.o: AST.h AST.cpp Visitor.h
	g++ -O2 -c -std=gnu++11 AST.cpp

parser.cpp: parser.yy
	bison --output=parser.cpp --defines=parser.h -v parser.yy

lexer.cpp: lexer.lex
	flex  --outfile=lexer.cpp --header-file=lexer.h lexer.lex

clean:
	rm -f lexer.cpp lexer.h parser.cpp parser.h parser.output *.exe *.o mitscript
