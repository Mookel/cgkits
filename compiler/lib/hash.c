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
#include <math.h>
#include <debug.h>
#include <hash.h>

/*macro definitions*/
#define HASH_TABLE_MAX_SYM 1024;

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

/*local macro definitions*/
#define _NBITS_IN_UNSIGNED (NBITS(unsigned int))
#define _75PERCENT         ((int)(_NBITS_IN_UNSIGNED * .75))
#define _125PERCENT        ((int)(_NBITS_IN_UNSIGNED * .125))
#define _HIGH_BITS         (~((unsigned)(~0) >> _125PERCENT))

PUBLIC unsigned hash_pjw(unsigned char *name)
{
    unsigned h  = 0;
    unsigned g;
    for (; *name; ++name) {
        h = (h << _125PERCENT) + *name;
        if (g = h & _HIGH_BITS) {
            h = (h ^ (g >> _75PERCENT)) & ~_HIGH_BITS;
        }
    }

    return h;
}

PUBLIC void *hash_new_sym(int size)
{
    BUCKET_S *sym;
    if(!(sym = (BUCKET_S *)calloc(size + sizeof(BUCKET_S), 1))){
        fprintf(stderr, "memory is not enough for BUCKET_S\n");
        raise(SIGABRT);
        return NULL;
    }

    return (void *)(sym + 1);  /*return pointer to user space*/
}

PUBLIC void hash_free_sym(void *sym)
{
    if(sym != NULL){
        free((BUCKET_S *)sym - 1);
    }
}

PUBLIC HASH_TAB_S *hash_make_tab(unsigned max_sym, fp_hash_t fp_hash, fp_cmp_t fp_cmp)
{
    HASH_TAB_S *p = NULL;

    if(!max_sym) max_sym = HASH_TABLE_MAX_SYM;

    if(NULL != (p = (HASH_TAB_S *)calloc(1, max_sym * sizeof(BUCKET_S *) + sizeof(HASH_TAB_S)))){
        p -> max_size = max_sym;
        p -> curr_num = 0;
        p -> fp_hash  = fp_hash;
        p -> fp_cmp = fp_cmp;
    } else {
        fprintf(stderr, "memory is not enough for hash table.\n");
        raise(SIGABRT);
        return NULL;
    }

    return p;
}

PUBLIC void *hash_add_sym(HASH_TAB_S *p_tab, void *p_sym)
{
    if(!p_tab || !p_sym) return NULL;

    BUCKET_S **p, *tmp;
    BUCKET_S *sym = (BUCKET_S *)p_sym;

    /*calculate the location of the bucket.*/
    p = &(p_tab->table)[(*(p_tab->fp_hash))(sym--) % p_tab->max_size];

    tmp = *p;
    *p  = sym;
    sym -> prev = p;
    sym -> next = tmp;

    if(tmp) tmp->prev = &sym->next;

    p_tab->curr_num++;
    return (void *)(sym + 1); /*return pointer to the user space.*/

}

PUBLIC void hash_del_sym(HASH_TAB_S *p_tab, void *p_sym)
{
    if(!p_tab || !p_sym) return;

    BUCKET_S *sym = (BUCKET_S *)p_sym;
    --p_tab->curr_num;
    --sym;  /*make it pointer to the real BUCKET location.*/

    if(*(sym->prev) = sym->next){
        sym->next->prev = sym->prev;  /*we don't free space here, it's up to higher user to do it.*/
    }

}

PUBLIC void *hash_find_sym(HASH_TAB_S *p_tab, void *p_sym)
{
    if(!p_tab || !p_sym) return NULL;

    BUCKET_S *p;

    p = (p_tab->table)[(*(p_tab->fp_hash))(p_sym) % p_tab->max_size];
    while(p && ((*p_tab->fp_cmp)(p_sym, p+1)))  /* note : p + 1 means points to the user space here.*/
        p = p->next;

    return (void*) (p ?  p + 1 : NULL);
}

PUBLIC void *hash_next_sym(HASH_TAB_S *p_tab, void *p_last_sym)
{
    if(!p_tab || !p_last_sym) return NULL;

    BUCKET_S *last = (BUCKET_S *)p_last_sym;
    --last; /*points to the real bucket location */

    for(; last -> next; last = last -> next){
        if((*p_tab->fp_cmp)(last + 1, last->next + 1) == 0)
            return (char *)(last->next + 1);
    }

    return NULL;
}

PUBLIC void hash_free_tab(HASH_TAB_S *p_tab)
{
    if(p_tab){
        free(p_tab);
    }
}

PRIVATE fp_cmp_t _user_cmp_func_;
PRIVATE int _bucket_cmp_func(BUCKET_S **p1, BUCKET_S **p2)
{
    return (*_user_cmp_func_)((void *)(*p1 + 1), (void *)(*p2 + 1));
}

/**
 * @fp_pnt : print function used for output;
 * @para   : parameter passed to the print function.
 * @sort   : sort table if true;
 * return 1 if printed success, or return 0.
 */
PUBLIC int hash_print_tab(HASH_TAB_S *p_tab, fp_tab_t fp_pnt, void *para, int sort)
{
    BUCKET_S **pp_out, **p_out, *sym, **symtab;

    if(!p_tab || p_tab->max_size == 0){
        return 1;
    }

    int size = p_tab->max_size;

    if(!sort) {
        for(symtab = p_tab->table; --size >= 0; symtab++){
            for(sym = *symtab; sym; sym = sym->next){
                (*fp_pnt)(sym + 1, para);
            }
        }
    }
    else {
        if( ! (pp_out = (BUCKET_S **)malloc(p_tab->curr_num * sizeof(BUCKET_S *))))
            return 0;

        p_out = pp_out;
        for(symtab = p_tab->table; --size >= 0; symtab++){
            for(sym = *symtab; sym; sym = sym->next){
                if(p_out > pp_out + p_tab->curr_num){
                    fprintf(stderr, "Internal error, table overflow.\n");
                    exit(1);
                }
                *p_out++ = sym;
            }
        }

        _user_cmp_func_ = p_tab->fp_cmp;
        qsort((void **)pp_out, (size_t)p_tab->curr_num, sizeof(BUCKET_S *),
              (int (*)(const void *, const void *))_bucket_cmp_func);

        for(p_out = pp_out, size = p_tab->curr_num; --size >= 0; p_out++){
            (*fp_pnt)((*p_out + 1), para);
        }

        free(pp_out);
    }

    return 1;
}

PUBLIC void hash_dump(HASH_TAB_S *p_tab)
{
    BUCKET_S **p, *buck;
    int i = 0;

    fprintf(stdout, "HASH_TABLE at 0x%08p (%d element table, %d symbol)\n", p_tab,
            p_tab->max_size, p_tab->curr_num);

    for(p = p_tab->table, i = 0; i < p_tab->max_size; ++p, ++i){
        if(!*p)continue;

        fprintf(stdout, "Htab[%3d] @ 0x%08p:\n", i, p);
        for(buck = *p; buck; buck = buck->next){
            fprintf(stdout, "\t0x%08p, prev = 0x%08p, next = 0x%08p user = 0x%08p",
                buck, buck->prev, buck->next, buck + 1);
            fprintf(stdout, "  (%s)\n", (char *)(buck + 1)); /*must ensure the name is the first field.*/
        }
        fprintf(stdout, "\n");
    }
}