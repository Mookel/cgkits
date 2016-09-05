//
// Created by Mookel on 16/9/5.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// input.c : 
//

#include "globals.h"

/*private functions*/
/*
 * Get a line of input.Get at most n-1 characters. Update *stringp
 * to point at the '\n' at the end of string .Return a lookhead
 * character(the character that follows the \n in the input).The
 * '\n' is not put into the string.
 * Return (1)the character follwing the \n normally.
 *        (2)EOF at end of file.
 *        (3)0 if the line is too long.
 * */
PRIVATE int get_line(char **stringp, int n, FILE *stream)
{
    static int lookhead = 0;
    char *str = *stringp;

    if(lookhead == 0) lookhead = getc(stream);

    if(n > 0 && lookhead != EOF){
        while(--n > 0){

            *str = lookhead;
            lookhead = getc(stream);

            if(*str == '\n' || *str == EOF) break;
            ++str;
        }

        *str = '\0';
        *stringp = str;
    }

    return (n <= 0) ? 0 : lookhead;
}


/*global functions*/
/*
 * Input routine for nfa(), Gets a regular expression and the associated string
 * from the input stream. Returns a pointer to the input string normally.
 * Returns NULL on end of file or if a line beginning with % is encountered.
 * All blank lines are discarded and all lines that start with whitespace are
 * concatenated to the previous line. The global variable lineno is set
 * to the line number of the top line of a multiple-line block.Actual lineno
 * has holds the real line number.
 * */
PUBLIC char *get_expr(void)
{
    static int lookhead = 0;
    int space_left;
    char *p;

    p = g_input_buffer;
    space_left = MAX_RULE_SIZE;

    if(g_verbose > 1) printf("b%d: ", g_actual_lineno);

    if(lookhead == '%') return NULL;

    g_lineno = g_actual_lineno;

    while((lookhead = get_line(&p, space_left - 1, g_ifile)) != EOF){

        if(lookhead == 0) lerror(1, "Rule too long.\n");

         ++g_actual_lineno;

        if(!g_input_buffer[0]) continue;  /*Note : The regular expression can't not begin with space.*/

        space_left = MAX_RULE_SIZE - (int)(p - g_input_buffer);

        if(!isspace(lookhead)) break;

        *p++ = '\n';
    }

    if(g_verbose > 1) printf("%s\n", lookhead ? g_input_buffer : "----EOF----");

    return lookhead ? g_input_buffer : NULL;

}