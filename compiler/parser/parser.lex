/*@A (c) 2006 recreated by mookel*/

%{
#include <stdio.h>
#include <compiler.h>
#include "llout.h"

#define CREATING_LLAMA_PARSER
#include "parser.h"
#include "error.h"

/*--------------------------------------------------------------------
 * Lexical analyzer for both llama and occs. Note that llama doesn't
 * support %left, %right, %noassoc, or %prec. They are recognized here
 * so that we can print an error message when they're encountered. By
 * the same token, occs ignores the %synch directive. Thought all legal
 * llam infput files can be processed by occs, the reverse is not true.
 * -------------------------------------------------------------------
 * Whitespace,comments,and otherwise illegal characters must be handled
 * specifally. When we're processing code blocks, we need to get at the
 * characters so that they can be passed to the output, otherwise, the
 * characters should be ignored.The ws() and nows() subroutines(at the
 * bottom of the file) switch between these behaviors by changing the
 * value of ignore.If ignore is true, white space is ignored.
 */

PRIVATE bool _ignore = false;
PRIVATE int  _start_line;

void stripcr(char *src) /*Remove carriage returns (but not linefeeds) from src*/
void nows();
void ws();

%}

c_name [A-Za-z_][A-Za-z_0-9]*

%%

"/*"    { /*Absorb a comment (treat it as WHITESPACE)*/
            int i;
            int start = yylineno;

            while(i = input()) {  /*return -1 means there are some errors.*/
                if(i < 0) {
                    ii_unterm();
                    ii_flush();
                    ii_term();
                    e_lerror(NONFATAL, "Comment starting on line ");
                    e_lerror(NOHDR, "%d too long, truncating.\n", start);
                } else if(i == '*' && ii_lookahead(1) == '/'){
                    input();
                    stripcr(yytext);
                    if(_ignore) goto end;
                    else return WHITESPACE;
                }
            }

            e_lerror(FATAL, "End of file encountered in comment\n");
            end:;
        }

        /*Suck up entire action.*/
\{      {
            int i;
            int nestlev;
            int lb1;            /*preivous character*/
            int lb2;            /*character before that*/
            bool in_string;     /*processing string constant*/
            bool in_char_const; /*processing char constant*/
            bool in_comment;    /*preocessing a comment*/

            lb1 = lb2 = 0;
            in_string = false;
            in_char_const = false;
            _start_line = yylineno;

            for(nestlev = 1; i = input(); lb2 = lb1, lb1 = i) {
                if(lb2 == '\n' && lb1 == '%' && i == '%') {
                    e_lerror(FATAL, "%%%% in code block starting on line %d\n", _start_line);
                }

                if(i < 0) {
                    ii_unterm();
                    ii_flush(1);
                    ii_term();
                    e_lerror(FATAL, "Code block starting on line %d too long.\n", _start_line);
                }

                if(i == '\n' && in_string) {
                    e_lerror(WARNING, "Newline in string ,inserting \"\n", _start_line);
                    in_string = false;
                }

                /*Take care of \{, "{", '{', \}, "}", '}' */
                if(i == '\\') {
                    if(!(i = input()) {
                        break;
                    } else {
                        continue;   /*dicard backslash and following char*/
                    }
                }

                if(i == '"' && !(in_char_const || in_comment))  {
                    in_string = !in_string;
                } else if( i == '\'' && !(in_string || in_comment)) {
                    in_char_const = !in_char_const;
                } else if(lb1 == '/' && i == '*' && !in_string) {
                    in_comment = true;
                } else if(lb1 == '*' && i == '/' && in_comment) {
                    in_comment = false;
                }

                if(!(in_string || in_char_const || in_comment)) {
                    if(i == '{') ++nestlev;
                    if(i == '}' && --nestlev <= 0) {
                        stripcr(yytext);
                        return ACTION;
                    }
                }
            }

            e_lerror(FATAL, "EOF in code block starting on line %d\n", _start_line);
        }

^"%%"   return SEPARATOR;

"%{"[\s\t]*    {
                    /*copy a code block to the output file*/
                    int c;
                    bool looking_for_brace = false;

                    if(!g_cmdopt.no_lines)
                        e_output("\n #line %d \"%s\"", yylineno, g_input_file_name);

                    while(c = input()) { /*while not at end of file*/
                        if(c == -1) {
                            ii_flushbuf();
                        } else if(c != '\r') { /*ignore '\r' */
                            if(looking_for_brace) { /*last char was a %*/
                                if(c == '}') break;
                                else e_output("%%%c", c);
                                looking_for_brace = false;
                            } else {
                                if(c == '%') looking_for_brace = 1;
                                else e_output("%c", c);
                            }
                        }
                    }

                    return CODE_BLOCK;
               }

<{c_name}>     return FIELD;          /*occs only*/
"%union"       return PERCENT_UNION;  /*occs only*/
"%token"       |
"%term"        return TERM_SPEC;
"%type"        return TYPE;           /*occs only*/
"%synch"       return SYNCH;          /*llama only*/
"%left"        return LEFT;           /*occs only*/
"%right"       return RIGHT;          /*occs only*/
"%nonassoc"    return NONASSOC;       /*occs only*/
"%prec"        return PREC;           /*occs only*/
"%start"       return START;          /*for error messages*/
":"            return COLON;
"|"            return OR;
";"            return SEMI;
"["            return START_OPT;
"]"            |
"]*"           return END_OPT;

[^\x00-\s%\{}[\]();|;,<>]+ return NAME;
\x0d                                  ;/*dicard carriage return '\r'*/
[\x00-\x0c\x0e-\s]         if(!_ignore) return WHITESPACE;

%%

PUBLIC void nows() {_ignore = true;}
PUBLIC void ws()   {_ignore = false;}

PUBLIC int start_action()
{
    return _start_line;
}

PRIVATE void stripcr(char *src)
{
    char *dest;
    for(dest = src; *src; src++){
        if(*src != '\r')
            *dest++ = *src;
        *dest = '\0';
    }
}

#ifdef MAIN
    CMDOPT_S g_cmdopt;
    g_cmdopt.no_lines = 0;
    char *g_input_file_name;
    FILE *g_output = stdout;

    #include <stdarg.h>

    void plex(int lex) {
        switch(lex) {
            case ACTION:	   printf("ACTION (%s)\n",	   yytext); break;
            case CODE_BLOCK:   printf("CODE_BLOCK (%s)\n", yytext); break;
            case COLON:	       printf("COLON (%s)\n",	   yytext); break;
            case END_OPT:      printf("END_OPT (%s)\n",	   yytext); break;
            case FIELD:	       printf("FIELD (%s)\n",	   yytext); break;
            case LEFT:	       printf("LEFT (%s)\n",	   yytext); break;
            case NAME:	       printf("NAME (%s)\n",	   yytext); break;
            case NONASSOC:	   printf("NONASSOC (%s)\n",   yytext); break;
            case OR:		   printf("OR (%s)\n",		   yytext); break;
            case OTHER:	       printf("OTHER (%s)\n",	   yytext); break;
            case PERCENT_UNION: printf("PERCENT_UNION (%s)\n",  yytext); break;
            case PREC:	       printf("PREC (%s)\n",	   yytext); break;
            case RIGHT:	       printf("RIGHT (%s)\n",	   yytext); break;
            case SEMI:	       printf("SEMI (%s)\n",	   yytext); break;
            case SEPARATOR:	   printf("SEPARATOR (%s)\n",  yytext); break;
            case START:	       printf("START (%s)\n",	   yytext); break;
            case START_OPT:	   printf("START_OPT (%s)\n",  yytext); break;
            case SYNCH:	       printf("SYNCH (%s)\n",	   yytext); break;
            case TERM_SPEC:	   printf("TERM_SPEC (%s)\n",  yytext); break;
            case TYPE:	       printf("TYPE (%s)\n",	   yytext); break;
            case WHITESPACE:   printf("WHITESPACE (%s)\n", yytext); break;
            default:		   printf("*** unknown *** (%s)\n",yytext); break;
        }
    }

    int main(int argc, char **argv)
    {
        int lex;
        if(argc == 1) {
            while(lex = yylex()) plex(lex);
        } else {
            if(ii_newfile(g_input_file_name = argv[1]) < 0) {
                perror(argv[1]);
            } else {
                while(lex = yylex()) plex(lex);
            }
        }
    }

#endif
























