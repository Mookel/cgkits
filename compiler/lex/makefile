# Created by Mookel on 16/9/30.
# Email : ltp0709@sina.com 
# Copyright (c) 2016 jlu.edu. All rights reserved.

INC_SYS_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_SYS_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
GC_LIB_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/lib
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
BIN_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/bin
TARGET  = lex
CC      = gcc
OBJ     = lex.o input.o nfa.o terp.o dfa.o mindfa.o print.o
COMM_HEADER = tydef.h globals.h
LIBCONFIG = $(LIB_DIR)/complib.a $(GC_LIB_DIR)/libgc.a 
CFLAGS = -I$(INC_SYS_DIR) -I$(GC_INC_SYS_DIR) -Wall

all:$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBCONFIG)
	@rm -rf $(BIN_DIR)/$(TARGET)
	@cp $(TARGET) $(BIN_DIR)
	@rm -rf $(TARGET)
	@rm -rf $(OBJ)

lex.o: lex.c $(COMM_HEADER)
	$(CC) -c $(CFLAGS) lex.c

input.o: input.c $(COMM_HEADER)
	$(CC) -c $(CFLAGS) input.c

nfa.o: nfa.c nfa.h $(COMM_HEADER)
	$(CC) -c $(CFLAGS) nfa.c

terp.o: terp.c terp.h $(COMM_HEADER)
	$(CC) -c $(CFLAGS) terp.c

dfa.o: dfa.c dfa.h $(COMM_HEADER)
	$(CC) -c $(CFLAGS) dfa.c

mindfa.o: mindfa.c mindfa.h $(COMM_HEADER)
	$(CC) -c $(CFLAGS) mindfa.c

print.o: print.c print.h $(COMM_HEADER)
	$(CC) -c $(CFLAGS) print.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
