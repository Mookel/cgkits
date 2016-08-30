//
// Created by Mookel on 16/8/29.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// set.h : 
//

#ifndef CGKITS_SET_H
#define CGKITS_SET_H

#include <debug.h>

typedef unsigned _SETTYPE;
typedef int (*fp_set_prnt) (void *para, char *fmt, int val);

#define _BITS_IN_WORD            4
#define _BYTES_IN_ARRAY(words)   ((words) << 2)
#define _DIV_WSIZE(bitpos)       (unsigned(bitpos) >> 5)
#define _MOD_WSIZE(bitpos)       (unsigned(bitpos) & 0x1f)
#define _DEFWORDS                16                             /*words in default set.*/
#define _DEFBITS                 (_DEFWORDS * _BITS_IN_WORD)    /*bits  in default set.*/
#define _ROUND(bit)              (((_DIV_WSIZE(bit) + 16) >> 4) << 4)

typedef struct SET_
{
    unsigned nwords;
    unsigned compl;
    unsigned nbits;
    _SETTYPE *map;
    _SETTYPE defmap[_DEFWORDS];
}SET_S;

/*global interface*/
extern int       _set_add(SET_S *set, int bit);
extern void      set_del(SET_S *set);
extern SET_S*    set_dup(SET_S *set);
extern void      set_invert(SET_S *set);
extern SET_S*    set_new(void);
extern int       set_next_member(SET_S *set);
extern int       set_num_ele(SET_S *set);
extern void      set_print(SET_S* set, fp_set_prnt p_prnt, void *param);
extern void      _set_op(int op, SET_S *src, SET_S *dst);
extern int       _set_test(SET_S *set1, SET_S *set2);
extern int       set_compare(SET_S *set1, SET_S *set2);
extern unsigned  set_hash(SET_S *set);
extern int       set_subset(SET_S *set, SET_S *subset);
extern void      set_truncate(SET_S *set);

/*operations opcode definitions passed to _set_op */
#define _OP_UNION               0
#define _OP_INTERSECT           1
#define _OP_DIFFERENCE          2   /*(x in s1) && (x not in s2)*/
#define _OP_ASSIGN              4

#define SET_UNIOIN(d, s)        _set_op(_OP_UNION, d, s)
#define SET_INTERSECT(d, s)     _set_op(_OP_INTERSECT, d, s)
#define SET_DIFFERENCE(d, s)    _set_op(_OP_DIFFERENCE, d, s)
#define SET_ASSIGN(d, s)        _set_op(_OP_ASSIGN, d, s)

#define SET_CLEAR(s)            memset((s)->map, 0, (s)->nwords *sizeof(_SETTYPE))
#define SET_FILL(s)             memset((s)->map, ~0, (s)->nwords, *sizeof(_SETTYPE))
#define SET_COMPLEMENT(s)       ((s)->compl = ~((s)->compl))
#define SET_INVERT(s)           set_invert(s)

/*value returned from _set_test*/
#define _SET_EQUIV             0
#define _SET_DISJ              1
#define _SET_INTER             2

#define SET_IS_DISJOINT(s1, s2)    (_set_test(s1, s2) == _SET_DISJ)
#define SET_IS_INTERSECT(s1, s2)   (_set_test(s1, s2) == _SET_INTER)
#define SET_IS_EQUIV(s1, s2)       (set_compare(s1, s2) == 0)
#define SET_IS_EMPTY(s)            (set_num_ele(s) == 0)

/*following macros has side-effects, be carefully.*/
#define _SET_BITOP(s, x, op)       (((s)->map)[_DIV_WSIZE(x)] op (1 << _MOD_WSIZE(x)))

#define SET_REMOVE(s, x)           (((x) >= (x)->nbits) ? 0 : _SET_BITOP(s, x , &= ~))
#define SET_ADD(s, x)              (((x) >= (x)->nbits) ? _set_add(s, x) : _SET_BITOP(s, x, |=))
#define SET_MEMBER(s, x)           (((x) >= (x)->nbits) ? 0 : _SET_BITOP(s, x, &))
#define SET_TEST(s, x)             ((MEMEBR(s, x))      ? !(s)->compl  : (s)->compl)

#endif //CGKITS_SET_H
