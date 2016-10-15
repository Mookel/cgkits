/*Created by Mookel*/

%term ID
%term NUM

%left PLUS
%left STAR
%left LP RP

%{
#include <stdio.h>
#include <ctype.h>

extern char *yytext;
extern int yyparse();

char *new_name();
void free_name();

typedef char *stype;
#define YYSTYPE stype

#define YYMAXDEPTH 64
#define YYMAXERR   10
%}

%%

s : e
  ;

e : e PLUS e {yy_code("%s += %s\n", $1, $3); free_name($3);}
  | e STAR e {yy_code("%s *= %s\n", $1, $3); free_name($3);}
  | LP e RP  {$$ = $2;}
  | NUM      {yy_code("%s = %s\n", $$ = new_name(), yytext);}
  | ID       {yy_code("%s = _%s\n", $$ = new_name(), yytext);}
  ;

%%

char *yypstk(void *val, char *symbol)
{
    return *(char **)val ? *(char **)val : "<empty>";
}

char *Names[] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7"};
char **Namep = Names;

char *new_name()
{
    if(Namep >= &Names[sizeof(Names)/sizeof(*Names)]) {
        yy_error("Expression too complex.\n");
        exit(1);
    }

    return (*Namep++);
}

void free_name(char *s)
{
    *--Namep = s;
}

void yy_init_occs(void *tos)
{
    yy_code("public word t0, t1, t2, t3;\n");
    yy_code("public word t4, t5, t6, t7;\n");
}

int main(int argc, char **argv)
{
#ifdef YYDEBUG
    yy_get_args(argc, argv);
#else
    if(argc < 2) {
        fprintf(stderr, "Need file name\n");
        exit(1);
    } else if(ii_newfile(argv[1]) < 0){
        fprintf(stderr, "Can't open %s\n", argv[1]);
        exit(2);
    }
#endif

    yyparse();
    return 0;
}