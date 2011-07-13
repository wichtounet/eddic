cmake_minimum_required(VERSION 2.8)

project("EDDI Compiler")

#Activate warnings
if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long")
endif()

include_directories(include)

add_executable(
	bin/eddic
	src/Branches.cpp src/eddi.cpp src/Options.cpp src/Variables.cpp src/ByteCodeFileWriter.cpp src/Lexer.cpp src/ParseNode.cpp src/Compiler.cpp src/Nodes.cpp src/Parser.cpp src/CompilerException.cpp src/Operators.cpp src/StringPool.cpp
)

target_link_libraries(bin/eddic eddi-commons)
