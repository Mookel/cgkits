//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yylib.c : 
//

#include <stdio.h>
#include <debug.h>

PUBLIC void yy_hook_a()
{

}

PUBLIC void yy_hook_b()
{

}

PUBLIC int yy_wrap()
{
    return 1;
}

PUBLIC char *yy_pstk(void *val, char *sym)
{
    static char buf[32];
    sprintf(buf, "%d", *(int *) val);
    return buf;
}

PUBLIC void yy_init_llama(void *tos)
{

}

PUBLIC void yy_init_lex()
{

}

PUBLIC void yy_init_occs(void *tos)
{

}