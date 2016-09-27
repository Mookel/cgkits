//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// syslib.h : 
//

#ifndef CGKITS_SYSLIB_H
#define CGKITS_SYSLIB_H

#include <debug.h>
#include <stdio.h>
#include <stdarg.h>

extern  char     *sys_bin_to_ascii(int c, int use_hex);

/*Map escape sequences into their equivalent symbols*/
extern  int      sys_esc(char **s);

#define FILE_ERR_NONE      0
#define FILE_ERR_DST_OPEN  -1
#define FILE_ERR_SRC_OPEN  -2
#define FILE_ERR_READ      -3
#define FILE_ERR_WRITE     -4

extern  int      sys_copyfile(char *dst, char *src, char *mode);
extern  int      sys_movefile(char *dst, char *src, char *mode);

/*driver1 and driver2 work together to transfer a template file to a lex or parser
 *file. driver1 must be called first.
 * */
extern  int      sys_searchenv(char *filename, char *envname, char *pathname);
extern  FILE*    sys_driver_1(FILE *output, int lines, char *file_name);
extern  int      sys_driver_2(FILE *output, int lines);

/*Compute the mean of a bunch of samples. reset must be set true the first time and
 * after set reset to false. *dev is modified to hold the standard deviation.
 * */
extern  double   sys_mean(int reset, double sample, double *dev);
extern  long     sys_stol (char **instr);
extern  unsigned long sys_stoul(char **instr);
extern  int*     sys_memiset(int *dst, int value, int count);

/*For char/mext pairs, mainly used for compress a table.*/
typedef int ATYPE;   /*type of input tables*/
#define NCOLS    10
extern  int      sys_pairs(FILE *fp, ATYPE *array, int nrows, int ncols,
                          char *name, int threshold, int numbers);
extern  int      sys_pnext(FILE *fp, char *name);

/*print like functions.*/
extern  void     sys_pchar(int c, FILE *stream);
extern  void     sys_print_array(FILE *fp, ATYPE *array, int nrows, int ncols);
extern  void     sys_printv(FILE* fp, char **argv);
extern  void     sys_comment(FILE* fp, char **argv);
extern  void     sys_print_defnext(FILE *fp, char *name);
extern  void     sys_fputstr(char *str, int maxlen, FILE *stream);

#endif //CGKITS_SYSLIB_H
