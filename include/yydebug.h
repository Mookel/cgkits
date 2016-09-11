//
// Created by Mookel on 16/9/11.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yydebug.h : 
//

#ifndef CGKITS_YYDEBUG_H
#define CGKITS_YYDEBUG_H

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

#endif //CGKITS_YYDEBUG_H
