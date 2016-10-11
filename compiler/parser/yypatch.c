//
// Created by Mookel on 16/10/10.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yypatch.c : Illustrated below.
//

/* This file contains functions to do a transition on grammars based on
 * facts explained below:
 * The Symbol table, though handy, needs some shuffling around to make it
 * useful in an LALR application.First of all,all actions must be applied
 * on a reduction rather than a shift which means that imbedded actions
 * like this:
 *
 * foo : bar {act(1); } cow {act(2);}
 *
 * have to be put in their own productions:
 *
 * foo : bar {0} cow {act(2);}
 * {0} : epsilon     {act(1);}
 *
 * where {0} is treated as a nonterminal symbol; Once this is done, you can
 * print out the actions and get rid of the strings.Note that since the new
 * productions go to epsilon, this transformation does not affect either the
 * first or follow sets.
 *
 * Note that you don't have to actually add any symbols to the table;you can
 * just modify the values of the action symbols to turn them into nonterminals.
 */

#include <stdlib.h>
#include <ctype.h>

#include "parser.h"

/*local static variables*/
PRIVATE int _last_real_nonterm;

/*local functions declarations*/
PRIVATE void dopatch(SYMBOL_S *sym);
PRIVATE void print_one_case(int case_val, char *action, int rhs_size,
                            int lineno, PRODUCTION_S *prod);

/*
 * This functions does several things:
 * -> Modifies the symbol table as described above.
 * -> Prints the actions subroutine.
 */
PUBLIC  void patch()
{
    static char *top[] = {
      "",
      "/* This function holds all the actions in the original input specification.",
      " * It normally return 0,but if any of your actions return a non-zero value,",
      " * then the parser halts immediately, returning that non-zero number to the,",
      " * calling function."
      " */"
      "int yy_act(int yypnum, YYSTYPE *yyvsp)",
      "{",
      "    switch(yypnum) {",
      NULL
    };

    static char *bot[] = {
      "",
      "#ifdef YYDEBUG",
      "        default: yy_comment(\"Production %d: no action.\\n\", yypnum);",
      "            break;",
      "#endif",
      "   }",
      "",
      "    return 0;",
      "}",
      NULL
    };

    _last_real_nonterm = g_currnonterm;

    if(g_cmdopt.make_actions) sys_printv(g_output, top);
    hash_print_tab(g_symtab, (fp_tab_print_t)dopatch, NULL, 0);
    if(g_cmdopt.make_actions) sys_printv(g_output, bot);
}

/*
 * This function does two things:it modifies the symbol table for use with occs
 * and prints the action symbols.
 * The alternative is to add another field to the production structure and keep
 * the action string there, but this is both a needless waste of memory and an
 * unnecessary complication because the strings will be attached to a production
 * number and once we know that number, there is not point in keeping them around.
 */
PRIVATE void dopatch(SYMBOL_S *sym)
{
    PRODUCTION_S *prod;
    SYMBOL_S **pp;
    SYMBOL_S *cur;
    int i;

    /* "sym_val > _last_real_nonterm" means we have do some transformation on
     * original action terms, so just ignore it.
     */
    if(!ISNONTERM(sym) || (sym->val > _last_real_nonterm)) {
        return;
    }

    for(prod = sym->productions; prod; prod = prod->next){
        if(prod->rhs_len == 0) continue;

        pp = prod->rhs + (prod->rhs_len - 1);

        cur = *pp;
        if(ISACT(cur)) {
            print_one_case(prod->num, cur->string, --(prod->rhs_len), cur->lineno, prod);
            hash_del_sym(g_symtab, cur);
            *pp-- = NULL;
        }

        for(i = ((pp - prod->rhs) + 1); --i >= 0; --pp) {
            cur = *pp;

            if(!ISACT(cur)) continue;

            if(g_currnonterm >= MAXNONTERM) {
                error(FATAL, "Too many nonterminals & actions (%d max).\n", MAXNONTERM);
            } else  {
                /*Transform the action into a nonterminal*/
                g_terms[cur->val = ++g_currnonterm] = cur;
                cur->productions = (PRODUCTION_S*)GC_MALLOC(sizeof(PRODUCTION_S));
                if(!cur->productions) error(FATAL, "INTERNAL [dopatch]: Out of memory.\n");
                print_one_case(g_num_productons, cur->string, pp - prod->rhs, cur->lineno, prod);

                cur->string = NULL;
                cur->productions->num = g_num_productons++;
                cur->productions->lhs = cur;
                cur->productions->rhs_len = 0;
                cur->productions->rhs[0] = NULL;
                cur->productions->next = NULL;
                cur->productions->prec = 0;

                cur->first = set_new();
                SET_ADD(cur->first, EPSILON);
            }
        }
    }
}

/*
 * Print out one action as a case statement.
 */
PRIVATE void print_one_case(int case_val, char *action, int rhs_size,
                            int lineno, PRODUCTION_S *prod)
{
    int num, i;
    char *fname[80], *fp;

    if(!g_cmdopt.make_actions) return;

    output("\n        case %d:  /* %s */\n\t\t", case_val, production_str(prod));

    if(!g_cmdopt.no_lines)
        output("#line %d\"%s\"\n\t", lineno, g_input_file_name);

    while(*action) {
        if(*action != '$') {
            output("%c", *action++);
        } else {

            if(*++action != '<') {
                *fname = '\0';
            }else {
                ++action;
                fp = fname;

                for(i = sizeof(fname); --i > 0&& *action && *action != '>';)
                    *fp++ = *action++;
                *fp = '\0';

                if(*action == '>') ++action;
            }

            if(*action == '$') {
                num = DOLLAR_DOLLAR;
                ++action;
            } else {
                num = atoi(action);
                if(*action == '-') ++action;
                while(isdigit(*action)) ++action;
            }

            output("%s", do_dollar(num, rhs_size, lineno, prod, fname));
        }
    }

    output("\n            break;\n");
}