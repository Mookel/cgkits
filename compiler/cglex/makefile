INC_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
TARGET  = cglex
CC      = gcc
OBJ     = lex.o input.o nfa.o
LIBCONFIG = $(LIB_DIR)/complib.a
COMPILING_CFLAGS = -I$(INC_DIR)

all:$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(COMPILING_CFLAGS) $(OBJ) $(LIBCONFIG) -DDEBUG
	@rm -rf $(OBJ)

lex.o: lex.c globals.h
	$(CC) -c $(COMPILING_CFLAGS) lex.c

input.o: input.c globals.h
	$(CC) -c $(COMPILING_CFLAGS) input.c

nfa.o: nfa.c nfa.h globals.h
	$(CC) -c $(COMPILING_CFLAGS) nfa.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
