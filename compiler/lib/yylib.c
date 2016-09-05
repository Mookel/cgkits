//
// Created by Mookel on 16/8/31.
// Email : ltp0709@sina.com 
// Copyright (c) 2016 jlu.edu. All rights reserved.
// yylib.c : 
//

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <debug.h>
#include <win.h>
#include <ncurses.h>
#include <com.h>
#include <yylib.h>
#include <input.h>
#include <sys/timeb.h>

/*macros definitions*/
#define _NEWLINE(win)       (_interactive ? (waddch(win, '\n'), wclrtoeol(win)) \
                                          : 0)
#define _WIN_STACK_WINSIZE  (_stack_size + 2)
#define _WIN_STACK_TOP      0
#define _WIN_DEFSTACK       11
#define _WIN_PROMPT_TOP     (WIN_SCRNSIZE - 3)
#define _WIN_PROMPT_WINSIZE 3
#define _WIN_IO_TOP         (_WIN_STACK_WINSIZE - 1)
#define _WIN_IO_WINSIZE     ((WIN_SCRNSIZE - (_WIN_STACK_WINSIZE + _WIN_PROMPT_WINSIZE)) + 2)

#define _WIN_TOKEN_WIDTH    22
#define _WIN_PRINTWIDTH     79

#define _ESC                0x1b
#define _BRKLEN             79

/*extern global variables */
extern char *yytext;
extern int  yylineno;
extern int  yyleng;

/*private static variables */
/*
 * A breakpoint is set with a 'b' command.It causes automatic-mode operation to terminate immediately
 * before applying a specific production or when a specific symbol is on the top of the stack.
 * _p_breakpoint holds the production breakpoint, _s_breakpoint holds the top of stack breakpoint;
 * _i_breakpoint is the input breakpoint._l_breakpoint holds the input line breakpoint.
 * */
PRIVATE int  _p_breakpoint = -1;
PRIVATE int  _l_breakpoint = -1;
PRIVATE char _s_breakpoint[_BRKLEN] = {'\0'};
PRIVATE char _i_breakpoint[_BRKLEN] = {'\0'};

PRIVATE int  _abort;       /*force input routine to return EOI*/
PRIVATE char *_vstack;     /*Base address of the value stack or null if called by cgllama-generated parser.*/
PRIVATE int  _vsize;       /*size of one element of value stack*/
PRIVATE char **_dstack;    /*Base address of debug(symbol) stack.*/
PRIVATE char ***_p_dsp;    /*pointer to debug-stack pointer*/
PRIVATE int  *_sstack;     /*Base address of state stack.*/
PRIVATE int  **_p_sp;       /*Pointer to state-stack pointer*/
PRIVATE int  _depth;       /*Stack depth*/

PRIVATE WINDOW   *_stack_window;
PRIVATE WINDOW   *_prompt_window;
PRIVATE WINDOW   *_code_window;
PRIVATE WINDOW   *_comment_window;
PRIVATE WINDOW   *_token_window;

PRIVATE int      _numelestk  = 0;              /*number of elements on stack.*/
PRIVATE int      _stack_size = _WIN_DEFSTACK;  /*number of active lines in the stack window.*/
PRIVATE int      _interactive = 1;             /*interactive mode*/
PRIVATE int      _single_step = 1;             /*single step through parse if true*/
PRIVATE int      _delay       = 0L;            /*amount of time to wait after printing each stack update when not
                                                 single stepping.(ms)*/
PRIVATE int      _inp_fm_file = 0;             /*1 if input file is open*/

PRIVATE FILE *   _log             = NULL;      /*pointer to the log file if open*/
PRIVATE int      _no_comment_pix  = 0;         /*1 if no comment-window outptut is printed*/
PRIVATE int      _no_stack_pix    = 0;         /*1 if no stack pictures are to be printed in the log file.*/
PRIVATE int      _horiz_stack_pix = 0;         /*1 if stack pictures are to be printed horizontally in the log file.*/
PRIVATE int      _parse_pix;                   /*if(_horiz_stack_pix) print state stack.*/
PRIVATE int      _sym_pix;                     /*if(_horiz_stack_pix) print symbol stack*/
PRIVATE int      _attr_pix;                    /*if(_horiz_stack_pix) print attib stack.*/

PRIVATE int        _char_avail = 0;
#define _kbhit()   _char_avail

/*private functions prototypes.*/
PRIVATE WINDOW *boxwin(int lines, int cols, int y_start, int x_start, char *title);
PRIVATE void   horriable_death(void);
PRIVATE int    win_putc_func(int c, WINDOW *win);
PRIVATE int    read_char_from_promtw(void);
PRIVATE void   wait_keypress_continue(void);
PRIVATE void   refresh_win(WINDOW *win);
PRIVATE void   display_file_cmd(char *name, int buf_size, int print_lines);
PRIVATE void   screen_snapshot_cmd(char *filename);
PRIVATE void   delay(void);
PRIVATE void   cmd_list(void);
PRIVATE int    breakpoint(void);
PRIVATE int    new_file_cmd(char *buf);
PRIVATE FILE   *enable_log_cmd(char *buf);
PRIVATE void   kbready();

PRIVATE void kbready() { _char_avail = 1; }

/*private functions.*/
PRIVATE WINDOW *boxwin(int lines, int cols, int y_start, int x_start, char *title)
{
    WINDOW *outer;

    outer = subwin(stdscr, lines, cols, y_start, x_start);
    box(outer, WIN_VERT, WIN_HORIZ);

    if(title && *title) {
        wmove(outer, 0, (cols - strlen(title))/2);
        wprintw(outer, "%s", title);
    }

    wrefresh(outer);
    return subwin(outer, lines - 2, cols - 2, y_start + 1, x_start + 1);
}

PRIVATE void horriable_death(void)
{
    signal(SIGINT, SIG_IGN);
    yy_quit_debug();
    exit(0);
}

PRIVATE int win_putc_func(int c, WINDOW *win)
{
    static WINDOW *last_win = NULL;
    static int     last_c   = 0;
    int    test_c = 0;

    if(_interactive && c != '\n') {
        test_c = (c < 0x7f) ? c : '|';
        if(win != last_win || !isspace(test_c) || !isspace(last_c)) {
            waddch(win, isspace(test_c) ? ' ' : c); /*just output one space to conserve space in window.*/
        }

        last_win = win;
        last_c   = test_c;
    }

    return c;
}

/**
 * Get a character from the input window and echo it explicitly.
 */
PRIVATE int read_char_from_promtw(void)
{
    int c;
    if((c = wgetch(_prompt_window) & 0x7f) != _ESC) {
        waddch(_prompt_window, c);
    }

    _char_avail = 0; /*read it and reset it waiting for next char.*/
    return c;
}

/*
 * Ask for a key to be pressed and wait for it. Note that this command
 * does a refresh.
 * */
PRIVATE void wait_keypress_continue(void)
{
    wprintw(_prompt_window, " Press any key: ");
    wrefresh(_prompt_window);
    read_char_from_promtw();   /*read any char, ignore it and keep going.*/
}

/**
 *  refresh window when needed.
 */
PRIVATE void refresh_win(WINDOW *win)
{
    if(_interactive) { wrefresh(win); }
}

/**
 *  Display an arbitrary file in the stack window, one page at a time.
 *  The stack window is not refreshed by this routine.
 */
PRIVATE void display_file_cmd(char *name, int buf_size, int print_lines)
{
    FILE *fd;
    int  i;
    int  lineno = 0;

    if(!(fd = fopen(name , "r"))) {
        _NEWLINE(_prompt_window);
        wprintw(_prompt_window, "Can't open %s", name);
        wrefresh(_prompt_window);
        wait_keypress_continue(); /*wait for next command.*/
    } else {
        for(i = _stack_size - 1;; i = (*name == ' ') ? 1 : _stack_size - 2){  /*name = ' ' when we enter space.*/
            while(--i >= 0 && fgets(name, buf_size, fd)){
                if(print_lines) {
                    wprintw(_stack_window, "%3d:", ++lineno);
                }
                wprintw(_stack_window, "%s", name);
                wrefresh(_stack_window);
            }

            if(i > 0) { break; }

            if(!yy_prompt("ESC quits. Space scrolls 1 line. Enter for screenful", name , 0)) { break;}
        }

        yy_prompt("*** End of file. Press any key to continue.***", name , 0);
        fclose(fd);
    }
}

/*
 * Print the current screen contents to the indicated file. Note that the right edge of the box
 * is not printed in order to let us have 79 lines.Otherwise,the saved screen shows up as double
 * -spaced on most printers. The screen image is appended to the end of the file.
 * */
PRIVATE void screen_snapshot_cmd(char *filename)
{
    char buf[2];
    char *mode = "a";
    int  row, col, y, x;
    FILE *file;

    if(access(filename, 0) == 0) {
        if(!yy_prompt("File exists, overwirte or append ? (o/a) :", buf, 0)){
            _NEWLINE(_prompt_window);
            yy_input("Aborting command.");
            wait_keypress_continue();
            return;
        } else {
            if(toupper(*buf) == 'o') { mode = "w";}
        }
    }

    if(file = fopen(filename, mode)) {
        yy_input("...%s %s...", *mode == 'w' ? "overwriting" : "appending to", filename);
    } else {
        yy_input("Can't open %s.", filename);
        wait_keypress_continue();
        return;
    }

    getyx(_prompt_window, y, x);
    for(row = 0; row < WIN_SCRNSIZE; ++row) {
        for(col = 0; col < _WIN_PRINTWIDTH; col++) {
            fputc(mvinch(row, col), file);
        }
        fputc('\n', file);
    }

    fclose(file);
    wmove(_prompt_window, y, x);
}

/*
 * Print a prompt and wait for either a carriage return or another command.
 * Note that the tiem returned by time() is the elapsed time in seconds from
 * 00:00:00 January 1, 1970 GMT. Since there are roughly 31557600 seconds in
 * a year and unsigned long int can hold 2147483647, the time won't roll over
 * until 2038. Dont use this program on 2038.
 * */
PRIVATE void delay(void)
{
    long start, current;
    char buf[80];
    int print_lines;
    struct timeb time_buf;

    if(!_interactive) return;

    if(!_single_step && _kbhit()) {
        read_char_from_promtw();
        _single_step = 1;
    }

    if(!_single_step) {
        ftime(&time_buf);
        start = (time_buf.time * 10000) + time_buf.millitm;
        while(1) {
            ftime(&time_buf);
            current = (time_buf.time *1000) + time_buf.millitm;

            if(current - start >= _delay) { break; }

            if(_kbhit()) {
                read_char_from_promtw();
                _single_step = 1;
                break;
            }
        }

        if(!_single_step) return ;
    }

    while(1) {
        yy_prompt("Enter command (Space to continue, ? for list) :", buf, 0);

        switch(*buf) {
            case '\0':
            case ' ' :
            case '\n': goto outside;

            case '?':  /*help list*/
                cmd_list();
                _NEWLINE(_prompt_window);
                wait_keypress_continue();
                yy_redraw_stack();
                break;

            case 'a': /*abort*/
                _abort = 1;
                _single_step = 0;
                goto outside;

            case 'b': /*breakpoints*/
                breakpoint();
                yy_redraw_stack();
                break;

            case 'd': /*set delay time*/
                if(yy_prompt("Delay time (in seconds, CR = 0, ESC cancels): ", buf, 1)) {
                    _delay = (long)(atof(buf) * 1000.0);
                }
                break;

            case 'f': /*read file*/
                if(!yy_prompt("Print line numbers ? (y/n, CR= y, ESC cancels): " , buf, 0)){
                    break;
                }

                print_lines = *buf != 'n';
                if(!yy_prompt("File name or ESC to cancel: ", buf, 1)) {
                    break;
                }

                werase(_stack_window);
                display_file_cmd(buf, sizeof(buf), print_lines);
                yy_redraw_stack();
                break;

            case 'g': /*go !*/
                _single_step = 0;
                goto outside;

            case 'i':
                if(yy_prompt("Input file name or ESC to cancel: ", buf, 1)){
                    new_file_cmd(buf);
                }
                break;

            case 'l': /*enable logging*/
                enable_log_cmd(buf);
                break;

            case 'N': /*noninteractive w/o logging*/
                _log = NULL;
                _no_stack_pix = 1;
                _interactive = 0;
                _single_step = 0;
                _delay = 0L;
                werase(_stack_window);
                goto outside;

            case 'n': /*noninteractive w/ logging */
                if(!_log && !enable_log_cmd(buf)) { break; }
                _interactive = 0;
                _single_step = 0;
                _delay = 0L;
                werase(_stack_window);
                goto outside;

            case 'q':   /*exit*/
                raise(SIGINT);
                exit(0);

            case 'r': /*redraw the stack window*/
                yy_redraw_stack();
                break;

            case 'w': /*screen shot*/
                if(yy_prompt("Output file name or ESC to cancel: ", buf, 1)){
                    screen_snapshot_cmd(buf);
                }
                break;

            case 'x': /*show lexemes*/
                yy_comment("current  [%0.*s]\n", yyleng, yytext);
                yy_comment("previous [%0.*s]\n", ii_plength(), ii_ptext());
                break;

            case 0x01: yy_hook_a(); break; /*Ctrl-A*/
            case 0x02: yy_hook_b(); break; /*Ctrl-B*/

            default:
                yy_prompt("Illegal command, press any key to continue", buf, 0);
                break;
        }
    }

    outside:
        werase(_prompt_window);
        wrefresh(_prompt_window);
}

/*Print a list of commands in the Stack window & prompt for an action*/
PRIVATE void cmd_list(void)
{
    werase(_stack_window);
    wmove(_stack_window, 0, 0);
    wprintw(_stack_window, "a (a)bort parse by reading EOI     \n");
    wprintw(_stack_window, "b modify or examime (b)reakpoint   \n");
    wprintw(_stack_window, "d set (d)elay time for go mode     \n");
    wprintw(_stack_window, "f read (f)ile                      \n");
    wprintw(_stack_window, "g (g)o (any key stops parse)       \n");
    wprintw(_stack_window, "i change (i)put file               \n");
    wmove(_stack_window, 0, 39);
    wprintw(_stack_window, "l (l)og output to file");
    wmove(_stack_window, 1, 39);
    wprintw(_stack_window, "n (n)oninteractive mode");
    wmove(_stack_window, 2, 39);
    wprintw(_stack_window, "q (q)uit (exit to os)");
    wmove(_stack_window, 3, 39);
    wprintw(_stack_window, "r (r)efresh stack window");
    wmove(_stack_window, 4, 39);
    wprintw(_stack_window, "w (w)rite screen to file or device\n");
    wmove(_stack_window, 5, 39);
    wprintw(_stack_window, "x show current and pre le(x)eme\n");
    wmove(_stack_window, 7, (78-29)/2);
    wprintw(_stack_window, "Space or Enter to single step");
    wrefresh(_stack_window);
}

/**
 * Set up a breakpoint by prompting the user for any required information.
 * Return true if we have to redraw the stack window because a help screen
 * was printed there.
 */
PRIVATE int breakpoint(void)
{
    int type;
    char **p;
    char buf[80];
    int rval = 0;
    static char *text[] ={
      "Select a breakpoint type(i,l,p or s) or command(c or l): ",
      "Type: Description:  Enter breakpoint as follows:",
      "i    input..................number for token value",
      "                         or string for lexeme or token name",
      "l    input line read........line number",
      "p    reduce by production...number for production number",
      "s    top-of-stack symbol....number for state-stack item",
      "                        or  string for symbol-stack item",
      "c = clear all breakpoints",
      "d = display (list) all breakpoints",
      NULL,
    };

    if(!yy_prompt("Enter type or command, ? for help, ESC cancel: ", buf, 0)){
        return 1;
    }

    if(*buf == '?') {
        rval = 1;
        werase(_stack_window);
        wmove(_stack_window, 0, 0);

        for (p = text; *p; p++) {
            wprintw(_stack_window, "%s\n", *p++);
        }

        wrefresh(_stack_window);
        if (!yy_prompt("Enter breakpoint type or command, ESC cancel: ", buf, 0)) {
            return rval;
        }
    }

    if ((type = *buf) == 'p') {
        if (yy_prompt("Production number or ESC cancel: ", buf, 1)) {
            if(!isdigit(*buf)) {
                yy_prompt("Must be a number, press any key to continue.", buf, 0);
            } else {
                _p_breakpoint = atoi(buf);
            }
        }
    } else if (type == 'l') {
        if (yy_prompt("Input line number or ESC cancel: ", buf, 1)) {
            if(!isdigit(*buf)) {
                yy_prompt("Must be a number, press any key to continue.", buf, 0);
            } else {
                _l_breakpoint = atoi(buf);
            }
        }
    } else if (type == 'i' || type == 's') {
        if(yy_prompt("Symbol value or ESC cancel: ", buf, 1)){
            strncpy(type == 'i' ? _i_breakpoint : _s_breakpoint, buf, _BRKLEN);
        }
    } else {

        switch (type) {
            case 'c':
                _p_breakpoint = -1;
                _l_breakpoint = -1;
                *_s_breakpoint = 0;
                *_i_breakpoint = 0;
                break;

            case 'd':
                rval = 1;
                werase(_stack_window);
                wmove(_stack_window, 0, 0);

                wprintw(_stack_window,
                        _p_breakpoint == -1 ? "Production = none\n"
                                            :"Production = %d\n", _p_breakpoint);

                wprintw(_stack_window, "Stack      = %s\n",
                        *_s_breakpoint ? _s_breakpoint : "none");

                wprintw(_stack_window, "Input      = %s\n",
                        *_i_breakpoint ? _i_breakpoint : "none");

                wprintw(_stack_window,
                        _l_breakpoint == -1 ? "Input line = none\n"
                                            :"Input line = %d\n", _l_breakpoint);
                wrefresh(_stack_window);
                _NEWLINE(_prompt_window);
                wait_keypress_continue();
                break;

            default:
                yy_prompt("Illegal command or type, Press any key.", buf, 0);
                break;
        }

    }

    return rval;
}

/*
 * Open up a new input file. Input must come from a file because the keyboard is used to get commands.
 * In theory, you can use both standard input and the keyboard, but i hate it, hehe...
 * */
PRIVATE int new_file_cmd(char *buf)
{
    _NEWLINE(_prompt_window);
    wrefresh(_prompt_window);

    if(ii_newfile(buf) != -1){
        _inp_fm_file = 1;
    } else {
        wprintw(_prompt_window, "Can't open %s.", buf);
        wait_keypress_continue();
    }

    return _inp_fm_file;
}

/*set up everything to log output to a file*/
PRIVATE FILE *enable_log_cmd(char *buf)
{
    if(!yy_prompt("Log-file name (CR for \"log\", ESC cancles): ", buf, 1)){
        return NULL;
    }

    if(!*buf) {strcpy(buf, "log");}

    if(!(_log = fopen(buf, "w"))) {
        _NEWLINE(_prompt_window);
        wprintw(_prompt_window, "Can't open %s", buf);
        wait_keypress_continue();
        return NULL;
    }

    if(!yy_prompt("Log comment-window output ? ESC cancel (y/n, CR = y): ", buf, 0)){
        return NULL;
    } else {
        _no_comment_pix =(*buf == 'n');
    }

    if(!yy_prompt("Print stack in log file ? ESC cancel (y/n, CR = y): ", buf, 0)){
        return NULL;
    }

    if(!(_no_stack_pix = (*buf == 'n'))) {

        if(!yy_prompt("Print SYMBOL stack (y/n, CR = y): ", buf, 0)){
            return NULL;
        }else {
            _sym_pix = (*buf != 'n');
        }

        if(!yy_prompt("Print PARSE stack (y/n, CR = y): ", buf, 0)){
            return NULL;
        }else {
            _parse_pix = (*buf != 'n');
        }

        if(!yy_prompt("Print VALUE stack (y/n, CR = y): ", buf, 0)){
            return NULL;
        }else {
            _attr_pix = (*buf != 'n');
        }

    }

    return _log;
}

extern int   yy_init_debug(int *sstack, int **p_sp, char **dstack, char ***p_dsp,
                           void *vstack, int v_ele_size, int depth)
{
    char buf[80];

    int flags;
    signal(SIGIO, kbready);
    flags = fcntl(fileno(stdin), F_GETFL, 0);
    fcntl(fileno(stdin), F_SETFL, flags | FASYNC);

    _sstack = sstack;
    _dstack = dstack;
    _vstack = (char *) vstack;
    _vsize  = v_ele_size;
    _p_sp   = p_sp;
    _p_dsp  = p_dsp;
    _depth  = depth;
    _abort  = 0;

    initscr();
    signal(SIGINT, horriable_death);

    noecho();
    crmode();

    _stack_window   = boxwin(_WIN_STACK_WINSIZE, 80, _WIN_STACK_TOP, 0, "[stack]");
    _comment_window = boxwin(_WIN_IO_WINSIZE,    40, _WIN_IO_TOP,    0, "[comments]");
    _code_window    = boxwin(_WIN_IO_WINSIZE,    41, _WIN_IO_TOP,   39, "[output]");

    _token_window   = boxwin(_WIN_PROMPT_WINSIZE, _WIN_TOKEN_WIDTH, _WIN_PROMPT_TOP,
                        80 - _WIN_TOKEN_WIDTH, "[lookahead]");

    _prompt_window  = boxwin(_WIN_PROMPT_WINSIZE, (80 - _WIN_TOKEN_WIDTH) + 1, _WIN_PROMPT_TOP, 0, "[prompts]");

    scrollok(_stack_window, TRUE);
    scrollok(_comment_window, TRUE);
    scrollok(_code_window, TRUE);
    scrollok(_prompt_window, TRUE);
    scrollok(_token_window, TRUE);

    _numelestk = 0;

    while(!_inp_fm_file) {
        if(!yy_prompt("Input file name or ESC to exit:", buf, 1)) {
            yy_quit_debug();
            return 0;
        }

        new_file_cmd(buf);
    }

    delay();
    return 1;
}


extern void   yy_quit_debug(void)
{
    echo();
    nocrmode();
    move(24,0);
    refresh();
    endwin();

    if(_log){ fclose(_log);}

    signal(SIGINT, SIG_DFL);


}

/**
 * Scan argv arguments for the debugging options and remove the argument
 * from argv. Arguments are:
 * -sN Set stack-winodw size to N lines.The size of other windows scale
 * accordingly. The stack window is not permitted to get so large that
 * the other windws will disapper however.
 *
 * The first argument that doesn't begin with a minus sign is taken to be
 * the input file name. That name is not removed from argv.All other
 * arguments are ignore and are not removed from argv, so you can process
 * them in your own program.This routine prints an error message and
 * terminates the program if it can't open the specified input file.
 * Command-line processing stops immediately after the file name is
 * processing. So, given the line:
 *      program -x -s15 -y foo -s1 bar
 *
 * Argv is modified to:
 *      program -x -y foo -s1 bar
 */
extern int   yy_get_args(int argc, char **argv)
{
    char **newargv;
    char **oldargv = argv;
    char *filename = NULL;
    int   ssize    = _WIN_DEFSTACK;
    newargv = ++argv;

    for(--argc; --argc >= 0; ++argv) {
        if(argv[0][0] != '-') {          /*filename*/
            if(filename) {
                *newargv++ = *argv;
            } else {
                filename = *newargv++ = *argv;
            }
        }
        else if(argv[0][1] == 's') {     /*-s*/
            ssize = atoi(&argv[0][2]);
        } else {
            *newargv++ = *argv;          /*-?*/
        }
    }

    _stack_size = (ssize < 1) ? _WIN_DEFSTACK : (ssize > (WIN_SCRNSIZE - 6) ?
                                                 WIN_SCRNSIZE - 6 : ssize);

    if(filename) {
        if(ii_newfile(filename) != -1) {
            _inp_fm_file = 1;
        } else {
            perror(filename);
            exit(1);
        }
    }

    return (int)(newargv - oldargv);
}

/**
 *  All code sent to yy_code(), yy_data(), and yy_bss() is  funneled here.
 *  where : 0-code 1-data 2-bss.
 */
extern void  yy_output(int where, char *fmt, va_list args)
{
    extern FILE *yycodeout, *yydataout, *yybssout;

     const char *wherestr[] = {
        "CODE->",
        "DATA->",
        "BSS-->",
    };

    if(_log){
        fprintf(_log, wherestr[where]);
        com_prnt((fp_print_t) fputc, _log, fmt, args);
        fputc('\n', _log);
    }

    _NEWLINE(_code_window);

    win_putc_func(wherestr[where][0], _code_window);
    win_putc_func(WIN_VERT, _code_window);

    com_prnt((fp_print_t) win_putc_func, _code_window, fmt, args);
    refresh_win(_code_window);

    if(where == 0 && yycodeout != stdout) {
        vfprintf(yycodeout, fmt, args);
    }

    if(where == 0 && yycodeout != stdout) {
        vfprintf(yydataout, fmt, args);
    }

    if(where == 0 && yycodeout != stdout) {
        vfprintf(yybssout, fmt, args);
    }
}

/*
 * Works like printf .Writes into the comment window, and outputs
 * a message to log file if loggig is enabled.
 * */
extern void  yy_comment(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if(_log && !_no_comment_pix) {
        com_prnt((fp_print_t) fputc, _log, fmt, args);
    }

    _NEWLINE(_comment_window);
    com_prnt((fp_print_t) win_putc_func, _comment_window, fmt, args);
    refresh_win(_comment_window);
}

extern void  yy_error(char *fmt, ...)
{
    int old_interactive;
    va_list args;
    va_start(args, fmt);

    old_interactive = _interactive;
    _interactive = 1;

    yy_comment("ERROR, line %d near <%s>\n", yylineno, yytext);

    if(_log){
        com_prnt((fp_print_t) fputc, _log, fmt, args);
    }

    _NEWLINE(_comment_window);
    com_prnt((fp_print_t) win_putc_func, _comment_window, fmt, args);
    refresh_win(_comment_window);

    _interactive = old_interactive;
    _single_step = 1;
    yy_pstack(0, 1);

}

extern void  yy_input(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    com_prnt((fp_print_t) win_putc_func, _prompt_window, fmt, args);
    refresh_win(_prompt_window);
}

extern int   yy_prompt(char *prompt, char *buf, int getstring)
{
    int c;
    int y,x;
    char *startbuf = buf;

    _NEWLINE(_prompt_window);
    wprintw(_prompt_window, "%s", prompt);
    wrefresh(_prompt_window);

    if(!getstring) {
        c = *buf++ = read_char_from_promtw();
    } else {
        while((c = read_char_from_promtw()) != '\n' && c != _ESC) {
            if(isspace(c) && buf == startbuf){
                continue;
            }

            if( c != '\b'){
                *buf++ = c;
            }
            else {
                getyx(_prompt_window, y, x);
                if(buf <= startbuf) {
                    wmove(_prompt_window, y, x+1);
                } else {
                    waddch(_prompt_window, ' ');
                    wmove(_prompt_window, y, x);
                    --buf;
                }
            }
            wrefresh(_prompt_window);
        }

        while(buf > startbuf && isspace(buf[-1]))
            --buf;
    }

    *buf = 0;

    return (c != _ESC);
}

/*
 * print the state debug and value stacks.
 * */
extern void  yy_pstack(int do_refresh, int print_it)
{
    int numele;
    int  *toss;   /*top of state stack*/
    char **tods;  /*top fo debug stack*/
    char *tovs;   /*top of value stack*/
    int  *state;  /*current state-stack pointer*/
    char **debug; /*current debug-stack pointer*/
    char *value;  /*current value-stack pointer*/
    int  width;   /*width of column in horiz stack*/
    static int  times_called = -1;
    char *p;
    int i ;

    state = *_p_sp;
    debug = *_p_dsp;
    numele = _depth - (int)(state - _sstack);
    value  = (_vstack + (int)(_depth - numele) *_vsize);

    if(_log && !_no_stack_pix && print_it) {
        if(!_horiz_stack_pix){
            fprintf(_log, "    +----+------------------------+\n");
            if(numele <= 0){
                fprintf(_log, "  *  |     *************      |   Stack is empty.\n");
            } else {
                toss = state;
                tods = debug;
                tovs = value;
                for(i = numele; --i >= 0; ++toss, ++tods, tovs += _vsize){
                    fprintf(_log, "%4d|%4d| %20.20s | %1.52s\n", (int)(toss-state), *toss, *tods, yy_pstk(tovs, *tods));
                }
            }
            fprintf(_log, "    +----+-----------------------+\n");
        }else {
            if(state < _sstack) {
                fprintf(_log, "*** Stack empty ***\n");
            } else {
                for(i = 0;i <= 2; ++i){
                    if(!_parse_pix && i == 0) continue;
                    if(!_sym_pix   && i == 1) continue;
                    if(!_attr_pix  && i == 2) continue;

                    switch(i) {
                        case 0: fprintf(_log, "   PARSE   ");break;
                        case 1: fprintf(_log, "   SYMBOL  ");break;
                        case 2: fprintf(_log, "   ATTRIB  ");break;
                        default: break;
                    }

                    toss = _sstack + (_depth + 1);
                    tods = _dstack + (_depth + 1);
                    tovs = _vstack + (_depth + 1)*_vsize;

                    for(; toss >= state; --toss, --tods, tovs -= _vsize){
                        p = yy_pstk(tovs, *tods);
                        width = 3;

                        if(_sym_pix)  width = max(width, (int)strlen(*tods));
                        if(_attr_pix) width = max(width, (int)strlen(p));

                        switch(i) {
                            case 0: fprintf(_log, "%-*d ", width, *toss); break;
                            case 1: fprintf(_log, "%-*s ", width, *tods); break;
                            case 2: fprintf(_log, "%-*s ", width, p);     break;
                            default: break;
                        }
                    }

                    fputc('\n', _log);
                }
            }
        }
    }

    if(!_interactive){
        if(++times_called % 25 == 0) {
            wprintw(_stack_window, "working: %d\r", times_called);
            wrefresh(_stack_window);
        }
        return;
    }

    /*break if conditions meet, why handled here ?? this function just prints stack.*/
    if(*_s_breakpoint && (state < _sstack + _depth)) {
        if(isdigit(*_s_breakpoint)){
            if(atoi(_s_breakpoint) == *state)
                _single_step = 1;
        } else if(!strcmp(_s_breakpoint, *debug)) {
            _single_step = 1;
        }
    }

    if(do_refresh) {
        yy_redraw_stack();
    } else if(numele > _numelestk) {  /*stack growing*/
        if(numele > _stack_size) {
            yy_redraw_stack();
            wmove(_stack_window, 0, 0);
        } else  {
            wmove(_stack_window, _stack_size - numele, 0);
        }

        wprintw(_stack_window, "%4d%c %20.20s %c %1.52s", *state, WIN_VERT, *debug, WIN_VERT, yy_pstk(value, *debug));
        wrefresh(_stack_window);
    } else {   /*shrinking */
        for(i = _numelestk; i > numele; --i){
            if(i > _stack_size) {
                wmove(_stack_window, _stack_size - 1, 77);
                _NEWLINE(_stack_window);
                wprintw(_stack_window, "%4d%c %20.20s %c %1.52s", (_sstack + _depth)[-i + _stack_size], WIN_VERT,
                        (_dstack + _depth)[-i + _stack_size], WIN_VERT,
                        yy_pstk((_vstack + (_depth * _vsize)) + ((-i + _stack_size)*_vsize),
                        (_dstack + _depth)[-i + _stack_size]));
            } else {
                wmove(_stack_window, _stack_size - i, 0);
                wclrtoeol(_stack_window);
            }
            wrefresh(_stack_window);
        }

    }

    delay();
    wrefresh(_stack_window);
    _numelestk = numele;
}

/*
 * Redraw the entire screen.
 * */
extern void  yy_redraw_stack(void)
{
    int i ;
    int numele;
    int *state = *_p_sp;
    char **debug = *_p_dsp;
    char *value;

    werase(_stack_window);
    scrollok(_stack_window, FALSE);

    numele = _depth - (int)(state - _sstack);
    value  = _vstack + (_depth - numele) * _vsize;

    wmove(_stack_window, numele <= _stack_size ? _stack_size - numele : 0, 0);

    for(i = min(_stack_size, numele); --i >= 0; ++state, ++debug, value += _vsize){
        wprintw(_stack_window, "%4d%c %20.20s %c %1.52s", *state, WIN_VERT, *debug, WIN_VERT, yy_pstk(value, *debug));
    }

    is_scrollok(_stack_window);
}

/*Input a token from yy_lex() and echo to both the token and comment windows.*/
extern int  yy_next_token(void)
{
    static int tok = -1;
    char *str;
    char *lexeme;
    char buf[_WIN_TOKEN_WIDTH];
    extern char *Yy_stok[]; /*Generated by cgoccs and cgllama*/
    extern int  yy_lex(void);
    extern int  yy_lex(void);

    if(tok >= 0 && (_interactive || _log)) {
        yy_comment("Advance past %s\n", Yy_stok[tok]);
    }

    lexeme = ((tok = (_abort ? 0 : yy_lex()) == 0) ? "" : yytext);

    if(_interactive || _log) {
        yy_comment("Read %s <%s>\n", str = Yy_stok[tok], lexeme);
    }

    if(_interactive) {
        com_concat(_WIN_TOKEN_WIDTH, buf, str, " ", lexeme, NULL);
        scrollok(_token_window, TRUE);
        _NEWLINE(_token_window);
        scrollok(_token_window, FALSE);
        wprintw(_token_window, "%0.*s", _WIN_TOKEN_WIDTH - 2, buf);
        wrefresh(_token_window);

        if(_l_breakpoint != -1 && _l_breakpoint <= yylineno) {
            _l_breakpoint = -1;
            _single_step = 1;
            yy_pstack(0, 1);
        } else if((*_i_breakpoint
                   && (isdigit(*_i_breakpoint) && (tok == atoi(_i_breakpoint))))
                || (!strcmp(lexeme, _i_breakpoint))
                || (!strcmp(Yy_stok[tok], _i_breakpoint))) {
            _single_step = 1;
            yy_pstack(0, 1);
        }

    }

    delay();
    return tok;
}

extern void  yy_break(int production_number)
{
    if((production_number == _p_breakpoint) || (production_number == -1)) {
        _single_step = 1;
        yy_pstack(0, 1);
    }
}

extern void yy_hook_a()
{

}

extern void yy_hook_b()
{

}

extern int   yy_wrap()
{
    return 1;
}

extern char* yy_pstk(void *val, char *sym)
{
    static char buf[32];
    sprintf(buf, "%d", *(int *)val);
    return buf;
}

extern void  yy_init_cgllama(void *tos)
{

}

extern void  yy_init_cglex()
{

}

extern void  yy_init_cgoccs(void *tos)
{

}

/*
extern int  main(int argc, char **argv)
{
    int yy_lex(void);
    if(argc == 2) {
        ii_newfile(argv[1]);
    }

    while(yy_lex())
        ;

    exit(0);
}
 */