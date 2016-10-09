//
// Created by Mookel on 16/10/9.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yydriver.c : 
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
    output("#include \"%s\"\n\n", TOKEN_FILE);

    if(g_cmdopt.public) output("#define PRIVATE\n");
    if(g_cmdopt.debug)  output("#define YYDEBUG\n");
    if(g_cmdopt.make_actions) output("#define YYACTION\n");
    if(g_cmdopt.make_parser)  output("#define YYPARSER\n");

    if(!(_driver_file = sys_driver_1(g_output, !g_cmdopt.no_lines, g_template)))
        error(NONFATAL, "%s not found--output file won't compile\n", g_template);
}

/*
 * This header is printed after the definitons section is processed,
 * but but before any tables or the driver is processed.
 */
void code_header()
{
    sys_driver_2(g_output, !g_cmdopt.no_lines);
}

void driver()
{
    if(g_cmdopt.make_parser) sys_driver_2(g_output, !g_cmdopt.no_lines);
    fclose(_driver_file);
}