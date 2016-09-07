//
// Created by Mookel on 16/9/6.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// dfa.h : 
//

#ifndef CGKITS_DFA_H
#define CGKITS_DFA_H

#include <compiler.h>
#include "tydef.h"

#define DFA_MAX    254         /*maximum number of DFA states.*/
#define F          -1          /*marks failure states in the table*/
#define MAX_CHARS 128          /*maximum width of dfa transitions table.*/

typedef unsigned char TTYPE;
typedef int ROW[MAX_CHARS];

typedef struct _ACCEPT {
    char *string;
    int  anchor;
}ACCEPT;

PUBLIC int dfa(fp_input_t input_func, ROW *dfap[], ACCEPT **acceptp);

#endif //CGKITS_DFA_H
