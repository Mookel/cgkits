//
// Created by Mookel on 16/10/1.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// stok.c : 
//

#include "parser.h"

/*
 * This subroutine generates the Yy_stok[] array that's indexed by
 * token value and evaluates to a string representing the token
 * name. Token values are adjusted so that the smallest token value
 * is 1 (0 is reserved for end of input).
 */
PUBLIC void make_yy_stok()
{
    register int i;
    static char *text[] = {
        "Yy_stok[] is used for debugging and error messages.It is indexed by",
        "the internal value used for a token(as used for a column index in the",
        "transition matrix) and evaluates to a string naming that token.",
        NULL
    };

    sys_comment(g_output, text);

    output("char *Yy_stok[] = \n{\n");
    output("\t/*   0 */    \"_EOI_\",\n");

    for(i = MINTERM; i <= g_currterm; i++) {
        output("\t/* %3d */    \"%-10s\"", (i-MINTERM)+1, g_terms[i]->name);
        if(i != g_currterm) outc(',');

        if(!(i & 0x1) || (i == g_currterm)) outc('\n');
    }

    output("};\n\n");
}

/*
 * This subroutine generates the yytokens.h file. Tokens have the same
 * value as in make_yy_stok(). A special token named _EOI_ (with a value
 * of 0) is also generated.
 */
PUBLIC void make_token_file()
{
    FILE *tok_file;
    int i;

    if(!(tok_file = fopen(TOKEN_FILE, "w")))
        error(FATAL, "Can't open %s\n", TOKEN_FILE);
    D(else if(g_cmdopt.verbose))
    D(  printf("Generating %s\n", TOKEN_FILE);)

    fprintf(tok_file, "#define _EOI_    0\n");

    for(i = MINTERM; i <= g_currterm; ++i)
        fprintf(tok_file, "#define %-10s %d\n", g_terms[i]->name, (i-MINTERM) + 1);

    fclose(tok_file);
}