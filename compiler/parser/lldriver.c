//
// Created by Mookel on 16/9/27.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// lldriver.c : 
//

#include "parser.h"

PRIVATE FILE *_driver_file = 0;

/*
 * This header is printed at the top of the output file, before
 * the definitions section is processed. Various #defines that
 * you might want to modify are put here.
 */
void file_header()
{
    if(g_cmdopt.public)
        output("#define YYPRIVATE \n");

    if(g_cmdopt.debug)
        output("#define YYDEBUG\n");

    output("\n/*---------------------------------------------*/\n\n");

    if(!(_driver_file = sys_driver_1(g_output, !g_cmdopt.no_lines, g_template)))
        error(NONFATAL, "%s not found--output file won't compile\n", g_template);

    output("\n/*---------------------------------------------*/\n\n");

}

/*
 * This header is printed after the definitons section is processed,
 * but but before any tables or the driver is processed.
 */
void code_header()
{
    output("\n\n/*-------------------------------------------*/\n\n");
    output("#include \"%s\"\n\n", TOKEN_FILE);
    output("#define YY_MINTERM     1\n");
    output("#define YY_MAXTERM     %d\n", g_currterm);
    output("#define YY_MINNONTERM  %d\n", MINNONTERM);
    output("#define YY_MAXNONTERM  %d\n", g_currnonterm);
    output("#define YY_START_STATE %d\n", MINNONTERM);
    output("#define YY_MINACT      %d\n", MINACT);
    output("\n");

    sys_driver_2(g_output, !g_cmdopt.no_lines);
}

void driver()
{
    sys_driver_2(g_output, !g_cmdopt.no_lines);
    fclose(_driver_file);
}

