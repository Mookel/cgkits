# Created by Mookel on 16/9/30.
# Email : ltp0709@sina.com 
# Copyright (c) 2016 jlu.edu. All rights reserved.

INC_SYS_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_SYS_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
GC_LIB_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/lib
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
BIN_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/bin
LEX = $(BIN_DIR)/lex

CFLAGS = $(PROG) -I$(INC_SYS_DIR) -I$(GC_INC_SYS_DIR) -Wall
CLIB = $(LIB_DIR)/complib.a $(GC_LIB_DIR)/libgc.a $(LIB_DIR)/llib.a
CC = gcc

ifeq ($(bin), )
TARGET = llama
PROG = -DLLAMA
else ifeq ($(bin), llama)
TARGET = llama
PROG = -DLLAMA
else ifeq ($(bin), occs)
TARGET = occs
PROG = -DOCCS
endif

##Compiling target
target: $(TARGET)

COMMOBJ = main.o acts.o lexyy.o error.o first.o stok.o
LLOBJ = lldriver.o llselect.o llfollow.o llcode.o lldollar.o
YYOBJ = yydriver.o yypatch.o yydollar.o yycode.o yystate.o

#LLPAR = llpar.o
LLPAR = llout.o

#-----------------llama---------------------
llama : $(LLPAR) $(LLOBJ) $(COMMOBJ) 
	$(CC) -o llama $(LLPAR) $(COMMOBJ) $(LLOBJ) $(CLIB) -lncurses
	@echo "\033[31mOK, Building is successful.\033[0m"
	cp llama $(BIN_DIR)/.
	@rm -rf *.o

#-----------------occs----------------------
occs: llout.o $(YYOBJ) $(COMMOBJ)
	$(CC) -o occs llout.o $(COMMOBJ) $(YYOBJ) $(CLIB) -lncurses
	@echo "\033[31mOK, Building is successful.\033[0m"
	cp occs $(BIN_DIR)/.
	@rm -rf *.o

#-----------------objects-------------------
main.o: main.c parser.h
	$(CC) -c $(CFLAGS) main.c

acts.o: acts.c parser.h
	$(CC) -c $(CFLAGS) acts.c 

error.o: error.c parser.h
	$(CC) -c $(CFLAGS) error.c

first.o: first.c parser.h
	$(CC) -c $(CFLAGS) first.c

stok.o: stok.c parser.h
	$(CC) -c $(CFLAGS) stok.c

llfollow.o: llfollow.c parser.h
	$(CC) -c $(CFLAGS) llfollow.c

llselect.o: llselect.c parser.h
	$(CC) -c $(CFLAGS) llselect.c

lldriver.o: lldriver.c parser.h
	$(CC) -c $(CFLAGS) lldriver.c

yydriver.o: yydriver.c parser.h
	$(CC) -c $(CFLAGS) yydriver.c

lldollar.o: lldollar.c parser.h
	$(CC) -c $(CFLAGS) lldollar.c

yydollar.o: yydollar.c parser.h
	$(CC) -c $(CFLAGS) yydollar.c

llpar.o: llpar.c parser.h
	$(CC) -c $(CFLAGS) llpar.c

llcode.o:llcode.c parser.h
	$(CC) -c $(CFLAGS) llcode.c

yycode.o: yycode.c parser.h
	$(CC) -c $(CFLAGS) yycode.c

llout.o: llout.c parser.h
	$(CC) -c $(CFLAGS) llout.c

yypatch.o : yypatch.c parser.h
	$(CC) -c $(CFLAGS) yypatch.c

yystate.o : yystate.c parser.h
	$(CC) -c $(CFLAGS) yystate.c

lexyy.o: lexyy.c parser.h
	$(CC) -c $(CFLAGS) lexyy.c

lexyy.c: parser.lex
	$(LEX) -vl parser.lex

clean:
	rm -rf *.o 
	rm -rf lexyy.c
	rm -rf llama occs
