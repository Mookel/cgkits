//
// Created by Mookel on 16/10/10.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yydollar.c : 
//

#include <string.h>
#include "parser.h"

PUBLIC char *do_dollar(int num, int rhs_size, int lineno, PRODUCTION_S *prod, char *fname)
{
    static char buf[128];
    int i, len;

    if(num == DOLLAR_DOLLAR) {  /*$$*/
        strcpy(buf, "Yy_val");
        if(*fname){
            sprintf(buf+6, ".%s", fname);
        } else if(fields_active()) {
            if(*prod->lhs->field) {
                sprintf(buf+6, ".%s", prod->lhs->field);
            } else {
                error(WARNING, "Line %d: No <field> assigned to $$, ", lineno);
                error(NOHDR, "using default int field\n");
                sprintf(buf+6, ".%s", DEF_FIELD);
            }
        }
    } else {                   /*handle $N*/
        if(num < 0) ++num;
        if(rhs_size < 0) {     /*handle $N in tail.*/
            sprintf(buf, "Yy_vsp[ Yy_rhslen-%d ]", num);
        } else {               /*handle $N in production.*/
            if((i = rhs_size - num) < 0) {
                error(WARNING, "Line %d: Illegal $%d in production.\n", lineno, num);
            } else {
                len = sprintf(buf, "yyvsp[%d]", i);
                if(*fname) {
                    sprintf(buf + len, ".%s", fname);
                } else if(fields_active()) {
                    if(num <= 0){
                        error(NONFATAL, "Can't use %%union field with negative");
                        error(NOHDR, " attributes. Use $<field>-N\n");
                    } else if(*(prod->rhs)[num - 1]->field) {
                        sprintf(buf + len, ".%s", (prod->rhs)[num-1]->field);
                    } else {
                        error(WARNING, "Line %d: No <field> assigned to $%d.", lineno, num);
                        error(NOHDR, " Using default int field\n");
                        sprintf(buf+len, ".%s", DEF_FIELD);
                    }
                }
            }
        }
    }

    return buf;
}