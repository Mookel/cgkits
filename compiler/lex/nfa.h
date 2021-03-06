//
// Created by Mookel on 16/9/5.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// nfa.h : 
//

#ifndef CGKITS_NFA_H
#define CGKITS_NFA_H

#include <compiler.h>
#include "tydef.h"

typedef struct _NFA{
    int edge;           /*lael for edge: character, CCL, EMPLTY, or EPSILON*/
    SET_S *bitset;      /*Set to store character classes.*/
    struct _NFA *next;  /*next state (or null if none)*/
    struct _NFA *next2; /**another next state if edge==EPSILON */
    char *accept;       /*NULL if not an accepting state, else a pointer to the action string.*/
    int  anchor;        /*says whether pattern is anchored*/
}NFA;

#define EPSILON  -1
#define CCL      -2
#define EMPTY    -3

#define NONE  0
#define START 1
#define END   2
#define BOTH  (START | END)

#define NFA_MAX 768         /*maximum munber of NFA states in a single machine.*/
#define STR_MAX (10*1024)   /*total space that can be used by the accept strings.*/

PUBLIC void new_macro(char *def);
PUBLIC void print_macs(void);
PUBLIC NFA *thompson(fp_input_t input_func, int *max_state, NFA **start_state);
PUBLIC void  print_nfa(NFA *nfa, int len, NFA *start);

#endif //CGKITS_NFA_H
