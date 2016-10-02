//
// Created by Mookel on 16/10/1.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// lldollar.c : 
//

#include "parser.h"

PUBLIC char *do_dollar(int num, int rhs_size, int lineno, PRODUCTION_S *prod, char *field)
{
    static char buf[32];

    if(num == DOLLAR_DOLLAR)  return "Yy_vsp->left";

    sprintf(buf, "(Yy_vsp[%d].right)", num);
    return buf;
}
