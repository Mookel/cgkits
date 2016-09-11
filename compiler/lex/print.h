//
// Created by Mookel on 16/9/7.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// print.h : 
//

#ifndef CGKITS_PRINT_H
#define CGKITS_PRINT_H

#include "dfa.h"
#include "nfa.h"

PUBLIC void pheader(FILE *fp, ROW dtran[], int nrows, ACCEPT *accept);
PUBLIC void pdriver(FILE *output, int nrows, ACCEPT *accept);

#endif //CGKITS_PRINT_H
