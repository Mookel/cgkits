#Created by Mookel

INC_SYS_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_SYS_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
GC_LIB_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/lib
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
BIN_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/bin

LEX = $(BIN_DIR)/lex
OCCS = $(BIN_DIR)/occs

CFLAGS = -I$(INC_SYS_DIR) -I$(GC_INC_SYS_DIR) -Wall
CLIB = $(GC_LIB_DIR)/libgc.a $(LIB_DIR)/llib.a -lncurses
CC = gcc

yyexpr: yyout.o lexyy.o
	$(CC) $(CFLAGS) -o yyexpr yyout.o lexyy.o $(CLIB)
	rm -rf *.o

yyout.o : yyout.c
	$(CC) -c $(CFLAGS) yyout.c

yyout.c : expr.y
	$(OCCS) -vl expr.y

lexyy.o : lexyy.c
	$(CC) -c $(CFLAGS) lexyy.c

lexyy.c : expr.lex
	$(LEX) -vl expr.lex

clean:
	rm -rf yyout.c yyout.h
	rm -rf *.o