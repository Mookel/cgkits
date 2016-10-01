//
// Created by Mookel on 16/9/30.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// llselect.c : Compute LL(1) set for all productions in
// a symbol table. The FIRST and FOLLOW must be computed
// before select() is called.
//

#include "parser.h"
#include "first.h"

/*local static functions declarations.*/
PRIVATE void find_select_set(SYMBOL_S *lhs);

/*public interfaces*/
PUBLIC void llselect()
{
    if(g_cmdopt.verbose) printf("Finding LL(1) select sets.\n");

    hash_print_tab(g_symtab, (fp_tab_print_t)find_select_set, NULL, 0);
}

/*
 * Find the LL(1) selection set for all productions attached to the indicated
 * left-hand side(lhs).The first_rhs() call puts the FIRST sets for the initial
 * symbols in prod->rhs into the select set. It returns true of if the entire
 * right-hand side was nullabe(EPSILON) was an element of the FIRST set of every
 * symbol on the right-hand side.
 */
PRIVATE void find_select_set(SYMBOL_S *lhs)
{
    PRODUCTION_S *prod;
    for(prod = lhs->productions; prod; prod = prod->next){
        if(first_rhs(prod->select, prod->rhs, prod->rhs_len))
            SET_UNION(prod->select, lhs->follow);

        SET_REMOVE(prod->select, EPSILON);
    }
}

