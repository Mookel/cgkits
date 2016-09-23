cmake_minimum_required(VERSION 3.5)
project(cgkits)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    compiler/lib/hash.c
    include/compiler.h
    include/debug.h
    include/hash.h
    include/stack.h
    include/win.h
    include/yystack.h gc/include/gc.h
    compiler/lib/unitest/stack_test.c compiler/lib/unitest/test_suit.c compiler/lib/unitest/test_suit.h compiler/lib/unitest/yystack_test.c compiler/lib/set.c include/set.h include/syslib.h compiler/lib/syslib.c compiler/lib/yylib.c include/yylib.h compiler/lib/input.c include/input.h include/l.h compiler/lib/lex_io.c compiler/lex/globals.h compiler/lex/lex.c compiler/lib/com.c include/com.h compiler/lex/input.c compiler/lex/nfa.h compiler/lex/nfa.c compiler/lex/terp.c compiler/lex/terp.h compiler/lex/input.h compiler/lex/tydef.h compiler/lex/dfa.h compiler/lex/dfa.c compiler/lex/mindfa.h compiler/lex/mindfa.c compiler/lex/print.h compiler/lex/print.c include/yydebug.h compiler/lib/yydebug.c compiler/parser/parser.h compiler/parser/main.c compiler/parser/error.h compiler/parser/error.c)

include_directories(.)
include_directories(compiler)
include_directories(compiler/C)
include_directories(compiler/lex)
include_directories(compiler/lib)
include_directories(gc/include)
include_directories(compiler/parser)
include_directories(include)

add_executable(cgkits ${SOURCE_FILES})