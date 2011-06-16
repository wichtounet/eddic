CC = g++
DEBUG = -g
CFLAGS = -Wall -o2 -c $(DEBUG) -Iinclude 
LFLAGS = -Wall $(DEBUG) -leddi-commons 
OBJECTS = bin/eddi.o bin/Compiler.o bin/Lexer.o bin/ByteCodeFileWriter.o bin/CompilerException.o bin/Variables.o

bin/eddic : $(OBJECTS)
	$(CC) $(LFLAGS) -o bin/eddic $(OBJECTS)

bin/eddi.o : src/eddi.cpp include/Compiler.h
	$(CC) $(CFLAGS) -o bin/eddi.o src/eddi.cpp

bin/Compiler.o : include/Compiler.h src/Compiler.cpp include/Lexer.h
	$(CC) $(CFLAGS) -o bin/Compiler.o src/Compiler.cpp

bin/CompilerException.o : include/CompilerException.h src/CompilerException.cpp
	$(CC) $(CFLAGS) -o bin/CompilerException.o src/CompilerException.cpp

bin/Lexer.o : include/Lexer.h src/Lexer.cpp
	$(CC) $(CFLAGS) -o bin/Lexer.o src/Lexer.cpp

bin/ByteCodeFileWriter.o : include/ByteCodeFileWriter.h src/ByteCodeFileWriter.cpp
	$(CC) $(CFLAGS) -o bin/ByteCodeFileWriter.o src/ByteCodeFileWriter.cpp

bin/Variables.o : include/Variables.h src/Variables.cpp
	$(CC) $(CFLAGS) -o bin/Variables.o src/Variables.cpp

clean:
	rm -f bin/*
	rm -f samples/*.v

