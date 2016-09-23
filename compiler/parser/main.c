//
// Created by Mookel on 16/9/22.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// main.c : 
//

#include <sys/param.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <l.h>
#include "error.h"

#ifdef LLAMA
#define ALLOCATE
#include "parser.h"
#undef ALLOCATE
#else
#define ALLOCATE
#include "parser.h"
#undef ALLOCATE
#endif

#define VERBOSE(str) if(g_cmdopt.verbose) { printf("%s:\n", (str)); } else

PRIVATE char* _output_file_name = "???";  /*name of output file*/

PRIVATE void init_cmdopt()
{
    g_cmdopt.debug = 0;
    g_cmdopt.threshold = 4;
    g_cmdopt.make_actions = true;
    g_cmdopt.make_parser = true;
    g_cmdopt.make_yyoutab = false;
    g_cmdopt.no_lines = false;
    g_cmdopt.warn_exit = 0;
    g_cmdopt.no_warnings = false;
    g_cmdopt.public = false;
    g_cmdopt.symbols = 0;
    g_cmdopt.uncompressed = false;
    g_cmdopt.use_stdout = false;
    g_cmdopt.verbose = 0;
}

PRIVATE void onintr()
{
    if(g_output != stdout) {
        fclose(g_output);
        unlink(_output_file_name);
    }

    exit(EXIT_USR_ABORT);
}

PRIVATE void parse_args(int argc, char **argv)
{
    char *p;
    char name_buf[80];

    static char *usage_msg[] ={
#ifdef LLAMA
    "Usage is : llama [-switch] file",
    ""
    "\tCreate an LL(1) parser from the specification in the",
    "\tinput file. Legal comman-line switches are:",
    "",
    "-cN        use N as the pairs threshold when (C)ompressing",
    "-D         enable (D)ebug mode in yyparse.c (implies -s)",
    "-f         (F)ast, uncompressed, tables",
#else
    "Usage is : occs [-switch] file",
    "",
    "\tCreate an LALR(1) parser from the specification in the",
    "\tinput file. Legal command-line switches are:",
    "",
    "-a         ouput actions only(see -p)",
    "-D         enable (D)ebug mode in yyparse.c (implies -s)",
#endif
    "-g         make static symbos (G)lobal in yyparse.c",
    "-l         suppress #(L)ine directives",
    "-m<file>   use <file> for parser te(M)plate",
    "-p         output parser only (can be used with -T also)",
    "-s         make (s)ymbol table",
    "-S         make more-complete (S)ymbol table",
    "-t         print all (T)ables (and the parser) to standard output",
    "-T         move large tables form yyout.c to yyoutab.c",
    "-v         print (V)erbose diagnostics (including symbol table)",
    "-V         more verbos than -v. Implies -t, & yyout.doc goes to stderr",
    "-w         suppress all warning messages",
    "-W         warnings (as well as errors) generate nonzero exit status",
    NULL
    };

    for(++argv, --argc; argc && *(p = *argv) == '-'; ++argv, --argc) {
        while(*++p) {
            switch(*p) {
             OX(case 'a': g_cmdopt.make_parser = false;)
             OX(          g_template = ACT_TEMPL;)
             OX(          break;)

                case 'D': g_cmdopt.debug = true; break;
                case 'g': g_cmdopt.public = true;break;
                LL(case 'f' : g_cmdopt.uncompressed = true; break;)
                case 'l': g_cmdopt.no_lines = true; break;
                case 'm': g_template = p + 1; goto out;
             OX(case 'p': g_cmdopt.make_actions = false; break;)
                case 's': g_cmdopt.symbols = 1; break;
                case 'S': g_cmdopt.symbols = 2; break;
                case 't': g_cmdopt.use_stdout = true; break;
                case 'T': g_cmdopt.make_yyoutab = true; break;
                case 'v': g_cmdopt.verbose = 1; break;
                case 'V': g_cmdopt.verbose = 2; break;
                case 'w': g_cmdopt.no_warnings = true; break;
                case 'W': g_cmdopt.warn_exit = true; break;
             LL(case 'c': g_cmdopt.threshold = atoi(++p);)
             LL(          while(*p && isdigit(p[1])))
             LL(                 ++p;               )
             LL(          break;                    )
                default: fprintf(stderr, "<-%c>: illegal argument\n", *p);
                         sys_printv(stderr, usage_msg);
                         exit(EXIT_ILLEGAL_ARG);

            }
        }
        out:;
    }

    if(g_cmdopt.verbose > 1) g_cmdopt.use_stdout = true;

    if(argc <= 0) { /*use standard input*/
        g_cmdopt.no_lines = 1;
    } else if(argc > 1 ){
        fprintf(stderr, "Too many arguments.\n");
        sys_printv(stderr, usage_msg);
        exit(EXIT_TOO_MANY);
    } else {
        if(ii_newfile(g_input_file_name = *argv) < 0) {
            sprintf(name_buf, "%s.%s", *argv, DEF_EXT);
            if(ii_newfile(g_input_file_name = name_buf) < 0)
                e_error(FATAL, "Can't open input file %s or %s: %s\n",
                    *argv, name_buf, e_open_errmsg());
        }
    }
}

PRIVATE void symbols()
{

}

PRIVATE void statistics(FILE *fp)
{

}

PRIVATE void tail()
{

}

PRIVATE int do_file(void)
{
    /*process the input file. return the number of errors.*/
    struct timeb start_time, end_time;
    long time;
    void nows(void); /*declared in parser.lex*/

    ftime(&start_time);
    end_time = start_time;

    //init_acts();
    //file_header();

    VERBOSE("parsing");

    //nows();
    //yyparse();

    if((e_nerrors() /*|| problems()*/)){
        VERBOSE("Analyzing grammar");
        //fisrt();
        //LL(follow());
        //LL(select());

        //code_header();
        //OX(patch();)
        ftime(&start_time);
        if(g_cmdopt.make_parser) {
            VERBOSE("making tables");
            //tables();
        }

        ftime(&end_time);
        VERBOSE("copying driver");

        //driver();

        if(g_cmdopt.make_actions) tail();
    }

    if(g_cmdopt.verbose) {
        time = (end_time.time * 1000) + end_time.millitm;
        time -= (start_time.time * 1000) + start_time.millitm;
        fprintf(stdout, "time required to make tables: %ld.%03ld seconds\n",
                (time/1000), (time%1000));
    }

    return e_nerrors();
}

#define MAIN_TEST
#ifdef MAIN_TEST
int yylineno = 0;
#endif

int main(int argc, char **argv)
{
    init_cmdopt();

    signal(SIGINT, onintr);

    /*1. parsing arguments.*/
    parse_args(argc, argv);
    if(g_cmdopt.debug && !g_cmdopt.symbols) g_cmdopt.symbols = 1;

    OX(if(g_cmdopt.make_parser))
    OX(    e_init();)

    /*2. setting output file name and output stream*/
    if(g_cmdopt.use_stdout) {
        _output_file_name = "/dev/tty";
        g_output = stdout;
    } else {
        OX(_output_file_name = !g_cmdopt.make_parser ? ACT_FILE : PARSE_FILE; )
        LL(_output_file_name = PARSE_FILE;)

        if((g_output = fopen(_output_file_name, "w")) == NULL) {
            e_error(FATAL, "Cant't open output file %s : %s\n", _output_file_name, e_open_errmsg());
        }
    }

    /*3. do the real work*/
    if(do_file() == 0){
        if(g_cmdopt.symbols)
            symbols();      /*print the symbol table*/
        statistics(stdout);

        OX(if(g_cmdopt.verbose && e_get_doc()))
        OX(    statistics(e_get_doc());)

    } else {                /* there are some errors */
        if(g_output != stdout) {
            fclose(g_output);
            if(unlink(_output_file_name) == -1) perror(_output_file_name);
        }
    }

    /*4. exit*/
    exit(e_nerrors() + (g_cmdopt.warn_exit ? e_nwarnings() : 0));
}
