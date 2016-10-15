//
// Created by Mookel on 16/10/11.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yystate.c : 
//

#include <stdlib.h>
#include "parser.h"
#include "llout.h"

/*-----------------various macro definitions--------------*/

#define MAXSTATE        1024 /*max number of LALR(1) states.*/
#define MAXOBUF         256  /*buffer size for various output routines.*/

#define MAXKERNEL       64   /*maximum number of kernel items in a state. */
#define MAXCLOSE        256  /*maximum number of closure items in a state.*/
#define MAXEPSILON      16   /*maximum number of epsilon productions that can be
                               in a closure set for any given state.*/

#define CHUNK           128  /*New() gets this many structures at once.*/
#define MAXUNFINISHED   257

#define MAXTOKPERLINE   10

#define NEW             0    /*possible return values from newstate().*/
#define UNCLOSED        1
#define CLOSED          2

#define RIGH_OF_DOT(p)  ((p)->right_of_dot ? (p)->right_of_dot->val : 0)

/*-----------------different typedef structures--------------*/

typedef struct item_ {                 /*LR(1) item */
    unsigned prod_num;                 /*production number */
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

/*-----------------static local variables definitions--------------*/

PRIVATE HASH_TAB_S *_states_hashtab = NULL; /*LR(1) states*/

PRIVATE int _nitems = 0;               /*number of LR(1) items*/
PRIVATE int _npairs = 0;               /*number of pairs in output tables.*/
PRIVATE int _ntab_entries   = 0;       /*number of transitions in tables.*/
PRIVATE int _nshift_reduce  = 0;       /*number of shift/reduce conflicts.*/
PRIVATE int _nreduce_reduce = 0;       /*number of reduce/reduce conflicts.*/
PRIVATE int _nstates = 0;              /*number of states.*/

PRIVATE ACT_S *_actions[MAXSTATE];     /*Array of pointers to the head of action chains,
                                         indexed by state number.*/
PRIVATE GOTO  *_gotos[MAXSTATE];       /*Array of pointers to the head of goto chains.*/

PRIVATE TNODE_S _heap[MAXUNFINISHED];  /*Source of all TNODES*/
PRIVATE TNODE_S *_next_alloc_tnode = _heap; /*pointer to next node to allocate.*/
PRIVATE TNODE_S *_available = NULL;    /*free list of available nodes linked list of TNODES.
                                         P->left is used as the link.*/
PRIVATE TNODE_S *_unfinised = NULL;    /*Tree of unfinished states.*/

PRIVATE ITEM_S *_recycled_items = NULL;/*head of free list of items.*/

PRIVATE int  _tokens_printed;          /*control number of lookaheads printed on a single line of
                                         yyout.doc*/

PRIVATE STATE_CMP_INFO_S _state_cmp_info;

/*-----------------local static functions declaration--------------*/

/*hash table relative*/
PRIVATE unsigned state_hash(STATE_S *sym);
PRIVATE int    state_cmp(STATE_S *new, STATE_S *tab_node);

/*factory functions*/
PRIVATE ACT_S  *newact();
PRIVATE ITEM_S *newitem(PRODUCTION_S *prod);
PRIVATE void   freeitem(ITEM_S *item);
PRIVATE void   free_recycled_items(void);
PRIVATE int    newstate(ITEM_S **items, int nitems, STATE_S **statep);

/*closure-relative functions*/
PRIVATE bool   add_lookahead(SET_S *dst, SET_S *src);
PRIVATE bool   do_close(ITEM_S *item, ITEM_S **closure_items, int *nitems, int *maxitems);
PRIVATE ITEM_S  *in_closure_items(PRODUCTION_S *prod, ITEM_S **closure_item, int nitems);
PRIVATE int    closure(STATE_S *kernel, ITEM_S **closure_items, int maxitems);
PRIVATE int    kclosure(STATE_S *kernel, ITEM_S **closure_items, int maxitems, int nclose);

/*lr-relative functions*/
PRIVATE void   add_unfinished(STATE_S *state);
PRIVATE STATE_S *get_unfinished(void);
PRIVATE int    item_cmp(ITEM_S **item1p, ITEM_S **item2p);
PRIVATE int    move_eps(STATE_S *cur_state, ITEM_S **closure_items, int nclose);
PRIVATE void   movedot(ITEM_S *item);
PRIVATE ACT_S *p_action(int state, int input_sym);
PRIVATE void   add_action(int state, int input_sym, int do_this);
PRIVATE GOTO  *p_goto(int state, int nonterminal);
PRIVATE void   add_goto(int state, int nonterminal, int go_here);
PRIVATE bool   merge_lookaheads(ITEM_S **dst_items, ITEM_S **src_items, int nitems);
PRIVATE int    lr(STATE_S *cur_state);

/*reduction-relative functions*/
PRIVATE void   add_reductions(STATE_S *state, void *junk);
PRIVATE void   reduce_one_item(STATE_S *state, ITEM_S *item);
PRIVATE void   reductions(void);

/*table making functions*/
PRIVATE void   make_yy_lhs(PRODUCTION_S **prodtab);
PRIVATE void   make_yy_reduce(PRODUCTION_S **prodtab);
PRIVATE void   make_yy_slhs(PRODUCTION_S **prodtab);
PRIVATE void   make_yy_srhs(PRODUCTION_S **prodtab);
PRIVATE void   mkprodtab(SYMBOL_S *sym, PRODUCTION_S **prodtab);
PRIVATE void   print_reductions(void);
PRIVATE void   print_tab(ACT_S **table, char *row_name, char *col_name, bool private);

/*printing-relative functions*/
PRIVATE void   sprint_tok(char **bp, char *format, int arg);
PRIVATE char  *stritem(ITEM_S *item, bool lookaheads);
PRIVATE char  *strprod(PRODUCTION_S *prod);
PRIVATE void   pstate(STATE_S *state);
PRIVATE void   pclosure_stdout(STATE_S *kernel, ITEM_S **closure_items, int nitems);
PRIVATE void   pstate_stdout(STATE_S *state);

/*-----------------public interfaces implementation--------------*/

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
    FILE *fp, *old_output;

    /* Make data structures used to produce the table, and create an initial LR(1)
     * item containing the start production and the EOI as the lookahead symbol.
     */
    _states_hashtab = hash_make_tab(MAXSTATE, (fp_hash_t)state_hash, (fp_hash_cmp_t)state_cmp);

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
            if(!(fp = fopen(TAB_FILE, "w"))) {
                error(NONFATAL, "Can't open %s, ignoring -T\n", TAB_FILE);
                print_tab(_actions, "Yya", "Yy_action", true);
                print_tab(_gotos, "Yyg", "Yy_goto", true);
            } else {
                output("extern YY_TTYPE *Yy_action[]; /*in yyoutab.c*/\n");
                output("extern YY_TTYPE *Yy_goto[];   /*in yyoutab.c*/\n");
                old_output = g_output;
                g_output = fp;
                fprintf(fp, "#include <stdio.h>\n");
                fprintf(fp, "typedef short YY_TTYPE;\n");
                fprintf(fp, "#define YYPRIVATE %s\n", g_cmdopt.public ? "/*empty*/" : "static");
                print_tab(_actions, "Yya", "Yy_action", false);
                print_tab(_gotos, "Yyg", "Yy_goto", false);
                fclose(fp);
                g_output = old_output;
            }
        }
        print_reductions();
    }
}

/*
 * Hash function for STATEs, Sum together production numbers and
 * do positions of the kernel items.
 */
PRIVATE unsigned state_hash(STATE_S *sym)
{
    (void)sym; /*just ignore.*/

    ITEM_S **items;
    int    nitems;
    unsigned total = 0;

    items = _state_cmp_info.state_items;
    nitems = _state_cmp_info.state_nitems;

    for(; --nitems >= 0; ++items) {
        total += (*items)->prod_num + (*items)->dot_posn;
    }

    return total;
}

/*
 * Compare two states as described in the text.
 * if sort_by_number is false, new is ignore.
 */
PRIVATE int state_cmp(STATE_S *new, STATE_S *tab_node)
{
    ITEM_S **tab_item;
    ITEM_S **item;
    int   nitems;
    int   cmp;

    if(_state_cmp_info.sort_by_number)
        return (new->num - tab_node->num);

    /*state with largest number of items is larger.*/
    if((cmp = _state_cmp_info.state_nitems - tab_node->nkitems))
        return cmp;

    nitems = _state_cmp_info.state_nitems;
    item   = _state_cmp_info.state_items;
    tab_item = tab_node->kernel_items;

    for(; --nitems >= 0; ++tab_item, ++item){
        if((cmp = (*item)->prod_num - (*tab_item)->prod_num))
            return cmp;

        if((cmp = (*item)->dot_posn - (*tab_item)->dot_posn))
            return cmp;
    }

    return 0;
}

/*
 * Return an area of memory that can be used as either an ACT or GOTO.
 */
PRIVATE ACT_S  *newact()
{
    static ACT_S *eheap = NULL;
    static ACT_S *heap  = NULL;

    if(heap == eheap) {
        if(!(heap = (ACT_S *)GC_MALLOC(sizeof(ACT_S) * CHUNK)))
            error(FATAL, "INTERNAL ERROR: No memory for action or goto.\n");
        eheap = heap + CHUNK - 1;
    }
    ++_ntab_entries;
    return heap++;
}

/*
 * return a new item by the prod.
 * Note: A free-list of item is maintained using prod as the next field and
 * _recycled_items as the head.
 */
PRIVATE ITEM_S *newitem(PRODUCTION_S *prod)
{
    ITEM_S *item;

    if(_recycled_items) {
        item = _recycled_items;
        _recycled_items = (ITEM_S *) _recycled_items->prod;  /*link-list like,prod as the next*/
        SET_CLEAR(item->lookaheads);
    } else {
        if(!(item = (ITEM_S *)GC_MALLOC(sizeof(ITEM_S))))
            error(FATAL, "INTERNAL ERROR: No memory for all LR(1) items.\n");
        item->lookaheads = set_new();
    }

    ++_nitems;
    item->prod = prod;
    item->prod_num = prod->num;
    item->dot_posn = 0;
    item->right_of_dot = prod->rhs[0]; /*if production is epsilon, the right of dot will be NULL.*/
    return item;
}

PRIVATE void freeitem(ITEM_S *item)
{
    --_nitems;
    item->prod = (PRODUCTION_S*)_recycled_items;
    _recycled_items = item;
}

/*
 * Free all items in free-list.
 */
PRIVATE void free_recycled_items(void)
{
    ITEM_S *p;

    while((p = _recycled_items)) {
        _recycled_items = (ITEM_S *)_recycled_items->prod;
        GC_FREE(p);
    }
}

/*
 * Create new state base on kernel items (number of nitems)
 */
PRIVATE int newstate(ITEM_S **items, int nitems, STATE_S **statep)
{
    STATE_S *state;
    STATE_S *existing;

    if(nitems > MAXKERNEL)
        error(FATAL, "Kernel of new state %d too large.\n", _nstates);

    _state_cmp_info.state_items = items;
    _state_cmp_info.state_nitems = nitems;

    if((existing = (STATE_S *) hash_find_sym(_states_hashtab, NULL))){
        *statep = existing;
        if(g_cmdopt.verbose > 1) {
            printf("Using existing state (%sclosed);",
            existing->closed ? "" : "un");
            pstate_stdout(existing);
        }
        return existing->closed ? CLOSED : UNCLOSED;
    } else {
        if(_nstates >= MAXSTATE)
            error(FATAL, "Too many LALR(1) states.\n");

        if(!(state = (STATE_S *)hash_new_sym(sizeof(STATE_S))))
            error(FATAL, "INTERNAL ERROR: Insufficient memroy for states.\n");

        /*copy items into state and initialize all other fields.*/
        memcpy(state->kernel_items, items, nitems * sizeof(ITEM_S*));
        state->nkitems = nitems;
        state->neitems = 0;
        state->closed  = 0;
        state->num = _nstates++;
        *statep = state;
        hash_add_sym(_states_hashtab, state);

        if(g_cmdopt.verbose > 1){
            printf("Forming new state: ");
            pstate_stdout(state);
        }

        return NEW;
    }
}

#ifdef DEBUG
void print_unfinished(TNODE_S *root)
{
    if(!root) return;

    print_unfinished(root->left);
    printf("Node %p, left = %p, right = %p, state = %p, state->num = %d\n",
    root, root->left, root->right, root->state, root->state->num);
    print_unfinished(root->right);
}
#endif

PRIVATE void add_unfinished(STATE_S *state)
{
    TNODE_S **parent, *root;
    int cmp;

    parent = &_unfinised;
    root = _unfinised;

    while(root) {
        if((cmp = state->num - root->state->num) == 0) {
            break;  /*found it.*/
        } else {
            parent = (cmp < 0) ? &root->left : &root->right;
            root = *parent;
        }
    }

    if(!root) {    /*not found it*/
        if(_available) {
            *parent = _available;
            _available = _available->left;  /*using left as the next pointer of free list*/
        } else {
            if(_next_alloc_tnode >= &_heap[MAXUNFINISHED])
                error(FATAL, "INTERNAL ERROR: Insufficient memory for unfinished state.\n");
            *parent = _next_alloc_tnode++;
        }

        (*parent)->state = state;
        (*parent)->left = (*parent)->right = NULL;
    }

    D(printf("\n Added state %d to unfinished tree: \n", state->num);)
    D(print_unfinished(_unfinised);)
    D(printf("\n");)
}

/*
 * Returns a pointer to the next unfinished state and delete that state from
 * the unfinished tree. Returns NULL if the tree is empty.
 */
PRIVATE STATE_S *get_unfinished(void)
{
    TNODE_S *root;
    TNODE_S **parent;

    if(!_unfinised) return NULL;

    parent = &_unfinised;
    if((root = _unfinised)) {
        while(root->left) {
            parent = &root->left;
            root = root->left;
        }
    }

    *parent = root->right;
    root->left = _available;
    _available = root;

    D(printf("\nReturning state %d from unfinished tree:\n", root->state->num);)
    D(print_unfinished(_unfinised);)
    D(printf("\n");)

    return root->state;
}

/*
 * Return the relative weight of two items, 0 if they are equivalent.
 */
PRIVATE int item_cmp(ITEM_S **item1p, ITEM_S **item2p)
{
    int rval;
    ITEM_S *item1 = *item1p;
    ITEM_S *item2 = *item2p;

    if(!(rval = RIGH_OF_DOT(item1) - RIGH_OF_DOT(item2)))
    if(!(rval = item1->prod_num - item2->prod_num))
        return item1->dot_posn - item2->dot_posn;
    return rval;
}

/*
 * Move the epsilon items from the closure_items set to the kernel of the
 * current state. If epsilon items already exist in the current state,just
 * merge the lookaheads. Note that,because the closure items were sorted
 * to partition them, the epsilon productions in the closure_items set
 * will be in the same order as those already in the kernel. Return the
 * number of items that were moved.
 */
PRIVATE int move_eps(STATE_S *cur_state, ITEM_S **closure_items, int nclose)
{
    ITEM_S **eps_items, **p;
    int nitems, moved;

    eps_items = cur_state->epsilon_items;
    nitems = cur_state->neitems;
    moved = 0;

    for(p = closure_items; (*p)->prod->rhs_len == 0 && --nclose >= 0;) {
        if(++moved > MAXEPSILON)
            error(FATAL, "Too many epsilon production in state %d\n", cur_state->num);
        if(nitems) {
            SET_UNION((*eps_items++)->lookaheads, (*p++)->lookaheads);
        } else  {
            *eps_items++ = *p++;
            D(if(!p || !(*p)->prod))
            D( error(FATAL, "Bad pointer in move_eps.\n");)
        }
    }

    if(moved) cur_state->neitems = moved;

    return moved;
    return 0;
}

/*
 * If the indicated production is in the closure_items already, return a pointer
 * to the existing item, otherwise return NULL.
 */
PRIVATE ITEM_S *in_closure_items(PRODUCTION_S *prod, ITEM_S **closure_item, int nitems)
{
    for(; --nitems >= 0; ++closure_item) {
        if((*closure_item)->prod == prod) return *closure_item;
    }

    return NULL;
}

/*
 * Merge the lookaheads in the src and dst sets. If the original src
 * was empty, or if it was already a subset of the dst destination
 * set, return false, or return true;
 */
PRIVATE bool add_lookahead(SET_S *dst, SET_S *src)
{
    if(!SET_IS_EMPTY(src) && !set_subset(dst, src)) {
        SET_UNION(dst, src);
        return true;
    }

    return false;
}

/*
 * Workhorse function used by closure().Performs LR(1) closure on the input item
 * ([A->b.Cd, e] add [C->x, FIRST(de)]). The new items are added to the closure_
 * items[] array and *nitems and *maxitems are modfied to reflect the number of
 * items in the closure set.Return true if do_close do something, false if no
 * items were added(as will be the case if the dot is at the far right of the
 * production or the symbol to the right of the dot is a terminal).
 */
PRIVATE bool do_close(ITEM_S *item, ITEM_S **closure_items, int *nitems, int *maxitems)
{
    bool did_something = false;
    bool rhs_is_nullable;
    PRODUCTION_S *prod;
    ITEM_S *close_item;
    SET_S *closure_set;
    SYMBOL_S **symp;

    if(!item->right_of_dot) return false;
    if(!ISNONTERM(item->right_of_dot)) return false;

    closure_set = set_new();
    /*
     * The symbol to the right of the dot is a nonterminal. Do the following:
     * (1) for(every production attached to the nonterminal)
     * (2)     if(the current production is not already in the set of closure items)
     * (3)         add it.
     * (4)     if(the d in [A->b.Cd, e] doesn't exist)
     * (5)         add e to the lookaheads in the closure production.
     *         else (the d in [A->b.Cd, e] does exist)
     * (6)         compute FIRST(de) and add it to the lookaheads for the current
     *             item if necessary.
     */
    for(prod = item->right_of_dot->productions; prod; prod = prod->next) {
        if(!(close_item = in_closure_items(prod, closure_items, *nitems))){
            if(--(*maxitems) < 0) error(FATAL, "LR(1) Closure set too large.\n");
            closure_items[(*nitems)++] = close_item = newitem(prod);
            did_something = true;
        }

        if(!*(symp = &(item->prod->rhs[item->dot_posn + 1]))) {
            did_something |= add_lookahead(close_item->lookaheads, item->lookaheads);
        } else  {
            set_truncate(closure_set);
            rhs_is_nullable = first_rhs(closure_set, symp, item->prod->rhs_len - item->dot_posn - 1);
            SET_REMOVE(closure_set, EPSILON);
            if(rhs_is_nullable) SET_UNION(closure_set, item->lookaheads);
            did_something |= add_lookahead(close_item->lookaheads, closure_set);
        }
    }

    return did_something;
}

/*
 * Do LR(1) closure on the kernel items array in the input STATE. When
 * finished, closure_items[] will hold the new items. The logic is:
 * (1) for (each kenel kernel item)
 *         do LR(1) closure on that item.
 * (2) while(items were added in the previous step or are added below)
 *         do LR(1) closure on the items that were added.
 */
PRIVATE int closure(STATE_S *kernel, ITEM_S **closure_items, int maxitems)
{
    int i;
    int nclose  = 0;    /*current number of closure items in closure_items.*/
    bool did_something = false;
    ITEM_S **p = kernel->kernel_items;

    for(i = kernel->nkitems; --i >= 0;){
        did_something |= do_close(*p++, closure_items, &nclose, &maxitems);
    }

    while(did_something) {
        did_something = false;
        p = closure_items;
        for(i = nclose; --i >= 0;) {
            did_something |= do_close(*p++, closure_items, &nclose, &maxitems);
        }
    }

    return nclose;
}

/*
 * Adds to the closure set those items from the kernel that will shift to new
 * states(ie. the items with dots somewhere other than the far right.)
 */
PRIVATE int kclosure(STATE_S *kernel, ITEM_S **closure_items, int maxitems, int nclose)
{
    int nitems;
    ITEM_S *item, **itemp, *citem;

    closure_items += nclose;
    maxitems -= nclose;

    itemp = kernel->kernel_items;
    nitems = kernel->nkitems;

    while(--nitems >= 0) {
        item = *itemp++;

        if(item->right_of_dot) {
            citem = newitem(item->prod);
            citem->dot_posn = item->dot_posn;
            citem->right_of_dot = item->right_of_dot;
            citem->lookaheads = set_dup(item->lookaheads);

            if(--maxitems < 0)
                error(FATAL, "Too many closure items in state %d.\n", kernel->num);
            *closure_items++ = citem;
            ++nclose;
        }
    }

    return nclose;
}

/*
 * Moves the dot one position to the right and updates the right_of_dot
 * symbol.
 */
PRIVATE void movedot(ITEM_S *item)
{
    D(if(item->right_of_dot == NULL))
    D(    error(FATAL, "Illegal movedot() call on epsilon production.");)

    item->right_of_dot = (item->prod->rhs)[++item->dot_posn];
}

/*
 * Return a pointer to the existing ACT structure repersenting the indicated
 * state and input symbol (or NULL if no such symbol exists).
 */
PRIVATE ACT_S *p_action(int state, int input_sym)
{
    ACT_S *p;

    D(if(state > MAXSTATE))
    D(  error(FATAL, "bad state argument to p_action (%d)\n", state);)

    for(p = _actions[state]; p ; p = p->next){
        if(p->sym == input_sym) return p;
    }

    return NULL;
}

/*
 * Add an element to the action part of the parse table. The cell is indexed
 * by state number and input symbol, and holds do_this.
 */
PRIVATE void add_action(int state, int input_sym, int do_this)
{
    ACT_S *p;
    if(g_cmdopt.verbose > 1)
        printf("Adding shift or reduce action from state %d: %d on %s.\n",
        state, do_this, g_terms[input_sym]->name);

    p = newact();
    p->sym = input_sym;
    p->do_this = do_this;
    p->next = _actions[state];
    _actions[state] = p;
}

/*
 * Return a pointer to the existing GOTO structure representing the indicated
 * state and nonterminal(or NULL if no such symbol.)
 */
PRIVATE GOTO *p_goto(int state, int nonterminal)
{
    GOTO *p;

    nonterminal = ADJ_VAL(nonterminal);
    D(if(nonterminal > NUMNONTERMS))
    D(  error(FATAL, "bad argument to p_goto.\n");)
    for(p = _gotos[state]; p; p = p->next) {
        if(p->sym == nonterminal) return p;
    }

    return NULL;
}

/*
 * Add an element to the goto part of the parse table, the cell is indexed
 * by current state number and nonterminal value ,and holds go_there, Note
 * that the input nonterminal value is the one that appears in the symbol
 * table. It is adjusted downwards(so that the smallest nonterminal will
 * have the value 0) befor being inserted into the table.
 */
PRIVATE void add_goto(int state, int nonterminal, int go_here)
{
    GOTO *p;
    int unadjusted;

    unadjusted = nonterminal;
    nonterminal = ADJ_VAL(nonterminal);

#ifdef DEBUG
    if(nonterminal > NUMNONTERMS)
        error(FATAL, "bad argument to add_goto.\n");
    if(p = p_goto(state, unadjusted)) {
        error(FATAL, "Tried to add duplicate goto on nonterminal %s\n"
                "    (1)goto %3d from %3d <- existing\n"
                "    (2)goto %3d from %3d <- new\n",
              g_terms[unadjusted]->name,
              p->do_this, state,/*bug fix: it's written with go_there in original code.*/
              go_here, state
        );
    }
#endif

    if(g_cmdopt.verbose > 1)
        printf("Adding goto from state %d to %d on %s.\n",
        state, go_here, g_terms[unadjusted]->name);
    p = newact();
    p->sym = nonterminal;
    p->do_this = go_here;
    p->next = _gotos[state];
    _gotos[state] = p;
}

/*
 * The routine is called if newstate has determined that a state having the
 * specified items already exists. If this is the case, the item list in the
 * STATE and the current item list will be identical in all respects expcept
 * lookaheads. This routine merges the lookahead of the input items(src_itesm)
 * to the items already in the state(dst_items). false is returned if nothing
 * was done, true otherwise.
 */
PRIVATE bool merge_lookaheads(ITEM_S **dst_items, ITEM_S **src_items, int nitems)
{
    bool did_something = false;

    while(--nitems >= 0) {
        if(((*dst_items)->prod != (*src_items)->prod)
                || (*dst_items)->dot_posn != (*src_items)->dot_posn){
            error(FATAL, "INTERNAL ERROR: [merge_lookaheads], item mismatched\n");
        }

        if(!set_subset((*dst_items)->lookaheads, (*src_items)->lookaheads)) {
            did_something = true;
            SET_UNION((*dst_items)->lookaheads, (*src_items)->lookaheads);
        }

        ++dst_items;
        ++src_items;
    }

    return did_something;
}

/*
 * Make LALR(1) state machine.The shifts and gotos are done here, the
 * reductions are done elsewhere.Return the number fo states.
 */
PRIVATE int lr(STATE_S *cur_state)
{
    ITEM_S **p;
    ITEM_S **first_item;
    ITEM_S *closure_items[MAXCLOSE];
    STATE_S *next;     /*next state.*/
    int  isnew;        /*next state is a new state*/
    int  nclose;       /*number of items in closure_items*/
    int  nitems;       /*number of items with same symbol to right of dot*/
    int val;           /*value of symbol to right of dot.*/
    SYMBOL_S *sym;     /*Actual symbol to right of dot.*/
    int nlr = 0;       /*_nstates + nlr == number of LR(1) states.*/

    add_unfinished(cur_state);

    /*
     * Note: there are three important functions used in this subroutine:
     *
     * (1) closure -> adds normal closure items to closure_items array.
     * (2) kclosure -> adds to that set all items in the kernel that have
     *     outgoing transitions(ie. whose dots aren't at the far right.)
     * (3) sort -> sorts the closure items by the symbol to the right
     *     of the dot. Epsilon transitions will sort to the head of the
     *     list, followed by transitions on nonterminals, followed by
     *     transitions on terminals.
     * (4) move_eps -> moves the epsilon transitions into the closure
     *     kernel set. It returns the number of items that it moved.
     */
    while((cur_state = get_unfinished())) {

        if(g_cmdopt.verbose > 1)
            printf("Next pass... working on state %d\n", cur_state->num);
        nclose = closure(cur_state, closure_items, MAXCLOSE);
        nclose = kclosure(cur_state, closure_items, MAXCLOSE, nclose);

        if(!nclose) {
            if(g_cmdopt.verbose > 1)
                printf("There were NO closure items added.\n");
        } else  {
            sys_ssort((void**)closure_items, nclose, sizeof(ITEM_S*),
                      (fp_cmp_func_t)item_cmp);
            nitems = move_eps(cur_state, closure_items, nclose);
            p = closure_items + nitems;
            nclose -= nitems;
            if(g_cmdopt.verbose > 1) pclosure_stdout(cur_state, p, nclose);
        }

        while(nclose > 0) {
            first_item = p;
            sym = (*first_item)->right_of_dot;
            val = sym->val;

            /*
             * Collect all items with the same symbol to the right of the dot.
             * On exiting the loop, nitems will hold the number of those items
             * and p will point at the first nonmatching item.Finally nclose is
             * decremented by nitems, items = 0;
             */
            nitems = 0;
            do{
                movedot(*p++);
                ++nitems;
            } while(--nclose > 0 && RIGH_OF_DOT(*p) == val);

            /*
             * Following will do the work:
             * (1) newstate() gets the next state. It returns NEW if the state
             * didn't exist previously, CLOSED if LR(0) closure has been performed
             * on the state, UNCLOSED otherwise.
             * (2) Add a transition from the current state to the next state.
             * (3) If it's a brand-new state, add it to the unfinished list.
             * (4) Otherwise merge the lookaheads created by the current closure
             * operation with the ones already in the state.
             * (5) If The merge operation added lookaheads to the existing set,
             * add it to the unfinished list.
             */
            isnew = newstate(first_item, nitems, &next);
            if(!cur_state->closed) {
                if(ISTERM(sym)) {
                    add_action(cur_state->num, val, next->num);
                } else {
                    add_goto(cur_state->num, val, next->num);
                }
            }

            if(isnew == NEW) {
                add_unfinished(next);
            } else {
                if(merge_lookaheads(next->kernel_items, first_item, nitems)) {
                    add_unfinished(next);
                    ++nlr;
                }
                while(--nitems >= 0) freeitem(*first_item++);
            }
            fprintf(stderr, "\rLR:%-3d LALR:%-3d\n", _nstates + nlr, _nstates);
        }
        cur_state->closed = 1;
    }

    free_recycled_items();
    if(g_cmdopt.verbose)
        fprintf(stderr, "states, %d items, %d shift and goto transitions.\n", _nitems, _ntab_entries);

    return _nstates;
}

PRIVATE void reduce_one_item(STATE_S *state, ITEM_S *item)
{
    int token;
    int pprec;    /*precedence of production.*/
    int tprec;    /*precedence of token*/
    int assoc;    /*associativity of token*/
    bool resovled;/*true if conflict can be resolved.*/
    int reduce_by;
    ACT_S *ap;

    if(item->right_of_dot) return; /*no reduction required.*/

    pprec = item->prod->prec; /*precedence of entire production.*/
    D(printf("ITEM: %s\n", stritem(item, true)) ;)

    for(set_next_member(NULL); (token = set_next_member(item->lookaheads)) >= 0;) {

        tprec = g_precedence[token].level;
        assoc = g_precedence[token].assoc;

        D(printf("TOKEN: %s (prec = %d, assoc = %c)\n", g_terms[token]->name,
        tprec, assoc);)

        /* Find action from action table, if exists, there are conflicts.*/
        if(!(ap = p_action(state->num, token))) {  /* 1. no conflicts.*/
            add_action(state->num, token, -(item->prod_num));
            D(printf("Action[%d][%s]=%d\n", state->num, g_terms[token]->name, -(item->prod_num));)

        } else if( ap->do_this <= 0) {             /* 2. reduce-reduce conflict.*/
            /* If there are redeuce-reduce conflict, resovle it in favor of the production with
             * with the smaller number and print a waring.*/
            ++_nreduce_reduce;
            reduce_by = min(-(ap->do_this), item->prod_num);
            error(WARNING, "State %2d: reduce/reduce conflict ", state->num);
            error(NOHDR, "%d/%d on %s (choose %d).\n",
                  -(ap->do_this),
                  item->prod_num,
                  token ? g_terms[token]->name : "<_EOI_>",
                  reduce_by);
            ap->do_this = -reduce_by; /*bug fix: should be here rather above in original code.*/

        } else {                                   /* 3. shift-reduce conflict.*/
            if((resovled = (pprec && tprec))) {
                if(tprec < pprec || (pprec == tprec && assoc != 'r')) {
                    ap->do_this = -(item->prod_num);
                }
            }
            if(g_cmdopt.verbose > 1 || !resovled) {
                ++_nshift_reduce;
                error(WARNING, "State %2d: shift/reduce conflict", state->num);
                error(NOHDR, " %s/%d (choose %s) %s.\n",
                      g_terms[token]->name,
                      item->prod_num,
                      ap->do_this < 0 ? "reduce" : "shift",
                      resovled ? "(resolved)" : "");
            }
        }
    }
}

/*
 * This routine is called for each state. It adds the reductions using the
 * disambiguating rules described in the text, and then prints the state to
 * yyout.doc if verbose is true.
 */
PRIVATE void add_reductions(STATE_S *state, void *junk)
{
    int i;
    ITEM_S **itemp;

    D(printf("-----------------------------------\n");)
    D(pstate_stdout(state);)
    D(printf("-----------------------------------\n");)

    for(i = state->nkitems, itemp = state->kernel_items; --i >= 0; ++itemp) {
        reduce_one_item(state, *itemp);
    }

    for(i = state->neitems, itemp = state->epsilon_items; --i >= 0; ++itemp) {
        reduce_one_item(state, *itemp);
    }

    if(g_cmdopt.verbose) {
        pstate(state);
        if(state->num % 10 == 0)fprintf(stderr, "%d\n", state->num);
    }
}

/*Do the reductions. If there are memory, sort the table by state number
 *first, so that yyout.doc look nice.
 */
PRIVATE void reductions(void)
{
    _state_cmp_info.sort_by_number = true;
    if(!hash_print_tab(_states_hashtab, (fp_tab_print_t)add_reductions, NULL, 1))
        hash_print_tab(_states_hashtab, (fp_tab_print_t)add_reductions, NULL, 0);
}

/*
 * Print one terminal symbol to a buffer maintained by the calling routine and
 * update the calling routinue's pointer.
 */
PRIVATE void sprint_tok(char **bp, char *format, int arg)
{
    if(arg == -1) *bp +=sprintf(*bp, "null ");
    else if(arg == -2) *bp += sprintf(*bp, "empty ");
    else if(arg == _EOI_) *bp += sprintf(*bp, "$ ");
    else if(arg == EPSILON) *bp += sprintf(*bp, "");
    else *bp += sprintf(*bp, "%s ", g_terms[arg]->name);

    if(++_tokens_printed >= MAXTOKPERLINE) {
        *bp += sprintf(*bp, "\n\t\t");
        _tokens_printed = 0;
    }
}

/*
 * Return a pointer to a string that holds a representation of an item.
 * The lookaheads are printed too if lookaheads is true or verbose is
 * bigger than 1 (-V is specified on command line).
 */
PRIVATE char *stritem(ITEM_S *item, bool lookaheads)
{
    static char buf[MAXOBUF*4];
    char *bp;
    int i;

    bp = buf;
    bp += sprintf(bp, "%s->", item->prod->lhs->name);

    if(item->prod->rhs_len <= 0) {
        bp += sprintf(bp, "<epsilon>");
    } else {
        for(i = 0; i < item->prod->rhs_len; ++i) {
            if(i == item->dot_posn) *bp++ = '.';
            bp += sprintf(bp, "%s", item->prod->rhs[i]->name);
        }
        if(i == item->dot_posn) *bp++ = '.';
    }

    if(lookaheads || (g_cmdopt.verbose > 1)){
        bp += sprintf(bp, " (production %d, precedence %d)\n\t\t[",
                      item->prod->num, item->prod->prec);
        _tokens_printed = 0;
        set_print(item->lookaheads, (fp_set_prnt)sprint_tok, &bp);
        *bp++ = ']';
    }

    if(bp >= &buf[sizeof(buf)])
        error(FATAL, "INTERNAL ERROR: [stritem], buffer overflow.\n");

    *bp = '\0';
    return buf;

}

/*
 * Return a pointer to a string that holds a representation of a production.
 */
PRIVATE char *strprod(PRODUCTION_S *prod)
{
    static char buf[MAXOBUF * 2];
    char *bp = buf;
    int i;

    bp += sprintf(bp, "%s->", prod->lhs->name);

    if(prod->rhs_len <= 0){
        bp += sprintf(bp, "<epsilon>");
    } else {
        for(i = 0; i < prod->rhs_len; ++i)
            bp += sprintf(bp, "%s ", prod->rhs[i]->name);
    }

    if(bp >= &buf[sizeof(buf)])
        error(FATAL, "INTERNAL ERROR: [strprod], buffer overflow.\n");

    *bp = '\0';
    return buf;
}

PRIVATE void pclosure_stdout(STATE_S *kernel, ITEM_S **closure_items, int nitems)
{
    printf("\n%d items in Clousure of ", nitems);
    pstate_stdout(kernel);

    if(nitems > 0) {
        printf("    --------closure items:------\n");
        while(--nitems >= 0) printf("    %s\n", stritem(*closure_items++, 0));
    }
}

/*
 * Print one row of  the parse table in human-readable form yyout.doc
 * (stderr if -V is specified).
 */
PRIVATE void pstate(STATE_S *state)
{
    int i;
    ITEM_S **item;
    ACT_S *p;

    doc("State %d:\n", state->num);

    /*print the kernel and epsilon items for the current state.*/
    for(i = state->nkitems, item = state->kernel_items; --i >= 0; ++item)
        doc("    %s\n", stritem(*item, (*item)->right_of_dot == NULL));
    for(i = state->neitems, item = state->epsilon_items; --i >= 0; ++item)
        doc("    %s\n", stritem(*item, true));
    doc("\n");

    /*print out the next-state transitions, first the actions then the gotos.*/
    for(i = 0;i < (MINTERM + USED_TERMS); ++i) {
        if((p = p_action(state->num, i))) {
            if(p->do_this == 0) {        /*accept*/
                if(p->sym == _EOI_) {
                    doc("    Accept on end of input\n");
                } else {
                    error(FATAL, "INTERNAL: state %d, Illegal accept", state->num);
                }
            } else if(p->do_this < 0) {  /*reduce*/
                doc("    Reduced by %d on %s\n", -(p->do_this), g_terms[p->sym]->name);
            } else {                     /*shift*/
                doc("    Shift to %d on %s\n", p->do_this, g_terms[p->sym]->name);
            }
        }
    }

    for(i = MINNONTERM; i < MINNONTERM + USED_NONTERMS; ++i){
        if((p = p_goto(state->num, i))) {
            doc("    Goto %d on %s\n", p->do_this, g_terms[p->sym]->name);
        }
    }

    doc("\n");
}

PRIVATE void pstate_stdout(STATE_S *state)
{
    doc_to(stdout);
    pstate(state);
    doc_to(stdout);
}

PRIVATE void make_yy_lhs(PRODUCTION_S **prodtab)
{
    static char *text[] = {
      "The Yy_lhs array is used for reductions. It is indexed by production number",
      "and holds the associated left-hand side adjusted so that the number can be",
      "used as an index into Yy_goto.",
      NULL
    };
    PRODUCTION_S *prod;
    int i;

    sys_comment(g_output, text);
    output("YYPRIVATE int Yy_lhs[%d] = \n{\n", g_num_productons);
    for(i = 0; i < g_num_productons; ++i) {
        prod = *prodtab++;
        output("\t/* %3d */\t%d", prod->num, ADJ_VAL(prod->lhs->val));
        if( i != g_num_productons) output(",");
        if(i % 3 == 2 || i == g_num_productons - 1) output("\n");
    }
    output("};\n");
}

PRIVATE void make_yy_reduce(PRODUCTION_S **prodtab)
{
    static char *text[] ={
      "The Yy_reduce array is indexed by production number and holds the number",
      "the number of symbols on the right-hand side of the production",
      NULL
    };
    PRODUCTION_S *prod;
    int i;

    sys_comment(g_output, text);
    output("YYPRIVATE int Yy_reduce[%d] = \n{\n", g_num_productons);
    for(i = 0;i < g_num_productons; ++i) {
        prod = *prodtab++;
        output("\t/* %3d */\t%d", prod->num, prod->rhs_len);
        if(i != g_num_productons - 1) output(",");

        if(i % 3 == 2 || i == g_num_productons - 1) output("\n");
    }

    output("};\n");
}

PRIVATE void make_yy_slhs(PRODUCTION_S **prodtab)
{
    static char *text[]={
      "Yy_slhs[] is a debugging version of Yy_lhs[].It is indexed by production number",
      "and evalutates to a string representing the left-hand side of the production.",
      NULL
    };

    PRODUCTION_S *prod;
    int i;

    sys_comment(g_output, text);
    output("YYPRIVATE char *Yy_slhs[%d] = \n{\n", g_num_productons);
    for(i = g_num_productons; --i >= 0;){
        prod = *prodtab++;
        output("\t/* %3d */\t\"%s\"", prod->num, prod->lhs->name);
        output(i != 0 ? ",\n" : "\n");
    }
    output("};\n");
}

PRIVATE void make_yy_srhs(PRODUCTION_S **prodtab)
{
    static char *text[]={
        "Yy_srhs[] is also used for debugging. It is indexed by production number and ",
        "evaluates to a string representing the right-hand side of the production.",
        NULL
    };
    PRODUCTION_S *prod;
    int i,j;

    sys_comment(g_output, text);
    output("YYPRIVATE char *Yy_srhs[%d] = \n{\n", g_num_productons);
    for(i = g_num_productons; --i >= 0;) {
        prod = *prodtab++;
        output("\t/* %3d */\t\"", prod->num);
        for(j = 0; j < prod->rhs_len; ++j) {
            output("%s", prod->rhs[j]->name);
            if(j != prod->rhs_len - 1) outc(' ');
        }
        output(i != 0 ? "\",\n" : "\"\n");
    }
    output("};\n");
}

PRIVATE void mkprodtab(SYMBOL_S *sym, PRODUCTION_S **prodtab)
{
    PRODUCTION_S *p;
    if(ISNONTERM(sym)) {
        for(p = sym->productions; p; p = p->next){
#ifdef DEBUG
            if(p->num >= g_num_productons)
                error(FATAL, "INTERNAL ERROR: [mkprod], bad prod num.\n");
#endif
            prodtab[p->num] = p;
        }
    }
}

/*
 * Output the various tables needed to do reductions.
 */
PRIVATE void print_reductions(void)
{
    PRODUCTION_S **prodtab;

    if(!(prodtab = (PRODUCTION_S **)GC_MALLOC(sizeof(PRODUCTION_S*) * g_num_productons))){
        error(FATAL, "INTERNAL ERROR: Not enough memory to output LALR(1) reduction tables\n.");
    } else {
        hash_print_tab(g_symtab, (fp_tab_print_t) mkprodtab, prodtab, 0);
    }

    make_yy_lhs(prodtab);
    make_yy_reduce(prodtab);

    output("#ifdef YYDEBUG\n");
    make_yy_slhs(prodtab);
    make_yy_srhs(prodtab);
    output("#endif\n");
}

/*
 * Output the action or goto table.
 */
PRIVATE void print_tab(ACT_S **table, char *row_name, char *col_name, bool private)
{
    int i,j;
    ACT_S *ele, **elep;
    ACT_S *e, **p;
    int count;
    int column = 0;
    SET_S *redundant = set_new();

    static char *act_text[] = {
        "The Yy_action table is action part of the LALR(1) transition matrix. It's",
        "compressed and can be accessed using the yy_next() function, as below.",
        "",
        "                Yya000[]={ 3  ,   5,3   ,  2,2  ,  1,1  };",
        "  state number------+      |      | |",
        "  number of pairs in list--+      | |",
        "  input symbol (terminal)---------+ |",
        "  action----------------------------+",
        "",
        "  action = yy_next(Yy_action, cur_state, lookahead_symbol);",
        "",
        "  action <  0   --Reduce by production n, n == -action.",
        "  action == 0   --Accept. (ie,Reduce by production 0).",
        "  action >  0   --Shift to state n, n == action.",
        "  action == YYF --Error.",
        NULL
    };

    static char *goto_text[] = {
        "The Yy_goto table is goto part of the LALR(1) transition matrix.",
        "",
        "  nonterminal = Yy_lhs[ production number by which we just reduced ]",
        "",
        "                Yyg000[]={ 3  ,   5,3   ,  2,2  ,  1,1  };",
        "  uncovered state---+      |      | |",
        "  number of pairs in list--+      | |",
        "  nonterminal---------------------+ |",
        "  goto this state-------------------|",
        "",
        "It is compressed and can be accessed using the yy_next() subroutine as ",
        "below like this:",
        "",
        "  goto_state = yy_next(Yy_goto, cur_state, nonterminal);",
        NULL
    };

    sys_comment(g_output, table == _actions ? act_text : goto_text);

    /*
     * Modify the matrix so that, if a duplicate row exists, only one copy of it
     * is kept around. The extra rows are marked as such by setting a bit in the
     * "redundant" set. (The memory used for the chains is just discarded.) The
     * redundant table element is made to point at the row that it duplicates.
     */
    for(elep = table, i = 0; i < _nstates; ++elep, ++i) {

        if(SET_MEMBER(redundant, i)) continue;

        for(p = elep + 1, j = i; ++j < _nstates; ++p) {
            if(SET_MEMBER(redundant, j)) continue;
            ele = *elep;
            e   = *p;

            if(!e || !ele) continue;

            for(; ele && e; ele=ele->next, e = e->next) {
                if((ele->do_this != e->do_this) || (ele->sym != e->sym))
                    break;
            }

            if(!e && !ele) { /*the chains are the same*/
                SET_ADD(redundant, j);
                table[j] = (ACT_S*)elep;
            }
        }
    }

    /*Output the row arrays.*/
    for(elep = table, i = 0;i < _nstates; ++elep, ++i) {
        if(!*elep || SET_MEMBER(redundant, i)) continue;

        count = 0;
        for(ele = *elep; ele; ele = ele->next) ++count;

        output("YYPRIVATE YY_TTYPE %s%-3d[] = { %2d,", row_name, (int)(elep-table), count);
        column = 0;
        for(ele = *elep; ele; ele = ele->next) {
            ++_npairs;
            output("%3d, %-4d", ele->sym, ele->do_this);
            if(++column != count) outc(',');
            if(column % 5 == 0) output("\n\t\t\t         ");
        }
        output("};\n");
    }

    /*Output the index array*/
    if(private) {
        output("\nYYPRIVATE YY_TTYPE *%s[%d] = {\n", col_name, _nstates);
    } else {
        output("\nYY_TTYPE *s[%d] = {\n", col_name, _nstates);
    }

    for(elep = table, i = 0; i < _nstates; ++i, ++elep) {
        output("\n/* %3d */ ", i);
        if(SET_MEMBER(redundant, i)){
            output("%s%-3d", row_name, (int)((ACT_S**)(*elep) - table));
        } else {
            output(*elep ? "%s%-3d" : "NULL", row_name, i);
        }

        if(i != _nstates - 1) output(", ");
    }

    output("\n};\n");
}
