/*@A (C) 2016 Mookel                                               */


  @----------------------------------------------------------------------
  @	This file contains the lex state-machine driver. It's
  @	path should be put into the libary environment.
  @----------------------------------------------------------------------
  @ This part goes at the top of the file, before any tables are
  @ printed but after the user-supplied header is printed.
  @----------------------------------------------------------------------


/* YY_TTYPE is used for the DFA transition table: Yy_nxt[], declared below.
 * YYF marks failure transitions in the DFA transition table. There's no failure
 * state in the table itself, these transitions must be handled by the driver
 * program. The DFA start state is State 0. YYPRIVATE is only defined here only
 * if it hasn't be #defined earlier. I'm assuming that if NULL is undefined,
 * <stdio.h> hasn't been included.
 */

#ifndef YYPRIVATE
#       define YYPRIVATE static
#endif

#ifndef NULL
#       include <stdio.h>
#endif

#include <l.h>	 /* Prototoyptes for the ii_ functions.*/

#ifdef YYDEBUG
	int	yydebug = 0;
#	define YY_D(x) if( yydebug ){ x; }else
#else
#	define YY_D(x)
#endif

typedef unsigned char	YY_TTYPE;

#define YYF		(( YY_TTYPE )(-1))


  @----------------------------------------------------------------------
  @
  @	The tables, etc. go here.
  @
  @----------------------------------------------------------------------


/*-----------------------------------------------------------------
 * Global variables used by the parser.
 */

char    *yytext;		/* Pointer to lexeme.           */
int    	 yyleng;		/* Length of lexeme.            */
int    	 yylineno;	    /* Input line number.           */
FILE	*yyout;

/*-----------------------------------------------------------------
 * Macros that duplicate functions in UNIX lex:
 */

#define output(c)   putc(c,yyout)
#define ECHO        fprintf(yyout, "%s", yytext )

#ifndef YYERROR
#    define YYERROR printf
#endif

#define yymore()    yymoreflg = 1

#define unput(c)    (ii_unput(c), --yyleng )
#define yyless(n)   (ii_unterm(), \
		            (yyleng -= ii_pushback(n) ? n : yyleng ), \
		             ii_term() \
		            )

int  input (void)   /*This is a macro in UNIX lex*/
{
    int c;

    if( (c = ii_input()) && (c != -1)) {
	    yytext   = (char *) ii_text();
	    yylineno = ii_lineno();
	    ++yyleng;
    }

    return c;
}

/*----------------------------------------------------------------------*/

int yylex()
{
    int        yymoreflg;         /* Set when yymore() is executed       */
    static int yystate   = -1;    /* Current state.                      */
    int        yylastaccept;      /* Most recently seen accept state     */
    int        yyprev;            /* State before yylastaccept           */
    int        yynstate;          /* Next state, given lookahead.        */
    int        yylook;            /* Lookahead character	             */
    int	       yyanchor;	      /* Anchor point for most recently seen */
				                  /* accepting state.			         */

    if( yystate == -1 ) {
	    yyout = stdout;
	    yy_init_lex();		      /* One-time initializations */
        ii_advance();
        ii_pushback(1);
    }

    yystate      = 0;		      /* Top-of-loop initializations */
    yylastaccept = 0;
    yymoreflg    = 0;
    ii_unterm();
    ii_mark_start();

    while( 1 ) {

        /* Check end of file. If there's an unprocessed accepting state,
	     * yylastaccept will be nonzero. In this case, ignore EOF for now so
	     * that you can do the accepting action; otherwise, try to open another
	     * file and return if you can't.
	     */

	    while( 1 ) {
            if( (yylook=ii_look(1)) != EOF ) {
		        yynstate = yy_next( yystate, yylook );
		        break;
	        } else {
		            if( yylastaccept ){	            /* still something to do */
		                yynstate = YYF;
		                break;
		            } else if( yy_wrap() )   { 		/* another file?  */
                        yytext = "";
                        yyleng = 0;
                        return 0;
                    } else {
		                ii_advance(); 		        /* load a new buffer */
		                ii_pushback(1);
                    }
            }
        }

        if( yynstate != YYF ) {

	        YY_D( printf(" Transition from state %d", yystate )     );
	        YY_D( printf(" to state %d on <%c>\n",   yynstate, yylook) );

            if( ii_advance() < 0 ){           	    /* Buffer full */
                YYERROR( "Line %d, lexeme too long. "
		         "Discarding extra characters.\n", ii_lineno() );
                ii_flush(1);
            }

            if(yyanchor = Yyaccept[ yynstate ]) {   /* saw an accept state */
		        yyprev       = yystate  ;
		        yylastaccept = yynstate ;
		        ii_mark_end();  	                /* Mark input at current character. */
                                                    /* A subsequent ii_move_back()      */
			        	                            /* returns us to this position.     */
            }
            yystate = yynstate;
        } else {
            if( !yylastaccept ) {	   			    /* illegal input */
#ifdef YYBADINP
		YYERROR( "Ignoring bad input\n" );
#endif
		        ii_advance();                       /* Skip char that caused failure.   */
	        } else {

                ii_move_back();                     /* Back up to previous accept state */

                if( yyanchor & 2 )                  /* If end anchor is active	   */
                    ii_pushback(1);                 /* push back the CR or LF	   */

		        if( yyanchor & 1 )		            /* if start anchor is active   */
		            ii_move_start();		        /* skip the leading newline    */

		        ii_term();              	        /* Null-terminate the string   */
		        yytext   = (char *) ii_text();
		        yyleng   = ii_length ();
		        yylineno = ii_lineno ();

		        YY_D( printf("Accepting state %d, ", yylastaccept )	);
		        YY_D( printf("line %d: <%s>\n",      yylineno, yytext )	);

		        switch( yylastaccept ) {

  @		   +-----------------------------------------------------------+
  @		   | The case statements associated with the accepting strings |
  @		   | go here						       |
  @		   +-----------------------------------------------------------+
  @

                    default: YYERROR("INTERNAL ERROR, yylex: Unknown accept state %d.\n",
								yylastaccept );
                        break;
		        }
            }

            ii_unterm();
            yylastaccept = 0;

            if( !yymoreflg ) {
                yystate = 0;
                ii_mark_start();
            } else {
		        yystate   = yyprev;  	/* Back up */
		        yymoreflg = 0;
	        }
        }
    }
}
