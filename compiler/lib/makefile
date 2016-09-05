INC_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
TARGET  = test_suit
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
CC      = gcc
COM_OBJS = com.o
COMPLIB_OBJS    = hash.o set.o syslib.o
LLIB_OBJS       = input.o yylib.o 
COMPILING_FLAGS = -I$(INC_DIR) 
AR = ar
COMPLIB = complib.a
LLIB    = llib.a

all:$(COMPLIB) $(LLIB)

$(COMPLIB): $(COMPLIB_OBJS) $(COM_OBJS)
	rm -rf $(LIB_DIR)/$(COMPLIB)
	$(AR) -r $(LIB_DIR)/$(COMPLIB) $(COMPLIB_OBJS)
	rm -rf $(COMPLIB_OBJS) $(COM_OBJS)

$(LLIB): $(LLIB_OBJS) $(COM_OBJS)
	rm -rf $(LIB_DIR)/$(LLIB)
	$(AR) -r $(LIB_DIR)/$(LLIB) $(LLIB_OBJS)
	rm -rf $(LLIB_OBJS) $(COM_OBJS)

hash.o: hash.c $(INC_DIR)/hash.h $(INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) hash.c

set.o: set.c $(INC_DIR)/set.h $(INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) set.c

syslib.o: syslib.c $(INC_DIR)/syslib.h
	$(CC) -c $(COMPILING_FLAGS) syslib.c

input.o: input.c $(INC_DIR)/input.h
	$(CC) -c $(COMPILING_FLAGS) input.c

yylib.o: yylib.c $(INC_DIR)/yylib.h
	$(CC) -c $(COMPILING_FLAGS) yylib.c

com.o: com.c $(INC_DIR)/com.h
	$(CC) -c $(COMPILING_FLAGS) com.c

clean:
	rm -rf $(COMPLIB_OBJS)
	rm -rf $(LLIB_OBJS)
	rm -rf $(COM_OBJS)
	rm -rf $(LIB_DIR)/$(COMPLIB)
	rm -rf $(LIB_DIR)/$(LLIB)



