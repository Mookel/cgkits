//
// Created by Mookel on 16/9/5.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// nfa.c : 
//

#include <stdlib.h>
#include "nfa.h"
#include "globals.h"

#ifdef DEBUG
        int  lev = 0;
#define ENTER(f) printf("%*senter %s [%c][%1.10s]\n", lev++ * 4, "", f, _lexeme, _input)
#define LEAVE(f) printf("%*sleave %s [%c][%1.10s]\n", --lev * 4, "", f, _lexeme, _input)
#else
#define ENTER(f)
#define LEAVE(f)
#endif

/*error handling*/
typedef enum {
    E_MEMOUT,
    E_BADEXPR,
    E_PAREN,
    E_STACK,
    E_RE,
    E_BRACKET,
    E_BOL,
    E_CLOSE,
    E_STRINGS,
    E_NEWLINE,
    E_BADMAC,
    E_NOMAC,
    E_MACDEPTH
}ERR_NUM;

PRIVATE char *_errmsg[] = {
  "Not enough memory for NFA",
  "Malformed regular expression",
  "Missing close parenthesis",
  "Internal error: Discard stack full",
  "Too many regular expressions or expression too long",
  "Missing [ in character class",
  "^ must be at start of expression or after [",
  "+ ? or * must follow an expression or subexpression",
  "Too many characters in accept actions",
  "Newline in quoted string ,use \\n to get newline into expression",
  "Missing } in macro expansion",
  "Macro dosen't exist",
  "Macro expansions nested too deeply."
};

typedef enum{
    W_STARTDASH,
    W_ENDDASH
}WARN_NUM;

PRIVATE char *_warnmsgs[] ={
  "Treating dash in [-...] as a literal dash",
  "Treating dash in [...-] as a literal dash"
};

/*macro handling definitions*/
#define MAC_NAME_MAX 34
#define MAC_TEXT_MAX 80

typedef struct _MACRO{
    char name[MAC_NAME_MAX];
    char text[MAC_TEXT_MAX];
}MACRO;

typedef enum{
    EOS = 1,        /*end of string*/
    ANY,            /*.*/
    AT_BOL,         /*^*/
    AT_EOL,         /*$*/
    CCL_END,        /*]*/
    CCL_START,      /*[*/
    CLOSE_CURLY,    /*}*/
    CLOSE_PAREN,    /*)*/
    CLOSRUE,        /***/
    DASH,           /*-*/
    END_OF_INPUT,   /*eof*/
    L,              /*literal character*/
    OPEN_CURLY,     /*{*/
    OPEN_PAREN,     /*(*/
    OPTIONAL,       /*?*/
    OR,             /*|*/
    PLUS_CLOSE      /*+*/
}TOKEN;

#define MATCH(t)      (_current_tok == t)

/*stack definitions*/
#define _SSIZE  32
#define STACK_OK()    (INBOUNDS(_stack, _sp))
#define STACK_USED()  ((int)(_sp - _stack) + 1)
#define CLEAR_STACK() (_sp = _stack - 1)
#define PUSH(x)       (*++_sp = (x))
#define POP(x)        (*_sp--)
PRIVATE NFA *_stack[_SSIZE];     /*stack used by new*/
PRIVATE NFA *_sp = &_stack[-1];  /*stack pointer*/

/*private variables*/
PRIVATE NFA *_nfa_states; /*state-machine array*/
PRIVATE int  _nstates;    /*# of NFA states in machine*/
PRIVATE int  _next_alloc; /*index of next element of the array*/

PRIVATE TOKEN _tok_map[] = {
/* ^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N */
    L, L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* ^O  ^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\  ^] */
    L, L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* ^^  ^_  SPACE   !   "   #   $        %   &   '             */
    L, L,  L,      L,  L,  L,  AT_EOL,  L,  L,  L,

/* (             )           *        +           ,   -    .  */
    OPEN_PAREN, CLOSE_PAREN, CLOSRUE, PLUS_CLOSE, L, DASH, ANY,

/* /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =  */
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* >    ?                                                     */
   L,   OPTIONAL,

/* @   A   B   C   D   E   F   G   H   I   J   K   L   M   N  */
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* O   P   Q   R   S   T   U   V   W   X   Y   Z              */
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* [          \  ]        ^                                   */
   CCL_START, L, CCL_END, AT_BOL,

/* _   `   a   b   c   d   e   f   g   h   i   j   k   l   m  */
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* n   o   p   q   r   s   t   u   v   w   x   y   z         */
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* {           |   }            DEL                          */
   OPEN_CURLY, OR, CLOSE_CURLY, L

};

PRIVATE  char _input = "";   /*current position in input string*/
PRIVATE  char *_s_input;     /*begining of input string*/
PRIVATE  TOKEN _current_tok; /*current token*/
PRIVATE  int   _lexeme;      /*value associated with literal*/

/*private functions*/
PRIVATE void errmsg(int type, char **table, char *msgtype)
{
    char *p;
    fprintf(stderr, "%s (line %d) %s\n", msgtype, g_actual_lineno, table[type]);
    for(p = _s_input; ++p <= _input; putc('-', stderr))
        ;
    fprintf(stderr, "v\n%s\n", _s_input);
    exit(1);
}

