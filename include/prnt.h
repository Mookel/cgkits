//
// Created by Mookel on 16/8/29.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// prnt.h : 
//

#ifndef CGKITS_PRNT_H
#define CGKITS_PRNT_H

#include <stdarg.h>

typedef int (*pf_prnt)(int, ...);
void prnt(pf_prnt pfunc, void *funct_arg, char *format, va_list, args);

#endif //CGKITS_PRNT_H
