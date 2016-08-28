INC_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/include
TARGET  = test_suit
CC      = gcc
OBJ     = test_suit.o stack_test.o
COMMON_HEADER = test_suit.h

COMPILING_CFLAGS = -I$(INC_DIR)

all:$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(COMPILING_CFLAGS) $(OBJ)

test_suit.o:  test_suit.c $(COMMON_HEADER)
	$(CC) -c $(COMPILING_CFLAGS) test_suit.c

stack_test.o: stack_test.c $(COMMON_HEADER) $(INC_DIR)/stack.h 
	$(CC) -c $(COMPILING_CFLAGS) stack_test.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
