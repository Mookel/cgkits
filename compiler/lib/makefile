INC_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/include
TARGET  = test_suit
LIB_DIR = /Users/mookel/Workspace/CompilerPro/cgkits/libary
CC      = gcc
COMPLIB_OBJS    = hash.o set.o syslib.o input.o
COMPILING_FLAGS = -I$(INC_DIR) 
AR = ar
COMPLIB = complib.a

all:$(COMPLIB)

$(COMPLIB): $(COMPLIB_OBJS)
	rm -rf $(LIB_DIR)/$(COMPLIB)
	$(AR) -r $(LIB_DIR)/$(COMPLIB) $(COMPLIB_OBJS)
	rm -rf $(COMPLIB_OBJS)

hash.o: hash.c $(INC_DIR)/hash.h $(INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) hash.c

set.o: set.c $(INC_DIR)/set.h $(INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) set.c

syslib.o: syslib.c $(INC_DIR)/syslib.h
	$(CC) -c $(COMPILING_FLAGS) syslib.c

input.o: input.c $(INC_DIR)/input.h
	$(CC) -c $(COMPILING_FLAGS) input.c

clean:
	rm -rf $(COMPLIB_OBJS)
	rm -rf $(LIB_DIR)/$(COMPLIB)


