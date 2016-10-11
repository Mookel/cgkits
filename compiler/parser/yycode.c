//
// Created by Mookel on 16/10/11.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yycode.c : 
//

#include "parser.h"

void tables()
{
    make_yy_stok();
    make_token_file();
    make_parse_tables();
}