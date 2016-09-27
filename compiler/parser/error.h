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
FILE *get_doc();
void output(char *fmt, ...);
void doc(char *fmt, ...);
void doc_to(FILE *fp);
void lerror(int fatal, char *fmt, ...);
void error(int fatal, char *fmt, ...);
int  nerrors();
int  nwarnings();
const char *open_errmsg();

#endif //CGKITS_ERROR_H
