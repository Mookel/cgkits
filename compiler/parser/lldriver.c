//
// Created by Mookel on 16/9/27.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// lldriver.c : 
//

#include "parser.h"
#include "error.h"

PRIVATE FILE *_driver_file = 0;

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

void code_header()
{

}

void driver()
{

}

