//
// Created by Mookel on 16/8/29.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// hash.c : functions using for hash table operations.
//

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <hash.h>

/*local static function*/
PRIVATE fp_cmp_t _f_user_cmp = NULL;

/*global interface*/
PUBLIC unsigned hash_add(unsigned char *name)
{
    unsigned h;
    for(h = 0; *name; h += *name++)
        ;
    return h;
}

#define _NBITS_IN_UNSIGNED (NBITS(unsigned int))
#define _75PERCENT         ((int)(NBITS_IN_UNSIGNED * .75))
#define _125PERCENT        ((int)(NBITS_IN_UNSIGNED * .125))
#define _HIGH_BITS         (~((unsigned)(~0) >> _125PERCENT))

PUBLIC unsigned hash_pjw(unsigned char *name)
{
    unsigned h;
    unsigned g;
    for (; *name; ++name) {
        h = (h << _125PERCENT) + *name;
        if (g = h & _HIGH_BITS) {
            h = (h ^ (g >> _75PERCENT)) & ~_HIGH_BITS;
        }
    }

    return h;
}






