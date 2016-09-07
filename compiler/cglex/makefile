INC_SYS_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_SYS_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
GC_LIB_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/lib
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
TARGET  = cglex
CC      = gcc
OBJ     = lex.o input.o nfa.o terp.o dfa.o mindfa.o print.o
COMM_HEADER = tydef.h globals.h
LIBCONFIG = $(LIB_DIR)/complib.a $(GC_LIB_DIR)/libgc.a 
COMPILING_CFLAGS = -I$(INC_SYS_DIR) -I$(GC_INC_SYS_DIR)

all:$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(COMPILING_CFLAGS) $(OBJ) $(LIBCONFIG) -DDEBUG
	@rm -rf $(OBJ)

lex.o: lex.c $(COMM_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) lex.c

input.o: input.c $(COMM_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) input.c

nfa.o: nfa.c nfa.h $(COMM_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) nfa.c

terp.o: terp.c terp.h $(COMM_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) terp.c

dfa.o: dfa.c dfa.h $(COMM_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) dfa.c

mindfa.o: mindfa.c mindfa.h $(COMM_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) mindfa.c

print.o: print.c print.h $(COMM_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) print.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
