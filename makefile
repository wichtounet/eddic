CC = g++
DEBUG = 
CFLAGS = -Wall -Wextra -o3 -c $(DEBUG) -Iinclude 
LFLAGS = -Wall -Wextra $(DEBUG) -leddi-commons 
OBJECTS = bin/eddi.o bin/Compiler.o bin/Lexer.o bin/ByteCodeFileWriter.o bin/CompilerException.o bin/Variables.o bin/Parser.o bin/ParseNode.o bin/Nodes.o bin/StringPool.o bin/Options.o bin/Operators.o bin/Branches.o

bin/eddic : $(OBJECTS)
	$(CC) $(LFLAGS) -o bin/eddic $(OBJECTS)

bin/eddi.o : src/eddi.cpp include/Compiler.hpp
	$(CC) $(CFLAGS) -o bin/eddi.o src/eddi.cpp

bin/Compiler.o : include/Compiler.hpp src/Compiler.cpp include/Lexer.hpp include/Parser.hpp
	$(CC) $(CFLAGS) -o bin/Compiler.o src/Compiler.cpp

bin/CompilerException.o : include/CompilerException.hpp src/CompilerException.cpp
	$(CC) $(CFLAGS) -o bin/CompilerException.o src/CompilerException.cpp

bin/Lexer.o : include/Lexer.hpp src/Lexer.cpp
	$(CC) $(CFLAGS) -o bin/Lexer.o src/Lexer.cpp

bin/ByteCodeFileWriter.o : include/ByteCodeFileWriter.hpp src/ByteCodeFileWriter.cpp
	$(CC) $(CFLAGS) -o bin/ByteCodeFileWriter.o src/ByteCodeFileWriter.cpp

bin/Variables.o : include/Variables.hpp src/Variables.cpp
	$(CC) $(CFLAGS) -o bin/Variables.o src/Variables.cpp

bin/Parser.o : include/Parser.hpp src/Parser.cpp include/Nodes.hpp include/ParseNode.hpp include/Operators.hpp include/Branches.hpp
	$(CC) $(CFLAGS) -o bin/Parser.o src/Parser.cpp

bin/ParseNode.o : include/ParseNode.hpp src/ParseNode.cpp
	$(CC) $(CFLAGS) -o bin/ParseNode.o src/ParseNode.cpp

bin/Nodes.o : include/Nodes.hpp src/Nodes.cpp include/ParseNode.hpp
	$(CC) $(CFLAGS) -o bin/Nodes.o src/Nodes.cpp

bin/Operators.o : include/Operators.hpp src/Operators.cpp include/Nodes.hpp include/ParseNode.hpp
	$(CC) $(CFLAGS) -o bin/Operators.o src/Operators.cpp

bin/Branches.o : include/Branches.hpp src/Branches.cpp include/Nodes.hpp include/ParseNode.hpp
	$(CC) $(CFLAGS) -o bin/Branches.o src/Branches.cpp

bin/StringPool.o : include/StringPool.hpp src/StringPool.cpp
	$(CC) $(CFLAGS) -o bin/StringPool.o src/StringPool.cpp

bin/Options.o : include/Options.hpp src/Options.cpp
	$(CC) $(CFLAGS) -o bin/Options.o src/Options.cpp

clean:
	rm -f bin/*
	rm -f samples/*.v

full: clean bin/eddic
	
