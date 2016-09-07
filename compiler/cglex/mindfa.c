//
// Created by Mookel on 16/9/7.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// mindfa.c : 
//

#include <string.h>
#include <compiler.h>
#include "mindfa.h"
#include "globals.h"

PRIVATE SET_S *_groups[DFA_MAX];  /*Groups of equivalent states in _dtran*/
PRIVATE int   _ngroups;           /*number of groups in _groups*/
PRIVATE int   _ingroup[DFA_MAX];  /*the inverse of group, that is , (state) vs (group number)*/

PRIVATE void pgroups(int nstates)
{
    SET_S **current;
    SET_S **end = &_groups[_ngroups];

    for(current = _groups; current < end; ++current){
        printf("\tgroup %ld: {", (long)(current - _groups));
        set_print(*current, fprintf, stdout);
        printf("}\n");
    }

    printf("\n");
    while(--nstates >= 0) printf("\t state %2d is in group %2d\n", nstates, _ingroup[nstates]);

}

PRIVATE void init_groups(int nstates, ACCEPT *accept)
{
    SET_S **last;
    int i, j;

    last = &_groups[0];
    _ngroups = 0;

    for(i = 0; i < nstates; ++i) {
        for(j = i; --j >= 0;) {
            if(accept[i].string == accept[j].string){
                SET_ADD(_groups[_ingroup[j]], i);
                _ingroup[i] = _ingroup[j];
                goto match;
            }
        }

        *last = set_new();
        SET_ADD(*last, i);
        _ingroup[i] = _ngroups++;
        ++last;

        match:;
    }

    if(g_verbose) {
        printf("Initial groupings: \n");
        pgroups(nstates);
    }
}

PRIVATE void fix_dtran(ROW *dfap[], ACCEPT **acceptp)
{
    SET_S **current;
    ROW   *newdtran;
    ACCEPT *newaccept;
    int state;
    int i;
    int *src, *dst;
    ROW *dtran = *dfap;
    ACCEPT *accept = *acceptp;
    SET_S **end = &_groups[_ngroups];

    newdtran = (ROW *) GC_MALLOC(_ngroups * sizeof(ROW));
    newaccept = (ACCEPT *)GC_MALLOC(_ngroups * sizeof(ACCEPT));

    if(!newaccept || !newdtran) com_ferr("fix_dtran: Out of memory!!!");

    set_next_member(NULL);
    for(current = _groups; current < end; ++current){
        dst = &newdtran[current - _groups][0];
        state = set_next_member(*current);
        src = &dtran[state][0];

        newaccept[current - _groups] = accept[state];

        for(i = MAX_CHARS; --i >= 0; src++, dst++) {
            *dst = (*src == F)  ? F : _ingroup[*src];
        }

    }

    GC_FREE(*dfap);
    GC_FREE(*acceptp);
    *dfap = newdtran;
    *acceptp = newaccept;

}

PRIVATE void minimize(int nstates, ROW *dfap[], ACCEPT **acceptp)
{
    int old_numgruoups;  /*used to see if we did anything in this pass*/
    int c;
    SET_S **current;     /*current group being processed.*/
    SET_S **new;         /*new partition being created.  */
    int first;
    int next;
    int goto_first;
    int goto_next;

    ROW *dtran = *dfap;
    ACCEPT *accept = *acceptp;

    init_groups(nstates, accept);

    do{
        old_numgruoups = _ngroups;
        for(current = &_groups[0]; current < &_groups[_ngroups]; ++current){
            if(set_num_ele(*current) <= 1) continue;

            new = &_groups[_ngroups];
            *new = set_new();
            set_next_member(NULL);
            first = set_next_member(*current);
            while((next = set_next_member(*current)) >= 0) {
                for(c = MAX_CHARS; --c >= 0;) {
                    goto_first = dtran[first][c];
                    goto_next  = dtran[next][c];
                    if((goto_first != goto_next)
                       && ( goto_first == F
                            || goto_next == F
                            || (_ingroup[goto_first] != _ingroup[goto_next]))
                            ) {
                        SET_REMOVE(*current, next);
                        SET_ADD(*new, next);
                        _ingroup[next] = _ngroups;
                        break;
                    }
                }
            }

            if(!SET_IS_EMPTY(*new)) {
                ++_ngroups;
            } else {
                set_del(*new);
            }
        }

    } while(old_numgruoups != _ngroups);

    if(g_verbose > 1) {
        printf("\nStates grouped as follws after minimization:\n");
        pgroups(nstates);
    }

    fix_dtran(dfap, acceptp);
}

/*
 * Make a minimal DFA, eliminating equivalent states.Return the number of
 * states in the minimized machine.
 * */
PUBLIC int min_dfa(fp_input_t input_func, ROW *dfap[], ACCEPT **accept)
{
    int nstates;

    memset(_groups, 0, sizeof(_groups));
    memset(_ingroup, 0, sizeof(_ingroup));

    _ngroups = 0;

    nstates = dfa(input_func, dfap, accept);
    minimize(nstates, dfap, accept);

    return _ngroups;
}

