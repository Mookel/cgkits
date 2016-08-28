//
// Created by Mookel on 16/8/27.
// Email  : ltp0709@sina.com
// Copyright (c) 2016. All rights reserved.
//

#ifndef CGKITS_DEBUG_H
#define CGKITS_DEBUG_H

#ifdef  DEBUG
#define PRIVATE
#define D(x)    x  /*expand only when debugging*/
#define ND(x)      /*expand only when not debugging*/
#else
#define PRIVATE static
#define D(x)
#define  ND(x)   x
#endif

#define PUBLIC

#define FARPTR   *
#define VA_LIST  ...
#define 0_BINARY 0

#define PHYS(p)  p

/* For array operation */
#define NUMELE(a)       (sizeof(a)/sizeof(*(a)))           /*Evaluates to the array size in elements*/
#define LASTELE(a)      ((a) + (NUMELE(a) - 1))            /*Evaluates to a pointer to the last element*/
#define TOOHIGH(a,p)    (((p) - (a)) > (NUMELE(a) - 1))    /*Evalutates to true if p is out of range of a*/
#define TOOLOW(a,p)     (((p) - (a)) < 0)                  /*Evalutates to true if p is lower than a[0]*/
#define INBOUNDS(a,p)   (!(TOOHIGH(a,p) || TOOLOW(a,p)))   /*Evalueates to true if p points into the array.*/

#define _IS(t,x)        (((t)(1UL << (x))) != 0)
#define NBITS(t)        (t)(4*(1 + _IS(t, 4) + _IS(t, 8) + _IS(t, 12) + _IS(t, 16) + _IS(t, 20) + _IS(t, 24) \
                            + _IS(t, 28) + _IS(t, 32)))

#ifndef max
#define max(a, b)       ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)       ((a) < (b) ? (a) : (b))
#endif
#define RANGE(a,b,c)    ((a) <= (b) && (b) <= (c))

#endif //CGKITS_DEBUG_H
