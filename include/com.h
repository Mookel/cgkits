//
// Created by Mookel on 16/9/5.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// comm.h : 
//

#ifndef CGKITS_COMM_H
#define CGKITS_COMM_H

#include <stdarg.h>

extern  int      com_concat(int size, char *dst, ...);

/*error output*/
extern  int      com_on_ferr(void);
extern  int      com_ferr(char *format, ...);

/*common print function*/
typedef int      (*fp_print_t)(int, ...);
extern  void     com_prnt(fp_print_t fp_prnt, void *fun_arg, char *format, va_list args);
extern  void     com_stop_prnt(void);

#endif //CGKITS_COMM_H
