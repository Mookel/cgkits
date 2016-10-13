# Created by Mookel on 16/9/30.
# Email : ltp0709@sina.com 
# Copyright (c) 2016 jlu.edu. All rights reserved.
SYS_INC_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_INC_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/include
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
CC      = gcc
COM_OBJS = com.o
COMPLIB_OBJS    = hash.o set.o syslib.o
LLIB_OBJS       = input.o yy_hook_a.o yy_hook_b.o yy_pstk.o yy_wrap.o yy_init_lex.o yy_init_llama.o yy_init_occs.o yydebug.o
CFLAGS = -I$(SYS_INC_DIR) -I$(GC_INC_DIR) -Wall
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
	$(CC) -c $(CFLAGS) hash.c

set.o: set.c $(SYS_INC_DIR)/set.h $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) set.c

syslib.o: syslib.c $(SYS_INC_DIR)/syslib.h
	$(CC) -c $(CFLAGS) syslib.c

input.o: input.c $(SYS_INC_DIR)/input.h
	$(CC) -c $(CFLAGS) input.c

yy_hook_a.o: yy_hook_a.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) yy_hook_a.c

yy_hook_b.o: yy_hook_b.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) yy_hook_b.c

yy_wrap.o: yy_wrap.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) yy_wrap.c

yy_pstk.o: yy_pstk.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) yy_pstk.c

yy_init_lex.o: yy_init_lex.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) yy_init_lex.c

yy_init_llama.o: yy_init_llama.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) yy_init_llama.c

yy_init_occs.o: yy_init_occs.c $(SYS_INC_DIR)/debug.h
	$(CC) -c $(CFLAGS) yy_init_occs.c	

yydebug.o: yydebug.c $(SYS_INC_DIR)/yydebug.h
	$(CC) -c $(CFLAGS) yydebug.c

com.o: com.c $(SYS_INC_DIR)/com.h
	$(CC) -c $(CFLAGS) com.c

clean:
	rm -rf $(COMPLIB_OBJS)
	rm -rf $(LLIB_OBJS)
	rm -rf $(COM_OBJS)
	rm -rf $(LIB_DIR)/$(COMPLIB)
	rm -rf $(LIB_DIR)/$(LLIB)



