//
// Created by Mookel on 16/9/27.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// llpar.c : 
//

#include <stdlib.h>
#include "parser.h"
#include "llout.h"

/* A recursive-descent parser for a very stripped down llama.
 * The small version grammar is as follows:
 *
 * spec -> definitions {first_sym()} body stuff            =>yyparse()
 *
 * definitions -> TERM_SPEC tnames definitions             =>definitions()
 *             |  CODE_BLOCK definitios                    =>definitions()
 *             |  SYNCH snames defintions                  =>definitions()
 *             |  SPEPARATOR                               =>definitions()
 *             |  _EOI_                                    =>definitions()
 *
 * snames ->   NAME {add_synch(yytext)} snames             =>definitions()
 * tnames ->   NAME {make_term(yytext)} tnames             =>definitions()
 *
 * body   ->   rule body                                   =>body()
 *         |   rule SEPARATOR
 *         |   rule _EOI_
 *
 * rule   ->   NAME {new nonterm(yytext, 1)} COLON right_sides
 *         |   epision
 *
 * right_sides -> {new_rhs()} rhs OR right_sides           =>right_sides()
 *              | {new_rhs()} rhs SEMI
 *
 * rhs ->  NAME {add_to_rhs(yytext, 0)} rhs                =>rhs()
 *     |   ACTION{add_to_rhs(yytext, start_action())} rhs
 *
 */

/*definitions in lex*/
extern  int   yylineno;
extern  char *yytext;
extern  int   yylex();
extern  void  ws();
extern  int start_action();

/*local variables*/
PRIVATE int _lookhead;

/*local functions*/
PRIVATE void advance();
PRIVATE void lookfor(int first, ...);
PRIVATE void definitions();
PRIVATE void body();
PRIVATE void right_sides();
PRIVATE void rhs();
PRIVATE void plex(int token);

#define match(x) ((x) == _lookhead)

/*public interface.*/
PUBLIC int yyparse()
{
    _lookhead = yylex();
    definitions();
    first_sym();
    body();
    return 0;
}

PRIVATE void advance()
{
    if(_lookhead != _EOI_)
        while((_lookhead = yylex()) == WHITESPACE)
            ;
}

PRIVATE void lookfor(int first, ...)
{
    int *obj;

    for(advance();; advance()) {
        for(obj = &first; *obj && !match(*obj); obj++)
            ;

        if(*obj)  {
            break;
        }else if(match(_EOI_)) {
            lerror(FATAL, "Unexpected end of file\n");
        }
    }
}

/*
 * Note: LeX copies the CODE_BLOCK contents to the output file
 * automatically on reading it.
 */
PRIVATE void definitions()
{
    while(!match(SEPARATOR) && !match(_EOI_)) {
        if(_lookhead == SYNCH) {
            for(advance(); match(NAME); advance())
                add_synch(yytext);
        } else if(_lookhead == TERM_SPEC) {
            for(advance(); match(NAME); advance())
                make_term(yytext);
        } else if(_lookhead == CODE_BLOCK) {
                advance();
        } else {
            lerror(NONFATAL, "Ignoring illegal <%s> in defintions\n", yytext);
            advance();
        }
    }

    advance(); /*advance past the %%*/
}

PRIVATE void body()
{
    while(!match(SEPARATOR) && !match(_EOI_)) {
        if(match(NAME)) {
            new_nonterm(yytext, 1);
            advance();
        } else {
            lerror(NONFATAL, "Illegal <%s>, nonterminal expected.\n", yytext);
            lookfor(SEMI, SEPARATOR, 0);
            if(match(SEMI)) advance();
            continue;
        }

        if(match(COLON)){
            advance();
        } else {
            lerror(NONFATAL, "Inserted missing ':' \n");
        }
        right_sides();
    }

    ws(); /*Enable white space.*/
    if(match(SEPARATOR)) yylex();
}

PRIVATE void right_sides()
{
    new_rhs();
    rhs();
    while(match(OR)) {
        advance();
        new_rhs();
        rhs();
    }

    if(match(SEMI)){
        advance();
    } else {
        lerror(NONFATAL, "Inserted missing semicolon\n");
    }
}

PRIVATE void rhs()
{
    while(match(NAME) || match(ACTION)) {
        add_to_rhs(yytext, match(ACTION), start_action());
        advance();
    }

    if(!match(OR) && !match(SEMI)) {
        lerror(NONFATAL, "Illegal <%s>, ignoring rest of production\n", yytext);
        lookfor(SEMI, SEPARATOR, OR, 0);
    }
}