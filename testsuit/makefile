INC_DIR = ../include

TARGET = test_suit

CC = gcc

OBJ = test_suit.o stack_test.o

COMPILING_CFLAGS = -I$(INC_DIR)

TEST_FLAG = -DTEST_STACK

all:$(TARGET)

$(TARGET) : $(OBJ) 
	$(CC) -o $@ $(OBJ)

test_suit.o : test_suit.c
	$(CC) -c $(COMPILING_CFLAGS) test_suit.c
	
stack_test.o : stack_test.c $(INC_DIR)/stack.h
	$(CC) -c $(COMPILING_CFLAGS) stack_test.c

clean:
	rm -rf $(OBJ)
	rm -rf $(TARGET)
