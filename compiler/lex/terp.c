//
// Created by Mookel on 16/9/6.
// Email : ltp0709@sina.com
// Copyright (c) 2016 jlu.edu. All rights reserved.
// terp.c :
//

#include <stdlib.h>
#include <limits.h>
#include "nfa.h"
#include "terp.h"

PRIVATE NFA *_nfa;      /*Base address of NFA array.*/
PRIVATE int _nstates;   /*Number of states in NFA*/

PUBLIC int nfa(fp_input_t input_func)
{
    NFA *sstate;
    _nfa = thompson(input_func, &_nstates, &sstate);
    return (sstate - _nfa); /*return the number index of the start state.*/
}

PUBLIC void free_nfa()
{
    GC_FREE(_nfa);
}

/**
 *  input:  the set of start states to examine.
 *  accept: is modified to point at the string associated with an accepting state or NULL if not.
 *  anchor: is modified to hold the anchor point, if any.
 */
PUBLIC SET_S *e_closure(SET_S *input, char **accept, int *anchor)
{
    int stack[NFA_MAX];
    int *tos;
    NFA *p;
    int i;
    int accept_num = INT_MAX;

    D(printf("e_closure of {");)
    D(set_print(input, fprintf, stdout);)
    D(printf("}=");)

    if(!input) goto abort;

    /* e-closure Algorithm:
     *    push all state in input set onto the stack.
     *    while(the statck is not empty)
     *      pop the element i
     *      if(state i is an accepting state)
     *          *accept = the accept string;
     *      if(there is an elision from state to State N)
     *           if(N is not in the closure set)
     *              Add N to the closure set.
     *              Push N onto the stack.
     * */

    *accept = NULL;
    tos = &stack[-1];

    for(set_next_member(NULL); (i = set_next_member(input)) >= 0;)
        *++tos = i;

    while(INBOUNDS(stack, tos)){
        i = *tos--;
        p = &_nfa[i];

        if(p ->accept && (i < accept_num)) { /*conflicting states that are higher in the input
                                               file take precedence over the ones that occur later*/
            accept_num = i;
            *accept = p->accept;
            *anchor = p->anchor;
        }

        if(p->edge == EPSILON) {
            if(p->next) {
                i = (int)(p->next - _nfa);
                if(!SET_MEMBER(input, i)){
                    SET_ADD(input, i);
                    *++tos = i;
                }
            }

            if(p->next2) {
                i = (int)(p->next2 - _nfa);
                if(!SET_MEMBER(input, i)) {
                    SET_ADD(input, i);
                    *++tos = i;
                }
            }
        }

    }

    abort:

        D(printf("{");)
        D(set_print(input, fprintf, stdout);)
        D(printf(*accept ? "} ACCEPTING <%s>\n" : "}\n", *accept);)

    return input;
}

/*
 * Return a set that contains all NFA states that can be reached by making
 * tansitions on "c" from any NFA state in "inp_set". Return null if there
 * no such transitions. The inp_set is not modified.
 * */
PUBLIC SET_S *move(SET_S *inp_set, int c)
{
    int i;
    NFA *p;
    SET_S *outset = NULL;

    D(printf("move({");)
    D(set_print(inp_set, fprintf, stdout);)
    D(printf("}, ");)
    D(sys_pchar(c, stdout);)
    D(printf(")=");)

    for (i = _nstates; --i >= 0;) {
        if (SET_MEMBER(inp_set, i)) {
            p = &_nfa[i];
            if (p->edge == c || ((p->edge == CCL) && (SET_TEST(p->bitset, c)))){
                if(!outset) outset = set_new();
                SET_ADD(outset, (int)(p->next - _nfa));
            }
        }
    }

    D(set_print(outset, fprintf, stdout);)
    D(pirntf("\n");)

    return outset;
}