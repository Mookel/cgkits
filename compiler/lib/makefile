INC_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/include
TARGET  = test_suit
LIB_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/libary
CC      = gcc
OBJS    = hash.o set.o
COMPILING_FLAGS = -I$(INC_DIR) 
AR = ar
LIB_OBJ = llib.a

all:$(LIB_OBJ)

$(LIB_OBJ): $(OBJS)
	rm -rf $(LIB_DIR)/$(LIB_OBJ)
	$(AR) -r $(LIB_DIR)/$(LIB_OBJ) $(OBJS)
	rm -rf $(OBJS)

hash.o: hash.c $(INC_DIR)/hash.h $(INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) hash.c

set.o: set.c $(INC_DIR)/set.h $(INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) set.c


clean:
	rm -rf $(OBJS)
	rm -rf $(LIB_DIR)/$(LIB_OBJ)


