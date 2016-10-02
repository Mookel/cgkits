SYS_INC_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
CC      = gcc
COM_OBJS = com.o
COMPLIB_OBJS    = hash.o set.o syslib.o
LLIB_OBJS       = input.o yy_hook_a.o yy_hook_b.o yy_pstk.o yy_wrap.o yy_init_lex.o yy_init_llama.o yy_init_occs.o yydebug.o
COMPILING_FLAGS = -I$(SYS_INC_DIR) -I$(GC_INC_DIR)
AR = ar
COMPLIB = complib.a
LLIB    = llib.a

all:$(COMPLIB) $(LLIB)

$(COMPLIB): $(COMPLIB_OBJS) $(COM_OBJS)
	rm -rf $(LIB_DIR)/$(COMPLIB)
	$(AR) rv $(LIB_DIR)/$(COMPLIB) $(COMPLIB_OBJS) $(COM_OBJS)
	rm -rf $(COMPLIB_OBJS)

$(LLIB): $(LLIB_OBJS) $(COM_OBJS)
	rm -rf $(LIB_DIR)/$(LLIB)
	$(AR) rv $(LIB_DIR)/$(LLIB) $(LLIB_OBJS) $(COM_OBJS)
	rm -rf $(LLIB_OBJS) $(COM_OBJS)

hash.o: hash.c $(SYS_INC_DIR)/hash.h $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) hash.c

set.o: set.c $(SYS_INC_DIR)/set.h $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) set.c

syslib.o: syslib.c $(SYS_INC_DIR)/syslib.h
	$(CC) -c $(COMPILING_FLAGS) syslib.c

input.o: input.c $(SYS_INC_DIR)/input.h
	$(CC) -c $(COMPILING_FLAGS) input.c

yy_hook_a.o: yy_hook_a.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) yy_hook_a.c

yy_hook_b.o: yy_hook_b.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) yy_hook_b.c

yy_wrap.o: yy_wrap.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) yy_wrap.c

yy_pstk.o: yy_pstk.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) yy_pstk.c

yy_init_lex.o: yy_init_lex.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) yy_init_lex.c

yy_init_llama.o: yy_init_llama.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) yy_init_llama.c

yy_init_occs.o: yy_init_occs.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(COMPILING_FLAGS) yy_init_occs.c	

yydebug.o: yydebug.c $(SYS_INC_DIR)/yydebug.h
	$(CC) -c $(COMPILING_FLAGS) yydebug.c

com.o: com.c $(SYS_INC_DIR)/com.h
	$(CC) -c $(COMPILING_FLAGS) com.c

clean:
	rm -rf $(COMPLIB_OBJS)
	rm -rf $(LLIB_OBJS)
	rm -rf $(COM_OBJS)
	rm -rf $(LIB_DIR)/$(COMPLIB)
	rm -rf $(LIB_DIR)/$(LLIB)



