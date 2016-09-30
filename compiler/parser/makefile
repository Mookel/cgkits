INC_SYS_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_SYS_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
GC_LIB_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/lib
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
BIN_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/bin
LEX = $(BIN_DIR)/lex

CFLAGS = $(PROG) -I$(INC_SYS_DIR) -I$(GC_INC_SYS_DIR)
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

COMMOBJ = main.o acts.o lexyy.o error.o first.o
LLOBJ = lldriver.o follow.o select.o
YYOBJ =

LLPAR = llpar.o
#LLPAR = llout.o

#-----------------llama---------------------
llama : $(LLPAR) $(LLOBJ) $(COMMOBJ) 
	$(CC) -o llama $(LLPAR) $(COMMOBJ) $(LLOBJ) $(CLIB) -lncurses
	@echo "Building target successful."
	@rm -rf *.o

#-----------------occs----------------------


#-----------------objects-------------------
main.o: main.c parser.h
	$(CC) -c $(CFLAGS) main.c

acts.o: acts.c parser.h
	$(CC) -c $(CFLAGS) acts.c 

error.o: error.c error.h parser.h
	$(CC) -c $(CFLAGS) error.c

first.o: first.c first.h parser.h
	$(CC) -c $(CFLAGS) first.c

follow.o: follow.c follow.h parser.h
	$(CC) -c $(CFLAGS) follow.c

select.o: select.c select.h parser.h
	$(CC) -c $(CFLAGS) select.c

lldriver.o: lldriver.c lldriver.h parser.h
	$(CC) -c $(CFLAGS) lldriver.c

llpar.o: llpar.c llpar.h parser.h
	$(CC) -c $(CFLAGS) llpar.c

lexyy.o: lexyy.c parser.h
	$(CC) -c $(CFLAGS) lexyy.c

lexyy.c: parser.lex
	$(LEX) -vl parser.lex

clean:
	rm -rf *.o
	rm -rf lexyy.c
	rm -rf $(TARGET)