/****************************************************************/
/*                                                              */
/*      MODUL:  window.c                                        */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - adapt_screen (Bildschirm anpassen)            */
/*              - init_win (Fenster-Standards setzen)           */
/*              - print_hilf (Hilfstext in Statuszeile anzeigen)*/
/*              - print_stat (Text in Statuszeile ausgeben)     */
/*              - clear_stat (Statuszeile loeschen)             */
/*              - read_stat (Text in Statuszeile einlesen)      */
/*              - rahmen (Rahmen zeichnen)                      */
/*              - kopf (Kopf in Window-Rahmen schreiben)        */
/*              - open_window (Fenster oeffnen)                 */
/*              - nfr_win (naechstfreie Windownummer ermitteln) */
/*              - koppel_win (Neues Fenster in Liste haengen)   */
/*              - gb_win_frei (Fensterlistenelement freigeben)  */
/*              - pos_cursor (Cursor positionieren)             */
/*              - setz_cursor (Cursor setzen mit kopf neu)      */
/*              - sw_ohne_refresh (aktuelles Window anzeigen)   */
/*              - show_win (Aktuelles Window zeigen und refrsh) */
/*              - text_down (scrolle Window eine Zeile runter)  */
/*              - text_up (scrolle Window eine Zeile hoch)      */
/*              - text_right (Text um eine Position nach rechts)*/
/*              - text_left (Text um eine Position nach links)  */
/*              - fastcharout (Zeichen in Window ausgeben)      */
/*              - lineout (aktuelle Zeile ausgeben)             */
/*              - mal_und_merk (Ecken zeichnen und alte merken) */
/*              - cpwins2stdscr (Fenster in stdscr kopieren)    */
/*              - eckenhw (Ecken zeichnen bzw. loeschen)        */
/*              - win_right (Fenster nach rechts bewegen)       */
/*              - win_left (Fenster nach links bewegen)         */
/*              - win_up (Fenster nach oben bewegen)            */
/*              - win_down (Fenster nach unten bewegen)         */
/*              - size_right (Fenster verbreitern)              */
/*              - size_left (Fenster verschmaelern)             */
/*              - size_up (Unteren Fensterrand nach oben)       */
/*              - size_down (Unteren Fensterrand nach unten)    */
/*              - toggle_size (Gespeicherte Groesse wird akt.)  */
/*              - make_akt_win (Fenster zum aktuellen machen)   */
/*              - sw_name (Fenster gemaess Name suchen)         */
/*              - push_win_back (Fenster in Hintergrund)        */
/*              - check_and_scroll_by_one (eine Z. scrollen)    */
/****************************************************************/

#include "defs.h"

void clear_stat(), setz_cursor(int), show_win(int),
     fastcharout(int,int,char*,int), lineout (int);

extern char *fastzeichen();
extern char space;
extern WINDOW *status;
extern char helpflag,backupflag,highblockflag,def_aiflag,linebrkflag;
extern int blockattr,in_block(),def_tab;
extern short int *cur_to_poi();

/* *** globale Daten und Initialisierung *** */
static int       statx=0;     /* Cursorpositioon X im Statusfenster */
static short int anz_win = 0; /* Anzahl geoeffneter Fenster         */

/*****************************************************************************
*
*  Funktion       Bildschirm anpassen (adapt_screen)
*  --------
*
*  Paramater    : blen     :
*                   Typ         : int
*                   Wertebereich: 0-MAXINT
*                   Bedeutung   : LÑnge des rechts am Rand noch sichtbar
*                                 zu machenden Bereichs
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE: show_win wurde ausgefÅhrt
*                                  FALSE: kein show_win, nur setz_cursor
*
*  Beschreibung : ws_col und ws_line werden so angepasst, dass die aktuelle
*                 Zeile nach Moeglichkeit in der Mitte des Bildschirms steht
*                 und ein Begriff der LÑnge blen noch vor den rechten Rand
*                 passt. Stand die aktuelle Zeile schon im Fenster, so wird
*                 sie nicht in der Fenstermitte plaziert, sondern bleibt
*                 an der aktuellen Position. Das Fenster wird mit show_win
*                 neu angezeigt, falls ws_col/ws_line geÑndert wurden.
*
*****************************************************************************/

int adapt_screen(blen)
int blen;
{
  /* *** interne Daten und Initialisierung *** */

  /* Steht der Cursor schon im Fenster und zwar mindestens blen   */
  /* Zeichen vom rechten Rand entfernt, dann ist alles in Ordnung */
  /* und es wird nur der Cursor an die richtige Position gesetzt  */
  if (akt_winp->textline >= akt_winp->ws_line
  && akt_winp->textline < akt_winp->ws_line+akt_winp->dy
  && akt_winp->screencol >= akt_winp->ws_col
  && akt_winp->screencol < akt_winp->ws_col+akt_winp->dx-blen)
  {
    setz_cursor(W_AKT);
    return FALSE;
  }

  else  /* Cursor nicht im Fenster oder zu weit rechts */
  {
    /* Cursorzeile in die Mitte setzen. Falls Zeilennummer zu klein, */
    /* alles ab erste Zeile anzeigen */
    if ((akt_winp->ws_line = akt_winp->textline - akt_winp->dy/2) < 0)
      akt_winp->ws_line=0;
    /* Fensterinhalt so weit wie moeglich nach rechts. Falls */
    /* Spaltennummer zu klein, alles ab 1. Spalte anzeigen   */
    if ((akt_winp->ws_col=akt_winp->screencol-akt_winp->dx+blen)<0)
      akt_winp->ws_col = 0;
    show_win(W_AKT); /* Fensterinhalt neu anzeigen */
    return TRUE;
  }
}

/*****************************************************************************
*
*  Funktion       Fenster-Standards setzen (init_win)
*  --------
*
*  Beschreibung : Fuer das aktuelle Fenster wird Scrolling verhindert
*                 und der Fensterinhalt geloescht.
*
*****************************************************************************/

void init_win()
{
  scrollok(akt_winp->winp,FALSE); /* Nicht scrollen bei Zeilenumbruch */
  werase (akt_winp->winp);        /* Fensterinhalt loeschen           */
}

/*****************************************************************************
*
*  Funktion       Hilfstext in Statuszeile anzeigen (print_hilf)
*  --------
*
*  Parameter    : txt       :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Auszugebender Text
*
*  Beschreibung : Der Text wird in der Statuszeile ausgegeben. Enthaelt er
*                 ein in runde Klammern eingeschlossenes Zeichen, so wird
*                 dieses Zeichen mit dem Attribut A_STANDOUT versehen.
*
*****************************************************************************/

void print_hilf(txt)
register char *txt;
{
  /* *** interne Daten *** */
  register short int c; /* auszugebendes Zeichen */

  clear_stat();   /* Statusfenster loeschen */
  wmove(status,0,statx);
  while (*txt)
  {
    if (*txt == '(' && txt[1] && txt[2] == ')')
    { /* Zeichen in Klammern eingeschlossen, dann highlighten */
      c = txt[1] | 256*A_STANDOUT;
      txt+=3;
    }
    else  /* Sonst normal darstellen */
      c = *txt++;
    waddch(status,c);  /* Ins Statusfenster ausgeben */
    statx++;
  }
  wrefresh(status);
}

/*****************************************************************************
*
*  Funktion       text in statuszeile ausgeben (print_stat)
*  --------
*
*  Parameter    : txt         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : auszugebender Text
*
*  Beschreibung : In dem Fenster status wird in Spalte statx txt
*                 ausgegeben.  Die Variable statx wird um die Laenge des aus-
*                 gegebenen Textes erhoeht.
*
*****************************************************************************/

void print_stat(txt)
char *txt;
{
  mvwaddstr(status,0,statx,txt);
  statx+=strlen(txt); /* Spaltenzaehler anpassen */
  wrefresh(status);
}

/*****************************************************************************
*
*  Funktion       Statuszeile loeschen (clear_stat)
*  --------
*
*  Beschreibung : Das Fenster status wird geloescht und statx auf 0 gesetzt.
*                 In der Statuszeile werden anschliessend die globalen Flags
*                 angezeigt.
*
*****************************************************************************/

void clear_stat()
{
  if (statx) /* Nur wenn schon etwas drinsteht */
  {
    statx = 0;        /* Spaltenzaehler loeschen */
    werase (status);
    wrefresh(status);
  }
}


/*****************************************************************************
*
*  Funktion       Text in Statuszeile einlesen (read_stat)
*  --------
*
*  Parameter    : input         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf Speicherbereich
*                   Bedeutung    : Bereich, in dem eingegebener Text gespei-
*                                  chert wird
*                 len           :
*                   Typ          : int
*                   Wertebereich : 0 - MAXLENGTH
*                   Bedeutung    : Anzahl einzulesender Zeichen
*
*                 typ           :
*                   Typ          : int
*                   Wertebereich : GS_NUM, GS_ANY
*                   Bedeutung    : GS_NUM: nur Ziffern einlesen
*                                  GS_ANY: alle Zeichen zulassen
*
*  Beschreibung : In der Statuszeile wird ein Text mit max. len Zeichen
*                 Laenge eingelesen.
*
*****************************************************************************/

void read_stat(input,len,typ)
char *input;
int len,typ;
{
  newgetstr(status,0,statx,input,len,typ);
  statx += strlen(input); /* Spaltenzaehler anpassen */
}


/*****************************************************************************
*
*  Funktion       rahmen zeichnen (rahmen)
*  --------
*
*  Parameter    : modus         :
*                   Typ          : int
*                   Wertebereich : W_AKT, W_NOTAKT
*                   Bedeutung    : Angabe, ob Fenster, fÅr das der Rahmen
*                                  gezeichnet werden soll, das aktuelle ist
*                                  oder nicht
*
*  Beschreibung : In das aktuelle Fenster wird rechts und links ein Rand
*                 gezeichnet.
*
*****************************************************************************/

void rahmen(modus)
{
  /* *** interne Daten und Initialisierung *** */
  register int i,                 /* Schleifenzaehler    */
	       bx=akt_winp->dx+1, /* Breite des Fensters */
	       by=akt_winp->dy+1; /* Hoehe des Fensters  */

  if(akt_winp->next != akt_winp) /* Nur zeichnen, falls aktuelles Fenster */
  {                              /* nicht das Dummy-Fenster ist. */
    if(modus == W_AKT)
      wstandout(akt_winp->winp); /* Falls aktuelles Fenster, dann highlighten */
    else
      wstandend(akt_winp->winp);
    mvwaddch(akt_winp->winp,1,bx,'');
    mvwaddch(akt_winp->winp,by-1,bx,'');
    for(i=1;i<by;i++)
    {
      mvwaddch(akt_winp->winp,i,0,BORDER_VER);
      mvwaddch(akt_winp->winp,i,bx,BORDER_VER);
    }
    mvwaddch(akt_winp->winp,1,bx,'');
    mvwaddch(akt_winp->winp,by-1,bx,'');
    if(modus == W_AKT)
      wstandend(akt_winp->winp); /* Wurde gehighlightet, dann Highlighten wieder abschalten */
  }
}


/*****************************************************************************
*
*  Funktion       Kopf in Window-Rahmen schreiben (kopf)
*  --------
*
*  Parameter    : modus         :
*                   Typ          : int
*                   Wertebereich : W_AKT, W_NOTAKT
*                   Bedeutung    : Angabe, ob Fenster, fÅr das der Kopf
*                                  gezeichnet werden soll, das aktuelle ist
*                                  oder nicht
*
*  Beschreibung : In das aktuelle Fenster werden in die oberste (Rahmen) und
*                 in die unterste Zeile Informationen geschrieben. Die Farbe,
*                 in der das geschieht, hÑngt vom Modus des Fensters ab.
*
*****************************************************************************/

void kopf(modus)
int modus;
{
  /* *** interne Daten und Initialisierung *** */
  char         tbuff[BUFFSIZE];   /* Zum Zusammensetzen des Kopf-/Fusszeile */
  register int i,                 /* Schleifenzaehler                       */
	       bx=akt_winp->dx+1, /* Fensterbreite                          */
	       by=akt_winp->dy+1; /* Fensterhoehe                           */
  static char  kl_template [BUFFSIZE], /* Schablonen zum Erstellen der */
	       kf_template [BUFFSIZE], /* Kopf- und Fu·zeilen          */
	       f_template  [BUFFSIZE],
	       init = FALSE;      /* gibt an, ob Schablonen initialisiert   */

  if (!init)
  {
    init = TRUE;
    sprintf(kf_template,"%s %%5d%c%s %%5d%c%%s%c%%s%c%%s%c%%s%c%%s%c%%s",
      PROMPT_LINE, BORDER_HOR, PROMPT_COLUMN, BORDER_HOR,
      BORDER_HOR, BORDER_HOR, BORDER_HOR, BORDER_HOR, BORDER_HOR);
    sprintf(kl_template,"%c%c%c%c %s %c%c%c%c%%s%c%%s%c%%s%c%%s%c%%s%c%%s",
      BORDER_HOR, BORDER_HOR, BORDER_HOR, BORDER_HOR, PROMPT_WINDOWEMP,
      BORDER_HOR, BORDER_HOR, BORDER_HOR, BORDER_HOR, 
      BORDER_HOR, BORDER_HOR, BORDER_HOR, BORDER_HOR, BORDER_HOR);
    sprintf(f_template,"%c%%d. %s %%s%c%c%%s%c%c%%s",
      BORDER_HOR, PROMPT_WINDOW, BORDER_HOR, BORDER_HOR, BORDER_HOR, 
      BORDER_HOR, BORDER_HOR, BORDER_HOR);
  }

  if(akt_winp->next != akt_winp) /* Nur zeichnen, falls aktuelles Fenster */
  {                              /* nicht das Dummy-Fenster ist. */
    if(modus == W_AKT)
      wstandout(akt_winp->winp); /* Bei aktuellen Fenster Rahmen highlighten */
    else
      wstandend(akt_winp->winp);
    mvwaddch (akt_winp->winp,0,bx,''); /* rechte obere Ecke */
    mvwaddch (akt_winp->winp,0,0,'');  /* linke obere Ecke  */
    mvwaddch(akt_winp->winp,by,bx-1,''); /* rechte untere Ecke */
    mvwaddch(akt_winp->winp,by,bx,'\\'); /* rechte untere Ecke */
    mvwaddch(akt_winp->winp,by,0,CORNER_LL);  /* linke untere Ecke  */
    mvwaddch(akt_winp->winp,by,1,'');  /* linke untere Ecke  */
    if (akt_winp->maxline >= 0) /* Fenster nicht leer ? */
      sprintf(tbuff,kf_template,
      akt_winp->textline+1, akt_winp->screencol+1,
      akt_winp->insflag?PROMPT_INSERT:PROMPT_OVERWRITE,
      akt_winp->underflag?PROMPT_UNDERLINE:"",
      akt_winp->autoindflag?"Indent":"",
      akt_winp->shellflag?"SHELL":"",
      akt_winp->tabflag?"Tabs":"Spcs",
      akt_winp->linebreak?PROMPT_LINEBREAK:"");
    else                        /* Fenster leer */
      sprintf(tbuff,kl_template, akt_winp->insflag?PROMPT_INSERT:
	PROMPT_OVERWRITE, akt_winp->underflag?PROMPT_UNDERLINE:"", 
	akt_winp->autoindflag?"Indent":"",
	akt_winp->shellflag?"SHELL":"", akt_winp->tabflag?"Tabs":"Spcs",
	akt_winp->linebreak?PROMPT_LINEBREAK:"");
    tbuff[akt_winp->dx] = '\0'; /* Zeile abschneiden, damit sie nicht ueber */
				/* den Rand hinausgeht. */
    /* Zeile zentriert in erster Fensterzeile anzeigen */
    mvwaddstr(akt_winp->winp,0,1+(akt_winp->dx - strlen(tbuff))/2,tbuff);
    /* Rest des Rahmens in Kopfzeile */
    for (i=1;i<1+(akt_winp->dx - strlen(tbuff))/2;i++)          
      mvwaddch(akt_winp->winp,0,i,BORDER_HOR);
    for (i=1+(akt_winp->dx - strlen(tbuff))/2+strlen(tbuff); i<bx;i++)
      mvwaddch(akt_winp->winp,0,i,BORDER_HOR);

    /* Fusszeile zusammenbasteln */
    sprintf(tbuff,f_template, akt_winp->wini, akt_winp->filename, 
      akt_winp->read_only?PROMPT_WRTPROTCT:"",
      akt_winp->changeflag?PROMPT_MODIFIED:"");
    tbuff[akt_winp->dx-1]='\0'; /* s.o. */
    mvwaddstr(akt_winp->winp,by,2,tbuff);
    /* Rest des Rahmens in Fusszeile */
    for(i=2+strlen(tbuff);i<bx-1;i++)           
      waddch(akt_winp->winp,BORDER_HOR);
    if(modus == W_AKT)
      wstandend(akt_winp->winp); /* Falls Kopf gehighlightet wurde, Highlighten wieder aus */
  }
}

/*****************************************************************************
*
*  Funktion       Fenster oeffnen (open_window)
*  --------
*
*  Beschreibung : Das aktuelle Fenster, fuer das schon Speicher alloziert
*                 sein muss, wird initialisiert : Die (Block-)Koordinaten,
*                 Cursorposition, Fensterstart und Fenstergroesse werden
*                 gesetzt, das Fenster wird durch Curses angelegt, der Inhalt
*                 geloescht und eingerahmt. Anschliessend wird der Kopf in
*                 die oberste Zeile und die Fusszeile in die unterste Zeile
*                 geschrieben.
*
*****************************************************************************/

void open_window()
{
  /* Fenster mit Curses anlegen */
  akt_winp->winp = newwin(START_HEIGHT,START_WIDTH,START_Y,START_X);

  akt_winp->ws_line = 0; /* Ab erster Zeile anzeigen */
  akt_winp->ws_col = 0;  /* Ab erster Spalte anzeigen */
  akt_winp->ax = akt_winp->x = START_X; /* Fensterposition und */
  akt_winp->ay = akt_winp->y = START_Y; /* alte Fensterposition setzen */
  akt_winp->adx = akt_winp->dx = START_WIDTH-2;  /* Fenstergroesse und alte */
  akt_winp->ady = akt_winp->dy = START_HEIGHT-2; /* Fenstergroesse setzen   */

  akt_winp->autoindflag = def_aiflag;
  akt_winp->shellflag = FALSE; /* Default: kein Shellmodus,   */
  akt_winp->underflag = FALSE; /* keine Unterstreichung       */ 
  akt_winp->insflag = TRUE;    /* Insert-Mode                 */ 
  akt_winp->tabflag = TRUE;    /* Blanks zu Tabs kompr.       */ 
  akt_winp->tablen = def_tab;  /* Tablaenge setzen            */ 
  akt_winp->linebreak = linebrkflag;

  akt_winp->lastline = akt_winp->lastcol = -1; /* letzte Position setzen */
  akt_winp->block.e_line = akt_winp->block.s_line =  /* kein Block */
  akt_winp->block.laenge = -1;
  akt_winp->block.bstart = (bzeil_typ*) NULL;
  init_win();  /* Curses-Defaults fuer Fenster setzen */
  show_win(W_AKT);  /* Fensterinhalt anzeigen */
}


/*****************************************************************************
*
*  Funktion      naechstfreie Windownummer ermitteln (nfr_win)
*  --------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : 0-maxint
*                   Bedeutung    : 0 = keine Windownummern mehr frei
*                                  1 - maxint = freie Windownummer
*
*  Beschreibung : Die Windowliste wird durchlaufen und nach noch nicht
*                 vergebenen Nummern durchsucht.
*
*****************************************************************************/

int nfr_win()
{
  /* *** interne Daten *** */
  register int     i=0; /* Laufende Fensternummer */
  register win_typ *wp; /* Zeiger zum Durchlaufen der Fensterliste */

  while (++i)
  {
    wp = akt_winp->next->next; /* Erstes Element hinter dummy-Element */
    while (wp != akt_winp->next && wp->wini != i)
      wp = wp->next; /* Ganze Liste nach Nummer i absuchen */
    if (wp == akt_winp->next)
      return (i); /* Nummer nicht gefunden, also zurueckgeben */
  }
  return (0); /* 0 : Keine Fensternummer mehr frei */
}


/*****************************************************************************
*
*  Funktion       Fensterelement allozieren und in Liste koppeln (koppel_win)
*  --------
*
*  Ergebnis     : Typ           : int
*                 Wertebereich  : TRUE, FALSE
*                 Bedeutung     : TRUE : Es gab noch freie Fensternummern
*                                 FALSE : Keine Nummer mehr frei
*
*  Beschreibung : Fuer ein neues Element vom Typ win_typ wird Speicher allo-
*                 ziert.  Das neue Element wird dann in die Windowliste hinter
*                 akt_winp eingehaengt.  Das neue Fenster wird anschliessend
*                 zum aktuellen Fenster.
*                 Achtung: Es wird keine Initialisierung der Struktur
*                 vorgenommen.
*
*****************************************************************************/

int koppel_win()
{
  /* *** interne Daten *** */
  register win_typ *hilf;   /* Temp. Zeiger auf neues Fenster */
  int              nfr_num; /* Nummer des neuen Fensters      */

  /* Testen, ob noch ein Fenster geoeffnet werden kann    */
  /* Klappt nicht, falls keine Fensternummer mehr frei    */
  if (nfr_num = nfr_win())
  {
    anz_win++; /* Ein Fenster mehr geoeffnet */
    hilf = (win_typ*) reserve_mem (sizeof (win_typ));
    hilf->winp = (WINDOW*) NULL; /* Damit z.B. print_err erkennt, da· */
				 /* noch kein curses-Fenster zugeordnet ist */
    hilf->next = akt_winp->next; /* Fenster hinter aktuellem */
    hilf->prev = akt_winp; /* Fenster in Liste einhaengen    */
    akt_winp->next = hilf;
    hilf->next->prev = hilf;
    hilf->wini = nfr_num; /* Naechste freie Windownummer einsetzen */
    hilf->winp = NULL; /* Noch kein Curses-Fenster, wird von print_err erkannt */
    akt_winp = hilf; /* Neues Fenster zum aktuellen machen */
    return (TRUE);
  }
  else
    return(FALSE);
}


/*****************************************************************************
*
*  Funktion       Fensterlistenelement freigeben  (gb_win_frei)
*  --------
*
*  Beschreibung : Das Element wird aus der Liste herausgenommen und der dazu-
*                 gehoerige Speicher freigegeben.
*
*****************************************************************************/

void gb_win_frei ()
{
  /* *** interne Daten *** */
  register win_typ *hilf; /* Zeiger auf momentan aktuelles Fenster */

  if ((hilf = akt_winp)->next != hilf) /* Mind. ein Fenster ? */
  {
    anz_win--; /* Ein Fenster weniger geoeffnet */
    line_free(hilf->filename); /* Filenamentext freigeben */
    akt_winp = hilf->prev;     /* Vorgaenger zum aktuellen machen */
    hilf->next->prev = akt_winp; /* Fenster aus Liste auskoppeln  */
    akt_winp->next = hilf->next;
    free (hilf); /* Speicherplatz fuer Fensterstruktur freigeben */
  }
}

/*****************************************************************************
*
*  Funktion       cursor positionieren (pos_cursor)
*  --------
*
*  Beschreibung : Cursor wird im aktuellen Fenster an die durch
*                 screencol und textline angegebene Position gesetzt.
*                 Ist der Text leer, wird der Cursor in die erste
*                 Zeile gesetzt.
*
*****************************************************************************/

void pos_cursor()
{
  wmove (akt_winp->winp,(akt_winp->maxline == -1)+ /* leer, dann 1 Z. weiter */
	 akt_winp->textline-akt_winp->ws_line+1,   /* runter */
	 akt_winp->screencol-akt_winp->ws_col+1);
  wrefresh (akt_winp->winp);
}

/*****************************************************************************
*
*  Funktion       cursor setzen (setz_cursor)
*  --------
*
*  Parameter    : modus         :
*                   Typ          : int
*                   Wertebereich : W_AKT, W_NOTAKT
*                   Bedeutung    : Angabe, ob Fenster das aktuelle ist und
*                                  somit der Kopf gehighlightet werden soll
*
*  Beschreibung : Cursor wird im aktuellen Fenster an die durch
*                 screencol und textline angegebene Position gesetzt.
*                 Der Kopf wird aktualisiert und das window refresht.
*
*****************************************************************************/

void setz_cursor(modus)
int modus;
{
  if(akt_winp->next != akt_winp) /* Nur, wenn ein Fenster offen */
  {
    kopf(modus);  /* Kopf- und Fusszeile anzeigen */
    pos_cursor(); /* Cursor positionieren */
  }
}

/*****************************************************************************
*
*  Funktion       Aktuelles Window anzeigen (sw_ohne_refresh)
*  --------
*
*  Parameter    : modus         :
*                   Typ          : int
*                   Wertebereich : W_AKT, W_NOTAKT
*                   Bedeutung    : Angabe, ob Fenster das aktuelle ist und
*                                  somit der Rahmen gehighlightet werden soll
*
*  Beschreibung : Mit der Zeile akt_winp->ws_line beginnend werden
*                 akt_winp->dy Zeilen in dem aktuellen Fenster, bei der
*                 obersten Zeile beginnend, ausgegeben.
*                 Gibt es keine weiteren Textzeilen, so wird der Rest
*                 des Fenster geloescht
*
*****************************************************************************/

void sw_ohne_refresh(modus)
int modus;
{
  /* *** interne Daten und Initialisierung *** */
  zeil_typ     *alte_zeile = akt_winp->alinep;   /* Zeiger auf Cursorzeile  */
  register int i,                                /* Zaehler fuer Zeilen     */
	       alte_nummer = akt_winp->textline; /* Zeiger auf Zeilennummer */

  gotox(akt_winp->ws_line); /* erste anzuzeigende Zeile anspringen */

  for(i=0;i<akt_winp->dy;i++)
  {
    lineout(i);    /* Zeile ausgeben */
    if(!down())    /* eine Zeile runter */
    {              /* geht das nicht, dann Rest des Bildschirms loeschen */
      wmove (akt_winp->winp, i+2, 1);
      wclrtobot(akt_winp->winp);
      break;
    }
  }
  rahmen(modus); /* Rahmen anzeigen */
  akt_winp->alinep = alte_zeile;    /* Cursorzeile restaurieren */
  akt_winp->textline = alte_nummer;
}


/*****************************************************************************
*
*  Funktion       Aktuelles Window anzeigen und refreshen (show_win)
*  --------
*
*  Parameter    : modus         :
*                   Typ          : int
*                   Wertebereich : W_AKT, W_NOTAKT
*                   Bedeutung    : Angabe, ob Fenster das aktuelle ist und
*                                  somit der Rahmen gehighlightet werden soll
*
*  Beschreibung : Mit der Zeile akt_winp->ws_line beginnend werden
*                 akt_winp->dy Zeilen in dem aktuellen Fenster, bei der
*                 obersten Zeile beginnend, ausgegeben.
*                 Gibt es keine weiteren Textzeilen, so wird der Rest
*                 des Fenster geloescht
*
*****************************************************************************/

void show_win(modus)
int modus;
{
  sw_ohne_refresh(modus);            /* Fensterinhalt anzeigen */
  setz_cursor(modus);                /* setz_cursor ruft refresh auf */
}

/*****************************************************************************
*
*  Funktion       scrolle window um eine zeile nach unten (text_down)
*  --------
*
*  Parameter    : line         :
*                   Typ          : int
*                   Wertebereich : 0 - akt_winp->dy-1
*                   Bedeutung    : Einzufuegende Zeile im Fenster
*
*  Beschreibung : Es wird in der <line>ten Zeile eine Zeile im Fenster ein-
*                 gefuegt. Falls die <line>te Bildschirmzeile exisitert, wird
*                 sie in dieser Zeile angezeigt.
*                 ws_line wird _a_u_f_ _k_e_i_n_e_n_ _F_a_l_l angepasst.
*
*****************************************************************************/

void text_down(line)
int line;
{
  /* *** interne Daten *** */
  zeil_typ  *alte_zeile; /* Zwischenspeicher Zeiger auf Cursorzeile */
  int       alte_nummer; /* Zwischenspeicher fuer Zeilennummer      */
  short int rc;          /* Zeichen zum Rahmenrestaurieren          */

  if(line == 0)
    rc = REST_ARR_UP;
  else
    if(line == akt_winp->dy-1)
      rc = REST_ARR_DN;
    else
      rc = REST_CHAR;
  wmove(akt_winp->winp,line+1,1); /* Cursor auf uebergebene Zeile stellen */
  winsertln(akt_winp->winp);      /* Eine Zeile im Fenster einfuegen */

  if (akt_winp->ws_line+line <= akt_winp->maxline)
  { /* Wenn gewuenschte Zeile existiert: */
    alte_zeile = akt_winp->alinep;    /* aktuelle Zeile merken */
    alte_nummer = akt_winp->textline;
    gotox(akt_winp->ws_line+line);    /* gewuenschte Zeile anspringen */
    lineout(line);                          /* output current line */
    akt_winp->alinep = alte_zeile;    /* Zeilenzeiger restaurieren */
    akt_winp->textline = alte_nummer;
  }
  mvwaddch(akt_winp->winp,line+1,0,REST_CHAR); /* Rahmen reparieren, da durch   */
  mvwaddch(akt_winp->winp,line+1,akt_winp->dx+1,rc); /* insertln korrupt */
  if(line != akt_winp->dy-1) /* Falls nicht in letzter Zeile eingefÅgt, */
    mvwaddch(akt_winp->winp,akt_winp->dy,akt_winp->dx+1,REST_ARR_DN);
  if(akt_winp->dy > 2) /* Falls Fenster mehr als 1 Zeile hat, gescrollten */
    /* Pfeil durch Rahmenzeichen ersetzen. */
    mvwaddch(akt_winp->winp,2,akt_winp->dx+1,REST_CHAR);
}

/*****************************************************************************
*
*  Funktion       scrolle Windowinhalt um eine Zeile nach oben (text_up)
*  --------
*
*  Parameter    : line         :
*                   Typ          : int
*                   Wertebereich : 0 - akt_winp_dy-1
*                   Bedeutung    : Zu loeschende Zeile im Fenster
*
*  Beschreibung : Falls dadurch das Fenster nicht leer wuerde, wird die
*                 <line>te Textzeile des Windows geloescht, so dass
*                 die letzte Zeile frei wird.  In dieser wird die neue
*                 letzte Zeile angezeigt.
*
*****************************************************************************/

void text_up(line)
int line;
{
  /* *** interne Daten *** */
  zeil_typ *alte_zeile; /* Zwischenspeicher Zeiger auf Cursorzeile */
  int      alte_nummer; /* Zwischenspeicher fuer Zeilennummer      */

  if(line || akt_winp->ws_line <= akt_winp->maxline)
  {
    wmove(akt_winp->winp,line+1,1); /* Cursor in gewuenschte Zeile */
    wdeleteln(akt_winp->winp);      /* Zeile im Fenster loeschen   */
    alte_zeile = akt_winp->alinep;  /* Cursorzeile merken          */
    alte_nummer = akt_winp->textline;
    gotox(akt_winp->ws_line+akt_winp->dy - 1); /* Zur <line>ten Zeile */
    /* Falls die letzte Fensterzeile nicht existiert, Zeilenzeiger */
    /* auf dummy-Element setzen, da dessen Text-Pointer NULL ist.  */
    if(akt_winp->textline != akt_winp->ws_line + akt_winp->dy - 1)
      akt_winp->alinep = akt_winp->dummyp;
    lineout(akt_winp->dy - 1);   /* output current line */
    akt_winp->alinep = alte_zeile;    /* Cursorzeile restaurieren */
    akt_winp->textline = alte_nummer;
    mvwaddch(akt_winp->winp,akt_winp->dy,0,REST_CHAR); /* Rahmen reparieren */
    mvwaddch(akt_winp->winp,akt_winp->dy,akt_winp->dx+1,REST_ARR_DN);
    mvwaddch(akt_winp->winp,1,akt_winp->dx+1,REST_ARR_UP);
    if(akt_winp->dy > 2) /* Falls Fenster mehr als 1 Zeile hat, gescrollten */
      /* Pfeil durch Rahmenzeichen ersetzen. */
      mvwaddch(akt_winp->winp,akt_winp->dy-1,akt_winp->dx+1,REST_CHAR);
  }
}

/*****************************************************************************
*
*  Funktion       Text um eine Position nach rechts (text_right)
*  --------
*
*  Beschreibung : alinep wird auf die erste Zeile im Fenster gesetzt.
*                 Dann wird vor jede Zeile ein neuer Character eingefuegt.
*                 Dadurch verschiebt sich der Restliche Text im Fenster
*                 um eine Position nach rechts, wobei das am weitesten
*                 rechts stehende Zeichen "aus dem Fenster faellt".
*
*****************************************************************************/

void text_right()
{
  /* *** interne Daten *** */
  zeil_typ     *alte_zeile; /* Zwischenspeicher Zeiger auf Cursorzeile */
  register int i,           /* Zaehler fuer Bildschirmzeile            */
	       alte_nummer; /* Zwischenspeicher fuer Zeilennummer      */

  if (akt_winp->ws_col > 0) /* Nur, wenn man noch scrollen kann */
  {
    check_buff(); /* evtl. Pufferinhalt in Text kopieren */
    alte_zeile = akt_winp->alinep;    /* Cursorzeile merken */
    alte_nummer = akt_winp->textline;
    akt_winp->ws_col--; /* Nummer der ersten sichtbaren Spalte reduzieren */
    gotox (akt_winp->ws_line); /* Zur ersten sichtbaren Zeile springen */
    for (i=0;i<akt_winp->dy && akt_winp->ws_line+i <= akt_winp->maxline;i++)
    { /* Von allen sichbaren Zeilen das erste sichtbare Zeichen einfuegen */
      fastcharout(i,0,fastzeichen(akt_winp->ws_col),INSERT);
      down(); /* Zur naechsten Zeile gehen */
    }
    akt_winp->alinep = alte_zeile;    /* Cursorzeile restaurieren */
    akt_winp->textline = alte_nummer;
  }
}


/*****************************************************************************
*
*  Funktion       scrolle Windowinhalt um eine Position nach links (text_left)
*  --------
*
*  Beschreibung : Der Windowinhalt wird um eine Position nach links gescrollt,
*                 so dass rechts neue Zeichen angezeigt werden muessen.
*
*****************************************************************************/

void text_left()
{
  /* *** interne Daten *** */
  zeil_typ     *alte_zeile; /* Zwischenspeicher Zeiger auf Cursorzeile */
  register int i,           /* Zaehler fuer Bildschirmzeile            */
	       alte_nummer; /* Zwischenspeicher fuer Zeilennummer      */
  short int    rc;          /* Zeichen zum Reparieren des Rahmens      */

  if(akt_winp->ws_col < MAXLENGTH)      /* dann nichts mehr auf dem Schirm */
  {
    alte_zeile = akt_winp->alinep;    /* Cursorzeile merken */
    alte_nummer = akt_winp->textline;
    akt_winp->ws_col++; /* Nummer der ersten sichtbaren Spalte erhoehen */
    gotox(akt_winp->ws_line); /* erste sichtbare Zeile anspringen */
    for (i=0;i<akt_winp->dy && akt_winp->ws_line+i-1 < akt_winp->maxline;i++)
    { /* In allen Fensterzeilen erstes Zeichen loeschen */
      mvwdelch(akt_winp->winp,i+1,1);
      fastcharout(i,akt_winp->dx - 1, /* Zeichen in letzer Spalte anzeigen */
		  fastzeichen(akt_winp->ws_col + akt_winp->dx-1),PUT);
      if(i == 0)
	  rc = REST_ARR_UP;
	else
	  if(i == akt_winp->dy-1)
	    rc = REST_ARR_DN;
	  else
	    rc = REST_CHAR;
      mvwaddch(akt_winp->winp,i+1,akt_winp->dx+1,rc);
      down(); /* Rahmen reparieren und zur naechsten Zeile gehen */
    }
    akt_winp->alinep = alte_zeile;    /* Cursorzeile restaurieren */
    akt_winp->textline = alte_nummer;
  }
}

/******************************************************************************
*
* Funktion     : Attribute zum Zeichen hinzufÅgen (add_attrs)
* --------------
*
* Parameter    : ib          :
*                  Typ          : int
*                  Wertebereich : Siehe Ergebnis von in_block
*                  Bedeutung    : Angabe, ob das Zeichen im Block liegt
*
*              : s           :
*                  Typ          : char *
*                  Wertebereich : Zeiger aus ASCII-Zeichenkette
*                  Bedeutung    : Zeiger auf auszugebendes Zeichen
*
* Ergebnis     :
*                  Typ          : short int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : Zeichen mit gesetzten Attributen
*
* Beschreibung : Zeigt s auf ein unterstrichenes Zeichen, so wird das
*                Attribut fÅr Unterstreichung zum Ergebnis hinzugeodert.
*                Handelt es sich um einen Controlcode, so wird das Attribut
*                fÅr Invertierung dazugesetzt. Liegt das Zeichen an einer
*                Position, die sich im Block befindet, und sollen Blîcke
*                hervorgehoben werden, dann wird das Blockhighlight-Attribut
*                dazugeodert.
*                Wurde kein Attribut hinzugefÅgt, so wird das Standardattribut
*                dazugeodert.
*
******************************************************************************/

short int add_attrs(ib,s)
int ib;
char *s;
{
  unsigned short int c;  /* Zur Aufbereitung des RÅckgabewertes */
  unsigned int vf, /* Vordergrundfarbe */
	       hf; /* Hintergundfarbe */

  if(*s == '_' && s[1] == '' && s[2]) /* unterstrichenes Zeichen? */
    c = s[2] | (256*A_UNDERLINE);
  else
    c = *s;

  /* Wenn Zeichen im Block, dann highlighten */
  if(highblockflag && ib & B_IN_BLOCK)
    c |= 256*blockattr;   /* Controlcodes werden wieder normal dargestellt */
#ifdef OWN_CURSES
  if(!(c >> 8)) /* Keine Attribute dazugeodert ? */
    c |= 256*STD_ATTR; /* Dann Standardattribut hinzufÅgen */
#endif

  if((c & 255) < 28) /* Controlcodes invertieren */
  {
    vf = c & 3840;  /* Vordergundfarbe ist c & (256*15) */
    hf = c & 61440; /* Hintergrundfarbe ist c & (256*240) */
    /* Vorder- und Hintergrund vertauschen  und 'A' zum Zeichen addieren */
    c = (c & 255) | (vf<<4) | (hf>>4) + 64;
  }
  return (c);
}

/*****************************************************************************
*
*  Funktion       Zeichen in window ausgeben (fastcharout)
*  --------
*
*  Parameter:     y         :
*                   Typ          : int
*                   Wertebereich : 0 - Fensterhoehe-1
*                   Bedeutung    : Zeile des Fensters, in dem das Zeichen
*                                  ausgegeben werden soll
*
*                 x         :
*                   Typ          : int
*                   Wertebereich : 0 - Fensterbreite-1
*                   Bedeutung    : Spalte des Fensters, in der das Zeichen
*                                  ausgegeben werden soll
*
*                 s         :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichen
*                   Bedeutung    : auszugebendes Zeichen (s. fastzeichen())
*
*                 mode      :
*                   Typ          : int
*                   Wertebereich : INSERT,PUT
*                   Bedeutung    : Gibt an, ob das Zeichen eingefuegt oder ob
*                                  es an die betreffende Stelle kopiert werden
*                                  soll
*
*  Beschreibung : Ist das Zeichen ein unterstrichenes Zeichen, so wird es
*                 unterstrichen ausgegeben; ist der ASCII-Code des Zeichens
*                 kleiner als 28, so wird der Zeichencode um 64 erhoeht und
*                 das Zeichen invers ausgegeben.
*                 Bei den Positionen wird der Rahmen beruecksichtigt, er kann
*                 jedoch im Modus INSERT beschaedigt werden.
*
*****************************************************************************/

void fastcharout(y,x,s,mode)
int y,x,mode;
char *s;
{
  /* *** interne Daten und Initialisierung *** */
  register short int c=0; /* Auszugebendes Zeichen     */
  int                ib;  /* Flag, ob Zeichen im Block */
  short int          rc;  /* Zeichen zum Restaurieren des Rahmens */

  ib = in_block(y+akt_winp->ws_line,x+akt_winp->ws_col);
  c = add_attrs(ib,s); /* Attribute korrekt setzen */

  if(mode & INSERT)
  {
    if(highblockflag && (ib & B_LINE) && !(ib & B_RIGHT)) /* Falls Zeichen in */
    /* einer Blockzeile vor oder im Block muss ganze Zeile neu gezeichnet     */
      lineout(y);         /* werden, da sich die Markierungen verschieben     */
    /* lineout ruft zwar fastcharout auf, jedoch mit dem Modus PUT, also      */
    /* keine Endlos-Rekursion */
    else  /* Wenn Zeichen nicht im Block, dann einfuegen und */
    {     /* Rahmen reparieren */
      if(y == 0)
	rc = REST_ARR_UP;
      else
	if(y == akt_winp->dy-1)
	  rc = REST_ARR_DN;
	else
	  rc = REST_CHAR;
      mvwinsch(akt_winp->winp,y+1,x+1,c);
      mvwaddch(akt_winp->winp,y+1,akt_winp->dx+1,rc);
    }
  }
  else   /* Bei Modus PUT einfach anzeigen */
    mvwaddch(akt_winp->winp,y+1,x+1,c);
}

/*****************************************************************************
*
*  Funktion       aktuelle zeile ausgeben (lineout)
*  --------
*
*  Parameter    : y         :
*                   Typ          : int
*                   Wertebereich : 0 - akt_winp->dy-1
*                   Bedeutung    : Fensterzeile in der Text ausgegeben werden
*                                  soll (Rahmen wird beruecksichtigt)
*
*  Beschreibung : Die aktuelle Zeile wird in dem aktuellen Fenster in Zeile
*                 y ausgegeben. Hat die Zeile weniger Zeichen als das Window
*                 breit ist, so wird der Rest der Zeile geloescht.
*                 Dies geschieht, wenn fastzeichen einen Zeiger auf space
*                 zurueckliefert.
*                 Die Zeichen werden unmittelbar ins Window-Image geschrieben,
*                 wodurch erst ein Refresh die gewollten Ergebnisse
*                 liefert (zu bedenken bei Portierung!)
*                 Die anschlie·ende Position des Cursors ist nicht definiert.
*
*****************************************************************************/

#ifdef CUR_DIRECT
void lineout(y)
int y;
{
  /* *** interne Daten *** */
  int           col;    /* aktuelle Fensterspalte */
  register char *s,     /* auszugebendes Zeichen  */
		*zeile; /* Aktuelles Zeichen      */
  unsigned short int *scr,   /* Puffer fÅr anzuzeigende Zeichen */
		 *scr_p,     /* Zeiger in scr */
		 *scr_image, /* Zeiger ins Screen-image des Fensters */
		 rc;         /* Zeichen zum Reparieren des Rahmens */

  /* Platz fÅr 1 Zeile plus Rahmenzeichen besorgen */
  scr_p = scr = (short int *) malloc (sizeof (short int)*(akt_winp->dx+1));
  wmove (akt_winp->winp, y+1, 1);
  scr_image = cur_to_poi (akt_winp->winp);
  check_buff(); /* evtl. Pufferinhalt in Text uebertragen */
  zeile = fastzeichen(akt_winp->ws_col); /* Zeiger auf erstes sichtb. Z. holen */

  /* Solange nicht alle Bildschirmspalten angezeigt wurden oder */
  /* die anzuzeigende Zeile zu Ende ist. */
  for(s = zeile, col=0;(col < akt_winp->dx) && (s!= &space);col++)
    if (!*zeile)  /* Zeilenende ? */
    {
      s = &space; /* Dann s auf globales Space setzen */
      col--; /* Obwohl kein Zeichen ausgegeben wurde, wird col erhoeht. */
    }        /* Also muss col hier dekrementiert werden.                */
    else     /* Kein Zeilenende */
    {
      /* Testen, ob zeile nicht hinter oder vor Zeile zeigt und ob */
      /* aktuelles Zeichen unterstrichen ist. */
      if(((s = zeile) != &space)
      && *zeile++ == '_' && *zeile == '' && zeile[1])
	zeile+=2; /* Unterstrichen, dann zeile um 2 weitere Pos. erhoehen */
      /* Jetzt mit der Funktion add_attrs die erforderlichen Attribute */
      /* hinzufÅgen und das Zeichen anzeigen                           */
      *scr_image++ =
      *scr_p++ = add_attrs(in_block(y+akt_winp->ws_line,col+akt_winp->ws_col),s);
    }

  if(col < akt_winp->dx)
  /* Wenn noch nicht am Fensterrand, dann Rest der Zeile loeschen */
    for (; col<akt_winp->dx; col++)
      *scr_image++ = *scr_p++ = 256*STD_ATTR;

  /* Jetzt Rahmen restaurieren */
  if(y == 0)
      *scr_image++ = *scr_p++ = REST_ARR_UP;
    else
      if(y == akt_winp->dy-1)
	*scr_image++ = *scr_p++ = REST_ARR_DN;
      else
	*scr_image++ = *scr_p++ = REST_CHAR;

  /* Jetzt den zusammengesetzten String ausgeben */
  VioWrtCellStr (scr, sizeof(short int) * (akt_winp->dx+1),
		 akt_winp->winp->_start_y+y+1,
		 akt_winp->winp->_start_x+1, 0);
  free (scr);
}

#else

void lineout(y)
int y;
{
  /* *** interne Daten *** */
  int           col;    /* aktuelle Fensterspalte */
  register char *s,     /* auszugebendes Zeichen  */
		*zeile; /* Aktuelles Zeichen      */
  unsigned short int *scr,   /* Zeiger in Fenster      */
		 rc;         /* Zeichen zum Reparieren des Rahmens */

  check_buff(); /* evtl. Pufferinhalt in Text uebertragen */
  zeile = fastzeichen(akt_winp->ws_col); /* Zeiger auf erstes sichtb. Z. holen */
  wmove(akt_winp->winp,y+1,1); /* Cursor zum Zeilenanfang bewegen */

  /* Solange nicht alle Bildschirmspalten angezeigt wurden oder */
  /* die anzuzeigende Zeile zu Ende ist. */
  for(s = zeile, col=0;(col < akt_winp->dx) && (s!= &space);col++)
    if (!*zeile)  /* Zeilenende ? */
    {
      s = &space; /* Dann s auf globales Space setzen */
      col--; /* Obwohl kein Zeichen ausgegeben wurde, wird col erhoeht. */
    }        /* Also muss col hier dekrementiert werden.                */
    else     /* Kein Zeilenende */
    {
      /* Testen, ob zeile nicht hinter oder vor Zeile zeigt und ob */
      /* aktuelles Zeichen unterstrichen ist. */
      if(((s = zeile) != &space)
      && *zeile++ == '_' && *zeile == '' && zeile[1])
	zeile+=2; /* Unterstrichen, dann zeile um 2 weitere Pos. erhoehen */
      /* Jetzt mit der Funktion add_attrs die erforderlichen Attribute */
      /* hinzufÅgen und das Zeichen anzeigen                           */
      waddch (akt_winp->winp, 
	      add_attrs(in_block(y+akt_winp->ws_line,col+akt_winp->ws_col),s));
    }

  if(col < akt_winp->dx)
  { /* Wenn noch nicht am Fensterrand, dann Rest der Zeile loeschen */
    wmove(akt_winp->winp,y+1,col+1);
    wclrtoeol(akt_winp->winp);
  }
  if(y == 0)
      rc = REST_ARR_UP;
    else
      if(y == akt_winp->dy-1)
	rc = REST_ARR_DN;
      else
	rc = REST_CHAR;
  mvwaddch(akt_winp->winp,y+1,akt_winp->dx+1,rc); /* Rahmen reparieren */
}
#endif

/*****************************************************************************
*
*  Funktion       Ecken zeichnen und alte Zeichen merken (mal_und_merk)
*  --------
*
*  Parameter    : y         :
*                   Typ          : int
*                   Wertebereich : 0 - LINES-2
*                   Bedeutung    : Zeile,in der Ecke erscheinen soll
*
*                 x         :
*                   Typ          : int
*                   Wertebereich : 0 - COLS-1
*                   Bedeutung    : Spalte, in der Ecke erscheinen soll
*
*                 num       :
*                   Typ          : int
*                   Wertebereich : 0-3
*                   Bedeutung    : Nummer der Ecke
*
*  Beschreibung : Die Funktion merkt sich die Zeichen, die da stehen, wo die
*                 Ecken erscheinen sollen. Dann werden diese Zeichen durch
*                 die alten gemerkten Zeichen (anfangs '+') ersetzt.
*
*****************************************************************************/

void mal_und_merk(y,x,num)
int y,x,num;
{
  /* *** interne Daten und Initialisierung *** */
  static short int alt_zeich [4] = {'+','+','+','+'}; /* alte Zeichen */
  short int        hilf;   /* Zwischenspeicher fuer gelesenes Zeichen */

  hilf = mvinch (y,x);  /* Zeichen aus dem Bildschirm lesen */
  mvaddch (y,x,alt_zeich [num]); /* Altes Zeichen hinsetzen */
  alt_zeich [num] = hilf; /* gelesenes Zeichen als altes Zeichen merken */
}

/*****************************************************************************
*
*  Funktion       Fensterinhalte nach stdscr uebertragen (cpwins2stdscr)
*  --------
*
*  Beschreibung : Es werden alle Fenster ausser dem aktuellen ins stdscr-
*                 Fenster uebertragen. Dazu wird die Funktion overlay benutzt.
*
*****************************************************************************/

void cpwins2stdscr()
{
  /* *** interne Daten und Initialisierung *** */
  win_typ *w = akt_winp->next->next; /* Pointer fuer Windowliste */

  while(w != akt_winp)
  {
    overwrite(w->winp,stdscr);
    w = w->next;
  }
  refresh();
}

/*****************************************************************************
*
*  Funktion       Ecken zeichnen bzw. loeschen (eckenhw)
*  --------
*
*  Beschreibung : Es werden die Eckpunkte des aktuellen Fensters durch die
*                 Zeichen, die vorher dort standen (anfangs '+') ersetzt.
*
*****************************************************************************/

void eckenhw()
{
  mal_und_merk (akt_winp->y,akt_winp->x,0);
  mal_und_merk (akt_winp->y,akt_winp->x+akt_winp->dx+1,1);
  mal_und_merk (akt_winp->y+akt_winp->dy+1,akt_winp->x,2);
  mal_und_merk (akt_winp->y+akt_winp->dy+1,akt_winp->x+akt_winp->dx+1,3);
  refresh();
}

/*****************************************************************************
*
*  Funktion       Fenster nach rechts bewegen (win_right)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verschoben werden soll
*
*  Beschreibung : Funktion prueft, ob Fenster nach rechts geschoben werden
*                 kann. Wenn ja, wird Startposition x inkrementiert.
*
*****************************************************************************/

void win_right(steps)
int steps;
{
  if ((akt_winp->x += steps)+akt_winp->dx+1 >= COLS)
    akt_winp->x = COLS-akt_winp->dx-2;
}

/*****************************************************************************
*
*  Funktion       Fenster nach links verschieben (win_left)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verschoben werden soll
*
*  Beschreibung : Fall Fenster noch nicht am linken Bildschirmrand, dann
*                 Fensterstart x dekrementieren.
*
*****************************************************************************/

void win_left(steps)
int steps;
{
  if ((akt_winp->x -= steps) < START_X)
    akt_winp->x = START_X;
}

/*****************************************************************************
*
*  Funktion       Window nach oben verschieben (win_up)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verschoben werden soll
*
*  Beschreibung : Falls Fenster nicht am oberen Bildschirmrand, Startadresse
*                 y dekrementieren
*
*****************************************************************************/

void win_up(steps)
int steps;
{
  if ((akt_winp->y -= steps) < START_Y)
    akt_winp->y = START_Y;
}

/*****************************************************************************
*
*  Funktion       Fenster nach unten verschieben (win_down)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verÑndert werden soll
*
*  Beschreibung : Falls Fenster noch nicht am unteren Bildschirmrand, wird
*                 Fensterstartadresse y inkrementiert.
*
*****************************************************************************/

void win_down(steps)
int steps;
{
  if ((akt_winp->y += steps)+akt_winp->dy+2 >= LINES)
    akt_winp->y = LINES-akt_winp->dy-3;
}

/*****************************************************************************
*
*  Funktion       Fenster verbreitern (size_right)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verÑndert werden soll
*
*  Beschreibung : Falls die rechte Seite des Fensters nicht am rechten Bild-
*                 schirmrand ist, wird das Fenster um eine Position
*                 verbreitert
*
*****************************************************************************/

void size_right(steps)
int steps;
{
  if (akt_winp->x+(akt_winp->dx += steps)+1 >= COLS)
    akt_winp->dx = COLS-akt_winp->x-2;
}

/*****************************************************************************
*
*  Funktion       Fenster verschmaelern (size_left)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verÑndert werden soll
*
*  Beschreibung : Falls das Fenster eine Breite > 1 hat, wird es um eine
*                 Position verschmaelert.
*
*****************************************************************************/

void size_left(steps)
int steps;
{
  if ((akt_winp->dx -= steps) < 1)
    akt_winp->dx = 1;
}

/*****************************************************************************
*
*  Funktion       Unteren Fensterrand nach oben ziehen (size_up)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verÑndert werden soll
*
*  Beschreibung : Falls das Fenster mehr als eine Zeile hat, wird
*                 die Hoehe des Fenstern dekrementiert.
*
*****************************************************************************/

void size_up(steps)
int steps;
{
  if ((akt_winp->dy -= steps) < 1)
    akt_winp->dy = 1;
}

/*****************************************************************************
*
*  Funktion       Unteren Fensterrand nach unten schieben (size_down)
*  --------
*
*  Parameter    : steps     :
*                   Typ          : int
*                   Wertebereich : 0 - MAXINT
*                   Bedeutung    : Schritte, um die verÑndert werden soll
*
*  Beschreibung : Falls der untere Fensterrand nicht am unteren Bild-
*                 schirmrand ist, wird die Fensterhoehe inkrementiert.
*
*****************************************************************************/

void size_down(steps)
int steps;
{
  if (akt_winp->y+(akt_winp->dy += steps)+1 >= LINES-1)
    akt_winp->dy = LINES-akt_winp->y-3;
}

/*****************************************************************************
*
*  Funktion       Gespeicherte Groesse wird aktuelle Groesse (toggle_size)
*  --------
*
*  Beschreibung : Die gespeicherte Groesse wird mit der aktuellen vertauscht.
*                 Die aktuelle Zeile wird moeglichst in die Mitte des
*                 Fensters gesetzt, seitlich wird der Fensterinhalt moeglichst
*                 weit nach rechts geschoben.
*
*****************************************************************************/

void toggle_size()
{
  swap_int(&akt_winp->dx,&akt_winp->adx);
  swap_int(&akt_winp->dy,&akt_winp->ady);
  swap_int(&akt_winp->x,&akt_winp->ax);
  swap_int(&akt_winp->y,&akt_winp->ay);

  if ((akt_winp->ws_line = akt_winp->textline - akt_winp->dy/2) < 0)
    akt_winp->ws_line=0;
  if ((akt_winp->ws_col=akt_winp->screencol-akt_winp->dx+1)<0)
    akt_winp->ws_col = 0;

  werase(akt_winp->winp);   /* Fensterinhalt loeschen */
  wrefresh(akt_winp->winp);
  delwin(akt_winp->winp);   /* Fenster entfernen und neu erstellen */
  akt_winp->winp = newwin(akt_winp->dy+2,akt_winp->dx+2,akt_winp->y,akt_winp->x);
  init_win();  /* Fensterdefault mit Curses setzen */
}

/*****************************************************************************
*
*  Funktion       Fenster zum aktuellen Fenster machen (make_akt_win)
*  --------
*
*  Parameter    : n         :
*                   Typ          : int
*                   Wertebereich : 0-maxint
*                   Bedeutung    : Nummer des Fensters, das zum aktuellen
*                                  gemacht werden soll.
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE  : Fenster noch da
*                                  FALSE : Fenster nicht mehr da
*
*  Beschreibung : Die Liste der Fenster wird auf ein Fenster mit der Nummer
*                 n durchsucht. Wird es gefunden, so wird es zum aktuellen,
*                 und es wird TRUE zurueckgegeben. Sonst bleibt das aktuelle
*                 Fenster unveraendert und es wird FALSE zurueckgegeben.
*
*****************************************************************************/

int make_akt_win(n)
register int n;
{
  /* *** interne Daten *** */
  register win_typ *w,     /* Zum Durchlaufen der Fensterliste */
		   *dummy; /* Zum Umhaengen der Fenster        */

  if(akt_winp->wini == n)       /* wieso in der Ferne schweifen... */
    return(TRUE);               /* aktuelles ist gewuenschtes Fenster */

  for(w=akt_winp->next->next;w != akt_winp && w->wini != n;w = w->next);

  if(w->wini == n)              /* gefunden? */
  {
    dummy = akt_winp->next;     /* Dann gefundenes Fenster zum aktuellen */
    akt_winp->next = w;         /* machen, indem es vor dummy einge-     */
    w->prev->next = w->next;    /* koppelt wird.                         */
    w->next->prev = w->prev;
    w->prev = akt_winp;
    w->next = dummy;
    dummy->prev = w;
    akt_winp = w;
    return(TRUE);
  }
  return(FALSE);
}

/*****************************************************************************
*
*  Funktion       Fenster gemaess Filename suchen (sw_name)
*  --------
*
*  Parameter    : fn        :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Name des Fensters, das zum aktuellen
*                                  gemacht werden soll.
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE  : Fenster gefunden
*                                  FALSE : Fenster nicht gefunden
*
*  Beschreibung : Die Liste der Fenster wird auf ein Fenster mit dem Namen
*                 fn durchsucht. Wird es gefunden, so wird es zum aktuellen,
*                 und es wird TRUE zurueckgegeben. Sonst bleibt das aktuelle
*                 Fenster unveraendert und es wird FALSE zurueckgegeben.
*
*****************************************************************************/

int sw_name(fn)
register char *fn;
{
  register win_typ *w,     /* Zum Durchlaufen der Fensterliste */
		   *dummy; /* Zum Umhaengen der Fenster        */

  for(w=akt_winp->next->next;w!=akt_winp && strcmp(w->filename,fn);w=w->next);

  if(w != akt_winp)                     /* gefunden? */
  {
    dummy = akt_winp->next;   /* Dann gefundenes Fenster zum aktuellen */
    akt_winp->next = w;       /* machen, indem man es vor dummy in     */
    w->prev->next = w->next;  /* die Fensterliste einhaengt.           */
    w->next->prev = w->prev;
    w->prev = akt_winp;
    w->next = dummy;
    dummy->prev = w;
    akt_winp = w;
    return(TRUE);
  }
  /* Wenn aktuelles Fenster richtig war, dann TRUE, sonst FALSE */
  if (akt_winp->filename)
    return(!strcmp(akt_winp->filename,fn));
  else
    return FALSE; /* Kein Fenster offen, dann nicht gefunden */
}

/******************************************************************************
*
* Funktion     : Fenster in Hintergrund schieben (push_win_back)
* --------------
*
* Beschreibung : Das aktuelle Fenster wird zum Fenster, das als erstes
*                gezeichnet wird. Dadurch erscheint es fÅr den Benutzer
*                im Hintergrund.
*
******************************************************************************/

void push_win_back()
{
  win_typ *dummy=akt_winp->next; /* Zum UmhÑngen der Fenster */

  if(akt_winp->next->next != akt_winp)
  {
    dummy->next->prev = akt_winp;
    akt_winp->next = dummy->next;
    dummy->prev = akt_winp->prev;
    dummy->prev->next = dummy;
    dummy->next = akt_winp;
    akt_winp->prev = dummy;
    akt_winp = dummy->prev;
  }
}

/******************************************************************************
*
* Funktion     : Um eine Zeile scrollen, falls nîtig (check_and_scroll_by_one)
* --------------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE  : Fenster gescrollt
*                                  FALSE : Fenster nicht gescrollt
*
* Beschreibung : Wenn sich der Cursor unterhalb des sichtbaren Bereichs
*                des aktuellen Fensters befindet, wird das Fenster um eine
*                Zeile nach oben gescrollt und der Cursor wieder korrekt
*                positioniert (mit setz_cursor()).
*
******************************************************************************/

int check_and_scroll_by_one ()
{
  if (akt_winp->textline >= akt_winp->ws_line+akt_winp->dy)
  {
    akt_winp->ws_line++;
    text_up (0);             /* Falls nîtig, scrollen */
    setz_cursor (W_AKT);
    return TRUE;
  }
  return FALSE;
}
