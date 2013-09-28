/****************************************************************/
/*                                                              */
/*      MODUL:  text_2.c                                        */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - textbeginn (an textbeginn)                    */
/*              - textende (an textende)                        */
/*              - lines_down (anzahl von zeilen runter)         */
/*              - lines_up (anzahl von zeilen hoch)             */
/*              - screen_up (eine fenstergroesse hoch)          */
/*              - screen_down (eine fenstergroesse runter)      */
/*              - half_up (halbe fenstergroesse hoch)           */
/*              - half_down (halbe fenstergroesse runter)       */
/*              - insert (zeichen an akt. position einfuegen)   */
/*              - enter_char (Zeichen an aktuelle Pos. kop.)    */
/*              - koppel_line (Neue Zeile in Text einkoppeln)   */
/*              - new_line (zeile splitten)                     */
/*              - del_line (Zeile loeschen)                     */
/*              - gotox (gehe zu Zeile X)                       */
/*              - indent_line (aktuelle Zeile einruecken)       */
/*              - fastleft (in Zeile ein Zeichen nach links)    */
/*              - fastright (in Zeile ein Zeichen nach rechts)  */
/*              - fastll (Zeilenlaenge der aktuellen Zeile)     */
/*              - fastzeichen (Hole Zeichen aus akt. Zeile)     */
/*              - fastzeile (hole aktuelle Zeile uhne Unterstr.)*/
/*              - put_zeile (Zeile in Datei schreiben)          */
/*              - schreib_file (Schreib File auf Platte)        */
/*              - tab_in_buff (Tab in Puffer einfuegen)         */
/*              - lies_file (Datei einlesen)                    */
/*              - free_text (Text freigeben)                    */
/*              - save_delline (aktuelle zeile sichern)         */
/*              - rest_delline (gespeicherte Zeile einfuegen)   */
/****************************************************************/

#include "defs.h"

extern char backupflag;
extern marker_typ marker[];

/* *** globale Daten und Initialisierung *** */
extern char linebuff [3*MAXLENGTH+1], /* Zeilenpuffer                  */
	    bufflag,         /* Flag, ob linebuff belegt ist           */
	    *sd_line,        /* Zwischenspeicher fuer geloeschte Zeile */
	    space;           /* Globales Leerzeichen                   */
char        *fastzeichen();

/****************************************************************
*
* FUNKTION:     textbeginn()    (an textanfang)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - die erste zeile wird zur aktuellen
*****************************************************************/

int textbeginn()
{
  check_buff();  /* evtl. Pufferinhalt in Text uebernehmen */
  if(akt_winp->maxline >= 0)    /* nur wenn Datei nicht leer */
  { /* Da dummy vor der ersten Zeile, ist erste Zeile Nachfolger von dummy */
    akt_winp->alinep = akt_winp->dummyp->next;
    akt_winp->textline = 0;
    return(TRUE);
  }
  return(FALSE);
}


/****************************************************************
*
* FUNKTION:     textende()      (an textende)
* ERGEBNIS:     TRUE
* BESCHREIBUNG: - die letzte zeile wird zur aktuellen
*****************************************************************/

int textende()
{
  check_buff(); /* evtl. Pufferinhalt in Text uebernehmen */
  /* dummy steht hinter letzter, also ist letzte Vorgaenger von dummy */
  akt_winp->alinep = akt_winp->dummyp->prev;
  akt_winp->textline = akt_winp->maxline;
  return(TRUE);
}


/****************************************************************
*
* FUNKTION:     lines_down()    (anzahl von zeilen runter)
*
* PARAMETER:    int anz: anzahl zu ueberspringender zeilen
* ERGEBNIS:     int: anzahl der wirklich uebersprungenen zeilen
* BESCHREIBUNG: - es wird versucht, anz zeilen im text nach unten
*               zu springen.
*****************************************************************/

int lines_down(anz)
register int anz;
{
  /* *** interne Daten und Initialisierung *** */
  register int hilf = anz; /* Zwischenspeicher fuer Parameter */

  if (anz > 0 && akt_winp->maxline >=0)
  {
    check_buff(); /* evtl. Pufferinhalt in Text uebernehmen */

    while(anz-- && (akt_winp->textline < akt_winp->maxline))
    {
      akt_winp->textline++;
      akt_winp->alinep = akt_winp->alinep->next;
    }
    return(hilf-anz-1);
  }
  return (0);
}


/****************************************************************
*
* FUNKTION:     lines_up()      (anzahl von zeilen hoch)
*
* PARAMETER:    int anz: anzahl zu ueberspringender zeilen
* ERGEBNIS:     int: anzahl der wirklich uebersprungenen zeilen
* BESCHREIBUNG: - es wird versucht, anz zeilen im text nach oben
*               zu springen.
*****************************************************************/

int lines_up(anz)
register int anz;
{
  /* *** interne Daten und Initialisierung *** */
  register int hilf = anz; /* Zwischenspeicher fuer Parameter */

  if (anz > 0 && akt_winp->maxline >= 0)
  {
    check_buff(); /* evtl. Pufferinhalt in Text uebernehmen */

    while(anz-- && akt_winp->textline)
    {
      akt_winp->textline--;
      akt_winp->alinep = akt_winp->alinep->prev;
    }
    return(hilf-anz-1);
  }
  return (0);
}


/****************************************************************
*
* FUNKTION:     screen_up()     (eine bildschirmseite hoch)
*
* ERGEBNIS:     int: anzahl der wirklich uebersprungenen zeilen
* BESCHREIBUNG: - es wird versucht, eine bildschirmseite im text
*               nach oben zu springen.
*****************************************************************/

int screen_up()
{
  return(lines_up(akt_winp->dy));
}


/****************************************************************
*
* FUNKTION:     screen_down()   (eine bildschirmseite runter)
*
* ERGEBNIS:     int: anzahl der wirklich uebersprungenen zeilen
* BESCHREIBUNG: - es wird versucht, eine bildschirmseite im text
*               nach unten zu springen.
*****************************************************************/

int screen_down()
{
  return(lines_down(akt_winp->dy));
}

/****************************************************************
*
* FUNKTION:     half_up()       (eine halbe bildschirmseite hoch)
*
* ERGEBNIS:     int: anzahl der wirklich uebersprungenen zeilen
* BESCHREIBUNG: - es wird versucht, eine halbe bildschirmseite
*               im text nach oben zu springen.
*****************************************************************/

int half_up()
{
  return(lines_up(akt_winp->dy / 2));
}


/****************************************************************
*
* FUNKTION:     half_down()     (eine halbe bildschirmseite runter)
*
* ERGEBNIS:     int: anzahl der wirklich uebersprungenen zeilen
* BESCHREIBUNG: - es wird versucht, eine halbe bildschirmseite
*               im text nach unten zu springen.
*****************************************************************/

int half_down()
{
  return(lines_down(akt_winp->dy / 2));
}

/****************************************************************
*
* FUNKTION:     insert()        (zeichen einfuegen)
*
* PARAMETER:    int n: anzahl einzufuegender zeichen
* ERGEBNIS:     int: Anzahl tatsaechlich eingefuegter Zeichen
* BESCHREIBUNG: - fuer n zeichen wird ab der aktuellen position
*               platz geschaffen
*               - die Anzahl der tatsaechlich eingefuegten
*               Zeichen wird zurueckgegeben
*****************************************************************/

int insert(n)
register int n;
{
  /* *** interne Daten und Initialisierung *** */
  register int n2 = 0, /* Anzahl wirklich einzufuegender Positionen */
	       old_sc = akt_winp->screencol, /* alte Cursorspalte   */
	       old_tc; /* alte Cursorspalte intern                  */

  fill_buff(); /* sicherstellen, dass aktuelle Zeile im Puffer ist */
  old_tc = akt_winp->textcol; /* Cursorspalte merken */
  eol(); /* Falls hinter Zeilenend nicht genug Platz fuer n Zeichen, */
  if((n2 = MAXLENGTH - akt_winp->screencol) > n) /* dann weniger einfuegen */
    n2 = n;                      /* Im Unterstreich-Modus pro Zeichen 3 */
  n2 *= 1+2*akt_winp->underflag; /* Bytes einfuegen. */
  akt_winp->textcol = old_tc;    /* Cursorspalte merken */
  akt_winp->screencol = old_sc;
  if(n2)                         /* noch platz da ? */
  {
    akt_winp->changeflag = TRUE;  /* Text als geaendert markieren */
    linebuff[3*MAXLENGTH-n2] = '\0'; /* Neues Ende setzen */
    /* Ab Cursor alles um n2 Postionen nach rects */
    revcpy(&linebuff[akt_winp->textcol+n2],&linebuff[akt_winp->textcol]);
    /* Falls die Anzahl einzufuegender Zeichen groesser als die Laenge
      des zu verschiebenden Textes ist, wird '\0' nicht durch revcpy
      ueberschrieben. Dies muss man dann hier nachholen.               */
    if (n2 > 3*MAXLENGTH-n2-akt_winp->textcol)
      linebuff[3*MAXLENGTH-n2] = ' ';     /* Blockgrenzen in screencol-Mass */
    insdel_blockadapt(n2/(1+2*akt_winp->underflag)); /* anpassen */
  }
  return(n2 / (1 + 2*akt_winp->underflag));
}

/****************************************************************
*
* FUNKTION:     enter_char()    (zeichen an aktuelle pos kopieren)
*
* PARAMETER:    char c: einzufuegendes zeichen
* ERGEBNIS:     TRUE/FALSE
* BESCHREIBUNG: - das zeichen c wird an die aktuelle position ko-
*               piert
*               - danach wird check_underl() aufgerufen
*               - das neue Zeichen wird angezeigt und evtl. der
*               Bildschirm gescrollt
*****************************************************************/

int enter_char(c)
register char c;
{
  /* *** interne Daten und Initialisierung *** */
  register int cu_ret = FALSE; /* Rueckgabewert der check_underl()-Funktion */

  fill_buff(); /* sicherstellen, dass aktuelle Zeile im Puffer ist */
  if(akt_winp->screencol < MAXLENGTH) /* Nur wenn Cursor nicht hinter */
  {                                   /* Zeilenende steht             */
    akt_winp->changeflag = TRUE; /* Text als geaendert markieren */
    if(akt_winp->underflag) /* Unterstreichung aktiv ? */
    {                       /* Dann vor dem Zeichen _ und  einsetzen */
      linebuff[akt_winp->textcol] = '_';
      linebuff[akt_winp->textcol+1] = '';
      linebuff[akt_winp->textcol+2] = c;
    }
    else
    {
      linebuff[akt_winp->textcol] = c;
      cu_ret = check_underl(); /* pruefen ob Unterstreichung entstanden ist */
    }
    if (akt_winp->screencol-akt_winp->ws_col >= akt_winp->dx-1)
    { /* Testen, ob horizontal gescrollt werden muss */
      /* Wenn ja, dann normal die voreingestellte Weite. Sollte diese */
      /* groesser als die Fensterbreite sein, dann 1/4 Fensterbreite  */
      akt_winp->ws_col += EC_SCROLL_WIDTH < akt_winp->dx ?
			  EC_SCROLL_WIDTH : akt_winp->dx / 4 + 1;
      if(akt_winp->ws_col >= MAXLENGTH) /* Zu weit, dann eins zurueck */
	akt_winp->ws_col = MAXLENGTH-1;
      show_win(W_AKT);  /* Fensterinhalt neu anzeigen */
      fill_buff(); /* aktuelle Zeile wieder in Puffer kopieren */
    }
    else  /* Wenn nicht gescrollt werden musste: */
      if (cu_ret) /*Bei neu entstandener Unterstreichung Zeile neu anzeigen */
	lineout(akt_winp->textline-akt_winp->ws_line);
      else        /* Sonst nur neu eingefuegtes Zeichen */
	fastcharout(akt_winp->textline-akt_winp->ws_line,
		    akt_winp->screencol-akt_winp->ws_col,
		    linebuff+akt_winp->textcol,akt_winp->insflag?INSERT:PUT);
    akt_winp->textcol+= 1+2*akt_winp->underflag;
    akt_winp->screencol++; /* Cursorspalte anpassen */
    return(TRUE);
  }
  return(FALSE); /* Cursor stand hinter dem Zeilenende */
}

/*****************************************************************************
*
*  Funktion      Neue Zeile in Text einkoppeln (koppel_line)
*  --------
*
*  Parameter    : modus     :
*                   Typ          : int
*                   Wertebereich : ADAPT_COORS,IGNORE_COORS
*                   Bedeutung    : ADAPT_COORS:  Block- und Markerpositionen
*                                  sowie letzte Position werden angepasst
*                                  IGNORE_COORS: ... werden nicht angepasst
*
*  Beschreibung : Fuer eine neue Zeile wird Speicherplatz alloziert.
*                 Anschliessend wird das Element korrekt hinter alinep
*                 eingehaengt. Die neu eingefuegte Zeile wird zur
*                 aktuellen Zeile.
*                 Abhaengig vom Modus wird nl_blockadapt aufgerufen
*
*****************************************************************************/

void koppel_line(modus)
int modus;
{
  /* *** interne Daten und Initialisierung *** */
  register zeil_typ *zeile = (zeil_typ*) reserve_mem(sizeof(zeil_typ));
	   /* zeile zeigt auf neu eingehaengte Zeile */

  if(modus == ADAPT_COORS)    /* evtl. Blockkoordinaten anpassen */
    nl_blockadapt();
  zeile->prev = akt_winp->alinep;       /* in Liste einhaengen      */
  zeile->next = akt_winp->alinep->next;
  zeile->next->prev = zeile;
  akt_winp->alinep->next = zeile;
  akt_winp->alinep = zeile;
  akt_winp->alinep->text = NULL;        /* Zeile als leer markieren */
  akt_winp->textcol = akt_winp->screencol = 0; /* Cursor in Spalte 0 */
  akt_winp->maxline++;  /* Zeilenzahl und Cursorzeile haben sich erhoeht */
  akt_winp->textline++;
  akt_winp->changeflag = TRUE; /* Text wurde geaendert */
}

/****************************************************************
*
* FUNKTION:     new_line()      (zeile splitten)
*
* ERGEBNIS:       TRUE, FALSE
* BESCHREIBUNG: - die aktuelle zeile wird an der aktuellen po-
*               sition gespalten und der rechte teil in eine
*               neue, der aktuellen zeile nachfolgenden zeile
*               kopiert
*               - diese neue zeile wird dann zur aktuellen zeile,
*               wobei der cursor in die erste spalte gesetzt
*               wird
*               - Wenn durch das Einfuegen der neuen Zeile das
*               Zeilenlimit ueberschritten wuerde, wird keine
*               Zeile eingefuegt und FALSE zurueckgegeben.
*               Sonst ist der RETURN-Wert TRUE.
*
*****************************************************************/

int new_line()
{
  /* *** interne Daten und Initialisierung *** */
  register char *zeil_anf, /* Zeiger auf Anfang der neuen Zeile        */
		*ltext;    /* Zeiger auf aktuelle Zeile                */
  int           old_sc = akt_winp->screencol, /* alte Cursorspalte     */
		y;         /* Zeilennummer absolut auf Bildschirm      */
  short int     rc;        /* Zeichen, mit dem Rahmen restauriert wird */

  if(akt_winp->maxline < MAX_ANZ_LINES - 1)
  {
    check_buff();                       /* eventuell Puffer in Text zurueck */
    ltext = akt_winp->alinep->text;
    zeil_anf = fastzeichen(akt_winp->screencol);/* Anfang neuer Zeile holen */
    koppel_line(ADAPT_COORS);   /* Eine neue Zeile erzeugen */
    if(zeil_anf != &space)   /* wenn nicht am Zeilenende gesplittet wurde */
    {
      y = akt_winp->textline-akt_winp->ws_line;
      wmove(akt_winp->winp, /* Zur Cursorposition auf Bildschirm */
	    y,old_sc-akt_winp->ws_col+1);
      /* Die Cursorposition liegt nun in der Zeile vor der aktuellen Zeile,
	 da koppel_line die neue Zeile zur aktuellen macht. */
      wclrtoeol(akt_winp->winp); /* Rest der aktuellen Zeile loeschen */
      if(y == 1)
	  rc = REST_ARR_UP | 256*A_STANDOUT;
	else
	  if(y == akt_winp->dy)
	    rc = REST_ARR_DN | 256*A_STANDOUT;
	  else
	    rc = REST_CHAR | 256*A_STANDOUT;
      mvwaddch(akt_winp->winp,y,
	       akt_winp->dx+1,rc); /* Rechten Rahmen restaurieren */
      akt_winp->alinep->text = save_text(zeil_anf);/* neue zeile abspeichern*/
      *zeil_anf = '\0';  /* ende alte zeile setzen und neue alte Zeile */
      akt_winp->alinep->prev->text = save_text(ltext);  /* abspeichern */
      free(ltext);                              /* alte zeile freigeben   */
    }
    return (TRUE);                              /* Einfuegen hat geklappt */
  }
  return (FALSE);
}

/*****************************************************************************
*
*  Funktion       Zeile loeschen (del_line)
*  --------
*
*  Parameter    : modus     :
*                   Typ          : int
*                   Wertebereich : IGNORE_COORS, ADAPT_COORS
*                   Bedeutung    : IGNORE_COORS: del_line wird von Blockfunktion
*                                  aufgerufen
*                                  ADAPT_COORS: del_line wird nicht von einer
*                                  Blockfunktion aufgerufen
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : NO_LINE_DEL,LAST_LINE_DEL,OTHER_LINE_DEL
*                   Bedeutung    : NO_LINE_DEL : Konnte keine Zeile loeschen
*                                  OTHER_LINE_DEL : Zeile ungleich letzter
*                                                   geloescht
*                                  LAST_LINE_DEL : Letzte Zeile geloescht
*
*  Beschreibung : Die aktuelle Zeile wird aus der Zeilenliste ausgekoppelt
*                 und der dafuer reservierte Speicherplatz freigegeben.
*                 Wird die letzte Zeile des Textes geloescht, dann wird der
*                 Cursor intern um eine Zeile nach oben bewegt und LAST_LINE_DEL
*                 zurueckgegeben.
*                 Wird die Zeile von einer Blockfunktion, z.B. del_normal
*                 aufgerufen, so wird nicht dl_blockadapt aufgerufen, da
*                 schon die Blockfunktion die Anpassung der Marker und der
*                 letzten Position vornimmt.
*
*****************************************************************************/

int del_line(modus)
int modus;
{
  /* *** interne Daten *** */
  register zeil_typ *zeile; /* Zeiger auf nachfolgende Zeile */

  if (akt_winp->textline != -1)               /* Dummyzeile nicht loeschen */
  {
    akt_winp->changeflag = TRUE;  /* Text als veraendert markieren */
    if(modus == ADAPT_COORS)      /* evtl. Blockgrenzen anpassen */
      dl_blockadapt();
    bufflag = FALSE;                    /* Pufferinhalt ist egal */
    /* Zeile aus Textliste auskoppeln */
    akt_winp->alinep->next->prev = akt_winp->alinep->prev;
    akt_winp->alinep->prev->next = zeile = akt_winp->alinep->next;
    line_free(akt_winp->alinep->text);  /* Zeile freigeben */
    free (akt_winp->alinep);
    akt_winp->alinep = zeile;
    if (akt_winp->textline == akt_winp->maxline--)   /* unterste Textzeile? */
    {
      if(akt_winp->maxline == -1)                 /* allerletzte Textzeile? */
      {
	akt_winp->textline--;                        /* textline auf -1 */
	akt_winp->screencol = 0;
      }
      else
	up();                                         /* eins hoch */
      return (LAST_LINE_DEL);
    }
    return (OTHER_LINE_DEL);
  }
  return (NO_LINE_DEL);
}

/*****************************************************************************
*
*  Funktion       Gehe zu Zeile x (gotox)
*  --------
*
*  Parameter    : zeile     :
*                   Typ          : int
*                   Wertebereich : 1-MAX_ANZ_LINES
*                   Bedeutung    : Zeile, die angesprungen werden soll.
*
*  Beschreibung : Es wird getestet, welcher Weg zur uebergebenen Zeile der
*                 kuerzeste ist : vom Start, vom Ende oder von der aktuellen
*                 Zeile aus. Ueber diesen Weg wird dann alinep des aktuellen
*                 Windows gesetzt. textline wird korrekt angepaát.
*
*****************************************************************************/

void gotox (zeile)
register int zeile;
{
  /* *** interne Daten *** */
  register int dist; /* Entfernung zur gewuenschten Zeile */

  if(akt_winp->maxline < 0) /* Text leer, dann raus */
    return;
  if(zeile > akt_winp->maxline) /* Zeilennummer zu gross, dann */
    zeile = akt_winp->maxline;  /* zur letzten Zeile           */
  if(zeile < 0)   /* Wenn Zeilennummer kleiner als 0, dann zur ersten */
    zeile = 0;

  dist = akt_winp->textline - zeile; /* Entfernung berechnen */
  check_buff(); /* Puffer in Text falls man in aktueller Zeile bleibt */
  /* Pruefen, ob es guenstig ist, von der aktuellen Postion loszulaufen */
  if (abs(dist) <= zeile && abs(dist) <= akt_winp->maxline-zeile)
    if (dist < 0)              /* Abhaengig vom Vorzeichen von dist */
      lines_down (abs(dist));  /* hoch oder runter gehen */
    else
      lines_up   (abs(dist));
  else /* Es bietet sich an, vom Anfang oder vom Ende loszulaufen */
  {
    akt_winp->alinep = akt_winp->dummyp;
    if (zeile < akt_winp->maxline-zeile) /* besser vom Anfang ? */
    {
      akt_winp->textline = -1;           /* dann vorwaerts */
      lines_down (zeile+1);
    }
    else                                 /* sonst rueckwaerts */
    {
      akt_winp->textline = akt_winp->maxline+1;
      lines_up (akt_winp->maxline-zeile+1);
    }
  }
}

/*****************************************************************************
*
*  Funktion       aktuelle  Zeile einruecken (indent_line)
*  --------
*
*  Beschreibung : Die aktuelle Zeile wird so eingerueckt, dass die Zeile
*                 am Anfang soviele Blanks enthaelt wie die darueberliegende.
*
*****************************************************************************/

void indent_line()
{
  /* *** interne Daten und Initialisierung *** */
  register int i,     /* Loopvariable fuer Marker und Blanks */
	       anz=0; /* Zaehler fuer Einrueckungsweite */
  int          sc=akt_winp->screencol; /* Zwischenspeicher fuer Cursorspalte */
  char         *txt,  /* Zeiger auf vorangehende Zeile  */
	       *txt2; /* Zeiger in neuen Zeilenstring   */

  check_buff(); /* sicherstellen, dass Pufferinhalt im Text ist */
  if (txt = akt_winp->alinep->prev->text)
  { /* Wenn vorhergehende Zeile nicht leer, dann fuehrende Blanks zaehlen */
    while(*txt == ' ' && fastright(&txt,1))
      anz++;    /* Zaehlen, wie weit eingerueckt wird */

    swap_int(&akt_winp->screencol,&sc); /* Alte Cursorspalte zur neuen machen */
    insdel_blockadapt(anz); /* Block, lastpos und Marker anpassen */
    swap_int(&akt_winp->screencol,&sc); /* Cursor wieder an neuen Zeilenstart */

    if(akt_winp->alinep->text) /* Wenn einzurueckende Zeile nicht leer: */
    { /* Platz fer Zeile und einzufuegende Blanks allozieren */
      txt2 = txt = reserve_mem(strlen(akt_winp->alinep->text)
			  + akt_winp->screencol+1);
      for(i=0;i<akt_winp->screencol;i++)   /* Blanks eintragen */
	*txt2++ = ' ';
      strcpy(txt2,akt_winp->alinep->text); /* Alte Zeile dranhaengen */
      free(akt_winp->alinep->text);   /* Alte Zeile freigeben */
      akt_winp->alinep->text = txt;
    }
  }
}

/*****************************************************************************
*
*  Funktion       in Zeile Anzahl Zeichen nach links (fastleft)
*  --------
*
*  Parameter    : zeile     :
*                   Typ          : char**
*                   Wertebereich : Doppelpointer auf ASCII-Zeichenkette
*                   Bedeutung    : Character, ab dem nach links bewegt werden
*                                  soll. Muss auf einen Character in
*                                  akt_winp->alinep->text zeigen.
*
*  Parameter    : anzahl    :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : Anzahl der Positionen, um die nach links
*                                  gegangen werden soll.
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE : konnte anzahl Zeichen nach links
*                                  FALSE: konnte nicht anzahl Zeichen nach
*                                         links
*
*  Beschreibung : Der pointer auf den String (*zeile) wird unter Berueck-
*                 sichtigung von unterstrichenen Zeichen so oft nach links
*                 bewegt, bis die vorgegebene Anzahl erreicht ist oder
*                 man am linken Zeilenrand angekommen ist.
*                 akt_winp->screencol wird angepasst.
*
*****************************************************************************/

int fastleft(zeile,anzahl)
register char **zeile;
register int  anzahl;
{
  while (anzahl--)
  {
    if (*zeile > akt_winp->alinep->text)  /* Zeilenanfang ? */
    {
      if (*zeile-2 >= akt_winp->alinep->text    /* Steht man auf einem      */
      && *(*zeile-1)=='' && *(*zeile-2)=='_') /* unterstrichenem Zeichen? */
	*zeile -= 3;   /* Dann 3 nach links, */
      else
	(*zeile)--;    /* sonst nur 1 nach links */
      akt_winp->screencol--;  /* screencol anpassen */
    }
    else
      return (FALSE);   /* Zeilenanfang, also FALSE zurueckgeben */
  }
  return (TRUE); /* Konnte anzahl Zeichen nach rechts, also TRUE zurueck */
}

/*****************************************************************************
*
*  Funktion       in Zeile Anzahl Zeichen nach rechts (fastright)
*  --------
*
*  Parameter    : zeile     :
*                   Typ          : char**
*                   Wertebereich : Doppelpointer auf ASCII-Zeichenkette
*                   Bedeutung    : Character, ab dem nach rechts bewegt werden
*                                  soll. Muss auf einen Character in
*                                  akt_winp->alinep->text zeigen.
*
*  Parameter    : anzahl    :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : Anzahl der Positionen, um die nach rechts
*                                  gegangen werden soll.
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE : konnte anzahl Zeichen nach rechts
*                                  FALSE: konnte nicht anzahl Zeichen nach
*                                         rechts
*
*  Beschreibung : Der pointer auf den String (*zeile) wird unter Berueck-
*                 sichtigung von unterstrichenen Zeichen so oft nach rechts
*                 bewegt, bis die vorgegebene Anzahl erreicht ist oder
*                 man am rechten Zeilenrand angekommen ist.
*                 akt_winp->screencol wird angepasst.
*
*****************************************************************************/

int fastright(zeile,anzahl)
register char **zeile;
register int  anzahl;
{
  while (anzahl--)
  {
    if (*(*zeile+1))  /* Zeilenende ? */
    {
      akt_winp->screencol++; /* screencol anpassen */
      (*zeile)++; /* rechts davon _^H, dann noch 2 Positionen weiter */
      if (**zeile=='_' && *(*zeile+1)=='' && *(*zeile+2))
	*zeile += 2;
    }
    else
      return (FALSE);   /* Zeilenende, also FALSE zurueckgeben */
  }
  return (TRUE); /* Konnte anzahl Zeichen nach rechts, also TRUE zurueck */
}

/*****************************************************************************
*
*  Funktion       Zeilenlaenge der aktuellen Zeile berechnen (fastll)
*  --------
*
*  Parameter    : txt       :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : String, vom dem Laenge in screencol-Mass
*                                  berechnet werden soll.
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : Laenge der aktuellen Zeile in screencol-Mass
*
*  Beschreibung : Die aktuelle Zeile (akt_winp->alinep->text) wird bis zum
*                 Ende durchgegangen, wobei berechnet wird, welche screencol-
*                 position das Zeilenende hat.
*
*****************************************************************************/

int fastll(txt)
register char *txt;
{
  /* *** interne Daten und Initialisierung *** */
  register int sc = 0; /* vorlaeufiger return-Wert */

  if (!txt)      /* Zeile leer ? */
    return (0);
  while (*txt)  /* Zeilenende ? */
  {
    sc++;
    if (*txt++ == '_' && *txt=='' && txt[1])
      txt += 2; /* bei unterstrichenem Zeichen insgesamt 3 nach rechts */
  }
  return (sc);   /* Zeilenlaenge in screencol-Mass zurueckgeben */
}

/*****************************************************************************
*
*  Funktion       hole zeichen aus aktueller zeile (fastzeichen)
*  --------
*
*  Parameter    : n         :
*                   Typ          : int
*                   Wertebereich : 0 - MAXLENGTH-1
*                   Bedeutung    : index des zu ermittelnden Zeichens
*
*  Ergebnis     :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichen
*                   Bedeutung    : Zeichen an Position n (s.u.)
*
*  Beschreibung : Die aktuelle Zeile wird bis zur Position n durchlaufen,
*                 wobei _^Hx als ein Zeichen gewertet wird. Daher ist der
*                 Rueckgabewert auch ein Pointer, da ein unterstrichenes
*                 Zeichen mit dem _ beginnt und aus drei Codes besteht.
*                 Ist die Zeile leer oder wird versucht, vor oder hinter der
*                 Zeile zuzugreifen, wird die Addresse der Variablen
*                 space zurueckgeliefert, die ein Leerzeichen enthaelt.
*
*****************************************************************************/

char *fastzeichen(n)
register int n;
{
  /* *** interne Daten *** */
  register int  sc;     /* Zaehler fuer Durchlaufen der Zeile */
  register char *zeile; /* Zeiger auf aktuellen Zeilentext    */

  check_buff(); /* sicherstellen, dass Pufferinhalt im Text ist */

  /* Wenn Zeile leer ist oder n hinters Zeilenende oder vor den       */
  /* Zeilenanfang zeigt, dann die Adresse des globalsn Blanks zurueck */
  if (!(zeile = akt_winp->alinep->text) || (n > MAXLENGTH) || (n < 0))
    return(&space);

  for(sc=0;(sc<n) && *zeile;sc++)  /* Zeile durchlaufen */
    if(*zeile++ == '_' && *zeile == '' && zeile[1])
      zeile+=2;

  if(!*zeile)        /* Zeilenende erreicht, dann Adresse des */
    return(&space);  /* globalen Blanks zurueckgeben          */

  return(zeile);     /* Sonst das Zeichen, auf dem man steht  */
}

/*****************************************************************************
*
*  Funktion       hole aktuelle Zeile uhne Unterstreichung (fastzeile)
*  --------
*
*  Parameter    : p         :
*                   Typ          : zeil_typ*
*                   Wertebereich : Pointer auf Zeilenstruktur
*                   Bedeutung    : Zeigt auf zu zu konvertierende Zeile
*
*  Ergebnis     :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichen
*                   Bedeutung    : Zeiger auf konvertierte Zeile
*                                  Achtung: statischer Puffer!!!
*
*  Beschreibung : Die aktuelle Zeile wird n”tigenfalls aus dem Puffer in
*                 den Text bernommen. Bemerkung: alinep wird dabei nicht
*                 ver„ndert, so daá Parameter Gltigkeit beh„lt.
*                 Anschlieáend wird in den lokalen
*                 statischen Puffer eine Kopie der angegebenen Zeile
*                 gezogen, die jedoch keine
*                 Unterstreichung enth„lt. Ist die aktuelle Zeile leer
*                 (text pointer ist NULL), so wird in die erste Position
*                 des Puffers ein '\0' geschrieben.
*
*****************************************************************************/

char *fastzeile(p)
zeil_typ *p;
{
  /* *** interne Daten *** */
  register int  sc;     /* Zaehler fuer Durchlaufen der Zeile */
  register char *zeile; /* Zeiger auf aktuellen Zeilentext    */
  static   char buf [BUFFSIZE]; /* Puffer, in dem Zeile zurckkommt */

  check_buff(); /* sicherstellen, dass Pufferinhalt im Text ist */

  if (!(zeile = p->text))
    *buf = '\0';
  else
  {
    sc = 0;
    while (*zeile)
    {
      if(*zeile == '_' && zeile [1] == '' && zeile[2])
	zeile+=2;
      buf [sc++] = *zeile++;
    }
    buf [sc] = '\0';  /* String terminieren */
  }
  return(buf);
}

/*****************************************************************************
*
*  Funktion       Zeile in Datei schreiben (put_zeile)
*  --------
*
*  Parameter    : line      :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : zu schreibende Zeile
*
*                 f         :
*                   Typ          : FILE *
*                   Wertebereich : Dateipointer
*                   Bedeutung    : Datei, in die Zeile geschrieben werden soll
*
*  Beschreibung : Die Zeile wird in die Datei geschrieben. Abhaengig von
*                 akt_winp->tabflag werden Spaces am Anfang - wenn moeglich -
*                 zu Tabs komprimiert.
*
*****************************************************************************/

void put_zeile(line,f)
register char *line;
FILE *f;
{
  /* *** interne Daten und Initialisierung *** */
  register int i,            /* Schleifenzaehler fuer Tabschreiben */
	       anz_spaces=0; /* Zaehler fuer fuehrende Blanks      */

  if(akt_winp->tabflag) /* Wenn Spaces zu Tabs komprimiert werden sollen: */
  {
    while(*line++ == ' ') /* Spaces am Zeilenanfang zaehlen */
      anz_spaces++;
    for(i=anz_spaces/STD_TAB;i>0;i--)  /* Tabs in Datei schreiben */
      fputc('\t',f);
    line -= anz_spaces%STD_TAB + 1;  /* Nichtkomprimierte Spaces schreiben */
  }
  fputs(line,f); /* Rest der Zeile in Datei schreiben */
}

/*****************************************************************************
*
*  Funktion       Schreib File auf Platte (schreib_file)
*  --------
*
*  Ergebnis     :   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : Flag, ob Operation erfolgreich
*
*  Beschreibung : Der im aktuellen Fenster stehende Text wird in die
*                 Datei geschrieben, deren Name in der Window-Struktur
*                 angegeben ist. NULL-Pointer werden in Leerzeilen kon-
*                 vertiert.
*
*****************************************************************************/

int schreib_file ()
{
  /* *** interne Daten *** */
  FILE     *f;  /* Pointer fuer Schreibdatei                        */
  zeil_typ *tp; /* Zeiger auf aktuelle Zeile                        */
  int      len; /* Laenge des Filenamens vor Anhaengen von .bak     */
  char     *fn, /* Zeiger auf Dateinamen ohne Pfad                  */
	   *sc, /* Zur Suche des Punktes im Filenamen               */
	   buff[MAXLENGTH+1]; /* fuer Fehlermeldungen und Filenamen */

  if (akt_winp->read_only)
  {
    sprintf(buff,PROMPT_WRTPROT,akt_winp->filename);
    pe_or(buff);
    return (FALSE);
  }
  check_buff(); /* Pufferinhalt evtl. in Text uebernehmen */
  sprintf(buff,PROMPT_SAVING,akt_winp->filename);
  print_stat(buff);
  if(backupflag && !access(akt_winp->filename,2))   /* write permission? */
  {                   /* .bak erzeugen */
    strcpy(buff,akt_winp->filename);
#ifdef OS2  /* OS/2 verkraftet auch xxx.c.bak !!! */
    strcat(buff,".bak");
#else
    if(sc = strchr(fn=sname(buff),'.')) /* Punkt im Filenamen suchen */
      len = sc-fn;      /* Falls gefunden, ist L„nge die L„nge bis zum Punkt */
    else                /* Sonst ist es die gesamte L„nge des Filenamens */
      len = strlen(fn); /* Kann nicht l„nger als 8 Zeichen sein. */
    strcpy(&fn[len],".bak");  /* Filename des .bak-Files erzeugen */
#endif
    unlink(buff);     /* falls schon ein .bak gleichen Namens existiert */
    rename(akt_winp->filename,buff);   /* Datei in ...bak umbenennen */
  }
  if (f = fopen (akt_winp->filename,"w"))
  {
    for (tp = akt_winp->dummyp->next; tp != akt_winp->dummyp; tp = tp->next)
    { /* Alle Zeilen durchgehen und in die Datei schreiben */
      if (tp->text)
	put_zeile(tp->text,f);
      putc('\n',f);
    }
  }
  else  /* Fehler beim Oeffnen der Datei */
  {
    clear_stat();
    print_stat(PROMPT_ERRWRITE);
    pe_or(akt_winp->filename);
    return(FALSE);
  }
  akt_winp->changeflag = FALSE;               /* changeflag zuruecksetzen */
  fclose(f);
  chmod(akt_winp->filename,akt_winp->attribs); /* Modus korrekt setzen */
  clear_stat();
  return(TRUE);
}


/*****************************************************************************
*
*  Funktion       Tab in Puffer einfuegen (tab_in_buff)
*  --------
*
*  Parameter    : buff      :
*                   Typ          : char *
*                   Wertebereich : Pointer auf char-Puffer
*                   Bedeutung    : Puffer, in den Tab eingefuegt werden soll
*
*                 sc        :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer
*                   Bedeutung    : aktuelle Bildschirmposition im Puffer
*
*                 count     :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer
*                   Bedeutung    : aktuelle Position im Puffer
*
*  Beschreibung : Bis zur naechsten Tab-Position werden im Puffer Spaces einge-
*                 fuegt.
*
*****************************************************************************/

void tab_in_buff(buff,sc,count)
register char *buff;
register int *sc,*count;
{
  do
  {
    buff[(*count)++] = ' ';
  } while(++*sc % STD_TAB && *sc < MAXLENGTH);
}

/*****************************************************************************
*
*  Funktion       Datei einlesen (lies_file)
*  --------
*
*  Beschreibung : In die Fensterstruktur des aktuellen Fensters wird der
*                 Text aus der Datei, deren Name in filename steht, geladen.
*                 Evtl. vorkommende Tabs werden expandiert.
*
*****************************************************************************/

int lies_file()
{
  /* *** interne Daten *** */
  FILE        *f;     /* Zeiger fuer Ausgabedatei                       */
  char        buff[3*MAXLENGTH+2],   /* Eingabepuffer                   */
	      buff2[3*MAXLENGTH+2],  /* Ausgabepuffer                   */
	      nlflag; /* Flag zeigt an ob komplette Zeile gelesen wurde */
  int         len,    /* Laenge der eingelesenen Zeile                  */
	      in,     /* Index in Eingabepuffer                         */
	      out,    /* Index in Ausgabepuffer                         */
	      sc;     /* Zeilenlaenge in screencol-Mass                 */
  struct stat f_info; /* Zum Einlesen der Dateiattribute                */

  /* Dummyelement fuer Textstruktur allozieren */
  akt_winp->dummyp = (zeil_typ*) reserve_mem (sizeof (zeil_typ));
  akt_winp->dummyp->prev = akt_winp->dummyp->next = akt_winp->alinep = akt_winp->dummyp;
  akt_winp->dummyp->text = NULL;

  /* Cursorposition und andere Variablen initialisieren */
  akt_winp->textline = akt_winp->maxline = -1;
  akt_winp->screencol = akt_winp->textcol = 0;
  akt_winp->attribs = STD_FATTR;
  akt_winp->read_only = FALSE;
  akt_winp->changeflag = FALSE;

  if (!(f = fopen (akt_winp->filename,"r")))
  {
    sprintf(buff,PROMPT_ASKNEW,akt_winp->filename);
    return (ja_nein(buff));
  }
  sprintf(buff,PROMPT_LOADING,akt_winp->filename);
  print_stat(buff);
  if (access(akt_winp->filename,2))  /* Testen, ob Datei schreibgeschuetzt */
    akt_winp->read_only = TRUE;      /* und evtl. vermerken */
  if (!stat(akt_winp->filename,&f_info)) /* Fileattribute bestimmen */
    akt_winp->attribs = f_info.st_mode & (S_IWRITE | S_IREAD);

  out = sc = 0;
  while (fgets(buff,3*MAXLENGTH + 1,f) && akt_winp->maxline < MAX_ANZ_LINES-1)
  { /* Zeile einlesen, testen ob das ging und ob maximale */
    /* Zeilenanzahl erreicht ist */
    if(buff[(len = strlen(buff))-1] == '\n')
    { /* testen, ob eine komplette Zeile eingelesen wurde */
      nlflag = TRUE;  /* Wenn ja, nlflag setzen */
      buff[--len] = '\0'; /* und '\n' streichen */
    }
    else /* Wenn keine komplette Zeile eingelesen werden konnte */
      nlflag = FALSE;
    in = 0;
    while(in < len)
    {
      if (buff[in] == '_' && buff[in+1] == '' && buff[in+2])
      { /* Wurde ein unterstrichenes Zeichen gelesen, dann den Unter-  */
	buff2[out++] = buff[in++];  /* strich und das Backspace ueber- */
	buff2[out++] = buff[in++];  /* nehmen */
      }
      if(buff[in] == '\t')   /* Tab ? */
	tab_in_buff(buff2,&sc,&out);
      else   /* kein Tab, dann Zeichen in Puffer uebernehmen */
      {
	buff2[out++] = buff[in];
	sc++;  /* Screencol-Laenge der Zeile erhoehen */
      }
      in++;    /* Naechstes Zeichen aus Eingabepuffer */
      if(sc == MAXLENGTH)       /* Aufbereitete Zeile voll ? */
      {
	buff2[out] = '\0'; /* Dann Puffer abschliessen */
	sc = out = 0;      /* Zeile eintragen und neue Zeile anfangen */
	koppel_line(IGNORE_COORS); /* Marker und lastpos sind eh korrupt */
	akt_winp->changeflag = FALSE; /* Falls reserve_mem h„ngt, soll */
				      /* Text nicht abgespeichert werden */
	akt_winp->alinep->text = save_text (buff2);
      }
    }
    if(nlflag && !(!sc && len)) /* volle Zeilen nicht doppelt eintragen */
    {
      buff2[out] = '\0';  /* Puffer abschliessen, eintragen und neue */
      sc = out = 0;       /* Zeile anfangen */
      koppel_line(IGNORE_COORS);
      akt_winp->changeflag = FALSE; /* Kommentar s.o. */
      akt_winp->alinep->text = save_text(buff2);
    }               /* buff wird durch fgets besetzt. Schlaegt fgets fehl, */
    buff[0] = '\0'; /* ist buff[0]=='\0', sonst nicht. Bricht Schleife     */
  }            /* nicht wegen fgets ab, dann war das Zeilenlimit erreicht. */

  /* Wurde die letzte Zeile nicht mit \n abgeschlossen, dann wurde die     */
  /* Zeile noch nicht in den Text eingetragen sondern steht noch in buff2. */
  if(out)
  {
    buff2[out] = '\0'; /* Puffer abschliessen und eintragen */
    koppel_line(IGNORE_COORS);
    akt_winp->changeflag = FALSE; /* Kommentar s.o. */
    akt_winp->alinep->text= save_text(buff2);
  }

  clear_stat();
  if (buff[0] && akt_winp->maxline >= MAX_ANZ_LINES - 1)
    print_err(PROMPT_FILETOLNG);
  fclose(f);
  akt_winp->alinep = akt_winp->dummyp->next; /* Erste Zeile zur aktuellen */
  akt_winp->textline = akt_winp->maxline != -1 ? 0 : -1;    /* machen */
  return (TRUE); /* Kein Fehler, Fenster kann geoeffnet werden */
}

/*****************************************************************************
*
*  Funktion       Text freigeben (free_text)
*  --------
*
*  Beschreibung : Der zum aktuellen Fenster gehoerige Text, inklusive
*                 akt_winp->dummyp, wird freigegeben.
*
*****************************************************************************/

void free_text()
{
  /* *** interne Daten *** */
  register zeil_typ *n; /* Zeiger auf naechste freizugebende Zeile */

  akt_winp->alinep = akt_winp->dummyp;
  do
  {
    line_free(akt_winp->alinep->text); /* Text freigeben */
    n = akt_winp->alinep->next;        /* Zeiger auf naechste Zeile merken */
    free(akt_winp->alinep);            /* Zeilenstruktur freigeben */
  }while((akt_winp->alinep = n) != akt_winp->dummyp);
}

/*****************************************************************************
*
*  Funktion       aktuelle Zeile abspeichern (save_delline)
*  --------
*
*  Beschreibung :  Der text-Pointer der alten Zeile wird in sd_line ge-
*                  speichert und anschliessend auf NULL gesetzt. Diese
*                  Funktion wird in do_delline() vor del_line aufgerufen.
*                  Der Speicherbereich, auf den sd_line zeigt, wird vorher
*                  mittels line_free() freigegeben.
*
*****************************************************************************/

void save_delline()
{
  line_free(sd_line); /* Alte gespeicherte geloeschte Zeile verwerfen */
  check_buff();  /* evtl. Pufferinhalt in den Text uebernehmen */
  sd_line = akt_winp->alinep->text; /* text sichern */
  akt_winp->alinep->text = NULL; /* funktioniert, da Zeile sowieso */
}                                /* geloescht wird */

/*****************************************************************************
*
*  Funktion       Gespeicherte Zeile wiederherstellen (rest_delline)
*  --------
*
*  Beschreibung : Die mit save_delline gespeicherte Zeile wird in den Text
*                 vor der aktuellen Zeile eingefuegt. Die neue Zeile wird
*                 zur aktuellen.  Es wird eine Kopie der gespeicherten Zeile
*                 angelegt, so dass die Zeile ggf. mehrmals eingefuegt werden
*                 kann.
*
*****************************************************************************/

void rest_delline()
{
  int old_sc = akt_winp->screencol; /* Zwischenspeicher Screencol */

  if(akt_winp->maxline < MAX_ANZ_LINES-1)
  { /* Noch Platz fuer Wiederherstellung der geloeschten Zeile ? */
    check_buff();  /* Pufferinhalt evtl. in Text uebernehmen */
    if(akt_winp->alinep != akt_winp->dummyp)
    { /* Wenn Datei Zeilen enthaelt, dann eine Zeile hoch */
      akt_winp->textline--; /* nummer bleibt gleich, da neue z. = alte z. */
      akt_winp->alinep = akt_winp->alinep->prev;
    }
    koppel_line(ADAPT_COORS); /* Neue Zeile einfuegen, Block anpassen */
    akt_winp->alinep->text = save_text(sd_line); /* mehrmals einfuegen */
    akt_winp->screencol = old_sc; /* Screencol restaurieren */
  } /* nur moeglich, wenn sichergestellt ist, dass sd_line gleichbleibt */
}
