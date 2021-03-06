//
// Created by Mookel on 16/8/29.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// set.c : 
//
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <set.h>
#include <gc.h>

PUBLIC SET_S *set_new()
{
    SET_S *p;

    if(!(p = (SET_S *)GC_MALLOC(sizeof(SET_S)))) {
        fprintf(stderr, "not enough memory for set.\n");
        raise(SIGABRT);
        return NULL;
    }

    memset(p, 0, sizeof(SET_S));
    p->map = p->defmap;
    p->nwords = _DEFWORDS;
    p->nbits  = _DEFBITS;

    return p;
}

PUBLIC void set_del(SET_S *set)
{
    if(!set) return;
    if(set->map != set->defmap) GC_FREE(set->map);
    GC_FREE(set);
}

PUBLIC SET_S *set_dup(SET_S *set)
{
    SET_S *new;
    if(!(new = (SET_S*) GC_MALLOC(sizeof(SET_S)))){
        fprintf(stderr, "not enough memory for dup set.\n");
        exit(1);
    }

    memset(new, 0, sizeof(SET_S));
    new->compl = set->compl;
    new->nbits = set->nbits;
    new->nwords = set->nwords;

    if(set->map == set->defmap){
        new->map = new->defmap;
        memcpy(new->defmap, set->defmap, _DEFWORDS * sizeof(_SETTYPE));
    } else {
        new->map = (_SETTYPE *)GC_MALLOC(set->nwords * sizeof(_SETTYPE));
        if(! new->map){
            fprintf(stderr, "not enough memory for set map allocation.");
            exit(1);
        }
        memcpy(new->map, set->map, set->nwords * sizeof(_SETTYPE));
    }

    return new;
}

PRIVATE void enlarge(SET_S *set, int need)
{
    if(!set || need <= set->nwords) return;

    _SETTYPE  *new;
    D(fprintf(stdout, "enlarging %d word map to %d words\n", set->nwords, need));

    if(!(new = (_SETTYPE *)GC_MALLOC(sizeof(_SETTYPE) * need))){
        fprintf(stderr, "not enough memory for set enlarge.\n");
        exit(1);
    }

    memcpy(new, set->map,  set->nwords * sizeof(_SETTYPE));
    memset(new + set->nwords, 0, (need - set->nwords) * sizeof(_SETTYPE));

    if(set->map != set->defmap) GC_FREE(set->map);
    set->map = new;
    set->nwords = need;
    set->nbits  = need * _BITS_IN_WORD;
}

PUBLIC int set_add(SET_S *set, int bit)
{
    if(!set) return 0;

    enlarge(set, _ROUND(bit));
    return _SET_GBIT(set, bit, |=);
}

PUBLIC int set_num_ele(SET_S *set)
{
    static unsigned char nbits[] = {
            /*   0-15  */    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
            /*  16-31  */    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            /*  32-47  */    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            /*  48-63  */    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            /*  64-79  */    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            /*  80-95  */    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            /*  96-111 */    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            /* 112-127 */    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            /* 128-143 */    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
            /* 144-159 */    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            /* 160-175 */    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            /* 176-191 */    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            /* 192-207 */    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
            /* 208-223 */    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            /* 224-239 */    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
            /* 240-255 */    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };

    int i = 0;
    unsigned count = 0;
    unsigned char *p = NULL;

    if(!set) return 0;

    p = (unsigned char *) set->map;
    for(i = _BYTES_IN_ARRAY(set->nwords); --i >= 0;)
        count += nbits[*p++];

    return count;
}

PUBLIC int set_test(SET_S *set1, SET_S *set2)
{
    int i = 0;
    int rval = _SET_EQUIV;
    _SETTYPE *p1, *p2;

    i = max(set1->nwords, set2->nwords);

    enlarge(set1, i);
    enlarge(set2, i);

    p1 = set1->map;
    p2 = set2->map;

    for(; --i >= 0;p1++, p2++){
        if(*p1 != *p2){
            if(*p1 & *p2){
                return _SET_INTER;
            }
            else rval = _SET_DISJ;
        }
    }

    return rval;
}

PUBLIC int set_compare(SET_S *set1, SET_S *set2)
{
    /*This function works like strcmp. */
    int i,j;
    _SETTYPE  *p1, *p2;

    D(if(!set1) fprintf(stderr, "set_compare(): set1 is NULL.\n");)
    D(if(!set2) fprintf(stderr, "set_compare(): set2 is NULL.\n");)

    if(set1 == set2)  {return 0; }
    if(set1 && !set2) {return 1; }
    if(!set1 && set2) {return -1;}

    i = j = min(set1->nwords, set2->nwords);
    for(p1 = set1->map, p2 = set2->map; --j >= 0; ++p1, ++p2){
        if(*p1 != *p2) return *p1 - *p2;
    }

    if((j = set1->nwords - i) > 0){  /*set1 is larger*/
        while(--j >= 0){
            if(*p1++) return 1;
        }
    } else if((j = set2->nwords - i) > 0) { /*set2 is larger*/
        while(--j >= 0) {
            if(*p2++) return -1;
        }
    }

    return 0;
}

PUBLIC unsigned set_hash(SET_S *set)
{
    if(!set) return 0;

    _SETTYPE *p = set->map;
    unsigned total = 0;
    int j = set->nwords;

    while(--j >= 0) total += *p++;

    return total;
}

/**
 * 1. return 1 if cmpset is subset of set, otherwise return 0
 * 2. empty sets are subsets of all other sets.
 * 3. if cmpset is larger than the set, then the extra bytes must
 *    be all zeros.
 */
PUBLIC int set_subset(SET_S *set, SET_S *cmpset)
{
    if(!set || !cmpset){
        fprintf(stderr, "set_subset:not allowed for null sets for this function.\n");
        exit(1);
    }

    _SETTYPE *subsetp, *setp;
    int common = 0;
    int bigger = 0;

    if(cmpset->nwords > set->nwords){
        common = set->nwords;
        bigger = cmpset->nwords - set->nwords;
    } else {
        common = cmpset->nwords;
        bigger = 0;
    }

    subsetp = cmpset->map;
    setp    = set->map;

    for(; --common >= 0; ++subsetp, ++setp) {
        if((*subsetp & *setp) != *subsetp) return 0;
    }

    while(--bigger >= 0) {
        if(*subsetp++) return 0;
    }

    return 1;
}

/**
 *  Performs binary operations depending on op:
 *  _UNION_OP     : dest = union of src and dst.
 *  _INTERSECT_OP : dest = intersection of src and dst.
 *  _DIFFERENCE_OP: dest = symmetric difference of src and dest.
 *  _ASSIGN_OP    : dest = src.
 */
PUBLIC void set_op(int op, SET_S *dst, SET_S *src)
{
    if(!src || !dst){
        fprintf(stderr, "_set_op: NULL sets are not allowed for this function.\n");
        exit(1);
    }

    _SETTYPE  *sp, *dp;
    int ssize = src->nwords;
    int bigger = 0;

    if(dst->nwords  < ssize){
        enlarge(dst, ssize);
    }

    bigger = dst->nwords - ssize;
    dp     = dst->map;
    sp     = src->map;

    switch(op){
        case _UNION_OP:
            while(--ssize >= 0) *dp++ |= *sp++;
            break;
        case _INTERSECT_OP:
            while(--ssize >= 0) *dp++ &= *sp++;
            while(--bigger >= 0) *dp++ = 0;
            break;
        case _DIFFERENCE_OP:
            while(--ssize >= 0) *dp++ ^= *sp++;
            break;
        case _ASSIGN_OP:
            while(--ssize >= 0) *dp++ = *sp++;
            while(--bigger >= 0) *dp++ = 0;
            break;
        default:
            fprintf(stderr, "_set_op: wrong op code.\n");
            break;
    }
}

PUBLIC void set_invert(SET_S *set)
{
    if(!set) return;

    _SETTYPE  *p, *end;
    for(p = set->map, end = p + set->nwords; p < end; ++p){
        *p = ~*p;
    }
}

/*
 * clear the set and make it back to the original.
 * */
PUBLIC void set_truncate(SET_S *set)
{
    if(!set) return;

    if(set->map != set->defmap){
        GC_FREE(set->map);
        set->map = set->defmap;
    }

    set->nwords = _DEFWORDS;
    set->nbits  = _DEFBITS;
    memset(set->defmap, 0, sizeof(set->defmap));
}

/**
 *  Attention : no reentrant function.
 *  if set == NULL : Reset
 *  if currset != lastset, then Reset and return first element of the current set.
 *  otherwise return next element or -1 if none.
 */
PUBLIC int set_next_member(SET_S *set)
{
    static SET_S *lastSet = NULL;
    static int current_member = 0;
    _SETTYPE  *map;

    if(!set) return (int)(lastSet = NULL);

    if(set != lastSet){
        lastSet = set;
        current_member = 0;
        for(map = set->map; *map == 0 && current_member < set->nbits; ++map){
            current_member += _BITS_IN_WORD;
        }
    }

    while(current_member++ < set->nbits){
        if(SET_TEST(set, current_member - 1)) {
            return (current_member - 1);
        }
    }

    return -1;

}

/**
 * fp_pnt function is called for each element of the set with following arguments:
 * (*fp_pnt)(param, "null", -1);   Null set
 * (*fp_pnt)(param,  "empty", -2); Empty set
 * (*fp_pnt)(param,  "%d ", N);    N is an element of the set.
 */
PUBLIC void set_print(SET_S *set, fp_set_prnt fp_pnt, void *para)
{
    int i = 0;
    int is_empty = 1;

    if(!set) (*fp_pnt)(para, "NULL\n", -1);
    else{
        set_next_member(NULL);
        while((i = set_next_member(set)) >= 0) {
            is_empty = 0;
            (*fp_pnt)(para, "%d ", i);
        }
        set_next_member(NULL);
    }

    if(is_empty) (*fp_pnt)(para, "empty\n", -2);
}

PUBLIC void set_dump(SET_S* set)
{
    if(!set) return;

    _SETTYPE  *p = set->map;
    fprintf(stdout, "\n-----------------Dump set status----------------\n");
    fprintf(stdout, "\t set nwords : %u\n", set->nwords);
    fprintf(stdout, "\t set nbits  : %u\n", set->nbits);
    fprintf(stdout, "\t compl      : %d\n", set->compl);
    fprintf(stdout, "\t map addr   : %p\n", set->map);
    fprintf(stdout, "\t defmap addr: %p\n", set->defmap);
    fprintf(stdout, "\t sets number: ");
    set_print(set, (fp_set_prnt)fprintf, stdout);
    printf("\n");
    fprintf(stdout, "\t bit words  :\n");
    for(int i = 0;i < set->nwords; ++i){
        if((i != 0) && (!(i & 0x7))) printf("\n");
        printf("0x%x\t", *p++);
    }
    fprintf(stdout, "\n-----------------set value end-----------------\n");
}