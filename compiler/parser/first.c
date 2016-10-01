//
// Created by Mookel on 16/9/30.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// first.c : 
//

#include "parser.h"

/*local static variables*/
PRIVATE bool _did_somthing;

/*private functions declarations*/
PRIVATE void first_closure(SYMBOL_S *lhs);

/*public interfaces.*/
PUBLIC void first()
{
    D(printf("Finding FIRST sets.\n"));

    do{
        _did_somthing = false;
        hash_print_tab(g_symtab, (fp_tab_print_t)first_closure, NULL, 0);
    }while(_did_somthing);
}

/*
 *Fill the destination set with FIRST(rhs) where rhs is the right-hand side of
 *a production represented as an array of pointers to symbol-table elements.
 *Return true if the entire right-hand side is nullable, otherwise return false.
 */
PUBLIC bool first_rhs(SET_S *dest, SYMBOL_S **rhs, int len)
{
    if(len <= 0) {
        SET_ADD(dest, EPSILON);
        return true;
    }

    for(; --len >= 0; ++rhs) {
        if(ISACT(rhs[0])) continue;

        if(ISTERM(rhs[0])){
            SET_ADD(dest, rhs[0]->val);
        } else {
            SET_UNION(dest, rhs[0]->first);
        }

        if(!NULLABLE(rhs[0])) break;
    }

    return len < 0;
}

/*
 * Called for every element in the FIRST sets.Add elements to the first sets.
 * The following rules are used:
 * 1) Given lhs->...Y...where Y is a terminal symbol preceded by any number
 * (including 0) of nullable nonterminal symbols or actions, Add Y to FIRST(x).
 *
 * 2) Given lhs->...y...Where y is a nonterminal symbol preceded by any number
 * (including 0) of nullable nonterminal symbols or actions, Add FIRST(y) to
 * FIRST(lhs).
 */
PRIVATE void first_closure(SYMBOL_S *lhs)
{
    PRODUCTION_S *prod;
    SYMBOL_S **y;
    static SET_S *set = NULL;
    int i;

    if(!ISNONTERM(lhs)) return;

    if(!set) set = set_new();

    SET_ASSIGN(set, lhs->first);

    for(prod = lhs->productions; prod; prod = prod->next) {
        if(prod->non_acts <= 0) {
            SET_ADD(set, EPSILON);
            continue;
        }

        for(y = prod->rhs, i = prod->rhs_len; --i >= 0; y++) {
           if(ISACT(*y)) continue;

            if(ISTERM(*y)){
                SET_ADD(set, (*y)->val);
            } else if(*y) {
                SET_UNION(set, (*y)->first);
            }

            if(!NULLABLE(*y)) break;
        }
    }

    if(!SET_IS_EQUIV(set, lhs->first)) {
        SET_ASSIGN(lhs->first, set);
        _did_somthing = true;
    }
}


