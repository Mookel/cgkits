//
// Created by Mookel on 16/10/1.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// llcode.c : Print various tables needed for a llama-generated
// LL(1) parser.
//

/* A llama-generated LL(1) parser including following tables:
 *
 * Yyd[][]       : The parser state machine's DFA transition table.
 *                 The horizontal axis is input symbol and the vertical axis is
 *                 top-of-stack symbol. Only nonterminal TOS symbols are in the
 *                 table.The table contains the production number of a production
 *                 to apply or -1 if this is an error transition.
 *
 * YydN[]        : (N is 1-3 decimal digits).It is used for compressed tables only.
 *                 Holds the compressed rows.
 *
 * Yy_pushtab[]  : Indexed by production number, evaluates to a list of objects to
 * YypDD[]         push on the stack when that production is replaced.The YypDD array
 *                 are lists of objects and Yy_pushtab is an array of pointers to
 *                 those lists.
 *
 * Yy_snonterm[] : For debugging, indexed by nonterminal, evaluates to the name of the
 *                 nonterminal.
 *
 * Yy_sact[]     : Same as above but for actions.
 * Yy_synch[]    : Array of synchronization tokens for error recovery.
 * yy_act()      : Subroutine containing the actions.
 * Yy_stok[]     : Made in stok.c. For the most part, the numbers in these tables are
 *                 the same as those in the symbol table. The exceptions are the token
 *                 values, which are shifted down so that the smallest token has the
 *                 value of 1(0 is used for EOI).
 */

#include "parser.h"

#define DTRAN "Yyd"  /*Name of DFA transition table*/

/*local static variables*/
PRIVATE int *_dtran;

/*local static functions declarations*/
PRIVATE void fill_row(SYMBOL_S *lhs);
PRIVATE void make_pushtab(SYMBOL_S *lhs);
PRIVATE void make_yy_pushtab(void);
PRIVATE void make_yy_dtran(void);
PRIVATE void make_yy_synch(void);
PRIVATE void make_yy_snonterm(void);
PRIVATE void make_yy_sact(void);
PRIVATE void make_acts(SYMBOL_S *lhs);
PRIVATE void make_yy_act(void);

PUBLIC void tables()
{
    make_yy_pushtab();
    make_yy_dtran();
    make_yy_act();
    make_yy_synch();
    make_yy_stok();
    make_token_file();

    output("\n #ifdef YYDEBUG\n");
    make_yy_snonterm();
    make_yy_sact();
    output("\n #endif\n");
}

/* Make one row of the parser's DFA transition table. Column 0 is used
 * for the EOI condition;other columns are indexed by nonterminal(with
 * the number normalized for the smallest nonterminal).That is, the
 * terminal values in the symbol table are shifted downwards so that
 * the smallest terminal value is 1 rather than MINTERM.The row indexes
 * are adjusted in the same way(so that row 0 is used for MINNONTERM).
 *
 */
PRIVATE void fill_row(SYMBOL_S *lhs)
{
    PRODUCTION_S *prod;
    int *row;
    int i   ;
    int rowsize;

    if(!ISNONTERM(lhs)) return;

    rowsize = USED_TERMS + 1;
    row = _dtran + ((i = ADJ_VAL(lhs->val)) * rowsize);

    for(prod = lhs->productions; prod; prod = prod->next) {
        set_next_member(NULL);
        while((i = set_next_member(prod->select)) >= 0) {
            if(row[i] == -1) {
                row[i] = prod->num;
            } else {
                error(NONFATAL, "Grammar not LL(1), select-set conflict in ");
                error(NOHDR, "<%s>, line %d\n", lhs->name, lhs->lineno);
            }
        }
    }
}

/*
 * Make the pushtab.The right-hand sides are output in reverse order
 * (to make the pushing easier) by stacking them and then printing
 * items off the stack.
 */
PRIVATE void make_pushtab(SYMBOL_S *lhs)
{
    register int i;
    PRODUCTION_S *prod;
    SYMBOL_S **sym;
    SYMBOL_S *stack[MAXRHS], **sp;

    sp = &stack[-1];
    for(prod = lhs->productions; prod; prod = prod->next) {
        output("YYPRIVATE int Yyp%-2d[ ] = { ", prod->num);
        for(sym = prod->rhs, i = prod->rhs_len; --i >= 0;)
            *++sp = *sym++;

        for(; INBOUNDS(stack, sp); output("%d, ", (*sp--)->val))
            ;

        output("0 };\n", prod->rhs[0]);
    }
}

PRIVATE void make_yy_pushtab()
{
    register int i;
    register int maxprod = g_num_productons - 1;
    static char *text[] = {
        "The YypNN arrays hold the right-hand sides of productions, listed back",
        "to front (so that they are pushed in reverse order), NN is the production",
        "number (to be found in the symbol-table listing output with a -s switch).",
        "",
        "Yy_pushtab[] is indexed by production number and points to the appropriate",
        "right-hand side (YypNN) array.",
        NULL
    };

    sys_comment(g_output, text);
    hash_print_tab(g_symtab, (fp_tab_print_t)make_pushtab, NULL, 0);

    output("\YYPRIVATE int *Yy_pushtab[ ] = \n{\n");
    for(i = 0;i < maxprod; ++i)
        output("\tYyp%-2d,\n", i);
    output("\tYyp%-2d\n};\n", maxprod);
}

/*print the DFA transition table.*/
PRIVATE void make_yy_dtran()
{
    int i;
    int nterms, nnonterms;
    static char *text[] = {
        "Yyd[][] is the DFA transition table for the parser. It is indexed as follows:",
        "",
        "                  Input symbol",
        "             +---------------------+",
        "          L  |  production number  |",
        "          H  |       or YYF        |",
        "          S  |                     |",
        "             +---------------------+",
        "",
        "The production number is used as an index into Yy_pushtab, which looks like: ",
        "",
        "          Yy_pushtab      YypDD:",
        "          +--------+      +----------------+",
        "          |   *----|----->|                |",
        "          +--------+      +----------------+",
        "          |   *----|----->",
        "          +--------+",
        "          |   *----|----->",
        "          +--------+",
        "",
        "YypDD is the tokenized right-hand side of the production.Generate a symbol",
        "table listing with llama's -l command-line switch to get both production",
        "numbers and the meanings of the YypDD string contents.",
        NULL
    };

    nterms = USED_TERMS + 1;   /*+1 for EOI*/
    nnonterms = USED_NONTERMS;

    i = nterms * nnonterms;
    if(!(_dtran = (int *)GC_MALLOC(i * sizeof(*_dtran))))
        com_ferr("Out of memory.\n");

    sys_memiset(_dtran, -1, i);
    hash_print_tab(g_symtab, (fp_tab_print_t)fill_row, NULL, 0);

    sys_comment(g_output, text);
    if(g_cmdopt.uncompressed) {
        fprintf(g_output, "YYPRIVATE YY_TTYPE %s[ %d ][ %d ] = \n", DTRAN, nnonterms, nterms);

        sys_print_array(g_output, _dtran, nnonterms, nterms);
        sys_print_defnext(g_output, DTRAN);

        if(g_cmdopt.verbose) printf("%d bytes required for tables.\n", i * sizeof(YY_TTYPE));
    } else {
        i = sys_pairs(g_output, _dtran, nnonterms, nterms, _dtran, g_cmdopt.threshold, 1);
        sys_pnext(g_output, DTRAN);

        if(g_cmdopt.verbose)
            printf("%d bytes required for compressed tables.\n", (i*sizeof(YY_TTYPE)) +
                    (nnonterms * sizeof(YY_TTYPE *)));
    }

    output("\n\n");
}

PRIVATE void make_yy_synch()
{
    int mem;
    int i;

    static char *text[] = {
        "Yy_synch[] is  an array of synchronization tokens. When an error is detected, ",
        "stack items are popped until one of the tokens in this array is encountered.",
        "The input is then read until the same item is found. Then parsing continues.",
        NULL,
    };

    sys_comment(g_output, text);
    output("YYPRIVATE int Yy_synch[] = \n{\n");
    i = 0;
    for(set_next_member(NULL); (mem = set_next_member(g_synch)) >= 0;){
        output("\t%s,\n", g_terms[mem]->name);  /*Note: name will be macro after output.*/
        ++i;
    }

    /*No members in synch set.*/
    if(i == 0) output("\t_EOI_,\n");  /*',' is added by mookel.*/

    output("\t-1\n};\n");
    set_next_member(NULL);
}

PRIVATE void make_yy_snonterm()
{
    register int i;

    static char *text[] = {
        "Yy_snonterm[] is used only for debugging.It is indexed by the tokenized left-hand",
        "side(as used for a row index in Yyd[]) and evaluates to a string naming that lhs.",
        NULL
    };

    sys_comment(g_output, text);
    output("char *Yy_snonterm[] = \n{\n");
    for(i = MINNONTERM; i <= g_currnonterm; ++i){
        if(g_terms[i]) output("\t/* %3d */ \"%s\"", i, g_terms[i]->name);
        if(i != g_currnonterm) outc(',');
        outc('\n');
    }

    output("};\n\n");
}

PRIVATE void make_yy_sact()
{
    register int i;
    static char* text[] = {
        "Yy_sact[] is alse used only for debugging. It is indexed by the internal value",
        "used for an action symbol and evaluates to a string naming that token symbol.",
        NULL
    };

    sys_comment(g_output, text);
    output("char *Yy_sact[] = \n{\n\t");

    if(g_curract < MINACT) {
        output("NULL /* There are no actions */");
    } else {
        for(i = MINACT; i <= g_curract; ++i) {
            output("\"{%d}\"%c", i - MINACT, i < g_curract ? ',' : ' ');
            if(i % 10 == 9) output("\n\t");
        }
    }

    output("\n};\n");
}

/*
 * This rubroutine is called indirectly from yy_act, through the subroutine
 * hash_print_tab(). It prints the text associated with one of the acts.
 */
PRIVATE void make_acts(SYMBOL_S *lhs)
{
    char *p;
    int num;
    char fname[80], *fp;
    int i;

    if(!lhs->string) return;

    output("        case %d:\n", lhs->val);

    if(g_cmdopt.no_lines){
        output("\t\t");
    } else {
        output("#line %d\"%s\"\n\t\t", lhs->lineno, g_input_file_name);
    }

    for(p = lhs->string; *p;) {
        if(*p == '\r') continue;

        if(*p != '$'){
            output("%c", *p++);
        }else {
            /*skip the attribute reference. The if statement handles $$, the else
             *clause handles the two forms: $N and $-N, where N is a decimal number.
             * When you hit the do_dollar call, "num" holds the number assocaiated
             * with N, or DOLLAR_DOLLAR in the case of $$.
             */

            //TODO: for occs;
        }
    }

    output("\n        break;\n");
}

PRIVATE void make_yy_act()
{
    static char *text[] ={
        "Yy_act() is the action subroutine. It is passed the tokenized value of",
        "an action and executes the corresponding code.",
        NULL
    };

    static char *top[] = {
        "YYPRIVATE int yy_act(int actnum)",
        "{",
        "   /*The actions. Returns 0 normally but a nonzero error code can be returned",
        "    *if one of the acts causes the parser to terminate abnormally.",
        "    */",
        "",
        "    switch(actum) {",
        NULL
    };

    static char *bottom[] = {
        "    default: printf(\"INTERNAL ERROR: Illegal action number (%s)\\n\",",
        "                                                              actnum);",
        "             break;",
        "    }",
        "    return 0;",
        "}",
        NULL
    };

    sys_comment(g_output, text);
    sys_printv(g_output, top);
    hash_print_tab(g_symtab, (fp_tab_print_t)make_acts, NULL, 0);
    sys_printv(g_output, bottom);

}



