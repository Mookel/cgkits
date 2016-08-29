INC_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/include
LIB_SRC = /Users/mookel/Workspace/CompilerPro/cgkits/compiler/lib
TARGET  = test_suit
CC      = gcc
OBJ     = test_suit.o stack_test.o yystack_test.o hash_test.o hash.o
COMMON_HEADER = test_suit.h

COMPILING_CFLAGS = -I$(INC_DIR)

all:$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(COMPILING_CFLAGS) $(OBJ)

test_suit.o:  test_suit.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) test_suit.c

stack_test.o: stack_test.c $(COMMON_HEADER) $(INC_DIR)/stack.h 
	$(CC) -c $(COMPILING_CFLAGS) stack_test.c

yystack_test.o: yystack_test.c $(COMMON_HEADER) $(INC_DIR)/yystack.h
	$(CC) -c $(COMPILING_CFLAGS) yystack_test.c

hash_test.o: hash_test.c $(COMMON_HEADER) $(INC_DIR)/hash.h
	$(CC) -c $(COMPILING_CFLAGS) hash_test.c

hash.o: $(LIB_SRC)/hash.c $(COMMON_HEADER) $(INC_DIR)/hash.h $(INC_DIR)/debug.h
	$(CC) -c $(COMPILING_CFLAGS) $(LIB_SRC)/hash.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
