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

COMMOBJ = main.o acts.o lexyy.o error.o first.o stok.o
LLOBJ = lldriver.o llselect.o llfollow.o llcode.o
YYOBJ =

LLPAR = llpar.o
#LLPAR = llout.o

#-----------------llama---------------------
llama : $(LLPAR) $(LLOBJ) $(COMMOBJ) 
	$(CC) -o llama $(LLPAR) $(COMMOBJ) $(LLOBJ) $(CLIB) -lncurses
	@echo "\033[31mOK, Building is successful.\033[0m"
	@rm -rf *.o

#-----------------occs----------------------


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

llpar.o: llpar.c parser.h
	$(CC) -c $(CFLAGS) llpar.c

llcode.o:llcode.c parser.h
	$(CC) -c $(CFLAGS) llcode.c

lexyy.o: lexyy.c parser.h
	$(CC) -c $(CFLAGS) lexyy.c

lexyy.c: parser.lex
	$(LEX) -vl parser.lex

clean:
	rm -rf *.o 
	rm -rf lexyy.c
	rm -rf $(TARGET)
