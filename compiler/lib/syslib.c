//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// syslib.c : 
//

#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <com.h>
#include <syslib.h>
#include <gc.h>

/*private macro definitions.*/
#define _IS_HEXDIGIT(x) (isdigit(x) || ('a' <= (x) && (x) <= 'f') \
                                    || ('A' <= (x) && (x) <= 'F'))

#define _IS_OCTDIGIT(x) ('0' <= (x) && (x) <= '7')
#define _MAX_PATH_NAME_LEN  (128+1)

/*staic or global variable definitions.*/
PRIVATE FILE *_input_file = NULL;   /*current input file.*/
PRIVATE int   _input_line = 0;      /*line number of most-recently read line.*/
PRIVATE char  _file_name[128];      /*template-file name.*/

/*private functions definitions*/
PRIVATE int hex2bin(int c)
{
    return isdigit(c) ? (c - '0') : (toupper(c) - 'A' + 10) & 0xf;
}

PRIVATE int oct2bin(int c)
{
    return (c - '0') & 0x7;
}

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
            case 'S' : rval = ' ' ;   break;
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

/*
 * Copy the src to the destination file, opening the destination in
 * the indicated mode. buffsize used on the first call will be used
 * on subsequent calls as well. errno will hold the appropriate error
 * code  if return value is 0.
 * mode : 'w' or 'a'
 * */
PUBLIC int sys_copyfile(char *dst, char *src, char *mode)
{
    int fd_dst = -1;
    int fd_src = -1;
    char *buf = NULL;
    char local_buf[256];
    ssize_t got;
    int ret_val = FILE_ERR_NONE;
    static long buf_size = 31 * 1024;

    while(buf_size > 0 && !(buf = GC_MALLOC((size_t) buf_size)))
        buf_size -= 1024L;

    if(!buf_size){
        buf_size = 256;
        buf = &local_buf[0];
    }

    fd_src = open(src, O_RDONLY | O_BINARY);
    fd_dst = open(dst, O_WRONLY | O_BINARY | O_CREAT |
            ((*mode == 'w')  ? O_TRUNC : O_APPEND), S_IRUSR | S_IWUSR);

    if(fd_src == -1) { ret_val = FILE_ERR_DST_OPEN;}
    else if(fd_dst == -1) {ret_val = FILE_ERR_DST_OPEN;}
    else {
        while((got = read(fd_src, buf, (size_t)buf_size)) > 0) {
            if (write(fd_dst, buf, got) == -1) {
                ret_val = FILE_ERR_WRITE;
                break;
            }
        }

        if(got == -1) ret_val = FILE_ERR_READ;
    }

    if(fd_dst != -1) close(fd_dst);
    if(fd_src != -1) close(fd_src);
    if(buf_size > 256) GC_FREE(buf);

    return ret_val;
}

/*works just like copyfile ,but deletes src if copy is successful.*/
PUBLIC int sys_movefile(char *dst, char *src, char *mode)
{
    int ret_val;
    if((ret_val = sys_copyfile(dst, src, mode)) == FILE_ERR_NONE){
        unlink(src);
    }
    return ret_val;
}

/*
 * Search for files by looking in the directories listed in the envname
 * environment.Put the full path name(if found) into the pathname, or
 * set it to 0.
 * Note : The pathname array must be at least 128 characters.If
 * the pathname is in the current dirctory, the pathname of the current
 * directory is appended to the front of the name.
 * Return 1 if success or 0 on failed.
 * */
PUBLIC int sys_searchenv(char *filename, char *envname, char *pathname)
{
    char pbuf[_MAX_PATH_NAME_LEN];
    char *p;

    getcwd(pathname, _MAX_PATH_NAME_LEN);
    com_concat(_MAX_PATH_NAME_LEN, pathname, pathname, "/", filename, NULL);

    /*first check current directory*/
    if(access(pathname, 0) != -1){
        return 1;
    }

    /*continue to serach in other directory*/
    if(strpbrk(filename, "/") || !(p = getenv(envname))){
        return (*pathname == '\0');
    }

    strncpy(pbuf, p, _MAX_PATH_NAME_LEN);
    if(p  = strtok(pbuf, ": ")) {
        do{
            sprintf(pathname, "%.90s/%.90s", p, filename);
            if(access(pathname, 0) != -1) {
                return 1;
            }
        }while(p = strtok(NULL, ": "));
    }

    return (*pathname = '\0');
}

/*driver1 and driver2 work together to transfer a template file to a lex or parser
 *file. driver1 must be called first.
 * */
PUBLIC FILE *sys_driver_1(FILE *output, int lines, char *file_name)
{
    char path[80];

    if(!(_input_file = fopen(file_name, "r"))){
        sys_searchenv(file_name, "CGKLIB", path);   /*Library to be modified....*/
        if(!*path) {
            errno = ENOENT;
            return NULL;
        }

        if(!(_input_file = fopen(path, "r"))){
            return NULL;
        }
    }

    strncpy(_file_name, file_name, sizeof(_file_name));
    _input_line = 0;
    sys_driver_2(output, lines);

    return _input_file;
}

PUBLIC int sys_driver_2(FILE *output, int lines)
{
    static char buf[256];
    char *p;
    int process_comment = 0;

    if(!_input_file){
        com_ferr("INTERNAL ERROR [driver_2], Tempalte file not open.\n");
    }

    if(lines)
        fprintf(output, "\n#line %d \"%s\"\n", _input_line + 1, _file_name);

    while(fgets(buf, sizeof(buf), _input_file)) {
        ++_input_line;
        if(*buf == '\f') break;
        for(p = buf; isspace(*p); ++p)
            ;
        if(*p == '@') {
            process_comment = 1;
            continue;
        }
        else if(process_comment) {
            process_comment = 0;
            if(lines)
                fprintf(output, "\n#line %d \"%s\"\n", _input_line + 1, _file_name);
        }

        if(fputs(buf, output) == -1){
            com_ferr("WRITE OUTPUT ERROR [driver_2].\n");
        }
    }

    return (feof(_input_file));
}

/*Compute the mean of a bunch of samples. reset must be set true the first time and
 * after set reset to false. *dev is modified to hold the standard deviation.
 * */
PUBLIC double sys_mean(int reset, double sample, double *dev)
{
    static double m_xhat, m_ki, d_xhat, d_ki;
    double mean;

    if(reset)
        return (m_ki = m_xhat = d_ki = d_xhat = 0.0);

    m_xhat += (sample - m_xhat) / ++m_ki;
    mean = m_xhat;
    *dev = sqrt(d_xhat += (pow(fabs(mean - sample), 2.0) - d_xhat)/++d_ki);

    return mean;
}

/**
 * This function will stops on encountering the first character which is
 * not a digit.*instr will be updated to point past the end of the number.
 * */
PUBLIC long sys_stol(char **instr)
{
    while(isspace(**instr)) ++*instr;

    if(**instr != '-') return (long)(sys_stoul(instr));

    ++*instr;
    return -(long)(sys_stoul(instr));
}

PUBLIC unsigned long sys_stoul(char **instr)
{
    unsigned long num = 0;
    char *str = *instr;

    while(isspace(*str)) ++str;

    if(*str != '0'){                            /*decimal */
        while(isdigit(*str)) {
            num = (num * 10) + (*str++ - '0');
        }
    } else {
        if(*++str == 'X' || *str == 'x') {       /*hex*/
            for(++str; isxdigit(*str); ++str){
                num = (num * 16)  + (isdigit(*str) ? (*str - '0') :
                                     toupper(*str) - 'A' + 10);
            }
        } else {
            while('0' <= *str && *str <= '7') { /*octal*/
                num = (num * 8) + *str++ - '0';
            }
        }
    }

    *instr = str;

    return num;
}

PUBLIC int *sys_memiset(int *dst, int value, int count)
{
    int *targ;
    for(targ = dst; --count >= 0; *targ++ = value)
        ;

    return dst;
}

#define TYPE     "YY_TTYPE"
#define SCLASS   "YYPRIVATE"
#define D_SCLASS "YYPRIVATE"

/*
 * This pairs/pnext compress a table horizontally (using char/next pairs) and then
 * print the compressed table. The compressed array looks like this:
 * Yy_nxt:        Yy_nxtDD:
 * +-------+    +------------------------------------------------+
 * |   *---|--->|  0 |  Next state array, indexed by character   |
 * +-------+    +------------------------------------------------+
 * |       |
 * +-------+    +------+-----+------+-----+------+------+
 * |   *---|--->| count| c1  |  s1  | c2  |  s2  | .... |
 * +-------+    +------+-----+------+-----+------+------+
 * | NULL  |
 * +-------+
 *
 * Sys_pairs generate the C source code for a pair-compressed DTRAN.
 * Returns the number of cells used for the YysDD arrays. The "numbers"
 * argument determines the output format of the character part of a
 * character/next-state pair. If numbers is true, then normal numbers
 * are used, otherwise ascii characters are used, for example:
 * 'a',100 as compared to 97,100
 *
 * fp : output file.
 * array: DFA transition table.
 * name : used for output array.
 * threshold: Array vs. pairs threshold.
 * numbers  : Use numbers for char.
 * */
PUBLIC int sys_pairs(FILE *fp, ATYPE *array, int nrows, int ncols,
                     char *name, int threshold, int numbers)
{
    int i, j, ntransitions, nprinted, ncommas;
    int num_cells = 0;
    ATYPE  *p;

    for(i = 0;i < nrows; ++i){
        ntransitions = 0;
        for(p = array + (i * ncols), j = ncols; --j >= 0; ++p) {
            if (*p != -1) {
                ++ntransitions;
            }
        }

        if(ntransitions) {
            fprintf(fp, "%s %s %s%-2d[] = {", SCLASS, TYPE, name, i);
            ++num_cells;
            if(ntransitions > threshold){    /*array*/
                fprintf(fp, "0,\n                   ");
            } else {                         /*pairs*/
                fprintf(fp, "%2d, ", ntransitions);
                if(threshold > 5){
                    fprintf(fp, "\n                ");
                }
            }

            nprinted = NCOLS;
            ncommas = ntransitions;

            for(p = array+ (i * ncols), j = 0; j < ncols; j++,++p){
                if(ntransitions > threshold) { /*array*/
                    ++num_cells;
                    --nprinted;
                    fprintf(fp, "%3d", *p);
                    if(j < ncols - 1){
                        fprintf(fp, ", ");
                    }
                }else if(*p != -1){  /*pairs*/
                    num_cells += 2;

                    if(numbers){
                        fprintf(fp, "%d,%d", j, *p);
                    }else{
                        fprintf(fp, "'%s',%d", sys_bin_to_ascii(j, 0), *p);
                        nprinted -= 2;
                        if(--ncommas > 0) {
                            fprintf(fp, ", ");
                        }
                    }

                }

                if(nprinted <= 0){
                    fprintf(fp, "\n                 ");
                    nprinted = NCOLS;
                }
            }

            fprintf(fp, "};\n");
        }
    }

    fprintf(fp, "\n%s %s *%s[ %d ] =\n{\n      ", SCLASS, TYPE, name, nrows);
    nprinted = 10;
    for(--nrows, i = 0; i < nrows;i++){
        ntransitions = 0;
        for(p = array + (i * ncols), j = ncols; --j >= 0; ++p){
            if(*p != -1){
                ++ntransitions;
            }
        }

        if(ntransitions) {
            fprintf(fp, "%s%-2d,    ", name , i);
        } else {
            fprintf(fp, "NULL,    ");
        }

        if(--nprinted <= 0) {
            fprintf(fp, "\n    ");
            nprinted = 10;
        }
    }

    fprintf(fp, "%s%-2d\n};\n\n", name, i);

    return num_cells;
}

/**
 * Print out a next(state, c) subroutine for a table compressed
 * into char/next-state pairs.
 */
PUBLIC int sys_pnext(FILE *fp, char *name)
{
    static char *toptext[] = {
        "unsigned int c;",
        "int      cur_state;",
        "{",
        "    /* Given the current state and the current input character, return",
        "     * the next state.",
        "     */",
        "",
        NULL,
    };

    static char *boptext[] = {
      "    int i;",
      "",
      "    if(p)",
      "    {",
      "        if((i = *p++) == 0)",
      "            return p[ c ];",
      "",
      "        for(; --i >= 0; p += 2)",
      "            if(c == p[0])",
      "                return p[1];",
      "    }",
      "    return YYF;",
      "}",
      NULL,
    };

    fprintf(fp, "\n/*-------------------------------------------------------*/\n");
    fprintf(fp, "%s %s yy_next( cur_state, c )\n", D_SCLASS, TYPE);
    sys_printv(fp, toptext);
    fprintf(fp, "    %s    *p = %s[ cur_state ] ;\n", TYPE, name);
    sys_printv(fp, boptext);
}

PUBLIC void sys_pchar(int c, FILE *stream)
{
    fputs(sys_bin_to_ascii(c, 1), stream);
}

/*
 * Print the C source code to initialize the two-dimensional array pointed
 * to by "array". Print only the initialization part of the declaration.
 * array: DFA transition table
 * nrows: number of  rows in array[];
 * ncols: number of columns in array[];
 * */
PUBLIC void sys_print_array(FILE *fp, ATYPE *array, int nrows, int ncols)
{
    int i;
    int col;

    fprintf(fp, "{\n");

    for(int i = 0;i < nrows; ++i){

        fprintf(fp, "/* %02d */ { ", i);

        for(col = 0; col < ncols; ++col){
            fprintf(fp, "%3d", *array++);
            if(col < ncols - 1) {
                fprintf(fp, ", ");
            }

            if(((col % NCOLS) == (NCOLS - 1)) && (col != (ncols - 1))) {
                fprintf(fp, "\n           ");
            }
        }

        if(col > NCOLS) {
            fprintf(fp, "\n          ");
        }

        fprintf(fp, "}%c\n", i < nrows - 1 ? ',' : ' ');
    }

    fprintf(fp, "};\n");
}

PUBLIC void sys_printv(FILE *fp, char **argv)
{
    while(*argv) fprintf(fp, "%s\n", *argv++);
}

PUBLIC void sys_comment(FILE *fp, char **argv)
{
    fprintf(fp, "\n/*--------------------------------------------------------------\n");
    while(*argv) fprintf(fp, " * %s\n", *argv++);
    fprintf(fp, " */\n\n");
}

/*
 * print the default yy_next(s, c) subroutine for an uncompressed table.
 * */
PUBLIC void sys_print_defnext(FILE *fp, char *name)
{
    static char *comment_text[] = {
            "yy_next(state, c) is given the current state and input character and evaluates to the next state.",
            NULL
    };

    sys_comment(fp, comment_text);
    fprintf(fp, "#define yy_next(state, c) %s[ state ][ c ]\n", name);
}

/*
 * Print a string with control characters mapped to readable string.
 * */
PUBLIC void sys_fputstr(char *str, int maxlen, FILE *stream)
{
    char *s;

    while(*str && maxlen >= 0){
        s = sys_bin_to_ascii(*str++, 1);
        while(*s && (--maxlen >= 0)) {
            putc(*s++, stream);
        }
    }
}