//
// Created by Mookel on 16/9/7.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// print.c : 
//

#include <stdio.h>
#include <string.h>
#include <compiler.h>
#include "print.h"
#include "globals.h"

/*print out a header comment that describes the uncompressed dfa*/
PUBLIC void pheader(FILE *fp, ROW dtran[], int nrows, ACCEPT *accept)
{
    int i, j;
    int last_transition;
    int chars_printed;

    fprintf(fp, "#ifdef __NEVER__\n");
    fprintf(fp, "/*------------------------------------------------\n");
    fprintf(fp, "* DFA (start state is 0) is:\n *\n");

    for(i = 0;i < nrows; ++i){
        if(!accept[i].string) {
            fprintf(fp, " * State %d [nonaccepting]", i);
        } else {
            fprintf(fp, " * State %d [accepting, line %d <",
                    i, ((int *)(accept[i].string))[-1]);
            sys_fputstr(accept[i].string, 20, fp);
            fprintf(fp, ">]");

            if(accept[i].anchor) {
                fprintf(fp, " Anchor: %s%s", accept[i].anchor & START ? "Start" : "",
                        accept[i].anchor & END ? "end" : "");
            }
        }

        last_transition = -1;
        for(j = 0; j < MAX_CHARS; ++j){
            if(dtran[i][j] != F) {
                if(dtran[i][j] != last_transition){
                    fprintf(fp, "\n *    goto %2d on ", dtran[i][j]);
                    chars_printed = 0;
                }

                fprintf(fp, "%s", sys_bin_to_ascii(j, 1));

                if((chars_printed += strlen(sys_bin_to_ascii(j, 1))) > 56) {
                    fprintf(fp, "\n *                ");
                    chars_printed = 0;
                }

                last_transition = dtran[i][j];
            }
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "*/\n\n");
    fprintf(fp, "#endif\n");
}

PUBLIC void pdriver(FILE *output, int nrows, ACCEPT *accept)
{
    int i;
    static char *text[] = {
            "The Yyaccept array has two purposes.If Yyaccept[i] is 0 then state",
            "i is nonaccepting. If it's nonzero then the number determines whether",
            "the string is anchored, 1=anchored at start of line, 2=at end of",
            "line, 3= both, 4=line not anchored",
            NULL
    };

    sys_comment(output, text);
    fprintf(output, "YYPRIVATE YY_TTYPE Yyaccept[] = \n");
    fprintf(output, "{\n");

    for(i = 0; i < nrows; ++i) {
        if(!accept[i].string){
            fprintf(output, "\t0  ");
        } else {
            fprintf(output, "\t%-3d", accept[i].anchor ? accept[i].anchor : 4);
        }
        fprintf(output, "%c    /* State %d-3d */\n", i == (nrows - 1) ? ' ' : ',', i);
    }
    fprintf(output, "};\n\n");

    sys_driver_2(output, !g_no_lines);

    for( i = 0;i < nrows; ++i){
        if(accept[i].string) {
            fprintf(output, "\t\t case %d:\t\t\t\t\t/* State %-3d */\n", i, i);
            if(!g_no_lines)
                fprintf(output, "#Line %d\" %s \"\n",
                        *((int *)(accept[i].string) - 1), g_input_file_name);

            fprintf(output, "\t\t    %s\n", accept[i].string);
            fprintf(output, "\t\t    break;\n");


        }
    }

    sys_driver_2(output, !g_no_lines);
}

