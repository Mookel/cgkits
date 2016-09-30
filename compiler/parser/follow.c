//
// Created by Mookel on 16/9/30.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// follow.c : Compute FOLLOW sets for all productions
// in a symbol table.The FIRST sets must be computed
// before follow() is called.
//

#include "parser.h"

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


}

