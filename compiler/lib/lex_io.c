//
// Created by Mookel on 16/9/3.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// lex_io.c :  Contains two functions that replace the ones in debug.c
// linking this file to a lex-generated lexical analyzer when an
// occs-generated parse is not present.
// yycomment() for messages to stdout, and yyerror() for messages to
// stdout.
//

#include <debug.h>
#include <stdio.h>
#include <stdarg.h>

PUBLIC void yy_comment(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

PUBLIC void yy_error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "ERROR on line %d, near  <%s>\n", yylineno, yytext);
    vfprintf(stderr, fmt, args);
    va_end(args);
}