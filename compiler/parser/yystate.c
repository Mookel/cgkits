//
// Created by Mookel on 16/10/11.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yystate.c : 
//

#include "parser.h"
#include "llout.h"

/*various macro definitions*/
#define MAXSTATE        1024 /*max number of LALR(1) states.*/
#define MAXOBUF         512  /*buffer size for various output routines.*/

#define MAXKERNEL       64   /*maximum number of kernel items in a state. */
#define MAXCLOSE        256  /*maximum number of closure items in a state.*/
#define MAXEPSILON      16   /*maximum number of epsilon productions that can be
                               in a closure set for any given state.*/

#define CHUNK           128  /*New() gets this many structures at once.*/
#define MAXUNFINISHED   128

#define MAXTOKPERLINE   10

#define NEW             0    /*possible return values from newstate().*/
#define UNCLOSED        1
#define CLOSED          2

#define RIGH_OF_DOT(p)  ((p)->right_of_dot ? (p)->right_of_dot->val : 0)

/*different typedef structures.*/
typedef struct item_ {                 /*LR(1) item */
    int prod_num;                      /*production number */
    PRODUCTION_S *prod;                /*the production itself */
    SYMBOL_S     *right_of_dot;        /*symbol to the right of dot */
    unsigned int  dot_posn;            /*offset of dot from start of production.*/
    SET_S        *lookaheads;          /*set of lookhead symbols for this item. */
}ITEM_S;

typedef unsigned int STATENUM;

typedef struct _state{                 /*LR(1) state */
    ITEM_S *kernel_items[MAXKERNEL];   /*set of kernel items */
    ITEM_S *epsilon_items[MAXEPSILON]; /*set of epsilon items */
    unsigned short nkitems: 7;         /*number of items in kernel_items */
    unsigned short neitems: 7;         /*number of items in epsilon_items*/
    unsigned short closed:  1;         /*state has had closure performed */
    STATENUM num;                      /*state number (0 as the start state.)*/
}STATE_S;

typedef struct act_or_goto_{
    int sym;                           /*given this input symbol. */
    int do_this;                       /*do action: >0 shift, <0 reduce*/
    struct act_or_goto_ *next;         /*Pointer to next ACT_S in the linked list.*/
}ACT_S;

typedef ACT_S GOTO;                    /*GOTO as an alias for ACT_S.*/

typedef struct stnode_{
    STATE_S *state;
    struct stnode_ *left, *right;
}TNODE_S;

typedef struct state_cmp_info_         /*used to pass info to state_cmp*/
{
    ITEM_S **state_items;
    int      state_nitems;
    bool     sort_by_number;

}STATE_CMP_INFO_S;

/*static local variables definitions.*/
PRIVATE int _nitems = 0;               /*number of LR(1) items*/
PRIVATE int _npairs = 0;               /*number of pairs in output tables.*/
PRIVATE int _ntab_entries   = 0;       /*number of transitions in tables.*/
PRIVATE int _nshift_reduce  = 0;       /*number of shift/reduce conflicts.*/
PRIVATE int _nreduce_reduce = 0;       /*number of reduce/reduce conflicts.*/

PRIVATE ACT_S *_actions[MAXSTATE];     /*Array of pointers to the head of action chains,
                                         indexed by state number.*/
PRIVATE GOTO  *_gotos[MAXSTATE];       /*Array of pointers to the head of goto chains.*/
PRIVATE HASH_TAB_S *_states = NULL;    /*LR(1) states*/
PRIVATE int   _nstates = 0;            /*number of states.*/

PRIVATE TNODE_S _heap[MAXUNFINISHED];  /*Source of all TNODES*/
PRIVATE TNODE_S *_next_alloc_tnode = _heap; /*pointer to next node to allocate.*/
PRIVATE TNODE_S *_available = NULL;    /*free list of available nodes linked list of TNODES.
                                         P->left is used as the link.*/
PRIVATE TNODE_S *_unfinised = NULL;    /*Tree of unfinished states.*/

PRIVATE STATE_CMP_INFO_S _state_cmp_info;

PRIVATE ITEM_S *_recycled_items = NULL;

PRIVATE int  _tokens_printed;          /*control number of lookaheads printed on a single line of
                                         yyout.doc*/

/*local static functions declaration*/
PRIVATE char *strprod(PRODUCTION_S *prod);
PRIVATE void  add_action(int state, int input_sym, int do_this);
PRIVATE void  add_goto(int state, int nonterminal, int go_here);
PRIVATE int   add_lookahead(SET_S *dst, SET_S *src);
PRIVATE void  add_reductions(STATE_S *state, void *junk);
PRIVATE void  add_unfinished(STATE_S *state);
PRIVATE int   closure(STATE_S *kernel, ITEM_S **closure_items, int maxitems);
PRIVATE int   do_close(ITEM_S *item, ITEM_S **closure_items, int *nitems, int *maxitems);
PRIVATE void  freeitem(ITEM_S *item);
PRIVATE void  free_recycled_item(void);
PRIVATE STATE_S *get_unfinished(void);
PRIVATE ITEM_S  *in_closure_items(PRODUCTION_S *prod, ITEM_S **closure_item, int nitems);
PRIVATE int   item_cmp(ITEM_S **item1p, ITEM_S **item2p);
PRIVATE int   kclosure(STATE_S *kernel, ITEM_S **closure_items, int maxitems, int nclose);
PRIVATE int   lr(STATE_S *cur_state);

/*table making functions*/
PRIVATE void  make_yy_lhs(PRODUCTION_S **prodtab);
PRIVATE void  make_yy_reduce(PRODUCTION_S **prodtab);
PRIVATE void  make_yy_slhs(PRODUCTION_S **prodtab);
PRIVATE void  make_yy_srhs(PRODUCTION_S **prodtab);

PRIVATE int   merge_lookaheads(ITEM_S **dst_items, ITEM_S **src_items, int nitems);
PRIVATE void  mkprod(SYMBOL_S *sym, PRODUCTION_S **prodtab);
PRIVATE void  movedot(ITEM_S *item);
PRIVATE int   move_eps(STATE_S *cur_state, ITEM_S **closure_items, int nclose);

PRIVATE ACT_S  *new();
PRIVATE ITEM_S *newitem(PRODUCTION_S *prod);
PRIVATE int    newstate(ITEM_S **items, int nitems, STATE_S **statep);

/*printing-relative functions*/
PRIVATE ACT_S *p_action(int state, int input_sym);
PRIVATE void  pclosure(STATE_S *kernel, ITEM_S **closure_items, int nitems);
PRIVATE GOTO  *p_goto(int state, int nonterminal);
PRIVATE void  print_reductions(void);
PRIVATE void  print_tab(ACT_S **table, char *row_name, char *col_name, bool private);
PRIVATE void  pstate(STATE_S *state);
PRIVATE void  pstate_stdout(STATE_S *state);
PRIVATE void  reduce_one_item(STATE_S *state, ITEM_S *item);
PRIVATE void  reductions(void);
PRIVATE void  sprint_tok(char **bp, char *format, int arg);
PRIVATE int   state_cmp(STATE_S *new, STATE_S *tab_node);
PRIVATE unsigned state_hash(STATE_S *sym);
PRIVATE char  *stritem(ITEM_S *item, int lookaheads);

/*public interfaces implementation*/
PUBLIC void lr_stats(FILE *fp)
{
    fprintf(fp, "%4d    LALR(1) states\n", _nstates);
    fprintf(fp, "%4d    items\n", _nitems);
    fprintf(fp, "%4d    nonerror transitions in tables\n", _ntab_entries);
    fprintf(fp, "%4ld/%-4d unfinished items\n", (long)(_next_alloc_tnode-_heap), MAXUNFINISHED);
    fprintf(fp, "%4d    bytes required for LALR(1) transition matrix\n",
            (2 * sizeof(char*) * _nstates) /*index arrays*/
            + _nstates                     /*count field*/
            + (_npairs * sizeof(short))    /*pairs*/
    );
    fprintf(fp, "\n");
}

PUBLIC int lr_conflicts(FILE *fp)
{
    fprintf(fp, "%4d    shift/reduce conflicts\n", _nshift_reduce);
    fprintf(fp, "%4d    reduce/reduce conflicts\n", _nreduce_reduce);
    return _nshift_reduce + _nreduce_reduce;
}

/*
 * Print an LALR(1) transition matrix for the grammar currently represented
 * in the symbol table.
 */
PUBLIC void make_parse_tables()
{
    ITEM_S *item;
    STATE_S *state;
    PRODUCTION_S *start_prod;

    /* Make data structures used to produce the table, and create an initial LR(1)
     * item containing the start production and the EOI as the lookahead symbol.
     */
    _states = hash_make_tab(MAXSTATE, state_hash, state_cmp);

    if(!g_goal_symbol) error(FATAL, "No goal symbol.\n");

    start_prod = g_goal_symbol->productions;

    if(start_prod->next)
        error(FATAL, "Start symbol must have only one right-hand side.\n");

    item = newitem(start_prod);
    SET_ADD(item->lookaheads, _EOI_);

    newstate(&item, 1, &state);

    if(lr(state)){
        if(g_cmdopt.verbose) printf("Adding reductions:\n");
        reductions();
        if(g_cmdopt.verbose) printf("Creating tables:\n");

        if(!g_cmdopt.make_yyoutab) { /*tables go in yyout.c*/
            print_tab(_actions, "Yya", "Yy_action", true);
            print_tab(_gotos, "Yyg", "Yy_goto", true);
        } else {
            if() {

            } else {

            }
        }
        print_reductions();
    }
}