//
// Created by Mookel on 16/9/22.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// parser.h.h : 
//

#ifndef CGKITS_PARSER_H_H
#define CGKITS_PARSER_H_H

#include <limits.h>
#include <compiler.h>
#include <stdbool.h>

#define LLAMA

#ifdef LLAMA
#define LL(x) x
#define OX(x)
#endif

#ifdef OCCS
#define LL(x)
#define OX(x) x
#endif

#define EXIT_ILLEGAL_ARG  255  /*illegal command-line swtich*/
#define EXIT_TOO_MANY     254  /*too many comand-line args*/
#define EXIT_NO_DRIVER    253  /*Cant find llama.par*/
#define EXIT_OTHER        252
#define EXIT_USR_ABORT    251  /*Ctrl-Break*/

#define MAXNAME           32   /*maximum length of a terminal of nonterminal name*/
#define MAXPROD           512  /*maximum number of productions in the input grammar*/

#define MINTERM           1    /*Token values assigned to terminals start here.*/
#define MINNONTERM        256  /*nonterminals start here*/
#define MINACT            512  /*acts start here*/
#define MAXTERM           (MINNONTERM - 2)
#define MAXNONTERM        (MINACT - 1)
#define NUMTERMS          ((MAXTERM - MINTERM) + 1)
#define NUMNONTERMS       ((MAXNONTERM - MINNONTERM) + 1)
#define USED_TERMS        ((g_curterm - MINTERM) + 1)
#define USED_NONTERMS     ((g_curnonterm - MINNONTERM) + 1)

#define ISTERM(x)         ((x) && (MINTERM <= (x)->val && (x)->val <= MAXTERM))
#define ISNONTERM(x)      ((x) && (MINNONTERM <= (x)->val && (x)->val <= MAXNONTERM))
#define ISACT(x)          ((x) && (MINACT <= (x)->val))

#define EPSILON           (g_curterm + 1)

#define ADJ_VAL(x)        ((x) - MINNONTERM)
#define UNADJ_VAL(x)      ((x) + MINNONTERM)

#define DOLLAR_DOLLAR     ((unsigned)~0 >> 1)

#ifdef LLAMA
#define TOKEN_FILE        "llout.h"
#define PARSE_FILE        "llout.c"
#define SYM_FILE          "llout.sym"
#define DOC_FILE          "llout.doc"
#define DEF_EXT           "lma"
#define PAR_TEMPL         "llama.par"
#define PROG_NAME         "llama"
#else
#define TOKEN_FILE        "yyout.h"
#define PARSE_FILE        "yyout.c"
#define ACT_FILE          "yyact.c"
#define TAB_FILE          "yyoutab.c"
#define SYM_FILE          "yyout.sym"
#define DOC_FILE          "yyout.doc"
#define DEF_EXT           "ox"
#define PAR_TEMPL         "occs.par"
#define ACT_TEMPL         "occs-act.par"
#define PROG_NAME         "occs"
#endif

#ifndef CREATING_LLAMA_PARSER
    LL(typedef unsigned char YY_TTYPE;)
    OX(typedef int           YY_TTYPE;)
#endif

typedef struct symbol_{
    char name[NAME_MAX];
    char field[NAME_MAX];      /*%type <field>*/
    unsigned val;              /*numeric value of symbol*/
    unsigned used;             /*symbol used on an rhs*/
    unsigned set;              /*symbol defined*/
    unsigned lineno;           /*input line num of string*/
    char *string;              /*actions code*/
    struct prod_ *productions; /*right-hand sides if nonterm*/
    SET_S *first;              /*first set*/
    LL(SET_S *follow;)         /*follow set*/
}SYMBOL_S;

#define NULLABLE(sym)          (ISNONTERM(sym) && SET_MEMBER((sys)->first, EPSILON))
#define MAXRHS    31           /*maximum number of objects on a right-hand side*/
#define RHSBITS   5            /*Number of bits required to hold MAXRHS*/

typedef struct prod_ {
    unsigned num;              /*production number*/
    SYMBOL_S *rhs[MAXRHS + 1];
    SYMBOL_S *lhs;
    unsigned char rhs_len;     /*number of elements in rhs array*/
    unsigned char non_acts;
    SET_S *select;
    struct prod_ *next;        /*pointer to next production for this left-hand side*/
    OX(int prec;)

}PRODUCTION_S;

typedef struct cmdopt_{
    bool debug;
    bool make_actions;
    bool make_parser;
    bool make_yyoutab;
    bool no_lines;
    bool no_warnings;
    bool warn_exit;
    bool public;
    int  symbols;
    bool uncompressed;
    bool use_stdout;
    int  threshold;
    int  verbose;
}CMDOPT_S;

#ifdef OCCS
typedef struct prectab_{
    unsigned char level;       /*relative precedence, 0 = non, 1 = lowest*/
    unsigned char assoc;       /*associativity: l = left, r = right, '\0' = non */
}PREC_TAB_S;

#define DEF_FIELD "yy_def"
#endif

#ifdef ALLOCATE
#define CLASS
#define I(x)  x
#define DEFAULT(x) x
#else
#define CLASS extern
#define I(x)
#define DEFAULT(x)
#endif

CLASS CMDOPT_S g_cmdopt;

CLASS char *g_input_file_name I(="console");    /*input file name*/
CLASS FILE *g_output          I(= 0);           /*output stream*/

CLASS SYMBOL_S *g_terms[MINACT];                /*pointer to the equivalent symbol-table entry*/

OX(CLASS PREC_TAB_S g_precedence[MINNONTERM];)  /*relative precedence and associatively information*/
LL(CLASS SET_S *g_synch;)

CLASS char       *g_template I(= PAR_TEMPL);
CLASS HASH_TAB_S *g_symtab;                     /*the symbol table*/
CLASS SYMBOL_S   *g_goal_symbol I( = NULL);     /*pointer to symbol-table entry for the start symbol*/
CLASS int         g_currtem     I( = MINTERM - 1);
CLASS int         g_currnonterm I( = MINNONTERM-1);
CLASS int         g_curract     I( = MINACT - 1);
CLASS int         g_num_productons I( = 0);

#undef CLASS
#undef I

#define outc(c)   putc(c, g_output);

#endif //CGKITS_PARSER_H_H