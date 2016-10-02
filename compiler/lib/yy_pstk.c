//
// Created by Mookel on 16/10/2.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yy_pstk.c : 
//

#include <debug.h>
#include <stdio.h>

PUBLIC char *yy_pstk(void *val, char *sym)
{
    static char buf[32];
    sprintf(buf, "%d", *(int *) val);
    return buf;
}