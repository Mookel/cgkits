//
// Created by Mookel on 16/9/30.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// follow.c : Compute FOLLOW sets for all productions
// in a symbol table.The FIRST sets must be computed
// before follow() is called.
//

#include "parser.h"
#include "llout.h"

/*local static variables*/
PRIVATE bool _did_something;

/*local functions declarations*/
PRIVATE void init(SYMBOL_S *lhs);
PRIVATE void follow_closure(SYMBOL_S *lhs);
PRIVATE void remove_epsilon(SYMBOL_S *lhs);

/*public interfaces*/
PUBLIC void follow()
{
    D(int pass = 0;)
    D(printf("Initializing FOLLOW set.\n");)

    hash_print_tab(g_symtab, (fp_tab_print_t)init, NULL, 0);

    /* This loop makes serveral passes through the entire grammar, adding FOLLOW
     * sets. The follow_closure routine is called for each grammr symbol,and sets
     * _did_something to true if it adds any elements to existing FOLLOW sets.
     */
    do {
        _did_something = false;
        hash_print_tab(g_symtab, (fp_tab_print_t)follow_closure, NULL, 0);
    } while(_did_something);

    /* This last pass is just for nicety and could probably be eliminated.Strictly
     * speaking, FOLLOW sets shouldn't contain epsilon, Nonetheless, it was much
     * easier to just add epsilon in the previous steps than try to filter it out
     * here. This last pass just removes epsilon from all the FOLLOW sets.
     */
    hash_print_tab(g_symtab, (fp_tab_print_t)remove_epsilon, NULL, 0);
    D(printf("Follow set computation done.\n"));
}

/*
 * Initialize the FOLLOW sets.This procedure adds to the initial follow set of
 * each production those elements that won't change during the closure process.
 * Note that in all the following cases, actions are just ignored.
 *
 * (1) FOLLOW(start) contains end of input($).
 *
 * (2) Given s->...xY... where x is a nonterminal and Y is a terminal.Add Y to
 * FOLLOW(x). x and Y can be separated by any number(including 0) of nullable
 * nonterminals or actions.
 *
 * (3) Given s->...xy... where x and y are both nonterminals.Add First(y) to
 * FOLLOW(x). Again, x and y can be separated by any number of nullable non-
 * terminals or actions.
 */
PRIVATE void init(SYMBOL_S *lhs)
{
    PRODUCTION_S *prod;
    SYMBOL_S **x;
    SYMBOL_S **y;

    D(printf("%s:\n", lhs->name);)

    if(!ISNONTERM(lhs)) return;

    if(lhs == g_goal_symbol) {
        D(printf("\t Adding _EOI_ to FOLLOW(%s)\n", lhs->name);)
        SET_ADD(lhs->follow, _EOI_);
    }

    for(prod = lhs->productions; prod; prod = prod->next) {
        for(x = prod->rhs; *x; x++) {
            if(ISNONTERM(*x)) {
                for(y = x + 1; *y; ++y) {

                    if(ISACT(*y)) continue;

                    if(ISTERM(*y)){
                        D(printf("\tAdding %s ", (*y)->name);)
                        D(printf("to FOLLOW(%s)\n", (*x)->name);)
                        SET_ADD((*x)->follow, (*y)->val); break;
                    } else {
                        SET_UNION((*x)->follow, (*y)->first);
                        if(!NULLABLE(*y)) break;
                    }
                }
            }
        }
    }
}

/*
 * Adds elements to the FOLLOW sets using the following rule:
 *
 * Given s->..x or s->...x... where all symbols following x are
 * nullable nonterminals or actions, add FOLLOW(s) to FOLLOW(x).
 *
 */
PRIVATE void follow_closure(SYMBOL_S *lhs)
{
    PRODUCTION_S *prod;
    SYMBOL_S **x;

    D(printf("%s:\n", lhs->name);)

    if(ISACT(lhs) || ISTERM(lhs)) return;

    for(prod = lhs->productions; prod; prod = prod->next) {
        for(x = prod->rhs + prod->rhs_len; --x >= prod->rhs;) {

            if(ISACT(*x)) continue;
            if(ISTERM(*x)) break;

            if(!set_subset((*x)->follow, lhs->follow)) {

                D(printf("\tAdding FOLLOW(%s) ", lhs->name);)
                D(printf("to FOLLOW(%s)\n", (*x)->name);)

                SET_UNION((*x)->follow, lhs->follow);
                _did_something = true;
            }

            if(!NULLABLE(*x)) break;
        }
    }
}

/*
 * Remove epsilon from the FOLLOW sets.The presence of epsilon is a
 * side effect of adding FIRST sets to FOLLOW sets willy nilly.
 */
PRIVATE void remove_epsilon(SYMBOL_S *lhs)
{
    if(ISNONTERM(lhs)) SET_REMOVE(lhs->follow, EPSILON);
}