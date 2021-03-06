/*@A (C) 2016 refactored by Mookel */

  @ This file is written to the yyact.c file when a -a is given to OCCS
  @ (instead of o\&ccs.par).
  @
  @ This section is output above any user-supplied code
  @
#ifdef YYDEBUG
#   define YYD(x) x
#else
#   define YYD(x)    /* empty */
#endif

#include <stdio.h>



  @
  @ The following section goes out after the header but before the acts.
  @

#ifndef YYSTYPE		     	/* Default value stack type 	*/
#    define YYSTYPE int
#endif

#undef YYD			        /* Redefine YYD in case YYDEBUG was defined  */
#ifdef YYDEBUG			    /* explicitly in the header rather than with */
#   define YYD(x)  x		/* a -D on the occs command line.	         */
#   define printf  yycode	/* Make printf() calls go to output window   */
#else
#   define YYD(x)           /* empty */
#endif

void	yy_code	    ( char *fmt, ...	); /* Supplied in parser and */
void	yy_data	    ( char *fmt, ...	); /* in yydebug.c           */
void	yy_bss 	    ( char *fmt, ...	);
void 	yy_error    ( char *fmt, ...  );
void 	yy_comment  ( char *fmt, ...  );

extern YYSTYPE *Yy_vsp;	     /* Value-stack pointer		                       */
extern YYSTYPE  Yy_val;      /* Must hold $$ after act is performed	           */
extern int      Yy_rhslen;   /* number of symbols on RHS of current production */

/*----------------------------------------------------------------------*/
