//
// Created by Mookel on 16/9/1.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// input.h : 
//

#include <sys/proc_info.h>

#ifndef CGKITS_INPUT_H
#define CGKITS_INPUT_H

typedef int (*fp_ii_open_t)(char *, int);
typedef int (*fp_ii_close_t)(int);
typedef int (*fp_ii_read_t)(int, void *, unsigned int);

extern void           ii_io(fp_ii_open_t fp_open, fp_ii_close_t fp_close, fp_ii_read_t fp_read);
extern int            ii_newfile(char *name);

extern unsigned char* ii_text(void);
extern int            ii_length(void);
extern int            ii_lineno(void);
extern unsigned char* ii_ptext(void);
extern int            ii_plength(void);
extern int            ii_plineno(void);

extern unsigned char* ii_mark_start(void);
extern unsigned char* ii_mark_end(void);
extern unsigned char* ii_move_start(void);
extern unsigned char* ii_to_mark(void);
extern unsigned char* ii_mark_prev(void);

extern int            ii_advance(void);
extern int            ii_flush(int force);
extern int            ii_fillbuf(unsigned char *starting_at);
extern int            ii_look(int n);
extern int            ii_pushback(int n);
extern void           ii_term(void);
extern void           ii_unterm(void);

extern int            ii_input(void);
extern void           ii_unput(int c);
extern int            ii_lookahead(int n);
extern int            ii_flushbuf(void);

#endif //CGKITS_INPUT_H
