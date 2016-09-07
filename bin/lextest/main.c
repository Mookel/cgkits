#include <stdio.h>

char *yytext;
int  yyleng;

int main(int argc, char **argv)
{
    int ret;

    if(argc < 2){
        fprintf(stderr, "Need file name.\n");
        exit(1);
    } else if (ii_newfile(argv[1]) < 0){
        fprintf(stderr, "Can't open %s\n", argv[1]);
        exit(1);
    }
   
    while(ret = yylex()){
        printf("ret = %c, string = %s\n", ret, yytext);
    }

    return 0;
}
