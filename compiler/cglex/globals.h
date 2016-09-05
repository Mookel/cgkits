//
// Created by Mookel on 16/9/4.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// globals.h : 
//

#ifndef CGKITS_GLOBALS_H
#define CGKITS_GLOBALS_H

#include <stdio.h>
#include <ctype.h>
#include <compiler.h>

#ifdef ALLOC
#define CLASS
#define I(x) x
#else
#define CLASS extern
#define I(x)
#endif

#define MAX_RULE_SIZE 2048  /*Maximum rule size*/

CLASS int  g_verbose         I(=0); /*print statistics*/
CLASS int  g_no_lines        I(=0); /*Suppress #line directives*/
CLASS int  g_unix_stype      I(=0); /*Use unix-style new lines*/
CLASS int  g_pulic_sym       I(=0); /*make static symbols public*/
CLASS char *g_template       I(="lex.m");  /*state-machine driver template.*/
CLASS int  g_actual_lineno   I(= 1);       /*current input line number*/
CLASS int  g_lineno          I(= 1);       /*line number of first line of a multiple-line rule.*/
CLASS char g_input_buffer[MAX_RULE_SIZE];  /*line buffer for input*/
CLASS char *g_input_file_name;             /*input file name.*/
CLASS FILE *g_ifile;                       /*input stream*/
CLASS FILE *g_ofile;                       /*output stream*/

typedef char* (*fp_input_t)(void);
PUBLIC void lerror(int status, char *fmt, ...);
PUBLIC char *get_expr(void);

#undef CLASS
#undef I

#endif //CGKITS_GLOBALS_H
