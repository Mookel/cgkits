#line 1 "lex.y"
  
                        
                     
   

#include <stdio.h>
#include <string.h>

     extern union{   /*struct*/
         char *p_char;
         int integer;
     }yylval;


    let [_a-zA-Z]   l       
    suffix [UuLl]    s       


