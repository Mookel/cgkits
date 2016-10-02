//
// Created by Mookel on 16/9/27.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// acts.c : Used by both llama and occs, including routines for building up
// the symbol table from the input specification.
//

#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "llout.h"

#undef stack_cls
#define stack_cls static

/*external global variables*/
extern int yylineno;

/*local static variables*/
PRIVATE char _field_name[NAME_MAX];
PRIVATE bool _goal_symbol_is_next = false; /*If true, the next nonterminal is the goal symbol.*/

#ifdef OCCS
PRIVATE int  _associativity;
PRIVATE int  _prec_lev = 0;
PRIVATE bool _fields_active = false;       /*fields are used in the input.*/
#endif

#define SSIZE 8
typedef struct cur_sym_
{
    char lhs_name[NAME_MAX];  /*Name associated with left-hand side*/
    SYMBOL_S *lhs;            /*Pointer to symbol-table entry for the current left-hand side*/
    PRODUCTION_S *rhs;        /*Pointer to the current production.*/
}CUR_SYM_S;

CUR_SYM_S Statck[SSIZE];
CUR_SYM_S *_sp = Statck + (SSIZE - 1);

/*private interfaces declerations.*/
PRIVATE bool c_identifier(char *name);
PRIVATE void find_problems(SYMBOL_S *sym);
/*subroutines used for printing */
PRIVATE void print_tok(FILE* stream, char *format, int arg);
PRIVATE void pterm(SYMBOL_S *sym, FILE *stream);
PRIVATE void pact(SYMBOL_S *sym, FILE *stream);
PRIVATE void pnonterm(SYMBOL_S *sym, FILE *stream);
PRIVATE char *production_str(PRODUCTION_S *prod);

/*public interfaces*/
PUBLIC void init_acts()
{
    static SYMBOL_S bogus_symbol;
    strcpy(bogus_symbol.name, "End of Input");
    g_terms[0] = &bogus_symbol;

    g_symtab = hash_make_tab(157, hash_pjw, strcmp);
    LL(g_synch = set_new();)
}

PUBLIC int problems()
{
    hash_print_tab(g_symtab, (fp_tab_print_t)find_problems, NULL, 0);
    return nerrors();
}

/*
 * Print out the symbol table. Nonterminal symbols come first for the sake
 * of the 's' option in yydebug(); symbols other
 */
PUBLIC void print_symbols(FILE *stream)
{
    putc('\n', stream);

    fprintf(stream, "-----------------------Symbol table-----------------\n");
    fprintf(stream, "\nNONTERMINAL SYMBOLS:\n\n");
    hash_print_tab(g_symtab, (fp_tab_print_t)pnonterm, stream, 1);
    fprintf(stream, "\nTERMINAL SYMBOLS:\n\n");
    OX(fprintf(stream, "name                value    prec    assoc    field\n");)
    LL(fprintf(stream, "name                value\n");)

    hash_print_tab(g_symtab, (fp_tab_print_t)pterm, stream, 1);

    LL(fprintf(stream, "\nACTION SYMBOLS:\n\n");)
    LL(hash_print_tab(g_symtab, (fp_tab_print_t)pact, stream, 1);)
    LL(fprintf(stream, "-------------------------------------------------\n");)
}

PUBLIC SYMBOL_S *make_term(char *name)
{
    SYMBOL_S *p;

    if(!c_identifier(name)) {
        lerror(NONFATAL, "Token names must be legitimate C identifiers\n");
    } else if(p = (SYMBOL_S *)hash_find_sym(g_symtab, name)) {
        lerror(WARNING, "Terminal symbol <%s> already declared\n", name);
    } else {
        if(g_currterm >= MAXTERM)
            lerror(FATAL, "Too many terminal symbols (%d max.).\n", MAXTERM);

        p = (SYMBOL_S*)hash_new_sym(sizeof(SYMBOL_S));
        strncpy(p->name, name, NAME_MAX);
        strncpy(p->field, _field_name, NAME_MAX);
        hash_add_sym(g_symtab, p);

        p->val = ++g_currterm;
        p->set = (unsigned)yylineno;
        g_terms[g_currterm] = p;
    }

    return p;
}

/*
 * Create and initialize a new nonterminal. is_lhs is used to
 * differentiate between implicit and explicit declarations.
 * It's false if the nonterminal is added because it was found
 * on a right-hand side. It's true if the nonterminal is on a
 * left-hand side.
 */
PUBLIC SYMBOL_S *new_nonterm(char *name, bool is_lhs)
{
    SYMBOL_S *p;

    if((p = (SYMBOL_S*)hash_find_sym(g_symtab, name)) != NULL){
        if(!ISNONTERM(p)) {
            lerror(NONFATAL, "Symbol on left-hand side must be nonterminal\n");
            p = NULL;
        }
    } else if(g_currnonterm >= MAXNONTERM) {
        lerror(FATAL, "Too many nonterminal symbols (%d max.).\n", MAXNONTERM);
    } else  {
        p = (SYMBOL_S *)hash_new_sym(sizeof(SYMBOL_S));
        strncpy(p->name, name, NAME_MAX);
        strncpy(p->field, _field_name, NAME_MAX);

        p->val = ++g_currnonterm;
        g_terms[g_currnonterm] = p;
        hash_add_sym(g_symtab, p);
    }

    if(p) {
        if(_goal_symbol_is_next) {
            g_goal_symbol = p;
            _goal_symbol_is_next = false;
        }

        if(!p->first) p->first = set_new();

        LL(if(!p->follow))
        LL(    p->follow = set_new();)

        p->lineno = (unsigned)yylineno;

        if(is_lhs){
            strncpy(_sp->lhs_name, name, NAME_MAX);
            _sp->lhs = p;
            _sp->rhs = NULL;
            _sp->lhs->set = (unsigned)yylineno;
        }
    }

    return p;
}

/*
 * Get a new PRODUCTION and link it to the head of the productions chain
 * of the current nonterminal. Note that the start production MUST be
 * production 0. AS a consequence, the first rhs associated with the first
 * nonterminal MUST be the start production. Num_productions is initialized
 * to 0 when it's declared.
 */
PUBLIC void new_rhs()
{
    PRODUCTION_S *p;
    if(!(p = (PRODUCTION_S*)GC_MALLOC(sizeof(PRODUCTION_S))))
        lerror(FATAL, "No memory for new right-hand side.\n");

    p->next = _sp->lhs->productions;
    _sp->lhs->productions = p;

    LL(p->select = set_new();)

    if((p->num = g_num_productons++) >= MAXPROD)
        lerror(FATAL, "Too many productions (%d max.)\n", MAXPROD);

    p->lhs = _sp->lhs;
    _sp->rhs = p;
}

/*
 * Add a new element to the RHS currently at top of statck.
 * First deal with forward references.If the item isn't in the table, add it.
 * Note that, since terminal symbols must be declared with a %term directive,
 * forward references always refer to nonterminals or action items.When we
 * exit the if statement, p points at the symbol table entry for the current
 * object.
 */
PUBLIC void add_to_rhs(char *object, bool is_an_action, int action_lineno)
{
    SYMBOL_S *p;
    int i;
    char buf[32];

    if(!(p = (SYMBOL_S*)hash_find_sym(g_symtab, object))){ /*terminal MUST existed,or it is an error.*/
        if(!is_an_action) {
            if(!(p = new_nonterm(object, false))) {
                lerror(FATAL, "(Internal) Unexpected terminal symbol\n");
                return;
            }
        } else {
            /* Add an action. All actions are named "{DDD}" where DDD is the action
             * number.The curly brace in the name guarantees that this name won't
             * conflict with  a normal name.
             */
            sprintf(buf, "{%d}", ++g_curract - MINACT);
            p = (SYMBOL_S *) hash_new_sym(sizeof(SYMBOL_S));
            strncpy(p->name, buf, NAME_MAX);
            hash_add_sym(g_symtab, p);
            p->val = g_curract;
            p->lineno = (unsigned)action_lineno;
            if (!(p->string = strdup(object)))
                lerror(FATAL, "Insufficient memory to save action\n");
        }
    }

    p->used = (unsigned)yylineno;
    if((i = _sp->rhs->rhs_len++) >= MAXRHS) {
        lerror(FATAL, "Right-hand side too long (%d max)\n", MAXRHS);
    } else {
        LL(if(i == 0 && p == _sp->lhs))
        LL(lerror(NONFATAL, "Illegal left recursion in productions.\n");)

        OX(if(ISTERM(p)))
        OX(_sp->rhs->prec = g_precedence[p->val].level;)

        _sp->rhs->rhs[i] = p;
        _sp->rhs->rhs[i+1] = NULL;

        if(!ISACT(p))
            ++(_sp->rhs->non_acts);
    }
}

PUBLIC void first_sym()
{
    _goal_symbol_is_next = true;
}

PRIVATE bool c_identifier(char *name)
{
    if(isdigit(*name)) return false;

    for(; *name; ++name) {
        if(!(isalnum(*name) || *name == '_'))
            return false;
    }

    return true;
}

PRIVATE void find_problems(SYMBOL_S *sym)
{
    if(!sym->used && sym!=g_goal_symbol)
        error(WARNING, "<%s> not used (defined on line %d)\n", sym->name, sym->set);

    if(!sym->set && !ISACT(sym))
        error(NONFATAL, "<%s> not defined (used on line %d)\n", sym->name, sym->used);
}

#ifdef LLAMA
PUBLIC void add_synch(char *name)
{
    SYMBOL_S *p;
    if(!(p = (SYMBOL_S *)hash_find_sym(g_symtab, name))) {
        lerror(NONFATAL, "%%synch: undeclared symbol <%s>.\n", name);
    } else if(!ISTERM(p)) {
        lerror(NONFATAL, "%%synch: <%s> not a terminal symbol\n", name);
    } else {
        SET_ADD(g_synch, p->val);
    }
}
#else
#endif

PRIVATE void print_tok(FILE *stream, char *format, int arg)
{
    if(arg == -1) fprintf(stream, "null ");
    else if(arg == -2) fprintf(stream, "empty ");
    else if(arg == _EOI_) fprintf(stream, "$ ");
    else if(arg == EPSILON) fprintf(stream, "<epsilon> ");
    else fprintf(stream, "%s ", g_terms[arg]->name);
}

PRIVATE void pterm(SYMBOL_S *sym, FILE *stream)
{
    OX(int i);
    if(!ISTERM(sym)) return;

#ifdef LLAMA
    fprintf(stream, "%-16s    %3d\n", sym->name, sym->val);
#else
    fprintf(stream, "%-16s    %3d    %2d    %c    <%s>\n",
            sym->name,
            sym->val,
            g_precedence[sym->val].level,
            (i = g_precedence[sym->val].assoc) ? i : '-',
            sym->field);
#endif
}

PRIVATE void pact(SYMBOL_S *sym, FILE *stream)
{
    if(!ISACT(sym)) return;
    fprintf(stream, "%-5s %3d", sym->name, sym->val);
    fprintf(stream, " line %-3d: ", sym->lineno);
    sys_fputstr(sym->string, 80, stream);
    fprintf(stream, "\n");
}

PRIVATE char *production_str(PRODUCTION_S *prod)
{
    int i, nchars, avail;
    static char buf[256];
    char *p;

    nchars = sprintf(buf, "%s ->", prod->lhs->name);

    p = buf + nchars;
    avail = sizeof(buf) - nchars;

    if(!prod->rhs_len){
        sprintf(p, " (epsilon)");
    } else {
        for(i = 0;i < prod->rhs_len && avail > 0; ++i) {
            nchars = snprintf(p, avail, " %s", prod->rhs[i]->name);
            avail -= nchars;
            p += nchars;
        }
    }

    return buf;
}

PRIVATE void pnonterm(SYMBOL_S *sym, FILE *stream)
{
    PRODUCTION_S *p;
    int chars_printed;

    stack_dcl(pstack, PRODUCTION_S *, MAXPROD);

    if(!ISNONTERM(sym)) return;

    fprintf(stream, "%s    (%3d)    %s", sym->name, sym->val, sym == g_goal_symbol ?
                                                              "(goal symbol)" : "");

    OX(fprintf(stream, "     <%s>\n", sym->field);)
    LL(fprintf(stream, "\n");)

    if(g_cmdopt.symbols > 1) {   /*more printed.*/
        fprintf(stream, "\tFIRST: ");
        set_print(sym->first, (fp_set_prnt) print_tok, stream);

        LL(fprintf(stream, "\n\tFOLLOW: ");)
        LL(set_print(sym->follow, (fp_set_prnt)print_tok, stream); )
        fprintf(stream, "\n");
    }

    for(p = sym->productions; p ; p = p->next) push(pstack, p);

    while(!stack_empty(pstack)) {
        p = pop(pstack);
        chars_printed = fprintf(stream, "\t%3d: %s", p->num, production_str(p));

        LL(for(; chars_printed <= 50; ++chars_printed))
        LL(putc('.', stream);)
        LL(fprintf(stream, ".SELECT: ");)
        LL(set_print(p->select, (fp_set_prnt)print_tok, stream);)

        //TODO: output occs

        putc('\n', stream);
    }

    fprintf(stream, "\n");
}



