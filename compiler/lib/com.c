//
// Created by Mookel on 16/9/5.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// com.c : 
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <com.h>

/*Concatenates an arbitrary number of strings into a single destination
 * array of size "size" .At most size-1 characters are copied.
 * use example: char target[SIZE];
 * concat(SIZE, target, "first", "second", ..., "last", NULL);
 * Return condition:
 * (1) size <= 1 , return -1;
 * (2) size > 1 but ... is NULL, return size;
 * (3) size > 1 but ... len is longer than size, return -1;
 * (4) size > 1 and ... len is smaller than size, return size - length;
 * */
PUBLIC int com_concat(int size, char *dst, ...)
{
    char *src;
    va_list args;
    va_start(args, dst);

    while((src = va_arg(args, char *)) && size > 1){
        while(*src && (size-- > 1)) {
            *dst++ = *src++;
        }
    }

    *dst++ = '\0';
    va_end(args);

    return (size <= 1 && src && *src)  ? -1 : size;
}

PUBLIC void com_prnt(fp_print_t fp_prnt, void *fun_arg, char *format, va_list args)
{
    char buf[256], *p;
    vsprintf(buf, format, args);
    for(p = buf; *p; ++p)
        (*fp_prnt)(*p,  fun_arg);
}

PUBLIC void com_stop_prnt(void)
{

}

PUBLIC int com_ferr(char *format, ...)
{
    D(void (**ret_addr_p)(); )
    va_list args;

    va_start(args, format);
    if(format) com_prnt((fp_print_t) fputc, stderr, format, args);
    else perror(va_arg(args, char *));
    va_end(args);

    D(ret_addr_p = (void(**)()) &fmt;)
    /*Attention : Be carefully in 64bit PC mac os, this is not reliable.*/
    D(fprintf(stderr, "\n\t--ferr() called from %p\n", ret_addr_p[-1]);)

    exit(com_on_ferr());
}

PUBLIC int com_on_ferr(void)
{
    return errno;
}