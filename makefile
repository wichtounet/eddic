CC = g++
DEBUG = -g
CFLAGS = -Wall -o2 -c $(DEBUG) -Iinclude 
LFLAGS = -Wall $(DEBUG) -leddi-commons 
OBJECTS = bin/eddi.o bin/Compiler.o bin/Lexer.o bin/ByteCodeFileWriter.o bin/CompilerException.o bin/Variables.o bin/Parser.o bin/ParseNode.o bin/Nodes.o bin/StringPool.o bin/Options.o bin/Operators.o

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

bin/Parser.o : include/Parser.h src/Parser.cpp
	$(CC) $(CFLAGS) -o bin/Parser.o src/Parser.cpp

bin/ParseNode.o : include/ParseNode.h src/ParseNode.cpp
	$(CC) $(CFLAGS) -o bin/ParseNode.o src/ParseNode.cpp

bin/Nodes.o : include/Nodes.h src/Nodes.cpp
	$(CC) $(CFLAGS) -o bin/Nodes.o src/Nodes.cpp

bin/Operators.o : include/Nodes.h include/Operators.h src/Operators.cpp
	$(CC) $(CFLAGS) -o bin/Operators.o src/Operators.cpp

bin/StringPool.o : include/StringPool.h src/StringPool.cpp
	$(CC) $(CFLAGS) -o bin/StringPool.o src/StringPool.cpp

bin/Options.o : include/Options.h src/Options.cpp
	$(CC) $(CFLAGS) -o bin/Options.o src/Options.cpp

clean:
	rm -f bin/*
	rm -f samples/*.v

full: clean bin/eddic
	
