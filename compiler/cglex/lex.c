//
// Created by Mookel on 16/9/4.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// lex.c : 
//

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslib.h>

#define ALLOC
#include "globals.h"

/*macro definitions.*/
#define DTRAN_NAME   "Yy_nxt"
#define E(x)         fprintf(stderr, "%s\n", x)

/*private variables*/
PRIVATE int _column_compress = 1;
PRIVATE int _no_compression  = 1;
PRIVATE int _threshold       = 4;
PRIVATE int _no_header       = 0;
PRIVATE int _header_only     = 0;

/*private functions*/
PRIVATE  void cmd_line_error(int usage, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "cglex: ");
    vfprintf(stderr, fmt, args);

    if(!usage){
        perror("");
    }
    else {
        E("\n\n Usage is:      cglex [options] file");
        E("-f  for (f)ast. Don't compress tables.");
        E("-h  suppress (h)eader comment that describes state machine.");
        E("-H  print the (H)eader only.");
        E("-l  suppress #(l)ine directives in the output.");
        E("-p  make symbols (p)ublic.");
        E("-mS use string S as template name rather than lex.m.");
        E("-cN use pair (c)ompression, N = threshold (default 4).");
        E("-t  Send output to standard out instead lexyy.c");
        E("-u  Unix mode, newline is \\n, not \\n or \\r.");
        E("-v  (v)erbose mode, print statistics.");
        E("-V  More (V)erbose, print internal diagnostics as lex runs." );
    }

    exit(1);
    va_end(args);
}

PRIVATE void strip_comments(char *string)
{
    static int incomment = 0;

    for(; *string; ++string){
        if(incomment){
            if(string[0] == '*' && string[1] == '/'){
                incomment = 0;
                *string++ = ' ';
            }
            if(!isspace(*string)) *string = ' ';
        } else {
            if(string[0] == '/' && string[1] == '*') {
                incomment = 1;
                *string++ = ' ';
                *string++ = ' ';
            }
        }
    }
}

PRIVATE void handle_head(int suppress_out)
{
    int transparent = 0;

    if(!suppress_out && g_pulic_sym)
        fputs("#define YYPRIVATE\n\n", g_ofile);

    if(!g_no_lines)
        fprintf(g_ofile, "#line 1 \"%s\"\n", g_input_file_name);

    while(fgets(g_input_buffer, MAX_RULE_SIZE, g_ifile)){
        ++g_actual_lineno;
        if(!transparent) strip_comments(g_input_buffer);

        if(g_verbose > 1) printf("h%d: %s", g_actual_lineno, g_input_buffer);

        if(g_input_buffer[0] == '%'){
            if(g_input_buffer[1] == '%') {
                if(!suppress_out) fputs("\n", g_ofile);
                break;
            } else {
                if(g_input_buffer[1] == '{') {
                    transparent = 1;
                } else if(g_input_buffer[1] == '}') {
                    transparent = 0;
                } else {
                    lerror(0, "Ignoring illegal %%%%c directive\n", g_input_buffer[1]);
                }
            }
        } else if(transparent || isspace(g_input_buffer[0])) {
            if(!suppress_out) fputs(g_input_buffer, g_ofile);
        } else {
            //new_macro(g_input_buffer);
            if(!suppress_out) fputs("\n", g_ofile);
        }
    }

    //if(g_verbose > 1) print_macs();
}

PRIVATE void handle_tail()
{
    fgets(g_input_buffer, MAX_RULE_SIZE, g_ifile);

    if(!g_no_lines) fprintf(g_ofile, "#line %d \"%s\"", g_actual_lineno + 1, g_input_file_name);

    while(fgets(g_input_buffer, MAX_RULE_SIZE, g_ifile)){
        if(g_verbose > 1) printf("t%d: %s", g_actual_lineno++, g_input_buffer);
        fputs(g_input_buffer, g_ofile);
    }
}

PRIVATE void work()
{
    handle_head(_header_only);

    handle_tail();

}

void lerror(int status, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "cglex, input line %d:  ", g_actual_lineno);
    vfprintf(stderr, fmt, args);
    if(status) exit(status);
    va_end(args);
}

int main(int argc, char **argv)
{
    static char *p;
    static int  use_stdout = 0;

    for(++argv, --argc; argc && *(p = *argv) == '-'; ++argv, --argc){
        while(*++p){
            switch(*p){
                case 'f': _no_compression = 1; break;
                case 'h': _no_header = 1;break;
                case 'H': _header_only = 1; break;
                case 'l': g_no_lines = 1; break;
                case 'm': g_template = p + 1; goto out;
                case 'p': g_pulic_sym = 1; break;
                case 't': use_stdout = 1; break;
                case 'u': g_unix_stype = 1; break;
                case 'v': g_verbose = 1; break;
                case 'V': g_verbose = 2; break;
                case 'c': _column_compress = 0;
                    if(!isdigit(p[1])){
                        _threshold  = 4;
                    } else {
                        _threshold = atoi(++p);
                        while(*p && isdigit(p[1])) ++p;
                    }
                    break;

                default: cmd_line_error(1, "-%c illegal argument.", *p);
                    break;
            }
        }

        out:;
    }

    if(argc > 1){
        cmd_line_error(1, "Too many arguments, Only one file name permitted.");
    } else if(argc <= 0) {
        cmd_line_error(1, "File name required.");
    } else {
        if(g_ifile = fopen(*argv, "r")) {
            g_input_file_name = *argv;
        } else {
            cmd_line_error(0, "Can't open input file %s", *argv);
        }
    }

    if(!use_stdout) {
        if(!(g_ofile = fopen(_header_only ? "lexyy.h" : "lexyy.c", "w")))
            cmd_line_error(0, "Can't open output file lexyy.[ch]");
    }

    work();

    fclose(g_ofile);
    fclose(g_ifile);

    exit(0);

}