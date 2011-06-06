CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

eddic : eddi.o Compiler.o Lexer.o
	$(CC) $(LFLAGS) -o bin/eddic bin/eddi.o bin/Compiler.o bin/Lexer.o

eddi.o : src/eddi.cpp src/Compiler.h
	$(CC) $(CFLAGS) -o bin/eddi.o src/eddi.cpp

Compiler.o : src/Compiler.h src/Compiler.cpp src/Lexer.h src/ByteCode.h
	$(CC) $(CFLAGS) -o bin/Compiler.o src/Compiler.cpp

Lexer.o : src/Lexer.h src/Lexer.cpp
	$(CC) $(CFLAGS) -o bin/Lexer.o src/Lexer.cpp

clean:
	rm -f bin/*

