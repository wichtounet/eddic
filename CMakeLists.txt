cmake_minimum_required(VERSION 2.8)

#Déclaration du projet
project("EDDI Compiler")

include_directories(include)

#Déclaration de l'exécutable
add_executable(
	eddic
	src/Branches.cpp src/eddi.cpp src/Options.cpp src/Variables.cpp src/ByteCodeFileWriter.cpp src/Lexer.cpp src/ParseNode.cpp src/Compiler.cpp src/Nodes.cpp src/Parser.cpp src/CompilerException.cpp src/Operators.cpp src/StringPool.cpp
)

target_link_libraries(eddic eddi-commons)
