/*
 * All rights reversed..
 * Written by mookel.
 */

%{
#include "token.h"
%}

%%
"+" return PLUS;
"*" return STAR;
"(" return LP;
")" return RP;
";" return SEMI;
[0-9]+ return NUM;
[a-z]+ return ID;
%%
