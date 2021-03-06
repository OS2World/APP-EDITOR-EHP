/* *********************** Include-Datei CURSES.H ******************* */
/* Diese Datei enth�lt Definitionen sowohl f�r die Curses-Module      */
/* als auch f�r die Module, die die Cursesfunktionen benutzen wollen. */

#define OWN_CURSES

#define TRUE  1
#define FALSE 0
#define SZ_SC (COLS*LINES*sizeof(short int)) /* Bildschirml�nge in Bytes */
#define STD_ATTR 7         /* Standardattribut (White) */
#define BUFF_SIZE 256      /* Gr��e des Zeichenpuffers f�r getch */

#include <stdlib.h>

typedef struct s_win
{
  short int _cursor_x,_cursor_y; /* Cursorposition */
  short int _start_x,_start_y;   /* Position linke obere Fensterecke */
  short int _width_x,_width_y;   /* Fenstergr��e */
  short int *_image;             /* Zeiger auf Fensterinhalt */
  char      _scrflag,            /* Flag, ob gescrollt werden darf */
	    _lvokflag,           /* Flag, ob Cursor sichtbar */
	    _dlyflag,            /* Flag, ob nodelay gesetzt ist */
	    _tabflag,            /* Flag, ob Tabs expandiert werden sollen */
	    _attribs;            /* Zeichenattribute */
} WINDOW;

#ifndef cur_main
extern WINDOW *curscr,*stdscr,*newwin();
extern char rawflag,echoflag,nlflag;
extern short int LINES,COLS;
#endif

#define move(y,x)    wmove(stdscr,y,x)
#define getyx(w,y,x) y=w->_cursor_y; x=w->_cursor_x
#define refresh()    wrefresh(stdscr)
#define addch(c)     waddch(stdscr,c)
#define insch(c)     winsch(stdscr,c)
#define inch()       winch(stdscr)
#define getch()      wgetch(stdscr)
#define getstr(s)    wgetstr(stdscr,s)
#define addstr(s)    waddstr(stdscr,s)
#define delch()      wdelch(stdscr)
#define attrset(a)   wattrset(stdscr,a)
#define attron(a)    wattron(stdscr,a)
#define attroff(a)   wattroff(stdscr,a)

#define erase()    werase(stdscr)
#define wclear(w)  werase(w)
#define clear()    wclear(stdscr)
#define clrtoeol() wclrtoeol(stdscr)
#define clrtobot() wclrtobot(stdscr)
#define deleteln() wdeleteln(stdscr)
#define insertln() winsertln(stdscr)

#define mvwaddch(w,y,x,c)  (wmove(w,y,x),waddch(w,c))
#define mvwinsch(w,y,x,c)  (wmove(w,y,x),winsch(w,c))
#define mvwinch(w,y,x)     (wmove(w,y,x),winch(w))
#define mvwgetstr(w,y,x,s) (wmove(w,y,x),wgetstr(w,s))
#define mvwaddstr(w,y,x,s) (wmove(w,y,x),waddstr(w,s))
#define mvwdelch(w,y,x)    (wmove(w,y,x),wdelch(w))
#define mvwdeltoeol(w,y,x) (wmove(w,y,x),wdeltoeol(w))
#define mvwdeleteln(w,y,x) (wmove(w,y,x),wdeleteln(w))
#define mvwinsertln(w,y,x) (wmove(w,y,x),winsertln(w))

#define mvaddch(y,x,c)  (wmove(stdscr,y,x),waddch(stdscr,c))
#define mvinsch(y,x,c)  (wmove(stdscr,y,x),winsch(stdscr,c))
#define mvinch(y,x)     (wmove(stdscr,y,x),winch(stdscr))
#define mvgetstr(y,x,s) (wmove(stdscr,y,x),wgetstr(stdscr,s))
#define mvaddstr(y,x,s) (wmove(stdscr,y,x),waddstr(stdscr,s))
#define mvdelch(y,x)    (wmove(stdscr,y,x),wdelch(stdscr))
#define mvdeltoeol(y,x) (wmove(stdscr,y,x),wdeltoeol(stdscr))
#define mvdeleteln(y,x) (wmove(stdscr,y,x),wdeleteln(stdscr))
#define mvinsertln(y,x) (wmove(stdscr,y,x),winsertln(stdscr))

#define A_NORMAL    0    /* Keine Attribute */
#define A_STANDOUT  2    /* Gr�n */
#define A_UNDERLINE 4    /* Rot */
#define A_BLINK     128  /* Binken */
#define A_DIM       8    /* Nur intense */
#define A_BOLD      2    /* Gr�n */
#define A_INVIS     136  /* Intense auf Intense */

#define OK  1
#define ERR 0

#define wstandend(w) wattrset(w,0)
#define wstandout(w) wattrset(w,A_STANDOUT)
#define standend()   wstandend(stdscr)
#define standout()   wstandout(stdscr)

#define raw()        (rawflag = TRUE)
#define noraw()      (rawflag = FALSE)
#define echo()       (echoflag = TRUE)
#define noecho()     (echoflag = FALSE)
#define nl()         (nlflag = TRUE)
#define nonl()       (nlflag = FALSE)
