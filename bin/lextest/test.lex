/*
 * All rights reversed..
 * Written by mookel.
 */

%{
#include "token.h"
%}

%%
[^\x00-\s%\{}[\]()*:|;,<>]+ return NAME;
%%
