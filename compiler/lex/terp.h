//
// Created by Mookel on 16/9/6.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// terp.h : 
//

#ifndef CGKITS_TERP_H
#define CGKITS_TERP_H

#include <compiler.h>
#include "tydef.h"

PUBLIC int nfa(fp_input_t input_func);
PUBLIC void free_nfa();

PUBLIC SET_S *e_closure(SET_S *input, char **accept, int *anchor);
PUBLIC SET_S *move(SET_S *inp_set, int c);


#endif //CGKITS_TERP_H
