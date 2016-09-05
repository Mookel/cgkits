/*
 * All rights reversed..
 * Written by mookel.
 */

%{
#include <stdio.h>
#include <string.h>

     extern union{   /*struct*/
         char *p_char;
         int integer;
     }yylval;

%}

let [_a-zA-Z] /*letter*/
suffix [UuLl]  /*suffix*/

%%
