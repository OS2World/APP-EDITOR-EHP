/******************************************************************************
*
* Modul      : cur_wget.c
*
* Funktionen : getch (Zeichen ungepuffert von Tastatur lesen)
*              edit_left (Cursor in Eingabe nach links)
*              edit_right (Cursor in Eingabe nach rechts)
*              edit_del (Zeichen in Eingabe lîschen)
*              wort_ende (Testen, ob Zeichen Worttrenner)
*              edit_ctrlright (Cursor in Eingabe ein Wort nach rechts)
*              edit_ctrlleft (Cursor in Eingabe ein Wort nach links)
*              edit_home (Cursor an Eingabeanfang)
*              edit_end (Cursor an Eingabeende)
*              treat_edit_keys (Edit-Tasten behandeln)
*              inp_out (Eingabe ausgeben)
*              treat_backspace (Backspace beim Edieren behandeln)
*              buf_insert (Ein Zeichen in Puffer einfÅgen)
*              getch_cooked (Zeichen gepuffet einlesen)
*              wgetch (Zeichen einlesen)
*
******************************************************************************/

#include "curses.h"
#include "keys.h"
#undef getch()   /* Damit C-getch aufgerufen werden kann */

void inp_out(WINDOW*, short int**, char**, short int**);

extern short int *cur_to_poi();
extern char rawflag,echoflag,nlflag;

char buffer[BUFF_SIZE+1],*p_in=buffer,*p_out=buffer; /* Zeichenpuffer fÅr wgetch */

#ifdef OS2
#define INCL_KBD
#include <os2.h>

/******************************************************************************
*
* Funktion     : Zeichen ungepuffert von Tastatur lesen (getch)
* --------------
*
* Parameter    : wait        :
*                  Typ          : char
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : Gibt an, ob auf das Eingabezeichen gewartet
*                                 werden soll.
*
* Ergebnis     :
*                  Typ          : short int
*                  Wertebereich : Ein Tastaturcode, der OS/2-spezifisch sein kann
*                                 Low-Byte=0 bedeutet Sondertaste (F, Ctrl-rechts o.Ñ.)
*                                 oder "kein Zeichen vorhanden". In letzterem Fall
*                                 ist der komplette RÅckgabewert jedoch 0.
*                  Bedeutung    : Das eingelesene Zeichen, oder 0, falls nicht
*                                 auf ein Zeichen gewartet werden sollte (wait==FALSE)
*                                 und kein Zeichen anlag.
*
* Beschreibung : Mittels des OS/2 Systemaufrufs KbdCharIn wird
*                ein Zeichen von der Tastatur gelesen und decodiert.
*                Wenn sich die Tastatur im Raw-Mode befindet, kînnen
*                auch <CTRL>-S und <CTRL>-C erkannt werden.
*
******************************************************************************/

short int getch (char wait)
{
  struct CharInBuffer { unsigned  char ascii,
				  scan_code,
				  status,
				  reserved;
			short int shifts;
			long  int time_stamp; } cib;

  KbdCharIn (&cib, (short) !wait, 0);
  if (!wait) /* Wenn nicht gewartet werden sollte, dann prÅfen, */
    if (!(cib.status & 64)) /* ob ein Zeichen gelesen werden konnte. */
      return 0;
  if (cib.ascii == (unsigned char) 0xE0)
     cib.ascii = 0;
  if (cib.ascii != 0)
    cib.scan_code = 0;
  return (short int) cib.ascii + 256 * (short int) cib.scan_code;
}
#endif

/******************************************************************************
*
* Funktion     : Cursor in Eingabe nach links (edit_left)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Falls eingegebene Zeichen angezeigt werden (echo), so wird
*                zunÑchst getestet, ob der Cursor noch eins nach links bewegt
*                werden kann. Falls nein, wird gar nichts gemacht. Sonst wird
*                der Zeiger in den Puffer dekrementiert. Ist man dadurch links
*                oben aus dem Schirm gelaufen, so wird der korrekte Teil der
*                Eingabe nochmal angezeigt und die nîtigen Variablen werden
*                angepasst (pos_first, pos_last etc.)
*
******************************************************************************/

void edit_left(w,buff_first,buff_last,pos_first,pos_last,pos_lasts,old_pos)
WINDOW *w;
char   **buff_first,**buff_last;
short int  **pos_first, **pos_last, **pos_lasts, **old_pos;
{
  int i,pos;

  if(p_in > buffer)
  {
    p_in--;
    if(echoflag)
    {
      if(p_in < *buff_first)  /* Links vom ersten sichtbaren Zeichen ? */
      {
	(*pos_first)++;
	(*pos_last)++;
	(*buff_first)--;
	inp_out(w,old_pos,buff_last,pos_lasts);
	wrefresh(w);
      }
      else
      {
	cur_left(w,1);
	place_cursor(w);
      }
    }
  }
}

/******************************************************************************
*
* Funktion     : Cursor in Eingabe nach rechts (edit_right)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_firsts  :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Es wird zunÑchst getestet, ob sich der Cursor nicht am Ende
*                des bisher eingegebenen Strings befindet. Ist dem so, dann
*                geschieht nichts. Sonst wird, falls echoflag gesetzt ist, der
*                Cursor um 1 nach rechts bewegt. Anschliessend wird p_in
*                angepasst. Wurde durch die Cursorbewegung gescrollt, wird
*                die nÑchste Zeile der Eingabe angezeigt und die Variablen
*                buff_first etc. angepasst.
*
******************************************************************************/

void edit_right(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos)
WINDOW *w;
char   **buff_first,**buff_last;
short int **pos_first, **pos_firsts, **pos_last, **pos_lasts, **old_pos;
{
  short int old_y;

  if(*p_in)  /* Geht nur, falls man nicht am Ende steht */
  {
    p_in++;
    if(echoflag)
    {
      old_y = w->_cursor_y;
      cur_right(w,1);   /* Cursor um 1 nach rechts */
      if(!w->_cursor_x && old_y == w->_width_y-1)  /* Wenn gescrollt wurde, dann in letzter Zeile */
      {                 /* korrekten Teil der Eingabe anzeigen         */
	*pos_first -= w->_width_x;  /* Erstes Zeichen eine Zeile weiter oben */
	*pos_last  -= w->_width_x;
	if(*pos_firsts > w->_image) /* Wo steht nun das erste sichtbare */
	{                         /* Zeichen der Eingabe ?            */
	  *pos_firsts -= w->_width_x;
	  if(*pos_firsts < w->_image)
	    *pos_firsts = w->_image;
	}
	*buff_first = buffer + (*pos_firsts - *pos_first);
	inp_out(w,old_pos,buff_last,pos_lasts);
	wrefresh(w);
      }
      else
	place_cursor(w);
    }
  }
}

/******************************************************************************
*
* Funktion     : Zeichen in Eingabe lîschen (edit_del)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Es wird zunÑchst getestet, ob sich der Cursor nicht am Ende
*                des bisher eingegebenen Strings befindet. Ist dem so, dann
*                geschieht nichts. Sonst wird, falls echoflag gesetzt ist, der
*                Cursor um 1 nach rechts bewegt. Anschliessend wird p_in
*                angepasst. Wurde durch die Cursorbewegung gescrollt, wird
*                die nÑchste Zeile der Eingabe angezeigt und die Variablen
*                buff_first etc. angepasst.
*
******************************************************************************/

void edit_del(w,buff_last,pos_last,pos_lasts,old_pos)
WINDOW *w;
char **buff_last;
short int **pos_last, **pos_lasts, **old_pos;
{
  if(*p_in) /* DEL geht nur, wenn man nicht hinter Eingabeende steht */
  {
    strcpy(p_in,p_in+1); /* Reststring 1 ranziehen */
    if(echoflag)
    {
      (*pos_last)--;
      inp_out(w,old_pos,buff_last,pos_lasts);
      wrefresh(w);
    }
  }
}

/******************************************************************************
*
* Funktion     : Testen ob Zeichen Worttrenner (wort_ende)
* --------------
*
* Parameter    : c           :
*                  Typ          : char
*                  Wertebereich : '\0' - '\255'
*                  Bedeutung    : zu testendes Zeichen
*
* Ergebnis     :
*                  Typ          : char *
*                  Wertebereich : NULL, ungleich NULL
*                  Bedeutung    : NULL: Kein Worttrenner; ungleich NULL: Wort-
*                                 trenner
*
* Beschreibung : Es wird mit der Funktion strchr getestet, ob sich das Åber-
*                gebene Zeichen in dem String der Worttrenner befindet. Ist
*                dem so, wird ein Pointer auf die entsprechende Stelle im
*                String der Worttrenner zurÅckgegeben, sonst wird NULL zurÅck-
*                gegeben.
*
******************************************************************************/

char wort_ende(c)
char c;
{
  return(strchr(" ,.;:#+-*[]{}|!$%&/\\()=?'`^\"",c));
}

/******************************************************************************
*
* Funktion     : Cursor in Eingabe ein Wort nach rechts (edit_ctrlright)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_firsts  :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Es wird zunÑchst getestet, ob sich der Cursor nicht am Ende
*                des bisher eingegebenen Strings befindet. Ist dem so, dann
*                geschieht nichts. Sonst wird der Cursor mit der Funktion
*                edit_right in der Eingabe um 1 nach rechts bewegt, wobei
*                der Bildschirm und alle nîtigen Variablen korrekt angepasst
*                werden. Das wird sooft wiederholt, bis das Zeichen links
*                vom Cursor ein Worttrenner ist.
*
******************************************************************************/

void edit_ctrlright(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos)
WINDOW *w;
char **buff_first,**buff_last;
short int **pos_first, **pos_firsts, **pos_last, **pos_lasts, **old_pos;
{
  if(*p_in)
    do
      edit_right(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos);
    while(*p_in && !wort_ende(p_in[-1]));
}

/******************************************************************************
*
* Funktion     : Cursor in Eingabe ein Wort nach links (edit_ctrlleft)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Es wird zunÑchst getestet, ob man nicht schon am Anfang der
*                Eingabe steht. Ist dem so, geschieht nichts. Sonst wird Åber
*                die Funktion edit_left der Cursor so oft (mindestens 1 mal)
*                nach links bewegt, bis das Zeichen links vom Cursor ein
*                Worttrenner ist. Dabei werden alle wichtigen Variablen und
*                der Bildschirm von der Funktion edit_left korrekt angepasst.
*
******************************************************************************/

void edit_ctrlleft(w,buff_first,buff_last,pos_first,pos_last,pos_lasts,old_pos)
WINDOW *w;
char **buff_first,**buff_last;
short int **pos_first, **pos_last, **pos_lasts, **old_pos;
{
  if(p_in > buffer)
    do
      edit_left(w,buff_first,buff_last,pos_first,pos_last,pos_lasts,old_pos);
    while(p_in > buffer && !wort_ende(p_in[-1]));
}

/******************************************************************************
*
* Funktion     : Cursor an Eingabeanfang (edit_home)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_firsts  :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Die Variable p_in wird auf buffer gesetzt. Liegt das erste
*                Zeichen der Eingabe momentan auaerhalb des Bildschirms, so
*                wird der String ab der linken oberen Bildschirmecke erneut
*                angezeigt und die Variablen buff_first etc. werden angepasst.
*
******************************************************************************/

void edit_home(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos)
WINDOW *w;
char **buff_first,**buff_last;
short int  **pos_first, **pos_firsts, **pos_last, **pos_lasts, **old_pos;
{
  p_in = buffer;
  if(echoflag)
  {
    if(*pos_first < *pos_firsts)
    {
      *pos_first = *pos_firsts = w->_image;
      *pos_last = *pos_first + strlen(buffer);
      *buff_first = buffer;
      wmove(w,0,0);
      inp_out(w,old_pos,buff_last,pos_lasts);
      wrefresh(w);
    }
    else
    {
      poi_to_cur(w,*pos_first);
      place_cursor(w);
    }
  }
}

/******************************************************************************
*
* Funktion     : Cursor an Eingabeende (edit_end)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_firsts  :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
* Beschreibung : Falls eingegebene Zeichen angezeigt werden, wird der String
*                ab der Position, wo der Cursor steht, angezeigt. Dadurch
*                steht danach auf dem Bildschirm der Cursor schon an der
*                richtigen Stelle. Alle Variablen (pos_first etc.) werden
*                angepasst. p_in wird an das Ende des in buffer enthaltenen
*                Strings gesetzt.
*
******************************************************************************/

void edit_end(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts)
WINDOW *w;
char **buff_first,**buff_last;
short int  **pos_first, **pos_firsts, **pos_last, **pos_lasts;
{
  int len = strlen(buffer);

  if(echoflag)
  {
    waddstr(w,p_in);
    wrefresh(w);
    *pos_last = *pos_lasts = cur_to_poi(w);
    *pos_first = *pos_firsts = *pos_last-len;
    if(*pos_firsts < w->_image)
      *pos_firsts = w->_image;
    *buff_first = buffer + (*pos_firsts - *pos_first);
  }
  *buff_last = p_in = buffer + len; /* p_in auf terminierende 0 setzen */
}

/******************************************************************************
*
* Funktion     : Edit-Tasten behandeln (treat_edit_keys)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : insert      :
*                  Typ          : char *
*                  Wertebereich : Pointer auf Character-Variable
*                  Bedeutung    : Insert-Modus
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_firsts  :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Diese Funktion wird von getch_cooked aufgerufen, wenn dort
*                '\0' eingelesen wurde. Hier wird dann das folgende Zeichen
*                eingelesen. Daraus ergibt sich normal eine Kontroll-Kombina-
*                tion. Ist das der Fall, so wird eine Funktion aufgerufen,
*                die die Kombination auswertet.
*
******************************************************************************/

void treat_edit_keys(c,w,insert,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos)
short int c;
WINDOW *w;
char   *insert,**buff_first,**buff_last;
short int  **pos_first, **pos_firsts, **pos_last, **pos_lasts, **old_pos;
{
  int *pos; /* eee nur damit Syntax in Ordnung ist */

#ifdef OS2
  switch(c)
#else
  switch(getch(TRUE)) /* warte auf Zeichen => TRUE */
#endif
  {
    case KEY_LEFT : edit_left(w,buff_first,buff_last,pos_first,pos_last,pos_lasts,old_pos); break; /* Pfeil links */
    case KEY_RIGHT: edit_right(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos); break; /* Pfeil rechts */
    case KEY_INS  : *insert ^= TRUE;       break; /* Insert togglen */
    case KEY_DEL  : edit_del(w,buff_last,pos_last,pos_lasts,old_pos); break; /* DEL */
    case KEY_WORDL: edit_ctrlleft(w,buff_first,buff_last,pos_first,pos_last,pos_lasts,old_pos);  break; /* CTRL-Pfeil links */
    case KEY_WORDR: edit_ctrlright(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos); break; /* CTRL-Pfeil rechts */
    case KEY_HOME : edit_home(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts,old_pos); break; /* HOME */
    case KEY_END  : edit_end(w,buff_first,buff_last,pos_first,pos_firsts,pos_last,pos_lasts); break; /* END */
  }
}

/******************************************************************************
*
* Funktion     : Eingabe ausgeben (inp_out)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem Eingabe angezeigt werden
*                                 soll
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Pointer auf Pointer auf short int
*                  Bedeutung    : Zeigt auf den Zeiger, der hinter das letzte
*                                 Zeichen der Eingabe auf dem Bildschirm
*                                 zeigt.
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
* Beschreibung : Die Eingabe wird ab der Position, an der der Cursor steht
*                (p_in) ausgegeben. Sollte die auszugebende Eingabe Åber das
*                Fensterende hinausgehen, so bricht die Ausgabe bei vollem
*                Fenster ab.
*                Falls dann der Cursor vor dem alten Ende der
*                Eingabe steht, die Eingabe also kÅrzer geworden
*                ist, wird der Rest der alten Eingabe auf dem Bildschirm
*                gelîscht. Anschliessend wird der Cursor wieder auf seine
*                alte Position gestellt. In der Variablen, auf die buff_last
*                zeigt, wird vermerkt, wo das letzte angezeigte Zeichen im
*                Eingabepuffer steht. In der Variablen, auf die pos_lasts
*                zeigt, wird eingetragen, wo das letzte angezeigte Zeichen
*                auf dem Bildschirm steht.
*
******************************************************************************/

void inp_out(w,old_pos,buff_last,pos_lasts)
WINDOW *w;
short int  **old_pos, **pos_lasts;
char **buff_last;
{
  short int  *help_pos, *new_pos;
  int       old_x = w->_cursor_x,old_y = w->_cursor_y;
  char      old_scroll = w->_scrflag,*p_in1 = p_in;

  w->_scrflag = FALSE; /* Kein Scrolling, damit letzte Zeile ganz genutzt wird */
  while(*p_in1 && (w->_cursor_y < w->_width_y-1 || w->_cursor_x < w->_width_x-1))
    waddch(w,*p_in1++);
  *buff_last = p_in1-1;
  *pos_lasts = cur_to_poi(w)-1;
  if(*p_in1)         /* Brach WHILE ab, weil Fenster voll ? */
  {
    *old_pos = cur_to_poi(w)+1;
    (*buff_last)++;
    (*pos_lasts)++;
    waddch(w,*p_in1); /* Ja, dann letztes Zeichen anzeigen   */
  }
  else
  {
    /* Falls neue Eingabe kÅrzer, Rest der alten lîschen */
    for(help_pos = new_pos = cur_to_poi(w) ; help_pos < *old_pos ; help_pos++)
      *help_pos = STD_ATTR*256;
    *old_pos = new_pos;
  }
  wmove(w,old_y,old_x);
  w->_scrflag = old_scroll;
}

/******************************************************************************
*
* Funktion     : Backspace beim Edieren behandeln (treat_backspace)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem evtl. Cursor bewegt werden
*                                 soll
*
*              : buff_first  :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Erstes sichtbares Zeichen in buffer
*
*              : buff_last   :
*                  Typ          : char **
*                  Wertebereich : Doppelpointer auf Zeichen in buffer
*                  Bedeutung    : Letztes sichtbares Zeichen in buffer
*
*              : pos_first   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das erste Zeichen der Eingabe
*                                 stehen wÅrde (kann kleiner als image sein)
*
*              : pos_last    :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 theoretisch das letzte Zeichen der Eingabe
*                                 stehen wÅrde (kann hinters image zeigen)
*
*              : pos_lasts   :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, wo
*                                 das letzte sichtbare Zeichen der Eingabe
*                                 steht.
*
*              : old_pos     :
*                  Typ          : short int **
*                  Wertebereich : Doppelpointer in Window-Image
*                  Bedeutung    : Zeigt auf die Stelle im Window-Image, bis wo
*                                 vor der letzten Ausgabe der Eingabe der
*                                 Text auf dem Bildschirm ging.
*
* Beschreibung : Falls mîglich, werden alle Zeichen ab der Cursorposition
*                um eine Position im Eingabepuffer nach vorne gezogen. p_in
*                wird dekrementiert. Im Normalfall wird dann Åber waddch das
*                Backspace ausgegeben, und anschlieaend der Reststring aus-
*                gegeben. Wurde jedoch ein nicht auf dem Bildschirm stehendes
*                Zeichen gelîscht, werden nur buff_first und pos_first ange-
*                passt.
*
******************************************************************************/

void treat_backspace(w,buff_first,buff_last,pos_first,pos_last,pos_lasts,old_pos)
WINDOW *w;
char **buff_first,**buff_last;
short int  **pos_first, **pos_last, **pos_lasts, **old_pos;
{
  if(p_in > buffer)  /* Falls noch ein Zeichen im Puffer, dieses lîschen */
  {
    strcpy(p_in-1,p_in); /* Reststring 1 ranziehen */
    p_in--;
    if(echoflag) /* Falls echo gesetzt, Zeichen auch auf dem Bildschirm lîschen */
      if(p_in>=*buff_first) /* Falls nicht vor erstem sichtbaren Zeichen gelîscht wurde */
      {
	waddch(w,'\b'); /* ein Zeichen lîschen */
	(*pos_last)--;
	inp_out(w,old_pos,buff_last,pos_lasts);
	wrefresh(w);
      }
      else
      {
	*buff_first = p_in; /* Jetzt kann man schon ein frÅheres Zeichen sehen */
	(*pos_first)++;     /* Stringanfang rÅckt eins auf */
      }
  }
}

/******************************************************************************
*
* Funktion     : Ein Zeichen in Puffer einfÅgen (buf_insert)
* --------------
*
* Beschreibung : In buffer werden alle Zeichen ab p_in um eins nach rechts
*                geschoben, falls dafÅr noch Platz ist.
*
******************************************************************************/

void buf_insert()
{
  char buff[BUFF_SIZE+1];

  if(strlen(buffer) == BUFF_SIZE)
    fatal("Eingabepuffer voll, kann kein Zeichen mehr einfÅgen");
  strcpy(buff,p_in);
  strcpy(p_in+1,buff);
}

/******************************************************************************
*
* Funktion     : Zeichen gepuffert einlesen (getch_cooked)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem gelesene Zeichen evtl. an-
*                                 gezeigt werden sollen.
*
* Ergebnis     :
*                  Typ          : char
*                  Wertebereich : '\0' - '\255'
*                  Bedeutung    : Eingelesenes Zeichen
*
* Beschreibung : Falls noch ein Zeichen im Eingabepuffer ist, wird es direkt
*                zurÅckgegeben. Sonst wird eine Zeile eingelesen, die mit
*                RETURN abgeschlossen werden mua. Die Zeile darf ediert werden.
*                Alle eingelesenen Zeichen werden in einen Puffer geschrieben,
*                aus dem sie hinterher in der Reihenfolge ihrer Eingabe wieder
*                ausgelesen werden kînnen. Es wird dann das erste Zeichen des
*                Puffers zurÅckgegeben.
*
******************************************************************************/

char getch_cooked(w)
WINDOW *w;
{
  short int c,old_tab = w->_tabflag,insert=FALSE;
  char *buff_first,*buff_last;
  short int  *pos_first, *pos_last, *pos_firsts, *pos_lasts;
  int  i;
  short int old_y,*old_pos;

  if(p_in != p_out) /* Falls noch ein Zeichen im Puffer, dieses zurÅckgeben */
    return(*p_out++);
  p_in = p_out = buff_first = buff_last = buffer; /* p_in statt auf Pufferende jetzt wieder auf Pufferanfang */
  pos_first = pos_last = pos_firsts = pos_lasts = old_pos = cur_to_poi(w);
  for(i=0 ; i<BUFF_SIZE ; i++)
    buffer[i]='\0';   /* Dadurch kann String-Ende leicht gefunden werden */
  w->_tabflag = FALSE; /* Tabs nicht zu Spaces expandieren */
  do
  {
    c = getch(TRUE); /* Warte auf Zeichen => TRUE */
    switch(c % 256) /* Low-Byte des gelesenen Zeichens betrachten */
    {
      case '\0':    /* Sonderfunktion, dann ist Low-Byte 0 */
	treat_edit_keys(c,w,&insert,&buff_first,&buff_last,&pos_first,&pos_firsts,&pos_last,&pos_lasts,&old_pos); /* Sondertaste lesen und evtl. ausfÅhren */
	break;
      case '\b':           /* Backspace */
	treat_backspace(w,&buff_first,&buff_last,&pos_first,&pos_last,&pos_lasts,&old_pos);
	break;
      case '\r':           /* CR als CR-LF anzeigen aber als CR eintragen */
	if(echoflag)       /* Wird dann evtl. von wgetch in NL umgewandelt */
	  waddstr(w,p_in); /* Cursor an Eingabeende bringen */
	p_in = buffer+strlen(buffer); /* '\r' immer am Ende anhÑngen */
	*p_in++ = (char) (c % 256); /* Zeichen aus dem Tastaturcode extrahieren */
	if(echoflag)
	{
	  waddstr(w,"\n\r");
	  wrefresh(w);
	}
	break;
      default:             /* Jedes andere Zeichen */
	if(insert)
	{
	  buf_insert();    /* Evtl. ein Zeichen in Puffer einfÅgen */
	  pos_last++;
	}
	*p_in++ = (char) (c % 256);       /* in Puffer eintragen */
	if(echoflag)
	{
	  old_y = w->_cursor_y;
	  waddch(w,c % 256);     /* Zeichen falls echo gesetzt ist auf dem Bildschirm anzeigen */
	  if(!w->_cursor_x && old_y == w->_width_y-1) /* Wurde gescrollt ? */
	  {
	    pos_first -= w->_width_x; /* Erstes Zeichen eine Zeile weiter oben */
	    if(pos_firsts > w->_image) /* Wo steht nun das erste sichtbare */
	    {                         /* Zeichen der Eingabe ?            */
	      pos_firsts -= w->_width_x;
	      if(pos_firsts < w->_image)
		pos_firsts = w->_image;
	    }
	    buff_first = buffer + (pos_firsts - pos_first);
	  }
	  inp_out(w,&old_pos,&buff_last,&pos_lasts); /* Rest der Eingabe anzeigen */
	  wrefresh(w);
	}
    }
  } while (c != '\r' && p_in < buffer+BUFF_SIZE);
  w->_tabflag = old_tab;
  if(p_in >= buffer+BUFF_SIZE)
    fatal("Eingabepuffer voll!");
  return(*p_out++);     /* Erstes Zeichen aus dem Puffer zurÅckgeben */
}

/******************************************************************************
*
* Funktion     : Zeichen einlesen (wgetch)
* --------------
*
* Parameter    : w           :
*                  Typ          : WINDOW *
*                  Wertebereich : Pointer auf WINDOW-Struktur
*                  Bedeutung    : Fenster, in dem eingelesenes Zeichen evtl.
*                                 angezeigt werden soll.
*
* Ergebnis     :
*                  Typ          : short int
*                  Wertebereich : '\0' - '\255'
*                  Bedeutung    : Eingelesenes Zeichen
*
* Beschreibung : Falls delay FALSE ist, wird zunÑchst getestet, ob ein
*                Zeichen eingegeben wurde oder noch eins im Puffer steht.
*                Wenn nein, wird -1 zurÅckgegeben.
*                Sonst wird, auch wenn delay TRUE ist, ein Zeichen gelesen,
*                abhÑngig von raw entweder mit der Funktion getch oder
*                getch_cooked. Ist echo und raw gesetzt, wird
*                anschliessend das eingelesene Zeichen mit waddch angezeigt.
*                getch_cooked zeigt die Zeichen bei gesetztem echo selber an.
*
******************************************************************************/

short int wgetch(w)
WINDOW *w;
{
  short int c = '\0';

  /* Aufruf von getch: warte nicht auf Zeichen => FALSE */
  if(!w->_dlyflag && p_in==p_out &&
#ifdef OS2
     !(c = getch (FALSE)))
#else
     !kbhit())
#endif
    return(-1);
  else
  {
    if(p_in != p_out) /* Falls noch ein Zeichen im Puffer, dieses zurÅckgeben */
      return(*p_out++);
    if(echoflag)  /* Falls eingegebene Zeichen angezeigt werden sollen, */
      place_cursor(w); /* Cursor an die richtige Stelle plazieren */
    if (!c) /* Wurde bei Test, ob Zeichen im Puffer, schon eins gelesen ? */
      c = rawflag ? getch(TRUE) : (short int) getch_cooked(w);
    if(c=='\r' && nlflag) /* Falls getch '\r' zurÅckliefert und das */
      c = '\n'; /* nlflag gesetzt ist, so soll doch '\n' zurÅckgegeben werden */
    if(echoflag && rawflag)
    {
      waddch(w,c);
      wrefresh(w);
    }
  }
  return(c);
}
