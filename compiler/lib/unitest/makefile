INC_SYS_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
GC_LIB_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/lib
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
TARGET  = test_suit
CC      = gcc
OBJ     = test_suit.o stack_test.o yystack_test.o hash_test.o set_test.o syslib_test.o yylib_test.o
COMMON_HEADER = test_suit.h
LIBCONFIG = $(LIB_DIR)/complib.a $(LIB_DIR)/llib.a $(GC_LIB_DIR)/libgc.a -lncurses
COMPILING_CFLAGS = -I$(INC_SYS_DIR) -I$(GC_INC_DIR)

all:$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(COMPILING_CFLAGS) $(OBJ) $(LIBCONFIG) -DDEBUG
	@rm -rf $(OBJ)
	@echo "\033[31m OK, test_suit is ready, just input ./test_suit to run all test suits....\033[0m"

test_suit.o:  test_suit.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) test_suit.c

stack_test.o: stack_test.c $(COMMON_HEADER) $(INC_SYS_DIR)/stack.h
	$(CC) -c $(COMPILING_CFLAGS) stack_test.c

yystack_test.o: yystack_test.c $(COMMON_HEADER) $(INC_SYS_DIR)/yystack.h
	$(CC) -c $(COMPILING_CFLAGS) yystack_test.c

hash_test.o: hash_test.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) hash_test.c

set_test.o: set_test.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) set_test.c

syslib_test.o: syslib_test.c $(COMMON_HEADER) $(INC_SYS_DIR)/syslib.h
	$(CC) -c $(COMPILING_CFLAGS) syslib_test.c

yylib_test.o : yylib_test.c $(COMMON_HEADER) $(INC_SYS_DIR)/yylib.h
	$(CC) -c $(COMPILING_CFLAGS) yylib_test.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
