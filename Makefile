bminor: main.o scanner.o parser.o decl.o stmt.o expr.o type.o param_list.o hash_table.o symbol.o scope.o scratch.o label.o library.o
	gcc main.o scanner.o parser.o decl.o stmt.o expr.o type.o param_list.o hash_table.o symbol.o scope.o library.o -o bminor

main.o: main.c token.h
	gcc main.c -c -o main.o

parser.o: parser.c token.h
	gcc parser.c -c -o parser.o

scanner.o: scanner.c token.h
	gcc scanner.c -c -o scanner.o

decl.o: decl.c decl.h
	gcc decl.c -c -o decl.o

stmt.o: stmt.c stmt.h
	gcc stmt.c -c -o stmt.o

expr.o: expr.c expr.h
	gcc expr.c -c -o expr.o

type.o: type.c type.h
	gcc type.c -c -o type.o

library.o: library.c library.h
	gcc library.c -c -o library.o

param_list.o: param_list.c param_list.h
	gcc param_list.c -c -o param_list.o

label.o: label.c label.h
	gcc label.c -c -o label.o

scratch.o: scratch.c scratch.h
	gcc scratch.c -c -o scratch.o

scope.o: scope.c scope.h symbol.o
	gcc scope.c -c -o scope.o

symbol.o: symbol.c symbol.h
	gcc symbol.c -c -o symbol.o

hash_table.o: hash_table.c hash_table.h
	gcc hash_table.c -c -o hash_table.o

scanner.c: scanner.flex
	flex -o scanner.c scanner.flex

parser.c token.h: parser.bison
	bison --defines=token.h --output=parser.c -v -t parser.bison

clean:
	rm -f scanner.c scanner.o token.h parser.c *.o parser.output parser.tab.bison bminor

