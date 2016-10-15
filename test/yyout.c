#include "yyout.h"

#define YYDEBUG
#define YYACTION
#define YYPARSER
/*@A (C) 2016 refactored by Mookel */


#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <l.h>

FILE  *yycodeout;		/* Output stream (code). */
FILE  *yybssout;		/* Output stream (bss ). */
FILE  *yydataout;		/* Output stream (data). */
int    yylookahead;     /* Lookahead token.      */

extern char *yytext;    /* Declared by lex in lexyy.c */
extern int  yylineno;
extern int  yyleng;
extern int  yylex();

void yy_code( char *fmt, ...);	/* Supplied below and */
void yy_data( char *fmt, ...);	/* in yydebug.c       */
void yy_bss( char *fmt, ...	);
void yy_error( char *fmt, ...);
void yy_comment( char *fmt, ...);
int	 yy_next_token();

extern unsigned char *ii_ptext();	/* Lookback function used by lex  */
extern int  ii_plength() ;		    /* in /compiler/lib/input.c.      */
extern int  ii_plineno() ;

#ifdef YYDEBUG			/* Define YYD here so that it can be used */
#   define YYD(x) x		/* in the user-supplied header.		  */
#else
#   define YYD(x)       /* empty */
#endif

/*----------------------------------------------------------------------*/

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

#undef YYD			        /* Redefine YYD in case YYDEBUG was defined  */
#ifdef YYDEBUG			    /* explicitly in the header rather than with */
#   define YYD(x) x		    /* a -D on the occs command line.	     */
#   define printf  yy_code	/* Make printf() calls go to output window   */
#else
#   define YYD(x)           /* empty */
#endif

#ifndef YYACCEPT
#    define YYACCEPT return(0)	/* Action taken when input is accepted.*/
#endif

#ifndef YYABORT
#    define YYABORT return(1)	/* Action taken when input is rejected.*/
#endif

#ifndef YYPRIVATE
#    define YYPRIVATE static	/* define to a null string to make public.*/
#endif

#ifndef YYMAXERR
#    define YYMAXERR 25      	/* Abort after this many errors.*/
#endif

#ifndef YYMAXDEPTH           	/* State and value stack depth.*/
#    define YYMAXDEPTH 128
#endif

#ifndef YYCASCADE           	/* Suppress error msgs. for this many cycles.*/
#    define YYCASCADE 5
#endif

#ifndef YYSTYPE		     	    /* Default value stack type.*/
#    define YYSTYPE int
#endif

#ifndef YYSHIFTACT              /* Default shift action: inherit $$*/
#    define YYSHIFTACT(tos)     ( (tos)[0] = yylval )
#endif

#ifdef YYVERBOSE
#    define YYV(x) x
#else
#    define YYV(x)
#endif

#undef  yystk_cls			    /* redefine stack macros for local */
#define yystk_cls YYPRIVATE		/* use.				               */

/* ----------------------------------------------------------------------
 * #defines used in the tables. Note that the parsing algorithm assumes that
 * the start state is State 0. Consequently, since the start state is shifted
 * only once when we start up the parser, we can use 0 to signify an accept.
 * This is handy in practice because an accept is, by definition, a reduction
 * into the start state. Consequently, a YYR(0) in the parse table represents an
 * accepting action and the table-generation code doesn't have to treat the
 * accepting action any differently than a normal reduce.
 *
 * Note that if you change YY_TTYPE to something other than short, you can no
 * longer use the -T command-line switch.
 */

#define	YY_IS_ACCEPT	0	   	    /* Accepting action (reduce by 0) */
#define	YY_IS_SHIFT(s)  ((s) > 0)  	/* s is a shift action		  */

typedef short YY_TTYPE;

#define YYF	((YY_TTYPE)( (unsigned short )~0 >>1 ))

/*----------------------------------------------------------------------
 * Various global variables used by the parser. They're here because they can
 * be referenced by the user-supplied actions, which follow these definitions.
 *
 * If -p or -a was given to OCCS, make Yy_rhslen and Yy_val (the right-hand
 * side length and the value used for $$) public, regardless of the value of
 * YYPRIVATE (yylval is always public). Note that occs generates extern
 * statements for these in yyacts.c (following the definitions section).
 */

#if !defined(YYACTION) || !defined(YYPARSER)
#    define YYP   /* nothing  */
#else
#    define YYP   YYPRIVATE
#endif

YYPRIVATE int	  yynerrs = 0;			 /* Number of errors.         */

yystk_dcl( Yy_stack, int, YYMAXDEPTH );  /* State stack.              */

YYSTYPE     yylval;			 	         /* Attribute for last token. */
YYP YYSTYPE Yy_val;				         /* Used to hold $$.          */

YYP YYSTYPE Yy_vstack[ YYMAXDEPTH ]; 	 /* Value stack. Can't use    */
YYP YYSTYPE *Yy_vsp;				     /* yystack.h macros because  */
						                 /* YYSTYPE could be a struct.*/

YYP int     Yy_rhslen;			 	     /* Number of nonterminals on */
						                 /* right-hand side of the    */
						                 /* production being reduced. */

/* Prototypes for internal functions (local statics) */
YYPRIVATE void	   yy_init_stack(void);
YYPRIVATE YY_TTYPE yy_next	(YY_TTYPE **table, YY_TTYPE cur_state, int inp);
YYPRIVATE int  	   yy_recover	(int tok, bool suppress);
YYPRIVATE void 	   yy_shift	(int new_state, int lookahead);
YYPRIVATE int	   yy_act		(int yy_production_number, YYSTYPE *yyvsp);
YYPRIVATE void	   yy_reduce	(int prod_num, int amount);


/* This function holds all the actions in the original input specification.
 * It normally return 0,but if any of your actions return a non-zero value,
 * then the parser halts immediately, returning that non-zero number to the
 * calling function.
 */
int yy_act(int yypnum, YYSTYPE *yyvsp)
{
    switch(yypnum) {

        case 5:  /* e -> ID */
		{yy_code("%s = _%s\n", Yy_val = new_name(), yytext);}
            break;

        case 4:  /* e -> NUM */
		{yy_code("%s = %s\n", Yy_val = new_name(), yytext);}
            break;

        case 3:  /* e -> LP e RP */
		{Yy_val = yyvsp[1];}
            break;

        case 2:  /* e -> e STAR e */
		{yy_code("%s *= %s\n", yyvsp[2], yyvsp[0]); free_name(yyvsp[0]);}
            break;

        case 1:  /* e -> e PLUS e */
		{yy_code("%s += %s\n", yyvsp[2], yyvsp[0]); free_name(yyvsp[0]);}
            break;

#ifdef YYDEBUG
        default: yy_comment("Production %d: no action.\n", yypnum);
            break;
#endif
   }

    return 0;
}

/*-----------------------------------------------------------------------
 * Yy_stok[] is used for debugging and error messages.It is indexed by
 * the internal value used for a token(as used for a column index in the
 * transition matrix) and evaluates to a string naming that token.
 */

char *Yy_stok[] = 
{
	/*   0 */    "_EOI_",
	/*   1 */    "ID        ",	/*   2 */    "NUM       ",
	/*   3 */    "PLUS      ",	/*   4 */    "STAR      ",
	/*   5 */    "LP        ",	/*   6 */    "RP        "
};


/*-----------------------------------------------------------------------
 * The Yy_action table is action part of the LALR(1) transition matrix. It's
 * compressed and can be accessed using the yy_next() function, as below.
 * 
 *                 Yya000[]={ 3  ,   5,3   ,  2,2  ,  1,1  };
 *   state number------+      |      | |
 *   number of pairs in list--+      | |
 *   input symbol (terminal)---------+ |
 *   action----------------------------+
 * 
 *   action = yy_next(Yy_action, cur_state, lookahead_symbol);
 * 
 *   action <  0   --Reduce by production n, n == -action.
 *   action == 0   --Accept. (ie,Reduce by production 0).
 *   action >  0   --Shift to state n, n == action.
 *   action == YYF --Error.
 */

YYPRIVATE YY_TTYPE Yya0  [] = {  3,  5, 3   ,  2, 2   ,  1, 1   };
YYPRIVATE YY_TTYPE Yya1  [] = {  4,  6, -5  ,  4, -5  ,  3, -5  ,  0, -5  };
YYPRIVATE YY_TTYPE Yya2  [] = {  4,  6, -4  ,  4, -4  ,  3, -4  ,  0, -4  };
YYPRIVATE YY_TTYPE Yya4  [] = {  3,  0, 0   ,  4, 7   ,  3, 6   };
YYPRIVATE YY_TTYPE Yya5  [] = {  3,  6, 8   ,  4, 7   ,  3, 6   };
YYPRIVATE YY_TTYPE Yya8  [] = {  4,  6, -3  ,  4, -3  ,  3, -3  ,  0, -3  };
YYPRIVATE YY_TTYPE Yya9  [] = {  4,  6, -1  ,  0, -1  ,  4, 7   ,  3, -1  };
YYPRIVATE YY_TTYPE Yya10 [] = {  4,  6, -2  ,  0, -2  ,  4, -2  ,  3, -2  };

YYPRIVATE YY_TTYPE *Yy_action[11] = {

/*   0 */ Yya0  , 
/*   1 */ Yya1  , 
/*   2 */ Yya2  , 
/*   3 */ Yya0  , 
/*   4 */ Yya4  , 
/*   5 */ Yya5  , 
/*   6 */ Yya0  , 
/*   7 */ Yya0  , 
/*   8 */ Yya8  , 
/*   9 */ Yya9  , 
/*  10 */ Yya10 
};

/*-----------------------------------------------------------------------
 * The Yy_goto table is goto part of the LALR(1) transition matrix.
 * 
 *   nonterminal = Yy_lhs[ production number by which we just reduced ]
 * 
 *                 Yyg000[]={ 3  ,   5,3   ,  2,2  ,  1,1  };
 *   uncovered state---+      |      | |
 *   number of pairs in list--+      | |
 *   nonterminal---------------------+ |
 *   goto this state-------------------|
 * 
 * It is compressed and can be accessed using the yy_next() subroutine as 
 * below like this:
 * 
 *   goto_state = yy_next(Yy_goto, cur_state, nonterminal);
 */

YYPRIVATE YY_TTYPE Yyg0  [] = {  1,  1, 4   };
YYPRIVATE YY_TTYPE Yyg3  [] = {  1,  1, 5   };
YYPRIVATE YY_TTYPE Yyg6  [] = {  1,  1, 9   };
YYPRIVATE YY_TTYPE Yyg7  [] = {  1,  1, 10  };

YYPRIVATE YY_TTYPE *Yy_goto[11] = {

/*   0 */ Yyg0  , 
/*   1 */ NULL, 
/*   2 */ NULL, 
/*   3 */ Yyg3  , 
/*   4 */ NULL, 
/*   5 */ NULL, 
/*   6 */ Yyg6  , 
/*   7 */ Yyg7  , 
/*   8 */ NULL, 
/*   9 */ NULL, 
/*  10 */ NULL
};

/*-----------------------------------------------------------------------
 * The Yy_lhs array is used for reductions. It is indexed by production number
 * and holds the associated left-hand side adjusted so that the number can be
 * used as an index into Yy_goto.
 */

YYPRIVATE int Yy_lhs[6] = 
{
	/*   0 */	0,	/*   1 */	1,	/*   2 */	1,
	/*   3 */	1,	/*   4 */	1,	/*   5 */	1,
};

/*-----------------------------------------------------------------------
 * The Yy_reduce array is indexed by production number and holds the number
 * the number of symbols on the right-hand side of the production
 */

YYPRIVATE int Yy_reduce[6] = 
{
	/*   0 */	1,	/*   1 */	3,	/*   2 */	3,
	/*   3 */	3,	/*   4 */	1,	/*   5 */	1
};
#ifdef YYDEBUG

/*-----------------------------------------------------------------------
 * Yy_slhs[] is a debugging version of Yy_lhs[].It is indexed by production number
 * and evalutates to a string representing the left-hand side of the production.
 */

YYPRIVATE char *Yy_slhs[6] = 
{
	/*   0 */	"s",
	/*   1 */	"e",
	/*   2 */	"e",
	/*   3 */	"e",
	/*   4 */	"e",
	/*   5 */	"e"
};

/*-----------------------------------------------------------------------
 * Yy_srhs[] is also used for debugging. It is indexed by production number and 
 * evaluates to a string representing the right-hand side of the production.
 */

YYPRIVATE char *Yy_srhs[6] = 
{
	/*   0 */	"e",
	/*   1 */	"e PLUS e",
	/*   2 */	"e STAR e",
	/*   3 */	"LP e RP",
	/*   4 */	"NUM",
	/*   5 */	"ID"
};
#endif
YYPRIVATE YY_TTYPE  yy_next(YY_TTYPE **table, YY_TTYPE cur_state, int inp )
{
    /* Next-state routine for the compressed tables. Given current state and
     * input symbol (inp), return next state.
     */

    YY_TTYPE   *p = table[ cur_state ] ;
    int        i;

    if( p )
        for( i = (int) *p++; --i >= 0 ; p += 2 )
            if( inp == p[0] )
                 return p[1];

    return  YYF;
}

#ifdef YYDEBUG
yystk_dcl( Yy_dstack, char*, YYMAXDEPTH ); /* Symbol stack       */

void yy_code(char *fmt, ...)
{
    va_list   	 args;
    va_start( args,  fmt );
    yy_output( 0, fmt, args );
    va_end(args);
}

void yy_data(char *fmt, ...)
{
    va_list   	 args;
    va_start( args,  fmt );
    yy_output( 1, fmt, args );
    va_end(args);

}

void yy_bss(char *fmt, ...)
{
    va_list   	 args;
    va_start( args,  fmt );
    yy_output( 2, fmt, args );
    va_end(args);
}

/* yy_comment() and yy_error() are defined in yydebug.c */

#else

#    define yy_next_token()	     yylex()  /* when YYDEBUG isn't defined.   */
#    define yy_quit_debug()
#    define yy_init_debug()
#    define yy_pstack(x,y)
#    define yy_sym()

/* Use the following routines just like printf() to create output. The only
 * differences are that yycode is sent to the stream called yycodeout, yydata
 * goes to yydataout, and yybss goes to yybssout. All of these are initialized
 * to stdout. It's up to you to close the streams after the parser terminates.
 */

void yy_code(char *fmt, ...)
{
    va_list	args;
    va_start( args,      fmt );
    vfprintf( yycodeout, fmt, args );
}

void yy_data(char *fmt, ...)
{
    va_list	args;
    va_start( args,  fmt );
    vfprintf( yydataout, fmt, args );
}

void yy_bss(char *fmt, ...)
{
    va_list args;
    va_start( args,  fmt );
    vfprintf( yybssout, fmt, args );
}

void yy_comment(char *fmt, ...)
{
    va_list	 args;
    va_start( args,   fmt );
    vfprintf( stdout, fmt, args );
}

void yy_error(char *fmt, ...)
{
    extern char *yytext;
    extern int  yylineno;
    va_list   args;

    va_start( args, fmt );
    fprintf ( stderr, "ERROR (line %d near %s): ", yylineno, yytext );
    vfprintf( stderr, fmt, args );
    fprintf ( stderr, "\n" );
}
#endif

YYPRIVATE  void yy_shift(int new_state, int lookahead ) /* shift: push new_state   */
{
    yypush( Yy_stack, new_state );
    --Yy_vsp;			 	      /* Push garbage onto value stack */
    YYSHIFTACT( Yy_vsp );         /* Then do default action 	   */

#ifdef YYDEBUG
    yy_comment( "Shift %0.16s (%d)\n", Yy_stok[ lookahead ], new_state);
    yypush_( Yy_dstack, Yy_stok[lookahead] );
    yy_pstack(0, 1);
#endif
}
/*----------------------------------------------------------------------*/
YYPRIVATE void yy_reduce(int prod_num, int amount)
{
    int	next_state;

    yypopn( Yy_stack,  amount );	/* Pop n items off the state stack */
    Yy_vsp += amount;			    /* and the value stack.		       */
    *--Yy_vsp = Yy_val;		  	    /* Push $$ onto value stack	       */

    next_state = yy_next( Yy_goto, yystk_item(Yy_stack,0), Yy_lhs[prod_num] );

#ifndef YYDEBUG
    yypush_ ( Yy_stack, next_state );
#else
    yy_break( prod_num );	    	/* activate production breakpoint */

    yypopn_ ( Yy_dstack, amount );

    YYV( yy_comment("    pop %d item%s\n", amount, amount==1 ? "" : "s"); )
    yy_pstack( 0, 0 );

    yypush_ ( Yy_stack,	next_state 	    );
    yypush_ ( Yy_dstack,  Yy_slhs[ prod_num ] );

    YYV( yy_comment("    push %0.16s (%d)", Yy_slhs[prod_num], next_state ); )

    yy_pstack ( 0, 1 );
#endif
}

/*----------------------------------------------------------------------*/
YYPRIVATE void yy_init_stack()			/* Initialize the stacks  */
{
    yystk_clear( Yy_stack );
    yypush_    ( Yy_stack,  0 );		/* State stack = 0    	 */

    Yy_vsp = Yy_vstack + (YYMAXDEPTH-1);/* Value stack = garbage */

#   ifdef YYDEBUG
    yystk_clear  ( Yy_dstack );
    yypush_	 ( Yy_dstack, "$" );
    yy_comment	 ( "Shift start state\n" );
    yy_pstack	 (0, 1);			    /* refresh stack window */
#   endif
}
/*----------------------------------------------------------------------*/
YYPRIVATE int yy_recover( int tok, bool suppress ) /*tok: token caused error. suppress:
                                                     no error message is printed if true*/
{
    int	        *old_sp  = yystk_p(Yy_stack);	   /* State-stack pointer */
    YYD( char  **old_dsp = yystk_p(Yy_dstack); )
    YYD( char   *tos;  				 )

    if( !suppress ) {
	    yy_error( "Unexpected %s\n", Yy_stok[tok] );
	    if( ++yynerrs > YYMAXERR ) {
	    yy_error("Too many errors, aborting\n");
	    return 0;
	    }
    }

    do {
        while( !yystk_empty(Yy_stack)
		    && yy_next( Yy_action, yystk_item(Yy_stack,0), tok) == YYF ) {

	        yypop_( Yy_stack );

	        YYD( tos = yypop_(Yy_dstack); )
	        YYD( yy_comment("Popping %s from state stack\n", tos); )
	        YYD( yy_pstack(0, 1);				   )
	    }

	    if( !yystk_empty(Yy_stack) ) { /* Recovered successfully */

	        /* Align the value (and debug) stack to agree with the current
	         * state-stack pointer.
	         */

	        Yy_vsp = Yy_vstack + (YYMAXDEPTH - yystk_ele(Yy_stack)) ;

#ifdef YYDEBUG
	    	yystk_p(Yy_dstack) = Yy_dstack +
					(YYMAXDEPTH - yystk_ele(Yy_stack) );
	   	    yy_comment("Error recovery successful\n");
	    	yy_pstack(0, 1);
#endif
	        return tok;
	    }

	    yystk_p( Yy_stack ) = old_sp ;

	    YYD( yystk_p( Yy_dstack ) = old_dsp ;			)
	    YYD( yy_comment("Restoring state stack."); 		)
	    YYD( yy_pstack(1, 1); 			 		)
	    YYD( yy_comment("discarding %s\n", Yy_stok[tok]);   )

    } while( ii_mark_prev(), tok = yy_next_token() );

    YYD( yy_comment("Error recovery failed\n");	)
    return 0;
}

/*----------------------------------------------------------------------*/
void yy_init_stream()
{
    yycodeout = stdout;
    yybssout  = stdout;
    yydataout = stdout;
}

int	yyparse()
{
    /* General-purpose LALR parser. Return 0 normally or -1 if the error
     * recovery fails. Any other value is supplied by the user as a return
     * statement in an action.
     */

    int	act_num ;	 /* Contents of current parse table entry	*/
    int	errcode ;	 /* Error code returned from yy_act()       */
    int tchar   ;	 /* Used to \0-terminate the lexeme         */
    int suppress_err ; 	 /* Set to YYCASCADE after an error is found    */
			             /* and decremented on each parse cycle. Error	*/
			             /* messages aren't printed if it's true.	    */

    yy_init_stream();

    #ifdef YYDEBUG
    if( !yy_init_debug( Yy_stack,  &yystk_p(Yy_stack ),
			Yy_dstack, &yystk_p(Yy_dstack),
			Yy_vstack, sizeof(YYSTYPE), YYMAXDEPTH) )
	    YYABORT;
    #endif

    yy_init_stack();			    /* Initialize parse stack	*/
    yy_init_occs  ( Yy_vsp );

    yylookahead  = yy_next_token();	/* Get first input symbol	*/
    suppress_err = 0;

    while( 1 ) {

	    act_num = yy_next( Yy_action, yystk_item(Yy_stack,0), yylookahead );

	    if( suppress_err ) --suppress_err;

	    if( act_num == YYF  ) {
	        if( !(yylookahead = yy_recover( yylookahead, suppress_err )) )
	            YYABORT;
	        suppress_err = YYCASCADE;
	    } else if( YY_IS_SHIFT(act_num) ) {		     /* Simple shift action */

	        /* Note that yytext and yyleng are undefined at this point because
	         * they were modified in the else clause, below. You must use
	         * ii_text(), etc., to put them into a reasonable condition if
	         * you expect to access them in a YY_SHIFT action.
	         */
	        yy_shift( act_num, yylookahead );

	        ii_mark_prev();
	        yylookahead = yy_next_token();

	    } else {

	        /* Do a reduction by -act_num. The activity at 1, below, gives YACC
	         * compatibility. It's just making the current lexeme available in
	         * yytext and '\0' terminating the lexeme. The '\0' is removed at 2.
	         * The problem is that you have to read the next lookahead symbol
	         * before you can reduce by the production that had the previous
	         * symbol at its far right. Note that, since Production 0 has the
	         * goal symbol on its left-hand side, a reduce by 0 is an accept
	         * action. Also note that ii_ptext()[ii_plength()] is used at (2)
	         * rather than yytext[yyleng] because the user might have modified
	         * yytext or yyleng in an action.
	         *
	         * Rather than pushing junk as the $$=$1 action on an epsilon
	         * production, the old tos item is duplicated in this situation.
	         */
	        act_num   = -act_num ;
	        Yy_rhslen = Yy_reduce[ act_num  ];
	        Yy_val    = Yy_vsp[ Yy_rhslen ? Yy_rhslen-1 : 0 ]; 	 /* $$ = $1 */

	        if( yytext = (char *) ii_ptext() ) {  /* (1) */
		        yylineno       = ii_plineno() ;
		        tchar          = yytext[ yyleng = ii_plength() ];
		        yytext[yyleng] = '\0' ;
	        } else {			                  /* no previous token */
		        yytext = "";
		        yyleng = yylineno = 0;
	        }

            #ifdef YYDEBUG
	        yy_comment("Reduce by (%d) %s->%s\n", act_num,
					Yy_slhs[act_num], Yy_srhs[act_num]);
            #endif

	        if( errcode = yy_act( act_num, Yy_vsp ) ) return errcode;

	        if( yylineno ) ii_ptext()[ ii_plength() ] = tchar;	/* (2)  */

	        if( act_num == YY_IS_ACCEPT ) {
	            break ;
	        } else {
		        yy_reduce( act_num, Yy_rhslen );
		    }
	   }
    }

    YYD(  yy_comment( "Accept\n" );	)
    YYD(  yy_quit_debug();		)

    YYACCEPT;
}

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