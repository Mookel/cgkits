//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// syslib.c : 
//

#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <syslib.h>

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

    while(buf_size > 0 && !(buf = malloc((size_t) buf_size)))
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
    if(buf_size > 256) free(buf);

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
PUBLIC int sys_concat(int size, char *dst, ...)
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
    sys_concat(_MAX_PATH_NAME_LEN, pathname, pathname, "/", filename, NULL);

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
        sys_ferr("INTERNAL ERROR [driver_2], Tempalte file not open.\n");
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
            sys_ferr("WRITE OUTPUT ERROR [driver_2].\n");
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
    return 0;
}


PUBLIC int sys_pairs(FILE *fp, ATYPE *array, int nrows, int ncols,
                     char *name, int threshold, int numbers)
{
    return 0;
}

PUBLIC int sys_pnext(FILE *fp, char *name)
{
    return 0;
}

PUBLIC void sys_prnt(fp_print_t fp_prnt, void *fun_arg, char *format, va_list args)
{
    char buf[256], *p;
    vsprintf(buf, format, args);
    for(p = buf; *p; ++p)
        (*fp_prnt)(*p,  fun_arg);
}

PUBLIC void sys_stop_prnt(void)
{

}

PUBLIC void sys_pchar(int c, FILE *stream)
{

}

PUBLIC void sys_print_array(FILE *fp, ATYPE *array, int nrows, int ncols)
{

}

PUBLIC void sys_printv(FILE *fp, char **argv)
{

}

PUBLIC void sys_comment(FILE *fp, char **argv)
{

}

PUBLIC void sys_defnext(FILE *fp, char *name)
{

}

PUBLIC void sys_fputstr(char *str, int maxlen, FILE *stream)
{

}


PUBLIC int sys_ferr(char *format, ...)
{
    D(void (**ret_addr_p)(); )
    va_list args;

    va_start(args, format);
    if(format) sys_prnt((fp_print_t)fputc, stderr, format, args);
    else perror(va_arg(args, char *));
    va_end(args);

    D(ret_addr_p = (void(**)()) &fmt;)
    /*Attention : Be carefully in 64bit PC mac os, this is not reliable.*/
    D(fprintf(stderr, "\n\t--ferr() called from %p\n", ret_addr_p[-1]);)

    exit(sys_on_ferr());
}

PUBLIC int sys_on_ferr(void)
{
    return errno;
}