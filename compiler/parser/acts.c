//
// Created by Mookel on 16/9/27.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// acts.c : 
//

#include "parser.h"
#include <string.h>

void init_acts()
{
    static SYMBOL_S bogus_symbol;
    strcpy(bogus_symbol.name, "End of Input");
    g_terms[0] = &bogus_symbol;

    g_symtab = hash_make_tab(157, hash_pjw, strcmp);
    LL(g_synch = set_new();)
}
