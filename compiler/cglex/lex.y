/*
 * All rights reversed..
 * Written by mookel.
 */

%{
#include "llout.h"
%}

%%
"+" return PLUS;
"*" return TIMES;
"(" return LP;
")" return RP;
";" return SEMI;
[0-9]+ |
[a-z_]+ return NUM_OR_ID;
%%
