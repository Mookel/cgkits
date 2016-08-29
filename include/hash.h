//
// Created by Mookel on 16/8/29.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// hash.h : header file pair with hash.c
//

#ifndef CGKITS_HASH_H
#define CGKITS_HASH_H

typedef struct BUCKET_
{
    struct BUCKET_ *next;
    struct BUCKET_ **prev;
}BUCKET_S;

typedef unsigned (*fp_hash_t)(void *);
typedef int(*fp_cmp_t)(void *, void *);
typedef void (*fp_tab_print_t) (void *, ...);

typedef struct HASH_TAB_
{
    int max_size;                      /*max number of elements in table*/
    int curr_num;                      /*current number of elements in table*/
    fp_hash_t fp_hash;                 /*hash function*/
    fp_cmp_t  fp_cmp;                  /*compare function, cmp(name, p_backet)*/
    BUCKET_S *table[1];                /*first element of actual hash table*/

}HASH_TAB_S;

/*global interface*/
extern HASH_TAB_S *hash_make_tab(unsigned max_sym, fp_hash_t fp_hash, fp_cmp_t fp_cmp);
extern void        *hash_new_sym(int sym_size);
extern void        hash_free_sym(void *sym);
extern void        *hash_add_sym(HASH_TAB_S *hash_tab, void *sym);
extern void        hash_del_sym(HASH_TAB_S *hash_tab, void *sym);
extern void        *hash_find_sym(HASH_TAB_S *hash_tab, void *sym);
extern void        *hash_next_sym(HASH_TAB_S *hash_tab, void *last);
extern void        hash_free_tab(HASH_TAB_S *hash_tab);
extern int         hash_print_tab(HASH_TAB_S *hash_tab, fp_tab_print_t fp_print, void *par, int srt);
extern void        hash_dump(HASH_TAB_S *hash_tab);

/*hash function, just provided two methods.*/
extern unsigned    hash_add(unsigned char *name);
extern unsigned    hash_pjw(unsigned char *name);

#endif //CGKITS_HASH_H
