//
// Created by Mookel on 16/9/6.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// dfa.c : 
//

#include <compiler.h>
#include "globals.h"
#include "dfa.h"
#include "terp.h"
#include "print.h"

typedef struct _DFA_STATE{
    unsigned group: 8;   /*group id, used by minimize*/
    unsigned mark : 1;   /*mark used by make_dtran*/
    char     *accept;
    int      anchor;
    SET_S    *set;       /*Set of NFA states represented by this DFA state.*/

}DFA_STATE;

PRIVATE DFA_STATE *_dstates;
PRIVATE ROW       *_dtran;
PRIVATE int       _nstates;
PRIVATE DFA_STATE *_last_marked;

#ifdef DEBUG
PRIVATE void ps(SET_S *set)
{
    putchar('{');
    set_print(set, fprintf, stdout);
    printf("}\n");
}

PRIVATE void pstate(DFA_STATE *state)
{
    printf("_dstates[%ld] ", (long)(state - _dstates));
    if(state->mark) printf("marked ");
    if(state->accept) printf("accepting %s<%s>%s", state->anchor & START ? "^" : "",
        state->accept, state->anchor & END ? "$" : "");
    ps(state->set);
}
#endif

/*
 * Return a pointer to an unmarked state in _dstates, if no such state
 * exists, return null. Print an asterisk for each state to tell the
 * user that the program hasn't died while the table is being constructed.
 * */
PRIVATE DFA_STATE *get_unmarked()
{
    for(; _last_marked < &_dstates[_nstates]; ++_last_marked){
        if(!_last_marked ->mark){
            putc('*', stderr);
            fflush(stderr);

            if(g_verbose > 1) {
                fputs("-----------------\n", stdout);
                printf("working on DFA state %d = NFA states: ", (int)(_last_marked - _dstates));
                set_print(_last_marked->set, fprintf, stdout);
                putchar('\n');
            }
            return _last_marked;
        }
    }

    return NULL;
}

PRIVATE int in_dstates(SET_S *NFA_set)
{
    DFA_STATE *p;
    DFA_STATE *end = &_dstates[_nstates];

    for(p = _dstates; p < end; ++p) {
        if(SET_IS_EQUIV(NFA_set, p->set))  return (int)(p - _dstates);
    }

    return -1;
}

PRIVATE int add_to_dstates(SET_S *NFA_set, char *accept, int anchor)
{
    int nextstate;

    if(_nstates > (DFA_MAX - 1))  com_ferr("Too many DFA states\n");

    nextstate = _nstates++;
    D(if(g_verbose > 1))
    D(printf("Adding new DFA state (%d)\n", nextstate);)

    _dstates[nextstate].set = NFA_set;
    _dstates[nextstate].accept = accept;
    _dstates[nextstate].anchor = anchor;

    return nextstate;
}

PRIVATE void make_trans(int sstate)
{
    SET_S *NFA_set;      /*Set of NFA states that define the next DFA state*/
    DFA_STATE *current;  /*state currently being expanded.*/
    int nextstate;       /*goto DFA state for current char*/
    char *isaccept;
    int anchor;
    int c;

    /*
     * Initially _dstates contains a single, unmarked, start state formed by
     * taking the epision closure of the NFA start state. So, _dstates[0] is
     * the DFA start state.
     * */

    NFA_set = set_new();
    SET_ADD(NFA_set, sstate);

    _nstates = 1;
    _dstates[0].set = e_closure(NFA_set, &_dstates[0].accept, &_dstates[0].anchor);
    _dstates[0].mark = 0;

    while(current = get_unmarked()) {
        D(printf("New unmarked state:\n\t");)
        D(pstate(current);)

        current->mark = 1;
        for(c = MAX_CHARS; --c >= 0;){
            if(NFA_set = move(current->set, c)) NFA_set = e_closure(NFA_set, &isaccept, &anchor);

            if(!NFA_set)  {       /*no move.*/
                nextstate = F;
            } else if ((nextstate = in_dstates(NFA_set)) != -1) {  /*has current in trans table.*/

                /*do nothing*/

            } else {  /*no current in table.*/
                nextstate = add_to_dstates(NFA_set, isaccept, anchor);
            }

            _dtran[current - _dstates][c] = nextstate;

            D(if(g_verbose > 1 && nextstate != F) \
               printf("Dfa state %d goes to %d on %s\n", \
               current-_dstates, nextstate, sys_bin_to_ascii(c, 1));)
        }
    }

    putc('\n', stderr);
}

/*
 * Turns an NFA with the indicated start state(sstate) into a DFA and
 * returns the number of states in the DFA transition table. *dfap is
 * modified to pointed at that transition table and *acceptp is modified
 * to point at an array of accepting states(indexed by state number).
 * dfa() discards all the memory used for the initial NFA.(Now we have
 * gc module, there is no need to release the memory.)
 * */
PUBLIC int dfa(fp_input_t input_func, ROW *dfap[], ACCEPT **acceptp)
{
    ACCEPT *accept_states;
    int i;
    int start;

    start = nfa(input_func);
    _nstates = 0;
    _dstates = (DFA_STATE*) GC_MALLOC(DFA_MAX * sizeof(DFA_STATE));
    _dtran   = (ROW *) GC_MALLOC(DFA_MAX * sizeof(ROW));
    _last_marked = _dstates;

    if(g_verbose) fputs("making DFA: ", stdout);

    if(!_dstates || !_dtran) com_ferr("No memeory for DFA transition matrix!");

    make_trans(start);
    free_nfa();

    _dtran = (ROW*)GC_REALLOC(_dtran, _nstates * sizeof(ROW));
    accept_states = (ACCEPT *)GC_MALLOC(_nstates * sizeof(ACCEPT));

    if(!accept_states || !_dtran) com_ferr("dfa: Out of memeory!!");

    for(i = _nstates; --i >= 0; ){
        accept_states[i].string = _dstates[i].accept;
        accept_states[i].anchor = _dstates[i].anchor;
    }

    GC_FREE(_dstates);
    *dfap = _dtran;
    *acceptp = accept_states;

    if(g_verbose) {
        printf("\n%d out of %d DFA states in initial machine.\n", _nstates, DFA_MAX);

        printf("%ld bytes required for uncompressed tables.\n\n", _nstates * MAX_CHARS * sizeof(TTYPE) +
        _nstates * sizeof(TTYPE));

        if(g_verbose > 1) {
            printf("The un-minimized DFA looks like this: \n\n");
            pheader(stdout, _dtran, _nstates, accept_states);
        }
    }
    return _nstates;
}