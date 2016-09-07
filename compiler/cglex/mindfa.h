//
// Created by Mookel on 16/9/7.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// mindfa.h : 
//

#ifndef CGKITS_MINDFA_H
#define CGKITS_MINDFA_H

#include <compiler.h>
#include "tydef.h"
#include "dfa.h"

PUBLIC int min_dfa(fp_input_t input_func, ROW *dfap[], ACCEPT **accept);

#endif //CGKITS_MINDFA_H
