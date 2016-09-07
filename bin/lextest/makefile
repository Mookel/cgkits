INC_SYS_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/include
GC_LIB_DIR  = /Users/mookel/Workspace/CompilerPro/private/cgkits/gc/lib
LIB_DIR = /Users/mookel/Workspace/CompilerPro/private/cgkits/libary
LEX = /Users/mookel/Workspace/CompilerPro/private/cgkits/bin/cglex

LIB_CFG = $(LIB_DIR)/llib.a -lncurses
TARGET = lextext
CFLAGS = -I$(INC_SYS_DIR) -DYYDEBUG

$(TARGET): main.o lexyy.o token.h
	$(CC) $(CFLAGS) -o $(TARGET)  main.o lexyy.o $(LIB_CFG)

main.o: main.c
	$(CC) -c main.c

lexyy.o: lexyy.c
lexyy.c: test.lex
	$(LEX) test.lex


clean:
	rm -rf *.o
	rm -rf $(TARGET)
	rm -rf lexyy.c
