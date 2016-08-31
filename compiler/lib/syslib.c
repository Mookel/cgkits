//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// syslib.c : 
//

#include <debug.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslib.h>

#define _IS_HEXDIGIT(x) (isdigit(x) || ('a' <= (x) && (x) <= 'f') || ('A' <= (x) && (x) <= 'F'))
#define _IS_OCTDIGIT(x) ('0' <= (x) && (x) <= '7')

/*
 * Return a pointer to a string that represents c.
 * example : '\n' --> '\'+'n'
 * use_hex if true : \xDD or \DDD
 * */
PUBLIC char *sys_bin_to_ascii(int c, int use_hex)
{
    static char buf[8];

    c &= 0xff;   /*ignore other bits except lower 8 bits.*/
    if(' ' <= c && c < 0x7f && c != '\'' && c != '\\') {
        buf[0] = c;
        buf[1] = '\0';
    } else {
        buf[0] = '\\';
        buf[2] = '\0';
        switch (c) {
            case '\\' : buf[1] = '\\'; break;
            case '\'' : buf[1] = '\''; break;
            case '\b' : buf[1] = '\b'; break;
            case '\f' : buf[1] = '\f'; break;
            case '\t' : buf[1] = '\t'; break;
            case '\r' : buf[1] = '\r'; break;
            case '\n' : buf[1] = '\n'; break;
            default   : sprintf(&buf[1], (use_hex ? "x%03x" : "%03o"), c);
                break;
        }
    }

    return buf;
}

PRIVATE int hex2bin(int c)
{
    return isdigit(c) ? (c - '0') : (toupper(c) - 'A' + 10) & 0xf;
}

PRIVATE int oct2bin(int c)
{
    return (c - '0') & 0x7;
}

/*Map escape sequences into their equivalent symbols
 * \b \f \n \r \s \t \e \DDD \xDDD \^C
 *
 * Note: \^C 's value is get through 'C' - '@' because
 * there is one to one map relationship.
 * examples : ^@(0)  ->  @ - @ = 0
 *            ^K(11) ->  K - @ = 11
 * ^? (del) is much more special, whose ascii value is  127. To get it,
 * we use ascii of ?(63) to subtract ascii of @(64), which get -1 ,just
 * equal to 127 in unsigned int representation.Really nice design !!!
 * */
PUBLIC int sys_esc(char **s)
{
    int rval = 0;

    if(**s != '\\') rval = *((*s)++);
    else {
        ++(*s);
        switch(toupper(**s)){
            case '\0': rval = '\\';   break;
            case 'B' : rval = '\b';   break;
            case 'F' : rval = '\f';   break;
            case 'N' : rval = '\n';   break;
            case 'R' : rval = '\r';   break;
            case 'T' : rval = '\t';   break;
            case 'E' : rval = '\033'; break;

            case '^' :  rval = *(++(*s));
                rval = toupper(rval) - '@';
                break;
            case 'X' : rval = 0;
                ++(*s);
                if(_IS_HEXDIGIT(**s)){
                    rval = hex2bin(*(*s)++);
                }
                if(_IS_HEXDIGIT(**s)){
                    rval <<= 4;
                    rval |= hex2bin(*(*s)++);
                }
                if(_IS_HEXDIGIT(**s)){
                    rval <<= 4;
                    rval |= hex2bin(*(*s)++);
                }
                --(*s);
                break;

            default:
                if(!_IS_OCTDIGIT(**s)) {
                    rval = **s;
                } else {
                    rval = oct2bin(*(*s)++);
                    if(_IS_OCTDIGIT(**s)) {
                        rval <<= 3;
                        rval |= oct2bin(*(*s)++);
                    }
                    if(_IS_OCTDIGIT(**s)) {
                        rval <<= 3;
                        rval |= oct2bin(*(*s)++);
                    }
                    --(*s);
                }
                break;
        }

        ++(*s);
    }

    return rval;
}

#if 0
/*
 * Copy the src to the destination file, opening the destination in
 * the indicated mode. buffsize used on the first call will be used
 * on subsequent calls as well. errno will hold the appropriate error
 * code  if return value is 0.
 * */
PUBLIC int sys_copyfile(char *dst, char *src, char *mode)
{
    int fd_dst, fd_src;
    char *buf;
    int got;
    int ret_val = FILE_ERR_NONE;
    static long buf_size = 31 * 1024;

    while(buf_size > 0 && !(buf = malloc((int) buf_size)))
        buf_size -= 1024L;



}

PUBLIC int sys_movefile(char *dst, char *src, char *mode)
{

}

#if 0
PUBLIC void sys_defnext(FILE *fp, char *name)

/*driver1 and driver2 work together to transfer a template file to a lex or parser
 *file. driver1 must be called first.
 * */
PUBLIC FILE *sys_driver_1(FILE *output, int lines, char *file_name)

PUBLIC int sys_driver_2(FILE *output, int lines)

/*Compute the mean of a bunch of samples. reset must be set true the first time and
 * after set reset to false. *dev is modified to hold the standard deviation.
 * */
PUBLIC double sys_mean(int reset, double sample, double *dev)

PUBLIC long sys_stol(char **instr)

PUBLIC unsigned long sys_stoul(char **instr)

PUBLIC int *sys_memiset(int *dst, int value, int count)

PUBLIC int sys_searchenv(char *filename, char *envname, char *pathname)

PUBLIC int sys_pairs(FILE *fp, ATYPE *array, int nrows, int ncols,
                     char *name, int threshold, int numbers)

PUBLIC int sys_pnext(FILE *fp, char *name)

PUBLIC void sys_prnt(fp_print_t fp_prnt, void *fun_arg, char *format, va_list args)

PUBLIC void sys_stop_prnt(void)

PUBLIC void sys_pchar(int c, FILE *stream)

PUBLIC void sys_print_array(FILE *fp, ATYPE *array, int nrows, int ncols)

PUBLIC void sys_printv(FILE *fp, char **argv)

PUBLIC void sys_comment(FILE *fp, char **argv)

PUBLIC void sys_fputstr(char *str, int maxlen, FILE *stream)

PUBLIC int sys_concat(int size, char *dst, ...)

PUBLIC int sys_ferr(char *format, ...)

PUBLIC int sys_on_ferr(void)
#endif