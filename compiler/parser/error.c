//
// Created by Mookel on 16/9/22.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// error.c : 
//

#include "parser.h"
#include "error.h"
#include <stdlib.h>
#include <errno.h>

PRIVATE int  num_warings_ = 0;            /*total warnings printed.*/
PRIVATE int  num_errors_ = 0;
PRIVATE FILE *_doc_file = NULL;           /*yyout.doc, Error log & machine description*/

void e_doc_file(FILE *fp)
{
    _doc_file = fp;
}

void e_output(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(g_output, fmt, args);
}

/*works like printf(), but writes to yyout.doc.*/
void e_doc(char *fmt, ...)
{
    va_list args;

    if(_doc_file) {
        va_start(args, fmt);
        vfprintf(_doc_file, fmt, args);
    }
}

void e_doc_to(FILE *fp)
{
    static FILE *oldfp;

    if(fp) {
        oldfp = _doc_file;
        _doc_file = fp;
    } else {
        _doc_file = oldfp;
    }
}

void e_lerror(int fatal, char *fmt, ...)
{
    va_list args;
    extern int yylineno;
    if(fatal == WARNING) {
        ++num_warings_;
        if(g_cmdopt.no_warnings) return;
        fprintf(stdout, "%s WARNING (%s, line %d): ", PROG_NAME, g_input_file_name, yylineno);
    } else if(fatal != NOHDR) {
        ++num_errors_;
        fprintf(stdout, "%s ERROR (%s, line %d): ", PROG_NAME, g_input_file_name, yylineno);
    }

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fflush(stdout);

#ifndef LLAMA
    if(g_cmdopt.verbose && _doc_file) {
        if(fatal != NOHDR) fprintf(_doc_file, "%s (line %d) ", fatal == WARNING ?
            "WARNING"  : "ERROR", yylineno);
        vfprintf(_doc_file, fmt, args);
    }
#endif

    if(fatal == FATAL) exit(EXIT_OTHER);
}

void e_error(int fatal, char *fmt, ...)
{
    va_list args;
    const char *type;

    if(fatal == WARNING) {
        ++num_warings_;
        if (g_cmdopt.no_warnings) return;
        type = "WARNING: ";
        fprintf(stdout, type);
    } else if(fatal != NOHDR) {
        ++num_errors_;
        type = "ERROR: ";
        fprintf(stdout, type);
    }

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fflush(stdout);

    OX(if(g_cmdopt.verbose && _doc_file))
    OX({)
    OX(   vfprintf(_doc_file, type);)
    OX(   vfprintf(_doc_file, fmt, args); )
    OX(})

    if(fatal == FATAL) exit(EXIT_OTHER);

}

int e_nerrors()
{
    return num_errors_;
}

int e_nwarnings()
{
    return num_warings_;
}

const char *e_open_errmsg(void)
{
    switch(errno) {
        case EACCES: return "File is read only or a directory";
        case EEXIST: return "File already exists";
        case EMFILE: return "Too many open files";
        case ENOENT: return "File not found";
        default:     return "Reason unknown";
    }
}