INC_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/include
LIB_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/libary
TARGET  = test_suit
CC      = gcc
OBJ     = test_suit.o stack_test.o yystack_test.o hash_test.o set_test.o
COMMON_HEADER = test_suit.h
LIBCONFIG = $(LIB_DIR)/llib.a
COMPILING_CFLAGS = -I$(INC_DIR)

all:$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(COMPILING_CFLAGS) $(OBJ) $(LIBCONFIG)
	@rm -rf $(OBJ)
	@echo "\033[31m OK, test_suit is ready, just input ./test_suit to run all test suits....\033[0m"

test_suit.o:  test_suit.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) test_suit.c

stack_test.o: stack_test.c $(COMMON_HEADER) $(INC_DIR)/stack.h
	$(CC) -c $(COMPILING_CFLAGS) stack_test.c

yystack_test.o: yystack_test.c $(COMMON_HEADER) $(INC_DIR)/yystack.h
	$(CC) -c $(COMPILING_CFLAGS) yystack_test.c

hash_test.o: hash_test.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) hash_test.c

set_test.o: set_test.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) set_test.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
