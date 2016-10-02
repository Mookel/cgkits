/*@A (C) 2016 refactored by Mookel */

/*--------------------------------------------------------------
 *  Parser for llama-generated tables
 */

#include <stdio.h>
#include <string.h> 	  /* For strcat(), strlen() prototypes only.	   */
#include <stdlib.h>	      /* Needed only for prototype for exit(). Can be  */
			              /* discarded in most non-ANSI systems.	   */
#include <stdarg.h>
#include <l.h>

extern int	yylineno;     /* Supplied by LeX. */
extern int	yyleng;
extern char	*yytext;
extern int 	yylex();

void yy_code   (char *fmt, ...);
void yy_data   (char *fmt, ...);
void yy_bss    (char *fmt,...);
void yy_error  (char *fmt, ...);
void yy_comment(char *fmt, ...);
int  yy_next_token();

@@  Stuff from code blocks in the header part of the LLama input file goes here.
@@  LLama removes all lines that begin with @@ when it copies llout.par
@@  to the output file.


typedef unsigned char   YY_TTYPE;	/* Type used for tables. 	*/

#define YYF	(YY_TTYPE)( -1 )	    /* Failure transition in table. */

/*----------------------------------------------------------------------*/
#define YY_ISTERM(x)    (YY_MINTERM    <= (x) && (x) <= YY_MAXTERM   )
#define YY_ISNONTERM(x) (YY_MINNONTERM <= (x) && (x) <= YY_MAXNONTERM)
#define YY_ISACT(x)     (YY_MINACT     <= (x)                        )

#ifndef YYACCEPT
#    define YYACCEPT return(0)	/* Action taken when input is accepted.	     */
#endif

#ifndef YYABORT
#    define YYABORT return(1)	/* Action taken when input is rejected.	     */
#endif

#ifndef YYPRIVATE
#    define YYPRIVATE static	/* Define to a null string to make public.   */
#endif

#ifndef YYMAXERR
#    define YYMAXERR 25  	    /* Abort after this many errors.          */
#endif

#ifndef YYMAXDEPTH           	/* Parse- and value-stack depth.	  */
#    define YYMAXDEPTH 128
#endif

#ifndef YYSTYPE           	    /* Used to declare fields in value stack. */
#    define YYSTYPE int
#endif

YYPRIVATE int 	   yy_act( int actnum );  /* Supplied by llama */
YYPRIVATE YY_TTYPE yy_next( int cur_state, int c );

/*----------------------------------------------------------------------
 * Parse and value stacks:
 */
#undef   yystk_cls
#define  yystk_cls YYPRIVATE
#undef   yystk_err
#define  yystk_err(o)  ((o) ? (yy_error("Stack overflow\n" ),exit(1)) \
			                : (yy_error("Stack underflow\n"),exit(1)) )

#define yytos(stk)  yystk_item(stk, 0) /* Evaluates to top-of-stack item.*/


yystk_dcl( Yy_stack, int, YYMAXDEPTH );

typedef struct			/* Typedef for value-stack elements. */
{
    YYSTYPE  left;		/* Holds value of left-hand side attribute.   */
    YYSTYPE  right;		/* Holds value of current-symbol's attribute. */

} yyvstype;

yyvstype  Yy_vstack[ YYMAXDEPTH ]; 		    /* Value stack.		*/
yyvstype *Yy_vsp = Yy_vstack + YYMAXDEPTH;	/* Value-stack pointer	*/

@@
@@ Tables go here. LLama removes all lines that begin with @@ when it copies
@@ 						   llama.m to the output file.



/*----------------------------------------------------------------------*/

FILE  *yycodeout = NULL ;	/* Output stream for code. 		         */
FILE  *yybssout  = NULL ;	/* Output stream for initialized data. 	 */
FILE  *yydataout = NULL ;	/* Output stream for uninitialized data. */
int    yynerrs   = 0;		/* Error count.	 			             */

void yy_init_stream()
{
    yycodeout = stdout;
    yybssout  = stdout;
    yydataout = stdout;
}

/*----------------------------------------------------------------------*/
#ifdef YYDEBUG                  	/* Debugging parse stack. */

yystk_dcl( Yy_dstack, char *, YYMAXDEPTH );

YYPRIVATE char *yy_sym(int sym)
{
    /* Return a pointer to a string representing the symbol, using the
     * appropriate table to map the symbol to a string.
     */
    return ( YY_ISTERM( sym ) || !sym ) ?   Yy_stok[sym]:
           ( YY_ISNONTERM( sym )      ) ?   Yy_snonterm [sym - YY_MINNONTERM]:
           /* assume it's an act        */  Yy_sact     [sym - YY_MINACT] ;
}

/* Stack-maintenance. yy_push and yy_pop push and pop items from both the
 * parse and symbol stacks simultaneously. The yycomment calls in both routines
 * print a message to the comment window saying what just happened. The
 * yy_pstack() call refreshes the stack window and also requests a new command
 * from the user. That is, in most situations, yy_pstack() won't return until
 * the user has typed another command (exceptions are go mode, and so forth).
 * yy_pstack() also checks for break points and halts the parse if a breakpoint
 * condition is met.
 */
YYPRIVATE void yy_push(int x, YYSTYPE val)
{
    yypush ( Yy_stack,  x		);
    yypush_( Yy_dstack, yy_sym(x)	);

    --Yy_vsp;					              /* The push() macro checked */
    Yy_vsp->left = Yy_vsp->right = val;		  /* for overflow already.    */

    yy_comment( "push %s\n", yy_sym( x )  );
    yy_pstack( 0, 1 );
}

YYPRIVATE int yy_pop (void)
{
    int prev_tos = yypop( Yy_stack  );
    ++Yy_vsp;

    yy_comment( "pop %s\n", yypop_( Yy_dstack ) );
    yy_pstack ( 0, 1 );

    return prev_tos;
}

YYPRIVATE void yy_say_whats_happening(int tos_item, int production)
{
    /* Print a message in the comment window describing the replace operation
     * about to be performed. The main problem is that you must assemble a
     * string that represents the right-hand side of the indicated production.
     * I do this using the appropriate Yy_pushtab element, but go through the
     * array backwards (the Yy_pushtab arrays have been reversed to make the
     * production-mode parse more efficient--you need to unreverse them here).
     * Note that you can't just decrement end, comparing it with start in
     * the for loop that does the assembling. That is, this won't work:
     * 			while( --end >= start )
     * in the 8086 compact or large models because start might be at the
     * beginning of a segment. If it wraps past the start of the segement,
     * then end will never be less than start. Consequently, you need the
     * loop counter (nterms) to control the number of iterations.
     */

    char  buf[80];	     /* Assemble string representing right-hand side here.*/
    int	  count;         /* Maximum size of string representing RHS.	      */
    int   *start, *end;  /* Start and end of Yy_pushtab array that holds RHS. */
    int   nterms;	     /* Number of items to process in the production      */

    start = Yy_pushtab[ production ];
    for( end = start; *end; ++end )     /* After loop, end is positioned */
	;				                    /* to right of last valid symbol */

    count = sizeof(buf);
    *buf  = '\0';
    for(nterms = end - start; --nterms >= 0 && count > 0 ; ){ 	/* Assemble */
    								                            /* string.  */
	    strncat( buf, yy_sym(*--end), count );
	    if( (count -= strlen( yy_sym(*end) + 1 )) < 1 )
	        break;
	    strncat( buf, " ", --count );
    }
    yy_comment( "Applying %s->%s\n", yy_sym(tos_item), buf );
}

/* Use the following routines just like printf() to create output. In debug
 * mode, all three routines print to the output window (yy_output() is in
 * yydebug.c). In production mode, equivalent routines print to the associated
 * streams (yycodeout, yybssout, or yydataout). The first argument to
 * yy_output() tells the routine which stream is being used. If the stream is
 * still set to the default stdout, then the message is written only to the
 * window. If the stream has been changed, however, the output is sent both
 * to the window and the associated stream.
 */
void yy_code(char *fmt, ...) /* Write to the code-segment stream. */
{
    va_list   	 args;
    va_start( args,  fmt );
    yy_output( 0, fmt, args );
}

void yy_data(char *fmt, ...) /* Write to the data-segment stream. */
{
    va_list   	 args;
    va_start( args,  fmt );
    yy_output( 1, fmt, args );
}

void yy_bss(char *fmt, ...)  /* Write to the bss-segment stream.  */
{
    va_list   	 args;
    va_start( args,  fmt );
    yy_output( 2, fmt, args );
}

/* Debugging versions of yy_comment() and yy_error() are pulled out of yydebug.c
 * when YYDEBUG is defined. Similarly, yy_break(), which halts the parse if a
 * break-on-production-applied breakpoint has been triggered, is found in
 * yydebug.c. It is eliminated from the production-mode output by defining it as
 * an empty macro, below. Finally, yy_next_token(), which evaluates to a yylex()
 * call in production mode, and which is defined in yydebug.c, both gets the
 * next token and prints it in the TOKEN window.
 */
#else  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define  yy_push(x,v)  	    ( yypush(Yy_stack, x),\
				              --Yy_vsp, Yy_vsp->left=Yy_vsp->right = v )

#define  yy_pop()    	    ( ++Yy_vsp, yypop(Yy_stack) )
#define  yy_next_token()	yylex()
#define  yy_quit_debug()
#define  yy_sym()
#define  yy_say_whats_happening(tos_item,prod)
#define  yy_redraw_stack()
#define  yy_pstack(refresh,print_it)
#define  yy_break(x)

void yy_code(char *fmt, ...) /* Write to the code-segment stream. */
{
    va_list args;
    va_start( args,      fmt );
    vfprintf( yycodeout, fmt, args );
}

void yy_data(char *fmt, ...) /* Write to the data-segment stream. */
{
    va_list args;
    va_start( args,  fmt );
    vfprintf( yydataout, fmt, args );
}

void yy_bss(char *fmt, ...)  /* Write to the bss-segment stream.  */
{
    va_list args;
    va_start( args,  fmt );
    vfprintf( yybssout, fmt, args );
}

void yy_comment(char *fmt, ...)
{
    va_list args;
    va_start( args,   fmt );
    vfprintf( stdout, fmt, args );
}

void yy_error(char *fmt, ...)
{
    va_list   args;
    extern char *yytext;
    extern int  yylineno;

    va_start( args, fmt );
    fprintf ( stderr, "ERROR (line %d near %s): ", yylineno, yytext );
    vfprintf( stderr, fmt, args );
    fprintf ( stderr, "\n" );
}
#endif

/*------------------------------------------------------------
 *  ERROR RECOVERY:
 */

YYPRIVATE bool yy_in_synch( int sym )
{
    /*  Return 1 if sym is in the synchronization set defined in Yy_synch. */

    int *p ;

    for( p = Yy_synch; *p  &&  *p > 0 ; p++ )
        if( *p == sym )
             return true;
    return false;
}

YYPRIVATE int yy_synch( int lookahead )
{
   /* Recover from an error by trying to synchronize the input stream and the
    * stack. Return the next lookahead token or 0 if we can't recover. Yyparse()
    * terminates if none of the synchronization symbols are on the stack. The
    * following algorithm is used:
    *
    *  (1) Pop symbols off the stack until you find one in the synchronization
    *	   set.
    *  (2) If no such symbol is found, you can't recover from the error. Return
    *      an error condition.
    *  (3) Otherwise, advance the input symbol either until you find one that
    *      matches the stack symbol uncovered in (1) or you reach end of file.
    */

    int    tok;

    if( ++yynerrs > YYMAXERR )
        return 0;

    while( !yystk_empty( Yy_stack ) &&
           !yy_in_synch( tok = yytos( Yy_stack )))  /* 1 */
        yy_pop();

    if( yystk_empty(Yy_stack) )						/* 2 */
        return 0;

    while( lookahead && lookahead != tok )		    /* 3 */
        lookahead = yy_next_token();

    return lookahead;
}

/*---------------------------------------------------------------------
 * The actual parser. Returns 0 normally, -1 if it can't synchronize after an
 * error, otherwise returns a nonzero value returned by one of the actions.
 */
int yyparse(void)
{
    int      *p;		    /* General-purpose pointer.       	      */
    YY_TTYPE prod;		    /* Production being processed.    	      */
    int      lookahead;		/* Lookahead token.              	      */
    int	     errcode = 0;	/* Error code returned by an act. 	      */
    int	     tchar;		    /* Holds terminal character in yytext.	  */
    int	     actual_lineno;	/* Actual input line number.		      */
    char     *actual_text;	/* Text of current lexeme.		          */
    int	     actual_leng;	/* Length of current lexeme.		      */
    YYSTYPE  val;		    /* Holds $$ value for replaced nonterm.	  */

    yy_init_stream();

#   ifdef YYDEBUG
	if( !yy_init_debug( Yy_stack,  &yystk_p(Yy_stack),
			    Yy_dstack, &yystk_p(Yy_dstack),
			    Yy_vstack, sizeof(yyvstype), YYMAXDEPTH) )
	    YYABORT;

	yystk_clear(Yy_dstack);
#   endif

    yystk_clear(Yy_stack);
    Yy_vsp = Yy_vstack + YYMAXDEPTH;

    yy_push( YY_START_STATE, (Yy_vsp-1)->left );  /* Push start state onto parse stack and junk   */
						                          /* onto the value stack.                        */
    yy_init_llama( Yy_vsp );			          /* User-supplied init.                          */
    lookahead = yy_next_token();

    while( !yystk_empty(Yy_stack) ) {

        if( YY_ISACT( yytos(Yy_stack) ) ) {	      /* if TOS is an action,do it and pop the action.*/

	        if( yytext = (char *) ii_ptext() ) {
		        yylineno       = ii_plineno() ;
		        tchar          = yytext[ yyleng = ii_plength() ];
		        yytext[yyleng] = '\0' ;
	        } else {                          /* no previous token */
		        yytext = "";
		        yyleng = yylineno = 0;
	        }

	        if( errcode = yy_act( yytos(Yy_stack) ))
		        return errcode;

	        yy_pop();
	        yy_redraw_stack();
	        if( yylineno ) ii_ptext()[ ii_plength() ] = tchar;

	    } else if( YY_ISTERM( yytos(Yy_stack) )){	/* Advance if it's a terminal.*/

            if( yytos(Yy_stack) != lookahead ) {	/* ERROR if it's not there.   */
                yy_error( "%s expected\n", Yy_stok[ yytos(Yy_stack) ]);
                if( !(lookahead = yy_synch(lookahead)) )
	                YYABORT;

            } else {

                /* Pop the terminal symbol at top of stack. Mark the current
	             * token as the previous one (we'll use the previous one as
	             * yytext in subsequent actions), and advance.
                 */
                yy_pop();
                ii_mark_prev();

                lookahead     = yy_next_token();
	            actual_lineno = yylineno;
	            actual_text   = yytext  ;
	            actual_leng   = yyleng  ;
            }
        } else {

	        /* Replace a nonterminal at top of stack with its right-hand side.
	         * First look up the production number in the table with the
	         * yy_next call. If prod==YYF, there was no legal transition and
	         * error-processing is activated. Otherwise the replace operation
	         * is done by popping the nonterminal, and pushing the right-hand
	         * side from the appropriate Yy_pushtab entry.
	         */

            prod = yy_next( yytos(Yy_stack)-YY_MINNONTERM, lookahead );

            if( prod == YYF ) {
                yy_error( "Unexpected %s\n", Yy_stok[ lookahead ] );
                if( !(lookahead = yy_synch(lookahead)) )
	                YYABORT;
            } else {
	            yy_say_whats_happening( yytos(Yy_stack), prod );
	            yy_break( prod );

	            val = Yy_vsp->right ;
                yy_pop();

                for( p = Yy_pushtab[ prod ] ; *p ; ++p )
                    yy_push( *p, val );
            }
        }
    }

    yylineno = actual_lineno ;	 /* Make these hold reasonable values in case */
    yytext   = actual_text   ;   /* the remainder of the input file must be   */
    yyleng   = actual_leng   ;   /* processed further.			              */

    yy_quit_debug();
    YYACCEPT;
}
