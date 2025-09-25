/******************************************************************************
*
* Modul      : cur_main.c
*
* Funktionen : fatal (Fatale Fehlermeldung ausgeben)
*              beep (Beep auf Lautsprecher ausgeben)
*              flash (Bildschirmrahmen aufblinken lassen)
*              napms (Programm fÅr bestimmte Zeit anhalten)
*              _gyx (Cursor auf Position y/x setzen)
*              show_cursor (Cursor sichtbar machen)
*              hide_cursor (Cursor unsichtbar machen)
*              place_cursor (Cursor im Fenster plazieren)
*              cur_left (Cursor nach links bewegen)
*              cur_right (Cursor nach rechts bewegen)
*              newwin (Neues Fenster anlegen)
*              delwin (Fenster freigeben)
*              mvwin (Fenster bewegen)
*              in_win (Test, ob Punkt in Fenster liegt)
*              calc_overlap (öberlappungsbereich berechnen)
*              overlay (Fensterinhalt fÅr öberlappung ohne Blanks kopieren)
*              overwrite (Fensterinhalt fÅr öberlappung mit Blanks kopieren)
*              mvcur (Cursor an absolute Position bewegen)
*              set_videomode (Anpassung des bildschirmmodus)
*              set_os2_raw (Tastatur unter OS/2 in Raw-Mode setzen)
*              is_kbd_raw (Raw-Mode der Tastatur unter OS/2 testen)
*              initscr (Curses initialisieren)
*              endwin (Curses beenden)
*              wnoutrefresh (Window-Image ins VSI kopieren)
*              doupdate (VSI in physikalischen Schirm kopieren)
*              wrefresh (Window-Image ins VSI und VSI auf Schirm kopieren)
*              cur_to_poi (Cursoradresse in Pointer umrechnen)
*              poi_to_cur (Pointer in Cursorposition wandeln)
*              scroll (Fensterinhalt um eine Zeile nach oben bewegen)
*              printw (Text formatiert im Standardfenster ausgeben)
*              scanw (Text formatiert aus Standardfenster einlesen)
*              mvprintw (Text an Position X/Y im Standard-Fenster ausgeben)
*              mvscanw (Daten an bestimmter Stelle im Std.-Fenster einlesen)
*              main (Hauptprogramm)
*
******************************************************************************/

#define cur_main    /* Damit stdscr etc. nicht als extern deklariert werden */

#ifdef OS2
#define INCL_DOSPROCESS
#define INCL_VIO
#include <os2.h>
#else
#include <dos.h>
#include <process.h>
#endif
#include <stdarg.h>

#include "curses.h"

void scroll (WINDOW*);

struct VioMode { short int length;
		 char      mode,
			   colours;
		 short int x_size_text,
			   y_size_text,
			   x_size_graph,
			   y_size_graph,
			   reserved [18]; };


extern char *reserve_mem();

WINDOW *stdscr,              /* Bildschirmgro·es Standardfenster */
       *curscr,              /* Aktueller Bildschirminhalt */            
       *last_win;            /* Zuletzt refreshtes Window */             
short  int *vsi,             /* Virtueller Bildschirminhalt */           
       *PH_SC,               /* Startadresse des Bildschirmram */        
       LINES=25,             /* Anzahl der sichbaren Bildschirmzeilen */ 
       COLS=80;              /* Anzahl der sichbaren Bildschirmspalten */
short int A_NORMAL    = 0;   /* Keine Attribute */
short int A_STANDOUT  = 2;   /* GrÅn */
short int A_UNDERLINE = 4;   /* Rot */
short int A_BLINK     = 128; /* Binken */
short int A_DIM       = 8;   /* Nur intense */
short int A_BOLD      = 2;   /* GrÅn */
short int A_INVIS     = 136; /* Intense auf Intense */
short int STD_ATTR    = 7;   /* Standard-Darstellung grau auf schwarz */

#ifdef OS2
struct VioMode old_videomode; /* Videomodus beim Aufruf des Editors */
#else
short  old_videomode;
#endif
char   rawflag=FALSE,     /* Flag, ob Eingabe ungepuffert ist */
       echoflag=TRUE,     /* Flag, ob Eingabe angezeigt wird */
       nlflag=TRUE,       /* Flag, ob \r in \n umgewandelt wertden soll */
       start_scan,        /* Cursor Start- und */
       end_scan;          /* Endsscan */

/******************************************************************************
*
* Funktion    : Fatale Fehlermeldung ausgeben (fatal)
* -------------
*
* Parameter   : s           :
*                 Typ          : char *
*                 Wertebereich : Pointer auf ASCII-Zeichenkette
*                 Bedeutung    : Auszugebende Fehlermeldung
*
* Bescheibung : Der Åbergene String s wird ausgegeben, das Programm mit dem
*               Fehlercode 1 abgebrochen.
*
******************************************************************************/

void fatal(s)
char *s;
{
  printf("\nFATAL: %s\nProgramm wird abgebrochen\n",s);
  exit(1);
}

/******************************************************************************
*
* Funktion     : Beep auf Lautsprecher ausgeben (beep)
* --------------
*
* Beschreibung : Es werden zwei geschachtelte Schleifen gestartet, in denen
*                stÑndig das Bit 1 des Port B getoggled wird. Dadurch wird
*                ein Ton erzeugt. Die innere Schleife spezifiziert die
*                Frequenz, die Ñu·ere die Dauer des Tones.
*
******************************************************************************/

void beep()
{
#ifdef OS2
  DosBeep (1000, 200);
#else
  char old_portb = inportb(0x61);
  int  i;

  outportb(0x61,old_portb | 3); /* GATE 2 setzen */
  outportb(0x43,0xB6);      /* Signal: Counter 2 setzen */
  outportb(0x42,0);         /* Reziprokwert der Frequenz lo */
  outportb(0x42,4);         /*  -------------"---------  hi */
  for (i=0;i<32767;i++);
  outportb(0x61,old_portb);
#endif
}

/******************************************************************************
*
* Funktion     : Bildschirmrahmen kurz blinken lassen (flash)
* --------------
*
* Beschreibung : Die Rahmenfarbe wird komplementiert und fÅr kurze Zeit
*                angezeigt. Danach wird wieder die ursprÅngliche Farbe
*                restauriert.
*
******************************************************************************/

void flash()
{
#ifdef OS2
  struct VioState { short int length, func, colour; } vs;
  vs.length = 6;
  vs.func   = 1;  /* Funktion Rahmenfarbe ermitteln */
  VioGetState (&vs, 0);
  vs.colour = 255 - vs.colour; /* Farbe invertieren */
  VioSetState (&vs, 0);
  DosSleep (10);
  vs.colour = 255 - vs.colour; /* Alte Farbe wiederherstellen */
  VioSetState (&vs, 0);
#else
  long i;

  outportb(0x3D9,15);
  for(i=1;i<8000;i++);
  outportb(0x3D9,0);
#endif
}

/******************************************************************************
*
* Funktion     : Programm fÅr bestimmte Zeit anhalten (napms)
* --------------
*
* Parameter    : ms          :
*                  Typ          : int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Zeit in Millisekunden
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : OK
*                  Bedeutung    : Immer OK
*
* Beschreibung : Zwei geschachtelte Schleifen laufen Åber die angegebene Zeit.
*
******************************************************************************/

int napms(ms)
int ms;
{
#ifdef OS2
  DosSleep (ms);
#else
  int i;

  while(ms--)
    for(i=0;i<300;i++);
  return(OK);
#endif
}

/******************************************************************************
*
* Funktion     : Cursor auf Position y/x setzen (_gyx)
* --------------
*
* Parameter    : y           :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Neue Y-Cursorkoordinate
*
*              : x           :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Neue X-Cursorkoordinate
*
* Beschreibung : Der Cursor wird mit dem Interrupt 16 auf die richtige
*                Position gesetzt. Bei OS/2 wird VioSetCurPos verwendet.
*
******************************************************************************/

void _gyx(y,x)
short int y,x;
{
#ifdef OS2
  VioSetCurPos (y, x, 0);
#else
  union REGS r;

  r.h.ah = 2;
  r.h.bh = 0;
  r.h.dl = x;
  r.h.dh = y;
  int86(16,&r,&r);
#endif
}

/******************************************************************************
*
* Funktion     : Cursor sichtbar machen (show_cursor)
* --------------
*
* Beschreibung : Der Cursor wird mit dem BIOS-Interrupt 16 sichtbar gemacht.
*                Bei OS/2 wird VioGet/SetCurType verwendet.
*
******************************************************************************/

void show_cursor()
{
#ifdef OS2
  struct CurType { short int start_y, end_y, size_x, attrs; } ct;

  VioGetCurType (&ct, 0);
  ct.attrs = 0;
  VioSetCurType (&ct, 0);
#else
  union REGS  r;

  r.h.ah = 1;
  r.h.ch = start_scan;
  r.h.cl = end_scan;
  int86(16,&r,&r);
#endif
}

/******************************************************************************
*
* Funktion     : Cursor unsichtbar machen (hide_cursor)
* --------------
*
* Beschreibung : Der Cursor wird mit dem BIOS-Interrupt 16 unsichtbar gemacht.
*
******************************************************************************/

void hide_cursor()
{
#ifdef OS2
  struct CurType { short int start_y, end_y, size_x, attrs; } ct;

  VioGetCurType (&ct, 0);
  ct.attrs = 1;
  VioSetCurType (&ct, 0);
#else
  union REGS r;

  r.h.ah = 1;
  r.h.ch = 31;
  int86(16,&r,&r);
#endif
}

/******************************************************************************
*
* Funktion     : Cursor im Fenster plazieren (place_cursor)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem der Cursor stehen soll
*
* Beschreibung : GemÑ· der in der WINDOW-Struktur gesetzten Cursor-Koordina-
*                ten und Fensterposition wird der Cursor Åber die Funktion
*                _gyx an die richtige Stelle gesetzt.
*
******************************************************************************/

void place_cursor(w)
WINDOW *w;
{
  _gyx(w->_start_y+w->_cursor_y,w->_start_x+w->_cursor_x);
  show_cursor();
}

/******************************************************************************
*
* Funktion     : Cursor nach links bewegen (cur_left)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Cursor nach links bewegt
*                                 werden soll
*
* Parameter    : anz         :
*                  Typ          : int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : Anzahl der Zeichen, um die der Cursor
*                                 nach links bewegt werden soll
*
* Beschreibung : Der Cursor wird anz mal nach links bewegt. Ist er am linken
*                Rand angelangt, so wird er ans Ende der darÅberliegenden
*                Zeile gestellt. Ist der Cursor in der oberen linken Fenster-
*                ecke angelangt, so passiert nichts mehr.
*
******************************************************************************/

void cur_left(w,anz)
WINDOW *w;
int anz;
{
  while(anz--)
  {
    if(w->_cursor_x)
      w->_cursor_x--;
    else
      if(w->_cursor_y)
      {
	w->_cursor_x = w->_width_x-1;
	w->_cursor_y--;
      }
  }
}

/******************************************************************************
*
* Funktion     : Cursor nach rechts bewegen (cur_right)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Cursor nach links bewegt
*                                 werden soll
*
* Parameter    : anz         :
*                  Typ          : int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : Anzahl der Zeichen, um die der Cursor
*                                 nach rechts bewegt werden soll
*
* Beschreibung : Der Cursor wird anz mal nach rechts bewegt. Ist er am rechten
*                Rand angelangt, so wird er an den Anfang der darunterliegenden
*                Zeile gestellt. Ist der Cursor in der unteren rechten Fenster-
*                ecke angelangt, so wird, falls scroll gesetzt ist, das Fenster
*                gescrollt, ansonsten wird er einfach an den Anfang der
*                letzten Zeile gestellt.
*
******************************************************************************/

void cur_right(w,anz)
WINDOW *w;
int anz;
{
  while(anz--)
  {
    if(w->_cursor_x < w->_width_x-1)
      w->_cursor_x++;
    else
    {
      w->_cursor_x = 0;
      if(w->_cursor_y < w->_width_y-1)
	w->_cursor_y++;
      else
	if(w->_scrflag)
	  scroll(w);
    }
  }
}

/******************************************************************************
*
* Funktion     : Neues Fenster anlegen (newwin)
* --------------
*
* Parameter    : dy          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Y-Grî·e des Fensters
*
*              : dx          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : X-Grî·e des Fensters
*
*              : sy          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Y-Startkoordinate des Fensters
*
*              : sx          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : X-Startkoordinate des Fensters
*
* Ergebnis     :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Pointer auf neu erzeugte WINDOW-Struktur
*
* Beschreibung : Es wird Speicherplatz fÅr eine neue WINDOW-Struktur ange-
*                fordert. Dort werden die Parameter als Position und Grî·e
*                eingetragen. Es wird Speicherplatz fÅr das Window-Image
*                angefordert. Dieser wird mit dem Standard-Attribut besetzt.
*                Dann werden die Flags (scroll etc.) initialisiert.
*                Der Pointer auf diese Fensterstruktur wird zurÅckgegeben.
*                Wird bei der Fenstergrî·e 0 angegeben, so wird der Wert auf
*                das Maximum gesetzt (LINES-sy bzw. COLS-sx).
*
******************************************************************************/

WINDOW *newwin(dy,dx,sy,sx)
short int sx,sy,dx,dy;
{
  WINDOW *w;

  w = (WINDOW *) reserve_mem(sizeof(WINDOW));
  if(sx < 0)        sx = 0;
  if(sx > COLS-1)   sx = COLS-1;
  if(sy < 0)        sy = 0;
  if(sy > LINES-1)  sy = LINES-1;
  if(dx < 1)        dx = COLS-sx;
  if(dy < 1)        dy = LINES-sy;
  if(sx+dx > COLS)  dx = COLS-sx;
  if(sy+dy > LINES) dy = LINES-sy;
  w->_start_x = sx;
  w->_start_y = sy;
  w->_width_x = dx;
  w->_width_y = dy;
  w->_cursor_x = w->_cursor_y = 0;
  w->_image = (short int *) reserve_mem(w->_width_y * w->_width_x * sizeof(short int)); /* Speicher fÅr Fensterinhalt */
  werase(w);
  w->_scrflag  = TRUE;  /* StandardmÑ·ig wird das Fenster gescrollt */
  w->_lvokflag = FALSE; /* Cursor ist sichtbar */
  w->_dlyflag  = TRUE;  /* wgetch wartet auf Zeichen */
  w->_tabflag  = TRUE;  /* Tabs werden zu Spaces expandiert */
  w->_attribs  = STD_ATTR; /* Standardattribute fÅr neues Fenster */
  return(w);
}

/******************************************************************************
*
* Funktion     : Fenster freigeben (delwin)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Freizugebendes Fenster
*
* Beschreibung : Der fÅr das Window-Image und die Window-Struktur reservierte
*                Speicherplatz werden freigegeben.
*
******************************************************************************/

void delwin(w)
WINDOW *w;
{
  if(w)
  {
    free(w->_image);
    free(w);
  }
}

/******************************************************************************
*
* Funktion     : Fenster verschieben (mvwin)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf WINDOW-Struktur
*                  Bedeutung    : Zu verschiebendes Fenster
*
*              : ny          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Neue Y-Startkoordinate des Fensters
*
*              : nx          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Neue X-Startkoordinate des Fensters
*
* Beschreibung : Falls das Fenster an der neuen Position komplett innerhalb
*                des Bildschirms liegt, werden die Startkoordinaten auf
*                die uebergebenen Werte gesetzt.                
*
******************************************************************************/

void mvwin(w,ny,nx)
WINDOW *w;
short int ny,nx;
{
  if(nx >= 0 && ny >= 0
  && nx+w->_width_x <= COLS && ny+w->_width_y <= LINES)
  {
    w->_start_x = nx;
    w->_start_y = ny;
  }
}

/******************************************************************************
*
* Funktion     : Testen, ob Punkt in Fenster liegt (in_win)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf Window-Struktur
*                  Bedeutung    : Fenster, in dem evtl. y/x liegen soll
*
*              : y           :
*                  Typ          : short int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : Y-Koordinate des zu testenden Punktes
*
*              : x           :
*                  Typ          : short int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : X-Koordinate des zu testenden Punktes
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE: Punkt innerhalb; FALSE: Punkt au·erhalb
*
* Beschreibung : Es wird getestet, ob der Punkt mit den absoluten Koordinaten
*                y und x im Fenster w liegt. Dazu werden die Fenstereckpunkte
*                absolut berechnet.
*
******************************************************************************/

int in_win(w,y,x)
WINDOW *w;
short int y,x;
{
  return (y >= w->_start_y && y < w->_start_y+w->_width_y
       && x >= w->_start_x && x < w->_start_x+w->_width_x);
}

/******************************************************************************
*
* Funktion     : öberlappungsbereich zweier Fenster berechnen (calc_overlap)
* --------------
*
* Parameter    : w1          :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf Window-Struktur
*                  Bedeutung    : 1. Fenster fÅr evtl. öberlappung
*
*              : w2          :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf Window-Struktur
*                  Bedeutung    : 2. Fenster fÅr evtl. öberlappung
*
*              : l           :
*                  Typ          : short int *
*                  Wertebereich : Pointer auf short int
*                  Bedeutung    : Linker Rand der öberlappung
*
*              : r           :
*                  Typ          : short int *
*                  Wertebereich : Pointer auf short int
*                  Bedeutung    : Rechter Rand der öberlappung
*
*              : o           :
*                  Typ          : short int *
*                  Wertebereich : Pointer auf short int
*                  Bedeutung    : Oberer Rand der öberlappung
*
*              : u           :
*                  Typ          : short int *
*                  Wertebereich : Pointer auf short int
*                  Bedeutung    : Unterer Rand der öberlappung
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE: öberlappung; FALSE: Keine öbrlp.
*
* Beschreibung : Mit der Funktion in_win wird fÅr jede Ecke der beiden
*                Åbergebenen Fenster w1 und w2 getestet, ob sie im jeweils
*                anderen Fenster liegen. Falls ja, werden die entsprechenden
*                Eckpunkte gesetzt.
*                Beispiel: Liegt die linke obere Ecke von w2 in w1, dann
*                wird l auf den linken, o auf den oberen Rand von w2 gesetzt.
*
******************************************************************************/

int calc_overlap(w1,w2,l,r,o,u)
WINDOW *w1,*w2;
short int *l,*r,*o,*u;
{
  int ret = FALSE;

  /* Zuerst checken, welche Ecken von w1 in w2 liegen */
  if(in_win(w2,w1->_start_y,w1->_start_x))
  { *l = w1->_start_x; *o = w1->_start_y; ret = TRUE; }
  if(in_win(w2,w1->_start_y+w1->_width_y-1,w1->_start_x))
  { *l = w1->_start_x; *u = w1->_start_y+w1->_width_y-1; ret = TRUE; }
  if(in_win(w2,w1->_start_y,w1->_start_x+w1->_width_x-1))
  { *r = w1->_start_x+w1->_width_x-1; *o = w1->_start_y; ret = TRUE; }
  if(in_win(w2,w1->_start_y+w1->_width_y-1,w1->_start_x+w1->_width_x-1))
  { *r = w1->_start_x+w1->_width_x-1; *u = w1->_start_y+w1->_width_y-1; ret = TRUE; }

  /* Dann checken, welche Ecken von w2 in w1 liegen */
  if(in_win(w1,w2->_start_y,w2->_start_x))
  { *l = w2->_start_x; *o = w2->_start_y; ret = TRUE; }
  if(in_win(w1,w2->_start_y+w2->_width_y-1,w2->_start_x))
  { *l = w2->_start_x; *u = w2->_start_y+w2->_width_y-1; ret = TRUE; }
  if(in_win(w1,w2->_start_y,w2->_start_x+w2->_width_x-1))
  { *r = w2->_start_x+w2->_width_x-1; *o = w2->_start_y; ret = TRUE; }
  if(in_win(w1,w2->_start_y+w2->_width_y-1,w2->_start_x+w2->_width_x-1))
  { *r = w2->_start_x+w2->_width_x-1; *u = w2->_start_y+w2->_width_y-1; ret = TRUE; }
  return(ret);
}

/******************************************************************************
*
* Funktion     : öberlappungsbereich ohne Blanks kopieren (overlay)
* --------------
*
* Parameter    : w1          :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf WINDOW-Struktur
*                  Bedeutung    : Fenster aus dem kopiert werden soll
*
*              : w2          :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf WINDOW-Struktur
*                  Bedeutung    : Fennster in das kopiert werden soll
*
* Beschreibung : ZunÑchst wird mit der Funktion calc_overlap der öberlappungs-
*                bereich berechnet. Ist dieser existent, werden alle Zeichen
*                des öberlappungsbereichs die kein Blank sind, aus Fenster w1
*                nach Fenster w2 kopiert.
*
******************************************************************************/

void overlay(w1,w2)
WINDOW *w1,*w2;
{
  short int l=0,r=0,o=0,u=0, /* Eckpunkte des öberlappungsbereichs */
	    zl,              /* ZÑhler fÅr Durchlaufen des öB */
	    sk1,sk2,         /* Wie weit Pointer nach Zeilenende
				erhîht werden mÅssen */
	    *z1,*z2;         /* Pointer in Images */
  char      inhalt;          /* Zu kopierendes Zeichen */

  if(calc_overlap(w1,w2,&l,&r,&o,&u)) /* öberlappungsbereich berechnen */
  {
    zl = l;
    /* Skipweite: <Fensterbreite> - <Breite des öB> */
    sk1 = w1->_width_x-(r-l)-1;
    sk2 = w2->_width_x-(r-l)-1;
    z1 = w1->_image + w1->_width_x*(o-w1->_start_y) + l-w1->_start_x;
    z2 = w2->_image + w2->_width_x*(o-w2->_start_y) + l-w2->_start_x;
    while (o <= u)
    {
      while (zl <= r)
      {
	if((inhalt = *z1 & 255) != ' ' && inhalt != '\0' && inhalt != '\377')
	  *z2++ = *z1++; /* Fensterinhalt von w1 nach w2 kopieren, */
	else             /* falls kein ' ' */
	{
	  z1++;   /* Sonst nur die Zeiger erhîhen */
	  z2++;
	}
	zl++;         /* Ein Zeichen weiter nach rechts */
      }
      z1 += sk1; /* Alles was nicht zum öB gehîrt Åberspringen */
      z2 += sk2;
      zl=l; /* An den Anfang der nÑchsten Zeile des */
      o++; /* öberlappungsbereiches gehen. */
    }
  } /* ende if öberlappung */
}

/******************************************************************************
*
* Funktion     : öberlappungsbereich mit Blanks kopieren (overwrite)
* --------------
*
* Parameter    : w1          :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf WINDOW-Struktur
*                  Bedeutung    : Fenster aus dem kopiert werden soll
*
*              : w2          :
*                  Typ          : WINDOW *
*                  Wertebereich : Zeiger auf WINDOW-Struktur
*                  Bedeutung    : Fennster in das kopiert werden soll
*
* Beschreibung : ZunÑchst wird mit der Funktion calc_overlap der öberlappungs-
*                bereich berechnet. Ist dieser existent, werden alle Zeichen
*                des öberlappungsbereichs aus Fenster w1 nach Fenster w2
*                kopiert.
*
******************************************************************************/

void overwrite(w1,w2)
WINDOW *w1,*w2;
{
  short int l=0,r=0,o=0,u=0, /* Eckpunkte des öberlappungsbereichs */
	    zl,              /* ZÑhler fÅr Durchlaufen des öB */
	    sk1,sk2,         /* Wie weit Pointer nach Zeilenende
				erhîht werden mÅssen */
	    *z1,*z2;         /* Pointer in Images */

  if(calc_overlap(w1,w2,&l,&r,&o,&u)) /* öberlappungsbereich berechnen */
  {
    zl = l;
    /* Skipweite: <Fensterbreite> - <Breite des öB> */
    sk1 = w1->_width_x-(r-l)-1;
    sk2 = w2->_width_x-(r-l)-1;
    z1 = w1->_image + w1->_width_x*(o-w1->_start_y) + l-w1->_start_x;
    z2 = w2->_image + w2->_width_x*(o-w2->_start_y) + l-w2->_start_x;
    while (o <= u)
    {
      while (zl <= r)
      {
	*z2++ = *z1++; /* Fensterinhalt von w1 nach w2 kopieren, */
	zl++;         /* Ein Zeichen weiter nach rechts */
      }
      z1 += sk1; /* Alles was nicht zum öB gehîrt Åberspringen */
      z2 += sk2;
      zl=l; /* An den Anfang der nÑchsten Zeile des */
      o++; /* öberlappungsbereiches gehen. */
    }
  } /* ende if öberlappung */
}

/******************************************************************************
*
* Funktion     : Cursor absolut bewegen (mvcur)
* --------------
*
* Parameter    : oy          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Alte Y-Koordinate, wird allerdings
*                                 ignoriert.
*
*              : ox          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Alte X-Koordinate, wird allerdings
*                                 ignoriert
*
*              : ny          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Neue Y-Koordinate
*
*              : nx          :
*                  Typ          : short int
*                  Wertebereich : 0-32767
*                  Bedeutung    : Neue X-Koordinate
*
* Beschreibung : Die Funktion setzt den Hardwarecursor mit der Funktion
*                _gyx auf die gewÅnschte Position, wobei dazu der
*                Interrupt 16 benutzt wird.
*
******************************************************************************/

void mvcur(oy,ox,ny,nx)
short int oy,ox,ny,nx;
{
  _gyx(ny,nx);
}

/******************************************************************************
*
* Funktion     : Videomodus setzen (set_videomode)
* --------------
*
* Beschreibung : Anhand der Werte der beiden Variablen LINES und COLS
*                wird die Nummer des Bildschirmmodus aus einer Tabelle
*                ermittelt. Anschlie·end wird dieser Modus aktiviert.
*                Die Korrektheit der Koordinaten wird bereits bei der
*                Analyse der Umgebungsvariablen geprÅft.
*
******************************************************************************/

void set_videomode()
{
#ifdef OS2
  struct VioMode vm;

  vm.length = sizeof (struct VioMode);
  VioGetMode (&vm, 0);
  vm.x_size_text = COLS;
  vm.y_size_text = LINES;
  VioSetMode (&vm, 0);
  VioGetMode (&vm, 0);
  COLS  = vm.x_size_text; /* Falls nicht umgeschaltet werden konnte, */
  LINES = vm.y_size_text; /* aktuelle Fenstergrî·e ermitteln         */
#else
  int tabelle[9][3] = { { 40,25, 0},   /* Tabelle enthÑlt die Spalten- */
			{ 80,25, 2},   /* und die Zeilenzahl sowie     */
			{ 80,30,80},   /* die Nummer der unterstÅtzten */
			{ 80,43,81},   /* Textmodi.                    */
			{ 80,60,82},
			{132,25,87},
			{132,30,88},
			{132,43,89},
			{132,60,90} },
      mode;                     /* Der zu aktivierende Bildschirmmodus */
  union REGS r;               /* Prozessorregister fÅr Video-Interrupt */

  for(mode=0;mode<10;mode++)
    if(tabelle[mode][0] == COLS && tabelle[mode][1] == LINES)
    {
      mode = tabelle[mode][2];
      break;
    }
  r.h.ah = 0; /* Funktion setze Video-Modus */
  r.h.al = mode;
  int86(16,&r,&r);  /* Modus mit dem BIOS-Int 16 setzen */
#endif
}

#ifdef OS2
/******************************************************************************
*
* Funktion     : Tastatur in Raw-Mode versetzen (set_os2_raw)
* --------------
*
* Parameter    : raw_on      :
*                  Typ          : char
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : Gibt an, ob der raw-Mode ein- oder ausge-
*                                 schaltet werden soll.
*
* Beschreibung : Es wird mittels der OS/2 Kernel-Aufrufe KbdGetStatus
*                und KbdSetStatus die Tastatur in den Raw-Mode versetzt.
*                Dadurch kînnen mittels KbdCharIn auch <CTRL>-S und <CTRL>-C
*                erkannt werden.
*
******************************************************************************/

void set_os2_raw (char raw_on)
{
  struct KbdState { unsigned short int length,
				       state,
				       lineend,
				       nls,
				       shift;   } kbst;

  kbst.length = 10;
  KbdGetStatus (&kbst, 0);
  if (raw_on)
    kbst.state = (kbst.state | 4) & ~8;
  else
    kbst.state = (kbst.state | 8) & ~4;
  KbdSetStatus (&kbst, 0);
}

/******************************************************************************
*
* Funktion     : Abfragen, ob Tastatur im Raw-Mode ist (is_kbd_raw)
* --------------
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE: Kbd ist im Raw-Mode;
*                                 FALSE: Kbd ist nicht im Raw-Mode
*
* Beschreibung : Es wird mittels des OS/2 Kernel-Aufrufs KbdGetStatus
*                geprÅft, ob sich die Tastatur im Raw-Mode befindet.
*                Ist das der Fall, so wird TRUE zurÅckgegeben, sonst FALSE.
*
******************************************************************************/

char is_kbd_raw()
{
  struct KbdState { unsigned short int length,
				       state,
				       lineend,
				       nls,
				       shift;   } kbst;

  KbdGetStatus (&kbst, 0);
  return (kbst.state & 4) != 0;
}
#endif

/******************************************************************************
*
* Funktion     : Curses initialisieren (initscr)
* --------------
*
* Beschreibung : Es wird die Cursorgrî·e ermittelt und in den globalen
*                Variablen start_scan und end_scan abgelegt. Falls sich
*                die Bildschirmkarte nicht im Monochrommodus befindet,
*                wird der Videomodus an LINES und COLS angepa·t. Sonst wird
*                LINES und COLS an die Grî·e des monochromen Schirms
*                angepasst.
*                Der Speicherplatz fÅr den Standardbildschirm wird angefor-
*                dert und mit dem Standardattribut besetzt (Åber die Funktion
*                newwin). Anschlie·end werden blinkende Zeichen zugelassen.
*
******************************************************************************/

void initscr()
{
#ifdef OS2
  struct VioMode  vm;
  struct VioState { short int length, func, blink; } vs;

  vm.length = sizeof (struct VioMode);
  VioGetMode (&vm, 0);
  old_videomode = vm;
#else
  union REGS  r;

  r.h.ah = 15; /* Video-Funktion <Get Current VDU Parameters> */
  int86(16,&r,&r);
  old_videomode = r.h.al; /* Aktuellen Video-Modus in old_videomode merken */
  r.h.ah = 3; /* Funktion 3: Cursorpos. und Scan-Start/-Ende ermitteln */
  r.h.bh = 0; /* Page 0 */
  int86(16,&r,&r);
  start_scan = r.h.ch; /* Cursor Start- und Endscan merken */
  end_scan   = r.h.cl;
#endif
  set_videomode(); /* AbhÑngig von LINES und COLS den Videomodus anpassen */
  last_win = curscr = newwin(LINES,COLS,0,0);
  vsi = curscr->_image;
  stdscr = newwin(LINES,COLS,0,0);
#ifdef OS2
  /* Bildschirm lîschen */
  VioWrtNCell ("\0", COLS*LINES, 0, 0, 0);
  vs.func   = 2;
  vs.length = 6;
  vs.blink  = 0; /* Blinkende Zeichen zulassen */
  VioSetState (&vs, 0);
#else
  PH_SC = (old_videomode == 7) ? /* Monochrom-Modus ? Dann VDU-RAM ab B000 */
	  (short int*) 0xB0000000 : (short int*) 0xB8000000;
  memcpy(PH_SC,vsi,COLS*LINES*sizeof(short int)); /* Physikalischen Bildschirm lîschen */
  outportb(0x3D8,41);  /* Blinkende Zeichen zulassen */
#endif
}

/******************************************************************************
*
* Funktion     : Curses beenden (endwin)
* --------------
*
* Beschreibung : Der physikalische Bildschirm wird gelîscht, das stdscr-
*                Fenster wird mit der Funmtion delwin freigegeben.
*                Anschlie·end wird der Video-Modus aktiviert, der beim Aufruf
*                des Editors aktiv war.
*
******************************************************************************/

void endwin()
{
#ifdef OS2
  /* Bildschirm lîschen */
  VioWrtNCell ("\0", COLS*LINES, 0, 0, 0);
  set_os2_raw (FALSE);
#else
  short int  *d;
  int        i;
  union REGS r;

  for (d = PH_SC , i=COLS*LINES; i ; i-- , d++)
    *d = STD_ATTR*256; /* Bildschirm lîschen */
#endif
  delwin(stdscr);     /* stdscr freigeben */
  delwin(curscr);     /* curscr freigeben */
#ifdef OS2
  VioSetMode (&old_videomode, 0);   /* Videomodus restaurieren */
#else
  r.h.ah = 0; /* Funktion setze Video-Modus */
  r.h.al = old_videomode;
  int86(16,&r,&r);  /* Modus mit dem BIOS-Int 16 setzen */
#endif
}

/******************************************************************************
*
* Funktion     : Window-Image ins VSI kopieren (wnoutrefresh)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Inhalt ins VSI kopiert
*                                 werden soll.
*
* Beschreibung : Der Inhalt des Window-Images wird an der richtigen Stelle
*                in das virtuelle Screen-Image (VSI) kopiert.
*
******************************************************************************/

void wnoutrefresh(w)
WINDOW *w;
{
  short int *p_vsi = vsi + COLS*w->_start_y + w->_start_x;
  short int *p_win = w->_image,y;

  for(y = w->_width_y;y;y--)
  {
    memcpy(p_vsi,p_win,w->_width_x*sizeof(short int)); /* Eine Zeile aus dem Fenster in den virtuellen Bildschirm kopieren */
    p_vsi += COLS; /* Eine Zeile im virtuellen Schirm nach unten */
    p_win += w->_width_x; /* Eine Zeile im Fenster nach unten */
  }
}

/******************************************************************************
*
* Funktion     : VSI in physikalischen Schirm kopieren (doupdate)
* --------------
*
* Beschreibung : Mit der Funktion memcpy wird das VSI ins physikalische
*                Bildschirmram kopiert. Anschlie·end wird der Cursor in das
*                zuletzt refreshte Fenster gesetzt (last_win).
*
******************************************************************************/

void doupdate()
{
#ifdef OS2
  VioWrtCellStr (vsi, SZ_SC, 0, 0, 0);
#else
  memcpy(PH_SC,vsi,SZ_SC); /* VSI in physikalischen Schirm kopieren */
#endif
  if(last_win->_lvokflag)
    hide_cursor();
  else
    place_cursor(last_win);
}

/******************************************************************************
*
* Funktion     : Window-Image ins VSI und VSI auf Schirm kopieren (wrefresh)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Inhalt ins VSI soll.
*
* Beschreibung : Die globale Variable last_win wird auf w gesetzt. Anschlies-
*                send wird mit der Funktion wnoutrefresh der Fensterinhalt
*                in das virtuelle Screen-Image kopiert, danach wird das mo-
*                difizierte VSI in den Bilschirmspeicher kopiert. Dazu wird
*                die Funktion doupdate verwendet.
*
******************************************************************************/

void wrefresh(w)
WINDOW *w;
{
#ifdef CUR_DIRECT
  WINDOW *old_last_win = last_win;
#endif

  last_win = w; /* Dieses Fenster ist das, in dem der Cursor angezeigt wird */
  wnoutrefresh(w);
#ifdef CUR_DIRECT
  if (old_last_win != last_win) /* Mu· Fenster erst "nach vorne" gebracht
				   werden? Dann auch doupdate() aufrufen. */
#endif
    doupdate();
#ifdef CUR_DIRECT
  else
    if(last_win->_lvokflag)    /* Cursor hier setzen, da doupdate() */
      hide_cursor();           /* nicht aufgerufen wurde.           */
    else
      place_cursor(last_win);
#endif
}

/******************************************************************************
*
* Funktion     : Cursoradresse in Pointer umrechnen (cur_to_poi)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, aus dessen Cursoradresse und
*                                 Window-Image-Pointer ein Pointer berechnet
*                                 werden soll, der auf das Zeichen zeigt, auf
*                                 dem der Cursor steht.
*
* Ergebnis     :
*                  Typ          : short int *
*                  Wertebereich : Zeiger auf Zeichen im Window-Image
*                  Bedeutung    : Zeigt auf das Zeichen im Window-Image, auf
*                                 dem der Cursor des Fensters w steht.
*
* Beschreibung : Aus der Startadresse des Window-Images des Fensters w und
*                der Cursorposition wird die Adresse des Zeichens im WI be-
*                rechnet, auf dem der Cursor steht. Dieser Zeiger wird
*                zurÅckgegeben.
*
******************************************************************************/

short int *cur_to_poi(w)
WINDOW *w;
{
  return(w->_image + w->_width_x*w->_cursor_y + w->_cursor_x);
}

/******************************************************************************
*
* Funktion     : Pointer in Cursorposition wandeln (poi_to_cur)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Cursorkoordinaten angepasst
*                                 werden sollen und in dessen Window-Image
*                                 der Åbergebene Pointer zeigt.
*
*              : poi         :
*                  Typ          : short int *
*                  Wertebereich : Pointer in Window-Image des Fensters *w
*                  Bedeutung    : Pointer, der in Cursorkoordinaten umgewan-
*                                 delt werden soll.
*
* Beschreibung : Es werden die Cursorkoordinaten des Fensters w so gesetzt,
*                da· die korrespondierende Speicherstelle im Window-Image die
*                ist, auf die der Åbergebene Pointer zeigt.
*
******************************************************************************/

void poi_to_cur(w,poi)
WINDOW *w;
short int *poi;
{
  int diff = poi - w->_image;

  w->_cursor_y = diff / w->_width_x;
  w->_cursor_x = diff % w->_width_x;
}

/******************************************************************************
*
* Funktion     : Fensterinhalt um eine Zeile nach oben bewegen (scroll)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, dessen Inhalt bewegt werden soll
*
* Beschreibung : Der Cursor wird in die oberste Zeile des Fensters bewegt.
*                Dann wird die Funktion wdeleteln aufgerufen, die die oberste
*                Zeile lîscht. Dadurch rÅcken die darunter gelegenen Zeilen je
*                um eine Zeile nach oben. Anschliessend wird der Cursor wieder
*                in seine alte Zeile gesetzt.
*
******************************************************************************/

void scroll(w)
WINDOW *w;
{
  short int old_y = w->_cursor_y;

  w->_cursor_y = 0;
  wdeleteln(w);
  w->_cursor_y = old_y;
}

/******************************************************************************
*
* Funktion     : Text formatiert im Standardfenster ausgeben (printw)
* --------------
*
* Parameter    : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei printf)
*
*              : ...         :
*                  Typ          : Parameterliste
*                  Wertebereich : ???
*                  Bedeutung    : Liste auszugebender Werte
*
* Beschreibung : öber die Funktion va_start wird ein Zeiger auf die Parame-
*                terliste gewonnen, der zusammen mit dem Formatstring an
*                wprintw_body Åbergeben werden kann. Dort wird der String
*                "zusammengebastelt" und ausgegeben.
*
******************************************************************************/

void printw(char *format, ...)
{
  va_list param;

  va_start(param,format);
  wprintw_body(stdscr,format,param);
  va_end(param);
}

/******************************************************************************
*
* Funktion     : Text formatiert aus Standardfenster einlesen (scanw)
* --------------
*
* Parameter    : format      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Formatstring (wie bei printf)
*
*              : ...         :
*                  Typ          : Parameterliste
*                  Wertebereich : ???
*                  Bedeutung    : Liste zu belegender Variablen
*
* Beschreibung : Es wird öber die Funktion va_start ein Zeiger auf die Parame-
*                terliste gewonnen, der zusammen mit dem Formatstring an
*                wscanw_body Åbergeben werden kann.
*                wscanw_body besetzt die Variablen korrekt.
*
******************************************************************************/

void scanw(char *format, ...)
{
  va_list param;

  va_start(param,format);
  wscanw_body(stdscr,format,param);
  va_end(param);
}

/******************************************************************************
*
* Funktion     : Text an Position X/Y im Standard-Fenster ausgeben (mvprintw)
* --------------
*
* Parameter    : y           :
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

void mvprintw(short int y, short int x, char *format, ...)
{
  va_list param;

  va_start(param,format);
  wmove(stdscr,y,x);
  wprintw_body(stdscr,format,param);
  va_end(param);
}

/******************************************************************************
*
* Funktion     : Daten an bestimmter Stelle im Std.-Fenster einlesen (mvscanw)
* --------------
*
* Parameter    : y           :
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

void mvscanw(short int y, short int x, char *format, ...)
{
  va_list param;

  va_start(param,format);
  wmove(stdscr,y,x);
  wscanw_body(stdscr,format,param);
  va_end(param);
}
