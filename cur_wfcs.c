/******************************************************************************
*
* Modul      : cur_wfcs.c
*
* Funktionen : tab_to_spc (Tab in Anzahl Blanks wandeln)
*              cals_pos (Position in der Eingabezeile berechnen)
*              check_special (Testen, ob Zeichen Sonderzeichen ist)
*              do_special (Sonderzeichen anzeigen)
*              waddch (Zeichen im Fenster anzeigen)
*              winsch (Zeichen im Fenster einfÅgen)
*              wdelch (Zeichen aus Fenster lîschen)
*              werase (Fensterinhalt lîschen)
*              wmove (Cursor im Fenster setzen)
*              wclrtoeol (Rest der Zeile lîschen)
*              wclrtobot (Rest des Fensters lîschen)
*              wdeleteln (Zeile im Fenster lîschen)
*              winsertln (Zeile in Fenster einfÅgen)
*              nodelay (Delay-Flag setzen/lîschen)
*              scrollok (Scroll-Modus setzen)
*              leaveok (Cursor unsichtbar machen)
*              wattrset (Attribut fÅr anzuzeigende Zeichen setzen)
*              wattron (Attribut fÅr anzuzeigende Zeichen einschalten)
*              wattroff (Attribut fÅr anzuzeigende Zeichen ausschalten)
*              waddstr (String im Fenster anzeigen)
*              wgetstr (String einlesen)
*              wprintw_body (Rumpf fÅr Text formatiert im Fenster ausgeben)
*              wscanw_body (Rumpf fÅr Daten aus Fenster einlesen)
*              wprintw (Text formatiert im Fenster ausgeben)
*              wscanw (Daten aus Fenster einlesen)
*              mvwprintw (Text formatiert an Position X/Y im Fenster ausgeben)
*              mvwscanw (Daten an bestimmter Stelle im Fenster einlesen)
*              box (Rahmen in Fenster zeichnen)
*              winch (Zeichen aus Fenster lesen)
*
******************************************************************************/

#ifdef OS2
#define INCL_VIO
#include <os2.h>
#endif

#include "curses.h"
#include <stdarg.h>

void waddch (WINDOW*, short int);
void wdelch (WINDOW*);

extern char buffer[],*p_in,*p_out;
extern short int *cur_to_poi();
extern char rawflag,nlflag;

/******************************************************************************
*
* Funktion     : Tab in Anzahl Blanks wandeln (tab_to_spc)
* --------------
*
* Parameter    : pos         :
*                  Typ          : int
*                  Wertebereich : 0-BUFF_SIZE
*                  Bedeutung    : Position des Cursors relativ zum Beginn
*                                 der Eingabe
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : 1-8
*                  Bedeutung    : Anzahl der Blanks, die eingefÅgt werden
*                                 mÅssen, um zur nÑchsten Tab-Grenze zu kommen
*
* Beschreibung : Es wird die Differenz zwischen der aktuellen relativen Po-
*                sition zum Beginn der Eingabe und der nÑchsten Tab-Grenze
*                berechnet und zurÅckgegeben.
*
******************************************************************************/

int tab_to_spc(pos)
int pos;
{
  return(8-(pos % 8)); /* Anzahl der Blanks, die zum AuffÅllen bis */
}                  /* zur nÑchsten Tab-Grenze erforderlich sind, zurÅckgeben */

/******************************************************************************
*
* Funktion     : Position in der Eingabezeile berechnen (calc_pos)
* --------------
*
* Parameter    : start       :
*                  Typ          : int
*                  Wertebereich : 0-COLS
*                  Bedeutung    : Position des Eingabestarts
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : 0 - BUFF_SIZE-1
*                  Bedeutung    : Position des Cursors relativ zum Beginn der
*                                 Eingabe
*
* Beschreibung : Der Eingabepuffer wird von Anfang an Zeichen fÅr Zeichen
*                durchgegangen. Dabei erhîht sich fÅr jedes normale Zeichen
*                der RÅckgabewert um 1. Handelt es sich jedoch um einen Tab,
*                so wird mit der Funktion tab_to_spc berechnet, wieviele Blanks
*                eingefÅgt werden mu·ten und das Ergebnis um diese Anzahl er-
*                hîht.
*
******************************************************************************/

int calc_pos(start)
int start;
{
  int i = start;
  char *hilf = buffer; /* Hilfszeiger in Puffer */

  while(hilf != p_in)
  {
    if(*hilf != '\t')
      i++;
    else
      i += tab_to_spc(i);
    hilf++;
  }
  return(i-start);
}

/******************************************************************************
*
* Funktion     : Testen, ob Zeichen Sonderzeichen ist (check_special)
* --------------
*
* Parameter    : c           :
*                  Typ          : char
*                  Wertebereich : '\0'-'\255'
*                  Bedeutung    : Zu testendes Zeichen
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE=Sonderzeichen, FALSE=kein Sonderzeichen
*
* Beschreibung : Das Zeichen wird auf Newline, Backspace und Tab getestet. Ist
*                es in dieser Gruppe, so wird TRUE, sonst FALSE zurÅckgegeben.
*
******************************************************************************/

int check_special(c)
char c;
{
  switch(c)
  {
    case '\n':
    case '\b':
    case '\t':
      return(TRUE);
    default:
      return(FALSE);
  }
}

/******************************************************************************
*
* Funktion     : Sonderzeichen anzeigen (do_special)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem das Zeichen angezeigt werden
*                                 soll
*
*              : c           :
*                  Typ          : char
*                  Wertebereich : '\0'-'\255'
*                  Bedeutung    : Anzuzeigendes Zeichen
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE : Es handelte sich um ein Sonderzeichen
*                                 FALSE: Es war ein normales Zeichen
*
* Beschreibung : Es wird getestet, ob das Zeichen ein Newline, ein Backspace
*                oder ein Tab ist. Ist dem so, wird die entsprechende Sonder-
*                behandlung vorgenommen und der Wert TRUE zurÅckgegeben. An-
*                sonsten wird FALSE zurÅckgegeben.
*
******************************************************************************/

int do_special(w,c)
WINDOW *w;
char c;
{
  int i;

  switch(c)
  {
    case '\r':          /* *** Carriage return *** */
      w->_cursor_x = 0;
      break;
    case '\n':          /* *** newline  *** */
      if(nlflag)  /* Nur X auf 0, falls \n in CR-LF umgewandelt werden soll */
	w->_cursor_x = 0;
      if(w->_cursor_y < w->_width_y-1)
	w->_cursor_y++;
      else
	if(w->_scrflag)
	  scroll(w);
      return(TRUE);
    case '\b':          /* *** backspace *** */
      if(w->_cursor_x)   /* Steht Cursor auch nicht in der ersten Spalte ? */
      {
	w->_cursor_x--;
	wdelch(w);
      }
      else              /* Wenn Cursor doch in erster Spalte, dann, falls es */
      {                 /* eine darÅberliegende Zeile gibt, deren letztes    */
	if(w->_cursor_y) /* Zeichen lîschen */
	{
	  w->_cursor_x = w->_width_x-1;
	  w->_cursor_y--;
	  wdelch(w);
	}
      }
      return(TRUE);
    case '\t':          /* *** tab *** */
      if(w->_tabflag)
      {
	for(i=tab_to_spc(w->_cursor_x) ; i ; i--)
	  waddch(w,' ');
	return(TRUE);
      }
      else
	return(FALSE);
    default:
      return(FALSE);
  }
}

/******************************************************************************
*
* Funktion     : Zeichen im Fenster anzeigen (waddch)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem das Zeichen angezeigt werden
*                                 soll
*
*              : c           :
*                  Typ          : short int
*                  Wertebereich : ('\0'-'\255') [ +256*<Attribut> ]
*                  Bedeutung    : Anzuzeigendes Zeichen, evtl. mit Attribut
*
* Beschreibung : In der Funktion do_special wird getestet, ob es sich bei
*                dem anzuzeigenden Zeichen um ein Sonderzeichen handelt. Falls
*                ja, wird dort die Behandlung vorgenommen. Ansonsten wird das
*                Zeichen ins Window-Image geschrieben und die Cursorposition
*                angepasst. Nîtigenfalls wird der Fensterinhalt gescrollt.
*
******************************************************************************/

void waddch(WINDOW *w, short int c)
{
  if(!do_special(w,c & 255)) /* Testen, ob Zeichen ein Sonderzeichen war */
  {
    if(c>>8)  /* Attribute wurden mit dem Zeichen mitgegeben ? */
    {
#ifdef CUR_DIRECT
      VioWrtNCell (&c, 1, w->_start_y+w->_cursor_y,
		   w->_start_x+w->_cursor_x, 0);
#endif
      *cur_to_poi(w) = c;
    }
    else
    {
      if(w->_attribs) /* Irgendwelche Attribute gesetzt ? */
	c = c | (w->_attribs*256);
      else
	c = c | (STD_ATTR*256);
#ifdef CUR_DIRECT
      VioWrtNCell (&c, 1, w->_start_y+w->_cursor_y,
		     w->_start_x+w->_cursor_x, 0);
#endif
      *cur_to_poi(w) = c;
    }
    cur_right(w,1);
  }
}

/******************************************************************************
*
* Funktion     : Zeichen im Fenster einfÅgen (winsch)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem das Zeichen eingefÅgt werden
*                                 soll
*
*              : c           :
*                  Typ          : short int
*                  Wertebereich : '\0'-'\255'
*                  Bedeutung    : Anzuzeigendes Zeichen
*
* Beschreibung : In der aktuellen Zeile im Window-Image wird fÅr das ein-
*                zufÅgende Zeichen Platz geschaffen, falls es sich nicht um
*                Newline oder Backspace handelt. Bei einem Tab wird die
*                korrekte Anzahl bis zur nÑchsten Tab-Grenze eingefÅgt.
*                Anschliessend wird die Funktion waddch aufgerufen, die
*                das Zeichen einfÅgt, bzw. das Sonderzeichen ausfÅhrt.
*
******************************************************************************/

void winsch(w,c)
WINDOW *w;
short int c;
{
  short int *s,*d,anz_copy, anz_copy_si, anz_ins=1;
#ifdef CUR_DIRECT
  short int *buffer;
#endif

  if(!check_special(c & 255)) /* Nur Platz schaffen, falls kein Sonderzeichen */
  {
    if((c & 255) == '\t') /* Bei Tab mÅssen mehrere Zeichen eingefÅgt werden */
      anz_ins = 8*(w->_cursor_x/8 + 1);
    /* Anzahl zu kopierender Zeichen bestimmen */
    anz_copy = w->_width_x - w->_cursor_x - anz_ins;
    if(anz_copy_si = sizeof(short int) * anz_copy)
    {
#ifdef CUR_DIRECT
      buffer = (short int*) malloc (anz_copy_si);
      VioReadCellStr (buffer, &anz_copy_si, w->_start_y+w->_cursor_y,
		      w->_start_x+w->_cursor_x, 0);
      VioWrtCellStr (buffer, anz_copy_si, w->_start_y+w->_cursor_y,
		     w->_start_x+w->_cursor_x+anz_ins, 0);
      free (buffer);
#endif
      s = cur_to_poi(w)+anz_copy-1; /* Zeiger auf aktuelles Zeichen */
      d = s+anz_ins;
      while(anz_copy--)
	*d-- = *s--;
    }
  }
  waddch(w,c);
}

/******************************************************************************
*
* Funktion     : Zeichen aus Fenster lîschen (wdelch)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, aus dem Zeichen gelîscht werden soll
*
* Beschreibung : Alle Zeichen, die in der Cursorzeile rechts des Cursors
*                stehen, werden um 1 nach links bewegt. Anschliessend wird
*                das letzte Zeichen der Zeile gelîscht.
*
******************************************************************************/

void wdelch(w)
WINDOW *w;
{
  short int *d,old_x = w->_cursor_x;
  char      c=256*STD_ATTR; /* zum Lîschen rechts am Rand */
#ifdef CUR_DIRECT
  short int *buffer;        /* Puffer fÅr zu verschiebende Zeichen */
  int       anz_copy;       /* Anzahl zu verschiebender Zeichen */
#endif

  d = cur_to_poi(w); /* Zeichen, auf dem der Cursor steht, merken */
  if (w->_cursor_x < w->_width_x-1) /* Steht Cursor nicht am rechten Rand ? */
  {
#ifdef CUR_DIRECT
    anz_copy = (w->_width_x-1-w->_cursor_x)*sizeof (short int);
    buffer = (short int*) malloc (anz_copy*sizeof(short int));
    VioReadCellStr (buffer, &anz_copy, w->_start_y+w->_cursor_y,
		    w->_start_x+w->_cursor_x+1, 0);
    VioWrtCellStr (buffer, anz_copy, w->_start_y+w->_cursor_y,
		   w->_start_x+w->_cursor_x, 0);
    free (buffer);
#endif
    memcpy(d,d+1,(w->_width_x - w->_cursor_x - 1)*sizeof(short int));
  }
  w->_cursor_x = w->_width_x-1;
#ifdef CUR_DIRECT
  VioWrtNCell (&c, 1, w->_start_y+w->_cursor_y,
	       w->_start_x+w->_cursor_x, 0);
#endif
  *cur_to_poi(w) = c; /* Letztes Zeichen in der Zeile lîschen */
  w->_cursor_x = old_x;
}

/******************************************************************************
*
* Funktion     : Fensterinhalt lîschen (werase)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Inhalt gelîscht werden soll
*
* Beschreibung : Der Inhalt des Window-Images des angegebenen Fensters wird
*                mit dem Standardattribut Åberschrieben, also gelîscht.
*
******************************************************************************/

void werase(w)
WINDOW *w;
{
  short int *d,i;
#ifdef CUR_DIRECT
  short int c = STD_ATTR*256;
  int   line;

  for (line=w->_start_y; line < w->_start_y+w->_width_y; line++)
    VioWrtNCell (&c, w->_width_x, line, w->_start_x, 0);
#endif
  for (d = w->_image , i=w->_width_y*w->_width_x ; i ; i-- , d++)
    *d = STD_ATTR*256;   /* Bildschirm lîschen */
}

/******************************************************************************
*
* Funktion     : Cursor im Fenster setzen (wmove)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Cursor gesetzt werden soll
*
*              : y           :
*                  Typ          : short int
*                  Wertebereich : 0 - w->_width_y-1
*                  Bedeutung    : Neue Y-Position des Cursors
*
*              : x           :
*                  Typ          : short int
*                  Wertebereich : 0 - w->_width_x-1
*                  Bedeutung    : Neue X-Position der Cursors
*
* Beschreibung : Falls die Angegebenen Koordinaten innerhalb des Fenster
*                liegen, werden wie Werte cursor_x und cursor_y angepasst.
*
******************************************************************************/

void wmove(w,y,x)
WINDOW *w;
short int y,x;
{
  if(y>=0 && y<w->_width_y && x>=0 && x<w->_width_x)
  {
    w->_cursor_x = x;
    w->_cursor_y = y;
  }
}

/******************************************************************************
*
* Funktion     : Rest der Zeile lîschen (wclrtoeol)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem Rest der Cursorzeile
*                                 gelîscht werden soll.
*
* Beschreibung : Ab der Cursorposition bis zum Zeilenende wird alles mit
*                dem Standardattribut besetzt, also gelîscht.
*
******************************************************************************/

void wclrtoeol(w)
WINDOW *w;
{
  short int *p,i;
#ifdef CUR_DIRECT
  short int c = 256*STD_ATTR;

  VioWrtNCell (&c, w->_width_x-w->_cursor_x, w->_start_y+w->_cursor_y,
	       w->_start_x+w->_cursor_x, 0);
#endif
  for(p=cur_to_poi(w) , i=w->_width_x-w->_cursor_x ; i ; p++ , i--)
    *p = STD_ATTR*256;
}

/******************************************************************************
*
* Funktion     : Rest der Fensters lîschen (wclrtobot)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem Rest ab Cursorposition
*                                 gelîscht werden soll.
*
* Beschreibung : Ab der Cursorposition bis zum Zeilenende und in allen Zeilen
*                unter der aktuellen Cursorzeile wird alles mit
*                dem Standardattribut besetzt, also gelîscht.
*
******************************************************************************/

void wclrtobot(w)
WINDOW *w;
{
  short int *p, *end=w->_image+w->_width_x*w->_width_y;
#ifdef CUR_DIRECT
  short int c = STD_ATTR*256;
  int   line;

  wclrtoeol(w);
  for (line=w->_start_y+w->_cursor_y+1; line < w->_start_y+w->_width_y; line++)
    VioWrtNCell (&c, w->_width_x, line, w->_start_x, 0);
#endif
  for(p=cur_to_poi(w) ; p < end ; p++)
    *p = STD_ATTR*256;
}

/******************************************************************************
*
* Funktion     : Zeile im Fenster lîschen (wdeleteln)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem Zeile gelîscht werden soll
*
* Beschreibung : Alle Zeilen hinter der, in der der Cursor steht, werden um
*                eine Zeile nach oben kopiert. Die letzte Zeile des Fensters
*                wird mit dem Standard-Attribut besetzt, also gelîscht.
*
******************************************************************************/

void wdeleteln(w)
WINDOW *w;
{
  short int i,*d = w->_image + w->_width_x*w->_cursor_y;
#ifdef CUR_DIRECT
  short int c = 256*STD_ATTR;
#endif

  if(w->_cursor_y < w->_width_y-1) /* Cursor vor letzter Zeile ? */
  {
#ifdef CUR_DIRECT
    VioScrollUp (w->_start_y+w->_cursor_y, w->_start_x,
		 w->_start_y+w->_width_y-1, w->_start_x+w->_width_x-1,
		 1, &c, 0);
#endif
    memcpy(d,d+w->_width_x,w->_width_x*(w->_width_y-w->_cursor_y-1)*sizeof(short int));
  }
  for(d=w->_image+w->_width_x*(w->_width_y-1) , i=w->_width_x ; i ; i-- , d++)
    *d = STD_ATTR*256; /* letzte Zeile lîschen */
}

/******************************************************************************
*
* Funktion     : Zeile in Fenster einfÅgen (winsertln)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem eine Zeile eingefÅgt wird
*
* Beschreibung : Alle Zeilen inklusive der aktuellen werden um eine Zeile nach
*                unten kopiert. Anschlie·end wird die aktuelle Zeile gelîscht.
*
******************************************************************************/

void winsertln(w)
WINDOW *w;
{
  short int i,*d,*s,n;
#ifdef CUR_DIRECT
  short int c = 256*STD_ATTR;
#endif

  if(w->_cursor_y < w->_width_y-1) /* Cursor nicht vor letzter Zeile ? */
  {
#ifdef CUR_DIRECT
    VioScrollDn (w->_start_y+w->_cursor_y, w->_start_x,
		 w->_start_y+w->_width_y-1, w->_start_x+w->_width_x-1,
		 1, &c, 0);
#endif
    n = w->_width_x*(w->_width_y-w->_cursor_y-1); /* Anzahl short ints */
    d = w->_image + w->_width_x * (w->_cursor_y+1) + n; /* Destination */
    s = w->_image + w->_width_x * w->_cursor_y + n; /* Source */
    while(n--)
      *--d = *--s; /* Zeilen kopieren */
  }
  for(d=w->_image+w->_width_x*w->_cursor_y , i=w->_width_x ; i ; i-- , d++)
    *d = STD_ATTR*256; /* aktuelle Zeile lîschen */
}

/******************************************************************************
*
* Funktion     : Delay-Flag setzen (nodelay)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, fÅr das das delay-Flag manipuliert
*                                 werden soll
*
*              : d           :
*                  Typ          : char
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : GewÅnschter Zustand des delay-Flags
*
* Beschreibung : dlyflag wird auf d gesetzt.
*
******************************************************************************/

void nodelay(w,d)
WINDOW *w;
char d;
{
  w->_dlyflag = !d;
}

/******************************************************************************
*
* Funktion     : Scroll-Modus setzen (scrollok)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, fÅr das der scroll-mode manipuliert
*                                 werden soll
*
*              : s           :
*                  Typ          : char
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : GewÅnschter Zustand des scroll-Flags
*
* Beschreibung : Das scroll-Flag wird auf s gesetzt.
*
******************************************************************************/

void scrollok(w,s)
WINDOW *w;
char s;
{
  w->_scrflag = s;
}

/******************************************************************************
*
* Funktion     : Leavok-Modus setzen (leaveok)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, fÅr das der leaveok-mode manipuliert
*                                 werden soll
*
*              : f           :
*                  Typ          : char
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : GewÅnschter Zustand des leaveok-Flags
*
* Beschreibung : Das leaveok-Flag wird auf s gesetzt.
*
******************************************************************************/

void leaveok(w,f)
WINDOW *w;
char f;
{
  w->_lvokflag = f;
}

/******************************************************************************
*
* Funktion     : Attribut fÅr anzuzeigende Zeichen setzen (wattrset)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, fÅr das Attribute manipuliert
*                                 werden sollen
*
*              : a           :
*                  Typ          : short int
*                  Wertebereich : 256 * (0 - 255)
*                  Bedeutung    : Neues Attribut
*
* Beschreibung : Die Attribute fuer Fenster w werden auf a gesetzt.
*
******************************************************************************/

void wattrset(w,a)
WINDOW *w;
short int a;
{
  w->_attribs = a;
}

/******************************************************************************
*
* Funktion     : Attribut fÅr anzuzeigende Zeichen einschalten (wattron)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, fÅr das Attribute manipuliert
*                                 werden sollen
*
*              : a           :
*                  Typ          : short int
*                  Wertebereich : 256 * (0 - 255)
*                  Bedeutung    : einzuschaltendes Attribut
*
* Beschreibung : Zum attribs des Fensters w wird a dazugeodert.
*
******************************************************************************/

void wattron(w,a)
WINDOW *w;
short int a;
{
  w->_attribs |= a;
}

/******************************************************************************
*
* Funktion     : Attribut fÅr anzuzeigende Zeichen ausschalten (wattroff)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, fÅr das Attribute manipuliert
*                                 werden sollen
*
*              : a           :
*                  Typ          : short int
*                  Wertebereich : 256 * (0 - 255)
*                  Bedeutung    : auszuschaltendes Attribut
*
* Beschreibung : Vom attribs des Fensters w wird a weggeandet.
*
******************************************************************************/

void wattroff(w,a)
WINDOW *w;
short int a;
{
  w->_attribs &= ~a;
}

/******************************************************************************
*
* Funktion     : String im Fenster anzeigen (waddstr)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem der String angeiezgt werden
*                                 soll
*
*              : s           :
*                  Typ          : char *
*                  Wertebereich : Zeiger auf ASCII-Zeichenkette
*                  Bedeutung    : Anzuzeigender String
*
* Beschreibung : Jedes Zeichen des String wird der Funktion waddch Åbergeben
*
******************************************************************************/

void waddstr(w,s)
WINDOW *w;
char *s;
{
  if(s)
    while(*s)
      waddch(w,(unsigned short int) *s++);
}

/******************************************************************************
*
* Funktion     : String einlesen (wgetstr)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem eingelesene Zeichen evtl.
*                                 angezeigt werden sollen
*
*              : s           :
*                  Typ          : char *
*                  Wertebereich : Pointer auf reservierten Speicherbereich
*                  Bedeutung    : Platz, wohin der String gelesen werden soll
*
* Ergebnis     :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Eingelesener String, identisch mit s
*
* Beschreibung : Es wird wgetch aufgerufen und das eingelesene Zeichen an den
*                bisher eingelesenen String angehÑngt. Ist das Zeichen '\n',
*                so bricht die Schleife ab, das '\n' wird durch '\0' ersetzt.
*                Der Pointer auf den Anfang des eingelesenen Strings wird
*                zurÅckgegeben.
*
******************************************************************************/

char *wgetstr(w,s)
WINDOW *w;
char *s;
{
  char *c = s,old_raw = rawflag;

  rawflag = FALSE; /* Strings werden stets im cooked-Mode eingelesen */
  do
    *c++ = (char) wgetch(w);
  while((c[-1] != '\n' && nlflag) || (c[-1] != '\r' && !nlflag));
  *(--c) = '\0'; /* Newline durch NULL-Charakter ersetzen */
  rawflag = old_raw;
  return(s);
}

/******************************************************************************
*
* Funktion     : Rumpf fÅr Text formatiert im Fenster ausgeben (wprintw_body)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in das Text kommen soll
*
*              : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei printf)
*
*              : param       :
*                  Typ          : va_list
*                  Wertebereich : Pointer auf Parameter
*                  Bedeutung    : Liste auszugebender Werte
*
* Beschreibung : Der Formatstring wird zusammen mit dem Pointer auf die
*                Parameterliste an vsprintf Åbergeben. Der mit vsprintf
*                erzeugte String wird mit waddstr in das gewÅnschte
*                Fenster ausgegeben.
*
******************************************************************************/

void wprintw_body(w,format,param)
WINDOW *w;
char *format;
va_list param;
{
  char buff[BUFF_SIZE+1];

  vsprintf(buff,format,param);
  waddstr(w,buff);
}

/******************************************************************************
*
* Funktion     : Rumpf fÅr Daten aus Fenster einlesen (wscanw_body)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in das Text kommen soll
*
*              : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei scanf)
*
*              : param       :
*                  Typ          : va_list
*                  Wertebereich : Pointer auf Parameter
*                  Bedeutung    : Adressen einzulesender Variablen
*
* Beschreibung : Mit wgetstr wird ein String eingelesen. Anschlie·end werden
*                öber die Funktion vsscanf die Variablen richtig besetzt.
*
******************************************************************************/

void wscanw_body(w,format,param)
WINDOW *w;
char *format;
va_list param;
{
  char buff[BUFF_SIZE+1];

  wgetstr(w,buff);
  vsscanf(buff,format,param);
}

/******************************************************************************
*
* Funktion     : Text formatiert im Fenster ausgeben (wprintw)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in das Text kommen soll
*
*              : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei printf)
*
*              : ...         :
*                  Typ          : Parameterliste
*                  Wertebereich : ???
*                  Bedeutung    : Liste auszugebender Werte
*
* Beschreibung : öber va_start wird der Pointer auf den ersten Parameter
*                ermittelt. Dann wird wprintw_body aufgerufen.
*
******************************************************************************/

void wprintw(WINDOW *w, char *format, ...)
{
  va_list param;

  va_start(param,format);
  wprintw_body(w,format,param);
  va_end(param);
}

/******************************************************************************
*
* Funktion     : Daten aus Fenster einlesen (wscanw)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in das Text kommen soll
*
*              : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei scanf)
*
*              : ...         :
*                  Typ          : Parameterliste
*                  Wertebereich : ???
*                  Bedeutung    : Adressen einzulesender Variablen
*
* Beschreibung : Der Anfang der Variablenliste wird mit va_start ermittelt.
*                Dann wird wscanw_body aufgerufen.
*
******************************************************************************/

void wscanw(WINDOW *w, char *format, ...)
{
  va_list param;

  va_start(param,format);
  wscanw_body(w,format,param);
  va_end(param);
}

/******************************************************************************
*
* Funktion     : Text formatiert an Position X/Y im Fenster ausgeben(mvwprintw)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in das Text kommen soll
*
*              : y           :
*                  Typ          : short int
*                  Wertebereich : 0-LINES
*                  Bedeutung    : Startposition fÅr Ausgabe Y
*
*              : x           :
*                  Typ          : short int
*                  Wertebereich : 0-COLS
*                  Bedeutung    : Startposition fÅr Ausgabe X
*
*              : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei printf)
*
*              : ...         :
*                  Typ          : Parameterliste
*                  Wertebereich : ???
*                  Bedeutung    : Liste auszugebender Werte
*
* Beschreibung : öber va_start wird der Pointer auf den ersten Parameter
*                ermittelt. Anschlie·end wird der Cursor an die gewÅnschte
*                Stelle bewegt. Dann wird wprintw_body aufgerufen.
*
******************************************************************************/

void mvwprintw(WINDOW *w, short int y, short int x, char *format, ...)
{
  va_list param;

  va_start(param,format);
  wmove(w,y,x);
  wprintw_body(w,format,param);
  va_end(param);
}

/******************************************************************************
*
* Funktion     : Daten an bestimmter Stelle im Fenster einlesen (mvwscanw)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in das Text kommen soll
*
*              : y           :
*                  Typ          : short int
*                  Wertebereich : 0-LINES
*                  Bedeutung    : Startposition fÅr Eingabe Y
*
*              : x           :
*                  Typ          : short int
*                  Wertebereich : 0-COLS
*                  Bedeutung    : Startposition fÅr Eingabe X
*
*              : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei scanf)
*
*              : ...         :
*                  Typ          : Parameterliste
*                  Wertebereich : ???
*                  Bedeutung    : Adressen einzulesender Variablen
*
* Beschreibung : Der Anfang der Variablenliste wird mit va_start ermittelt.
*                Dann wird der Cursor an die richtige Stelle bewegt.
*                Dann wird wscanw_body aufgerufen.
*
******************************************************************************/

void mvwscanw(WINDOW *w, short int y, short int x, char *format, ...)
{
  va_list param;

  va_start(param,format);
  wmove(w,y,x);
  wscanw_body(w,format,param);
  va_end(param);
}

/******************************************************************************
*
* Funktion     : Rahmen in Fenster zeichnen (box)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem Rahmen erscheinen soll
*
*              : sr          :
*                  Typ          : char
*                  Wertebereich : '\0'-'\255'
*                  Bedeutung    : Zeichen fÅr senkrechte RÑnder
*
*              : wr          :
*                  Typ          : char
*                  Wertebereich : '\0'-'\255'
*                  Bedeutung    : Zeichen fÅr waagerechte RÑnder
*
* Beschreibung : In das angegebene Fenster wird ein Rahmen eingezeichnet.
*                Es wird kein Refresh vorgenommen.
*
******************************************************************************/

void box(w,sr,wr)
WINDOW *w;
char sr,wr;
{
  int i;
  char old_scroll = w->_scrflag;

  w->_scrflag = FALSE; /* Fenster soll nicht scrollen, wenn unten rechts die */
  wmove(w,0,0);      /* Ecke eingezeichnet wird. */
  waddch(w,'⁄');
  wmove(w,0,w->_width_x-1);
  waddch(w,'ø');
  wmove(w,w->_width_y-1,0);
  waddch(w,'¿');
  wmove(w,w->_width_y-1,w->_width_x-1);
  waddch(w,'Ÿ');
  for(i=1 ; i < w->_width_x-1 ; i++)
  {
    wmove(w,0,i);    /* Rand oben */
    waddch(w,wr);
    wmove(w,w->_width_y-1,i);
    waddch(w,wr);   /* Rand unten */
  }
  for(i=1 ; i < w->_width_y-1 ; i++)
  {
    wmove(w,i,0);    /* Rand links */
    waddch(w,sr);
    wmove(w,i,w->_width_x-1);
    waddch(w,sr);   /* Rand rechts */
  }
  w->_scrflag = old_scroll;
}

/******************************************************************************
*
* Funktion     : Zeichen aus Fenster lesen (winch)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, aus dem gelesen werden soll
*
* Ergebnis     :
*                  Typ          : short int
*                  Wertebereich : '\0' - MAXINT
*                  Bedeutung    : Zeichen samt Attributen, das an der
*                                 angegebenen Position steht
*
* Beschreibung : Im gewÅnschten Fenster wird an der aktuellen Position
*                mit cur_to_poi das dort befindliche Zeichen samt Attributen
*                ermittelt und zurÅckgegeben.
*
******************************************************************************/

short int winch(w)
WINDOW *w;
{
  return(*cur_to_poi(w));
}
