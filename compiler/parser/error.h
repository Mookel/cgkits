//
// Created by Mookel on 16/9/22.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// error.h : 
//

#ifndef CGKITS_ERROR_H
#define CGKITS_ERROR_H

#include <stdio.h>

/*error type definitions */
#define NONFATAL          0
#define FATAL             1
#define WARNING           2
#define NOHDR             3

void e_init();
FILE *e_get_doc();
void e_output(char *fmt, ...);
void e_doc(char *fmt, ...);
void e_doc_to(FILE *fp);
void e_lerror(int fatal, char *fmt, ...);
void e_error(int fatal, char *fmt, ...);
int e_nerrors();
int e_nwarnings();
const char *e_open_errmsg();

#endif //CGKITS_ERROR_H
