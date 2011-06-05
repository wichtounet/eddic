OBJS = main.o Compiler.o Lexer.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

eddic : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o eddic

main.o : main.cpp Compiler.h
	$(CC) $(CFLAGS) main.cpp

Compiler.o : Compiler.h Compiler.cpp Lexer.h
	$(CC) $(CFLAGS) Compiler.cpp

Lexer.o : Lexer.h Lexer.cpp
	$(CC) $(CFLAGS) Lexer.cpp

clean:
	\rm *.o eddic

