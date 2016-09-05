//
// Created by Mookel on 16/9/5.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// nfa.c : 
//
#ifdef MAIN
#define ALLOC
#endif

#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "nfa.h"

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
    E_LENGTH,
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
#define POP()         (*_sp--)
PRIVATE NFA *_stack[_SSIZE];     /*stack used by new*/
PRIVATE NFA **_sp = &_stack[-1];  /*stack pointer*/

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
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* [          \  ]        ^                                   */
   CCL_START, L, CCL_END, AT_BOL,

/* _   `   a   b   c   d   e   f   g   h   i   j   k   l   m  */
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* n   o   p   q   r   s   t   u   v   w   x   y   z         */
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/* {           |   }            DEL                          */
   OPEN_CURLY, OR, CLOSE_CURLY, L

};

PRIVATE  fp_input_t _input_func;
PRIVATE  char *_input = "";   /*current position in input string*/
PRIVATE  char *_s_input;      /*begining of input string*/
PRIVATE  TOKEN _current_tok;  /*current token*/
PRIVATE  int   _lexeme;       /*value associated with literal*/
PRIVATE  HASH_TAB_S *_macros; /*symbol table for macro definitions*/

/*------------------------------------------------------------------*/
PRIVATE void errmsg(int type, char **table, char *msgtype)
{
    char *p;
    fprintf(stderr, "%s (line %d) %s\n", msgtype, g_actual_lineno, table[type]);
    for(p = _s_input; ++p <= _input; putc('-', stderr))
        ;
    fprintf(stderr, "v\n%s\n", _s_input);
    exit(1);
}

PRIVATE void warning(WARN_NUM type)
{
    errmsg((int)type, _warnmsgs, "WARNING");
}

PRIVATE void parse_err(ERR_NUM type)
{
    errmsg((int)type, _errmsg, "ERROR");
}

/*--------------------------------------------------------------*/
/*macro handling functions*/
PRIVATE char *get_macro(char **namep)
{
    char *p;
    MACRO *mac;

    if(!(p = strchr(++(*namep), '}'))){
        parse_err(E_BADMAC);
    }else {
        *p = '\0';
        if(!(mac = (MACRO*)hash_find_sym(_macros, *namep)))
            parse_err(E_NOMAC);

        *p++ = '}';
        *namep = p;
        return mac->text;
    }

    return "ERROR";
}

PRIVATE void print_a_macro(MACRO *mac)
{
    printf("%-16s--[%s]--\n", mac->name, mac->text);
}

PUBLIC void print_macs(void)
{
    if(!_macros) {
        printf("\tThere no macros.\n");
    } else {
        printf("\n MACROS:\n");
        hash_print_tab(_macros, (fp_tab_print_t) print_a_macro, NULL, 1);
    }
}

PUBLIC void new_macro(char *def)
{
    char *name;
    char *text;
    char *edef;
    MACRO *p;
    static int first_time = 1;

    if(first_time){
        first_time = 0;
        _macros = hash_make_tab(31, hash_add, strcmp);
    }

    name = def;
    for( ;*def && !isspace(*def); ++def)
        ;

    if(*def) *def++ = '\0';

    while(isspace(*def)) def++;

    text = def;
    edef = NULL;

    while(*def) {
        if(!isspace(*def)){
            ++def;
        }else {
            for(edef = def++; isspace(*def); ++def)
                ;
        }
    }

    if(edef) *edef = '\0';

    p = (MACRO *) hash_new_sym(sizeof(MACRO));
    strncpy(p->name, name , MAC_NAME_MAX);
    strncpy(p->text, text,  MAC_TEXT_MAX);
    hash_add_sym(_macros, p);

    D(printf("Added macro definition, macro table now is : \n");)
    D(print_macs();)
}

/*----------------------------------------------------------------*/
PRIVATE NFA *new()
{
    NFA *p;
    static int first_time = 1;

    if(first_time){
        if(!(_nfa_states = (NFA*)calloc(NFA_MAX, sizeof(NFA))))
            parse_err(E_MEMOUT);

        first_time = 0;
        _sp = &_stack[-1];
    }

    if(++_nstates >= NFA_MAX)
        parse_err(E_LENGTH);

    p = (!STACK_OK()) ? &_nfa_states[_next_alloc++] : POP();
    p->edge = EPSILON;
    return p;
}

PRIVATE void discard(NFA *nfa_to_discard)
{
    --_nstates;
    memset(nfa_to_discard, 0, sizeof(NFA));
    nfa_to_discard->edge = EMPTY;
    PUSH(nfa_to_discard);

    if(!STACK_OK()) parse_err(E_STACK);
}

/*----------------------------------------------------------------
 * Lexical analyzer:
 * Lexical is trival here because all lexemes are single-character
 * values.The only complications are escape sequences and quoted
 * strings.
 *
 * Macros expansion is handled by means of a stack.when an expansi-
 * on is encounted, the current input buffer is stacked, and input
 * is read from the macro text.
 */
PRIVATE TOKEN advance()
{
    static int inquote = 0;
    int saw_esc;
    static char *stack[_SSIZE], **sp = NULL;

    if(!sp) sp = stack - 1;

    if(_current_tok == EOS) {
        if(inquote)
            parse_err(E_NEWLINE);
        do {
            /*sit in this loop until a non-blank line is read into the input array.*/
            if(!(_input = (*_input_func)())) {
                _current_tok = END_OF_INPUT;
                goto exit;
            }

            while(isspace(*_input)) _input++;

        }while(!*_input);
        _s_input = _input;
    }

    while(*_input == '\0') { /*here is hard to understand, but you can do it, hehe.*/
        if(INBOUNDS(stack, sp)) {
            _input = *sp--;
            continue;
        }

        _current_tok = EOS;
        _lexeme = '\0';
        goto exit;
    }

    if(!inquote) {
        while(*_input == '{'){
            *++sp = _input;
            _input = get_macro(sp); /*must be careful here, because *sp will be modified by get_macro function*/
            if(TOOHIGH(stack, sp)) parse_err(E_MACDEPTH);
        }
    }

    if(*_input == '"') {
        inquote = ~inquote;
        if(!*++_input) {
            _current_tok = EOS;
            _lexeme = '\0';
            goto exit;
        }
    }

    saw_esc = (*_input == '\\');

    if(!inquote) {
        if(isspace(*_input)) {
            _current_tok = EOS;
            _lexeme = '\0';
            goto exit;
        }
        _lexeme = sys_esc(&_input);
    } else {
        if(saw_esc && _input[1] == '"'){
            _input += 2;
            _lexeme = '"';
        } else {
            _lexeme = *_input++;
        }
    }

    _current_tok = (inquote || saw_esc) ? L : _tok_map[_lexeme];
    exit:
        return _current_tok;
}

PRIVATE char *save(char *str)
{
    char *textp, *startp;
    int len;
    static int first_time = 1;
    static char size[8];
    static int *strings;  /*place to save accepting strings.*/
    static int *savep;    /*current position in strings array.*/

    if(first_time) {
        if(!(savep = strings = (int *) malloc(STR_MAX)))
            parse_err(E_MEMOUT);
        first_time = 0;
    }

    if(!str) {
        sprintf(size, "%ld", (long)(savep- strings));
        return size;
    }

    if(*str == '|') return (char *)(savep + 1);

    *savep++ = g_actual_lineno;

    for(textp = (char *)savep; *str; *textp++ = *str++){
        if(textp >= (char *) strings + (STR_MAX - 1))
            parse_err(E_STRINGS);
    }

    *textp++ = '\0';
    startp = (char *)savep;
    len = (int)(textp - startp);
    savep += (len / sizeof(int)) + (len % sizeof(int) != 0);

    return startp;

}

/* ---------------------------------------------------------------------
 * The parser:
 * A simple recursive descent parser that creats a thompson NFA for
 * a regular expression.
 */
PRIVATE NFA  *machine();
PRIVATE NFA  *rule();
PRIVATE void expr(NFA **startp, NFA **endp);
PRIVATE void cat_expr(NFA **startp, NFA **endp);
PRIVATE int  first_in_cat(TOKEN tok);
PRIVATE void factor(NFA **startp, NFA **endp);
PRIVATE void term(NFA **startp, NFA **endp);
PRIVATE void dodash(SET_S *set);

/*
 * machine -> rule machine
 *         -> rule END_OF_INPUT
 * */
PRIVATE NFA *machine()
{
    NFA *start;
    NFA *p;

    ENTER("machine");

    p = start = new();
    p->next = rule();

    while(!MATCH(END_OF_INPUT)) {
        p->next2 = new();
        p = p->next2;
        p->next = rule();
    }

    LEAVE("machine");
    return start;
}

/*
 * rule -> expr  EOS action
 *         ^expr EOS action
 *         expr$ EOS action
 *
 *action -> <tabs> <string of character>
 *       -> epsilon
 * */
PRIVATE NFA *rule()
{
    NFA *start = NULL;
    NFA *end   = NULL;
    int anchor = NONE;

    ENTER("rule");

    if(MATCH(AT_BOL)){
        start = new();
        start->edge = '\n';
        anchor |= START;
        advance();
        expr(&start->next, &end);
    } else {
        expr(&start, &end);
    }

    if(MATCH(AT_EOL)) {
        advance();
        end->next = new();
        end->edge = CCL;

        if(!(end->bitset = set_new())) parse_err(E_MEMOUT);

        SET_ADD(end->bitset, '\n');

        if(!g_unix_stype)SET_ADD(end->bitset, '\r');

        end = end->next;
        anchor |= END;
    }

    while(isspace (*_input)) _input++;
    end->accept = save(_input);
    end->anchor = anchor;
    advance();

    LEAVE("rule");
    return start;
}

/*
 * expr -> expr OR cat_expr
 *       | cat_expr
 * must be translated into :
 *
 * expr -> cat_expr expr'
 * expr'-> OR cat_expr expr'
 *      -> epsilon
 *
 * which can be implemented with this loop:
 *
 * cat_expr
 * while(match(OR))
 *     cat_expr
 *     do the OR
 *
 * */
PRIVATE void expr(NFA **startp, NFA **endp)
{
    NFA *e2_start = NULL;
    NFA *e2_end   = NULL;
    NFA *p;

    ENTER("expr");

    cat_expr(startp, endp);

    while(MATCH(OR)) {
        advance();
        cat_expr(&e2_start, &e2_end);
        p = new();
        p->next2 = e2_start;
        p->next  = *startp;
        *startp = p;

        p = new();
        (*endp)->next = p;
        e2_end->next = p;
        *endp = p;
    }

    LEAVE("expr");
}

PRIVATE int first_in_cat(TOKEN tok)
{
    switch(tok){
        case CLOSE_PAREN:
        case AT_EOL:
        case OR:
        case EOS: return 0;

        case CLOSRUE:
        case PLUS_CLOSE:
        case OPTIONAL: parse_err(E_CLOSE); return 0;

        case CCL_END: parse_err(E_BRACKET); return 0;
        case AT_BOL:  parse_err(E_BOL); return 0;
    }

    return 1;
}

/*
 * cat_expr -> cat_expr | factor
 *             factor
 * is translated to :
 * cat_expr -> facter cat_expr'
 * cat_expr'-> | factor cat_expr'
 *          -> epsilon
 * */
PRIVATE void cat_expr(NFA **startp, NFA **endp)
{
    NFA *e2_start, *e2_end;

    ENTER("cat_expr");

    if(first_in_cat(_current_tok))
        factor(startp, endp);

    while(first_in_cat(_current_tok)){
        factor(&e2_start, &e2_end);
        memcpy(*endp, e2_start, sizeof(NFA));
        discard(e2_start);
        *endp = e2_end;
    }

    LEAVE("cat_expr");
}

/**
 * factor -> term* | term+ | term?
 */
PRIVATE void factor(NFA **startp, NFA **endp)
{
    NFA *start, *end;

    ENTER("factor");

    term(startp, endp);

    if(MATCH(CLOSRUE) || MATCH(PLUS_CLOSE) || MATCH(OPTIONAL)){
        start = new();
        end = new();
        start->next = *startp;
        (*endp)->next = end;

        if(MATCH(CLOSRUE) || MATCH(OPTIONAL)) /* * or ? */
            start->next2 = end;

        if(MATCH(CLOSRUE) || MATCH(PLUS_CLOSE)) /* * or + */
            (*endp)->next2 = *startp;

        *startp = start;
        *endp = end;
        advance();
    }

    LEAVE("factor");

}

PRIVATE void dodash(SET_S *set){
    register int first;

    if(MATCH(DASH)) /*treat [-...] as a literal dash*/
    {
        warning(W_STARTDASH);
        SET_ADD(set, _lexeme);
        advance();
    }

    for(; !MATCH(EOS) && !MATCH(CCL_END); advance()) {
        if(!MATCH(DASH)) {
            first = _lexeme;
            SET_ADD(set, _lexeme);
        }
        else {
            advance();
            if(MATCH(CCL_END)) { /*treate [...-] as literal*/
                warning(W_ENDDASH);
                SET_ADD(set, '-');
            } else {
                for(; first <= _lexeme; ++first) {
                    SET_ADD(set, first);
                }
            }
        }
    }

}

/*
 * term --> [...] | [^...] | [] | [^] | . | (expr) | <character>
 * the [] is nonstandard. It matchs a space tab/formfeed, or newline,
 * but not a carriage return(\r). All of these are single nodes in the
 * NFA.
 * */
PRIVATE void term(NFA **startp, NFA **endp)
{
    NFA *start;
    int c;

    ENTER("term");

    if(MATCH(OPEN_PAREN)){
        advance();
        expr(startp, endp);
        if(MATCH(CLOSE_PAREN)) {
            advance();
        }else {
            parse_err(E_PAREN);
        }
    }else {
        *startp = start = new();
        *endp = start->next = new();

        if(!(MATCH(ANY) || MATCH(CCL_START))) { /*character*/
            start->edge = _lexeme;
            advance();
        }else {
            start->edge = CCL;
            if(!(start->bitset = set_new())) parse_err(E_MEMOUT);

            if(MATCH(ANY)) {                  /* . */
                SET_ADD(start->bitset, '\n');
                if(!g_unix_stype) SET_ADD(start->bitset, '\r');
                SET_COMPLEMENT(start->bitset);
            } else {                          /* [ */
                advance();
                if(MATCH(AT_BOL)) {          /*[^xxxxx]*/
                    advance();
                    SET_ADD(start->bitset, '\n');
                    if(!g_unix_stype) SET_ADD(start->bitset,'\r');
                    SET_COMPLEMENT(start->bitset);
                }
                if(!MATCH(CCL_END)) {
                    dodash(start->bitset);
                } else { /*[] or [^]*/
                    for(c = 0; c <= ' '; ++c){
                        SET_ADD(start->bitset, c);
                    }
                }
            }

            advance(); /*skip ] */
        }
    }

    LEAVE("term");
}

PUBLIC NFA *thompson(fp_input_t input_func, int *max_state, NFA **start_state)
{
    CLEAR_STACK();

    _input_func = input_func;
    _current_tok = EOS;
    advance();

    _nstates = 0;
    _next_alloc = 0;

    *start_state = machine();
    *max_state  = _next_alloc;

    if(g_verbose > 1) print_nfa(_nfa_states, *max_state, *start_state);
    if(g_verbose) {
        printf("%d/%d NFA states used\n", *max_state, NFA_MAX);
        printf("%s/%d bytes used for accept strings.\n\n", save(NULL), STR_MAX);
    }

    return _nfa_states;
}

PRIVATE void printccl(SET_S *set)
{
    static int i;

    putchar('[');
    for (i = 0; i <= 0x7f; ++i) {
        if (SET_TEST(set, i)) {
            if (i < ' ') {
                printf("^%c", i + '@');
            } else {
                printf("%c", i);
            }
        }
    }

    putchar(']');
}

PRIVATE char *plab(NFA *nfa, NFA *state)
{
    static char buf[32];

    if(!nfa || !state)
        return("--");

    sprintf(buf, "%2ld", (long)(state - nfa));
    return buf;
}

PUBLIC void  print_nfa(NFA *nfa, int len, NFA *start)
{
    NFA *s = nfa;

    printf("\n-----------NFA-----------\n");

    for(; --len >= 0; nfa++) {
        printf("NFA state %s: ", plab(s, nfa));


        if(!nfa->next) {
            printf("TERMINAL");
        } else {
            printf("--> %s", plab(s, nfa->next));
            printf("(%s) on ", plab(s, nfa->next2));

            switch(nfa->edge) {
                case CCL: printccl(nfa->bitset); break;
                case EPSILON: printf("EPSILON"); break;
                default: sys_pchar(nfa->edge, stdout); break;
            }
        }

        if(nfa == start)printf("(START STATE)");

        if(nfa->accept) printf(" accepting %s<%s>%s", nfa->anchor &START ? "^": "", nfa->accept,
                                                      nfa->anchor & END ? "$": "");
        printf("\n");
    }

    printf("\n------------------------------------\n");
}

#ifdef MAIN
PRIVATE char *get_line()
{
    static char buf[80];
    static int i = 1;
    printf("%d: ", g_lineno++);

    if(i-- ==  0) return NULL;
    sprintf(buf, "%s", "^he?lffke.*l\\n(hii)*lo*[^a-b^]c$ return OK;");
    return buf;
}

int main(int argc, char **argv) {
    NFA *nfa, *start_state;

    int max_state;

    g_verbose = 2;

    nfa = thompson(get_line, &max_state, &start_state);

    free(nfa);
}
#endif