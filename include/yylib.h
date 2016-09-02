//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yylib.h : 
//

#ifndef CGKITS_YYLIB_H
#define CGKITS_YYLIB_H

#include <stdarg.h>

extern int   yy_init_debug(int *sstack, int **p_sp, char **dstack, char ***p_dsp,
                         void *vstack, int v_ele_size, int depth);
extern void  yy_quit_debug(void);
extern int   yy_get_args(int argc, char **argv);

extern void  yy_output(int where, char *fmt, va_list args);
extern void  yy_comment(char *fmt, ...);
extern void  yy_error(char *fmt, ...);
extern void  yy_input(char *fmt, ...);
extern int   yy_prompt(char *prompt, char *buf, int getstring);

extern void  yy_pstack(int do_refresh, int print_it);
extern void  yy_redraw_stack(void);
extern int   yy_next_token(void);
extern void  yy_break(int production_number);

extern void  yy_hook_a();
extern void  yy_hook_b();

extern int   yy_wrap();
extern char* yy_pstk(void *val, char *sym);
extern void  yy_init_cgllama(void *tos);
extern void  yy_init_cglex();
extern void  yy_init_cgoccs(void *tos);
extern int   main(int argc, char **argv);


#endif //CGKITS_YYLIB_H
