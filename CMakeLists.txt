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
    include/yystack.h
    testsuit/stack_test.c
    testsuit/test_suit.c
    testsuit/test_suit.h
    testsuit/yystack_test.c compiler/lib/set.c include/set.h include/syslib.h compiler/lib/syslib.c compiler/lib/yylib.c include/yylib.h compiler/lib/input.c include/input.h include/l.h compiler/lib/lex_io.c compiler/cglex/globals.h compiler/cglex/lex.c compiler/lib/com.c include/com.h compiler/cglex/input.c compiler/cglex/nfa.h compiler/cglex/nfa.c)

include_directories(.)
include_directories(compiler)
include_directories(compiler/C)
include_directories(compiler/cglex)
include_directories(compiler/lib)
include_directories(compiler/parser)
include_directories(include)

add_executable(cgkits ${SOURCE_FILES})