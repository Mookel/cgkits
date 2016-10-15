%{
#include "yyout.h"
%}
%%
"+" return PLUS;
"*" return STAR;
"(" return LP;
")" return RP;
[0-9]+ return NUM;
[a-z]+ return ID;
%%

