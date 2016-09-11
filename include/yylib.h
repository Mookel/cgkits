//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yylib.h : 
//

#ifndef CGKITS_YYLIB_H
#define CGKITS_YYLIB_H

extern void  yy_hook_a();
extern void  yy_hook_b();

extern int   yy_wrap();
extern char* yy_pstk(void *val, char *sym);
extern void  yy_init_cgllama(void *tos);
extern void  yy_init_cglex();
extern void  yy_init_cgoccs(void *tos);

#endif //CGKITS_YYLIB_H
