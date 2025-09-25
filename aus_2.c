/****************************************************************/ 
/*                                                              */
/*      MODUL:  aus_2.c                                         */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - do_textbeginn (gehe zum Textbeginn)           */
/*              - do_eot (gehe zum Textende)                    */
/*              - do_deleol (loesche bis Zeilenende)            */
/*              - do_del_word (loesche Wort, auf dem Cursor st.)*/
/*              - do_wleft (gehe Wort nach links)               */
/*              - do_wright (gehe Wort nach rechts)             */
/*              - do_right (gehe ein Zeichen nach rechts)       */
/*              - do_left (gehe ein Zeichen nach links)         */
/*              - do_up (gehe eine Zeile nach oben)             */
/*              - do_down (gehe eine Zeile nach unten)          */
/*              - do_settab (Tablaenge setzen)                  */
/*              - do_backtab (an vorherige Tabposition springen)*/
/*              - do_tab (Tab einfuegen)                        */
/*              - do_pgup (Page Up ausfuehren)                  */
/*              - do_pgdn (Page Down ausfuehren)                */
/*              - do_open (leere Zeile vor aktueller einfuegen) */
/*              - do_hopen (leere Zeile hinter aktueller efg.)  */
/*              - do_newline (newline ausfuehren)               */
/*              - do_delline (loesche eine Zeile)               */
/*              - do_join (Zeilenverknuepfung ausfuehren)       */
/*              - do_goto (Springe bestimmte Zeile an)          */
/*              - do_ende (beende Editor)                       */
/*              - ex_load (Loadfile ausfÅhren)                  */
/*              - do_endemit (beende Editor mit Load-File)      */
/*              - quit (verlasse Editor)                        */
/*              - quitmit (verlasse Editor mit Load-File)       */
/****************************************************************/

#ifdef OS2
#define INCL_DOSPROCESS
#include <os2.h>
#else
extern long old_int;
#endif

#include "defs.h"
#include <process.h>

extern char backupflag,highblockflag,clear_buff,bufflag;
extern int schreib_file(),to_shell();
extern short int letter,lastcode,aktcode,taste();
extern int save_delline(),rest_delline();
extern int tst_overlap();
extern void do_find(),do_replace(),do_repfr();
extern char bufflag,*loadfile,*mktemp();
extern bzeil_typ *save_normal(),*save_rechteck();
extern block_typ global_block,*dup_block();
extern puff_typ macro[],puff_feld[];
extern int ks_index;
extern short int *keystack,*e_keystack,newwgetch();
extern WINDOW *status;
extern marker_typ marker[];
#ifdef MOUSE
#ifdef OS2
extern int mouse_handle;
extern char mouse_active;
extern TID mouse_ThreadID;
#endif
#endif

/* *** interne Daten und Initialisierung *** */
void do_join();
void do_up();
void do_eol();
extern char *on_off[], /* Hilfstexte */
		       /* fuer Togglen der globalen Flags */
	    helpflag;  /* Flag: Hilfstexte anzeigen       */

/*****************************************************************************
*
*  Funktion       Gehe zum Textbeginn (do_textbeginn)
*  --------
*
*  Beschreibung : Der Cursor wird in die erste Textzeile bewegt.
*
*****************************************************************************/

void do_textbeginn()
{
  akt_winp->lastline = akt_winp->textline;  /* Cursorposition als letzte */
  akt_winp->lastcol  = akt_winp->screencol; /* Position merken */
  textbeginn();          /* Cursor intern an Textanfang positionieren */
  akt_winp->ws_line = 0; /* Erste Zeile wird erste sichtbare Zeile */
  show_win(W_AKT);           /* Text im Fenster neu anzeigen     */
}

/*****************************************************************************
*
*  Funktion       Cursor an Textende (do_eot)
*  --------
*
*  Beschreibung : Der Cursor wird an das Textende bewegt.
*
*****************************************************************************/

void do_eot()
{
  akt_winp->lastline = akt_winp->textline;  /* Cursorposition als letzte */
  akt_winp->lastcol  = akt_winp->screencol; /* Position merken */
  textende();  /* Cursor intern ans Textende setzen */
  /* Erste Zeile wird die Zeile, die eine halbe Bildschirmlaenge ueber */
  /* dem Textende liegt. Ist das Ergebnis kleiner 0 (zu wenig Zeilen), */
  /* wird die erste Textzeile erste sichtbare Zeile. */
  if((akt_winp->ws_line = akt_winp->maxline - akt_winp->dy/2) < 0)
    akt_winp->ws_line = 0;
  show_win(W_AKT); /* Fensterinhalt neu darstellen */
}

/*****************************************************************************
*
*  Funktion       loesche bis Zeilenende (do_deleol)
*  --------
*
*  Beschreibung : Mit der aktuellen Cursorposition beginnend, wird der Rest
*                 der Zeile geloescht.
*
*****************************************************************************/

void do_deleol()
{
  if (delete_eol())  /* Zeileninhalt bis zum Zeilenende loeschen */
  { /* hat das geklappt, wird die Zeile neu angezeigt */
    lineout(akt_winp->textline-akt_winp->ws_line);
    setz_cursor(W_AKT);  /* und der Cursor auf seine richtige Position gesetzt */
  }
}

/*****************************************************************************
*
*  Funktion       loesche Wort, auf dem der Cursor steht (do_del_word)
*  --------
*
*  Beschreibung : Das Wort, auf dem der Cursor steht, wird geloescht.
*
*****************************************************************************/

void do_del_word()
{
  if (is_last()) /* an oder hinter Zeilenende ? */
    do_join();   /* Dann nÑchste Zeile ranziehen */
  else
    if (delete_word())  /* Wort in Textstruktur loeschen */ 
    { /* hat das geklappt, Zeile neu anzeigen */
      lineout(akt_winp->textline-akt_winp->ws_line);
      setz_cursor(W_AKT); /* und Cursor auf seine richtige Position setzen */
    }
}

/*****************************************************************************
*
*  Funktion       Wort links (do_wleft)
*  --------
*
*  Beschreibung : Der Cursor wird um ein Wort nach links bewegt..
*
*****************************************************************************/

void do_wleft()
{
  if (word_left())    /* Cursor ein Wort nach links */
  {
    if (akt_winp->screencol < akt_winp->ws_col) /* klappte das, und steht der */
    /* Cursor nun links vom Bildschirm, muss das Fenster angepasst werden. */
    {
      akt_winp->ws_col = akt_winp->screencol; /* Aktuelle Spalte wird erste */
      sw_ohne_refresh(W_AKT); /* Fensterinhalt wird neu angezeigt */
    }
  }
  else
  {
     do_up();
     do_eol();
  }
  setz_cursor(W_AKT);  /* Cursor auf seine richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       Wort rechts (do_wright)
*  --------
*
*  Beschreibung : Der Cursor wird um ein Wort nach rechts bewegt.
*
*****************************************************************************/

void do_wright()
{
  if (word_right() /* Cursor intern um ein Wort nach rechts bewegen */
  && akt_winp->screencol >= akt_winp->ws_col + akt_winp->dx)
  /* klappte das, und steht der Cursor dadurch rechts vom Bildschirm, */
  /* so muss der Fensterinhalt angepasst werden */
  {
    /* Aktuelle Spalte wird zur letzten sichtbaren */
    akt_winp->ws_col = akt_winp->screencol - akt_winp->dx + 1;
    sw_ohne_refresh(W_AKT);  /* Fensterinhalt neu darstellen */
  }
  setz_cursor(W_AKT);  /* Cursor an seine richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       fuehre Move nach rechts aus (do_right)
*  --------
*
*  Beschreibung : Der Cursor wird intern um ein Zeichen nach rechts bewegt.
*                 Bewegt sich dabei der Cursor aus dem Fenster, so wird
*                 das Fenster gescrollt.
*
*****************************************************************************/

void do_right()
{
  /* *** interne Daten *** */
  register int hilf,  /* Zum Einlesen einer Tastenkombination */
	       i=0;   /* Zaehler, um wieviele Spalten gescrollt werden muss */

  nodelay (akt_winp->winp,TRUE); /* taste soll -1 bei nichtgedrueckter */
  do                             /* Taste liefern */
    if (right()  /* Wenn Cursor nach rechts bewegt werden konnte und */
    && akt_winp->screencol >= akt_winp->ws_col+akt_winp->dx)
      i++; /* Rand ueberschritten wurde, dann Scrollzaehler erhoehen */
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Naechste Tastenkom- */
  /* bination lesen. Falls gleich der letzten, Aktion wiederholen */
  lastcode = hilf; /* letzte Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* taste soll wieder auf Tastendruck warten */
  if(i>1)   /* Wenn um mehr als eine Spalte gescrollt werden soll, ist es */
  { /* guenstiger, den gesamten Fensterinhalt neu zu zeigen. */
    akt_winp->ws_col+=i; /* Fensterinhalt um i Spalten nach links */
    sw_ohne_refresh(W_AKT);   /* Fensterinhalt neu anzeigen */
  }
  else if (i==1)
    text_left(); /* Sonst Inhalt mit text_left um 1 nach links bewegen */
  setz_cursor(W_AKT); /* Cursor an seine richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       fuehre Move nach links aus (do_left)
*  --------
*
*  Beschreibung : Der Cursor wird intern um ein Zeichen nach links bewegt.
*                 Bewegt sich dabei der Cursor aus dem Fenster, so wird
*                 das Fenster gescrollt.
*                 Befand sich der Cursor in der ersten Spalte, so wird
*                 er um eine Zeile nach oben und dann zum Ende dieser Zeile
*                 bewegt.
*
*****************************************************************************/

void do_left()
{
  /* *** interne Daten *** */
  register int hilf,  /* Zum Einlesen einer Tastenkombination */
	       i=0,   /* Zaehler, um wieviele Spalten gescrollt werden muss */
	       j=0;   /* Zaehler, um wieviele Zeilen gescrollt werden muss */ 
  char redrawn=FALSE, /* Flag, ob Fenster schon neu gezeichnet wurde */
       must_redraw=FALSE; /* Flag, ob auf jeden Fall neu gezeichnet werden mu· */

  nodelay (akt_winp->winp,TRUE); /* Funktion taste soll /1 liefern, */
  do                             /* Falls keine Taste gedrueckt ist */
    /* konnte der Cursor nach links bewegt werden und geriet er */
    /* dadurch ausserhalb des Bildschirms, wird der Scrollzaehler erhoeht. */
    if (left())
    {
      if (akt_winp->screencol < akt_winp->ws_col-i)
	i++;
    }
    else  /* Man stand am Zeilenanfang, also eine Zeile hoch und ans Ende */
      if (up())
      {
	if (akt_winp->textline < akt_winp->ws_line)
	  j++;
	eol();
	/* Sprung ans Zeilenende kînnte Scrolling nîtig machen */
	if (akt_winp->screencol >= akt_winp->ws_col-i+akt_winp->dx)
	{
	  must_redraw = TRUE;
	  akt_winp->ws_col = akt_winp->screencol - akt_winp->dx + 1;
	  i = 0; /* Weite fÅr horizontales Scrolling zurÅcksetzen */
	}
      }
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Naechste Tasten- */
  /* kombination einlesen. Ist die gleich der letzten, Aktion wiederholen */
  lastcode = hilf; /* Letzte Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Funktion Taste soll wieder auf Tasten- */
				  /* druck warten */
  if(i>1 || must_redraw) /* Muss um mehr als eine Spalte gescrollt werden, */
  {       /* ist es guenstiger, den Fensterinhalt neu anzuzeigen */
    akt_winp->ws_col-=i; /* Fensterinhalt um i Spalten nach rechts */
    akt_winp->ws_line-=j; /* Fensterinhalt um j Zeilen nach unten */
    sw_ohne_refresh(W_AKT);   /* Fensterinhalt neu anzeigen */
    redrawn = TRUE;
  }
  else if (i==1) /* Muss nur um eine Spalte gescrollt werden, so kann das */
    text_right(); /* mit der Funktion text_right geschehen. */
  if (!redrawn)
  {
    akt_winp->ws_line-=j; /* Fensterinhalt um j Zeilen nach unten */
    if(j>1) /* Muss um mehr als eine Zeile gescrollt werden, ist es */
    {       /* guenstiger, den Fensterinhalt neu anzuzeigen */
      sw_ohne_refresh(W_AKT);   /* Fensterinhalt neu anzeigen */
    }
    else if (j==1) /* Muss nur um eine Zeile gescrollt werden, so kann das */
      text_down(0); /* mit der Funktion text_down geschehen. */
  }
  setz_cursor(W_AKT);  /* Cursor an seine richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       fuehre Move nach oben aus (do_up)
*  --------
*
*  Beschreibung : Der Cursor wird intern um eine Zeile nach oben bewegt.
*                 Bewegt sich dabei der Cursor aus dem Fenster, so wird
*                 das Fenster gescrollt.
*
*****************************************************************************/

void do_up()
{
  /* *** interne Daten und Initialisierung *** */
  register int hilf,  /* Zum Einlesen einer Tastenkombination */
	       i=0;   /* Zaehler, wie oft gescrollt werden muss */

  nodelay (akt_winp->winp,TRUE); /* Funktion taste soll -1 liefern, wenn */
  do                             /* keine Taste gedrueckt ist. */
  {
    if(up()  /* Wenn Cursor 1 Zeile nach oben bewegt werden konnte und */
    && akt_winp->textline < akt_winp->ws_line) /* dadurch Cursor ausserhalb */
      i++; /* des Fensters steht, Scrollzaehler erhoehen. */
  }while ((hilf=taste(akt_winp->winp)) == aktcode); /* Naechste Tasten- */
  /* kombination einlesen. Falls diese gleich der letzten, Aktion wiederholen */
  lastcode = hilf; /* letzte Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Funktion taste soll wieder auf */
				  /* Tastendruck warten */
  akt_winp->ws_line -= i; /* Fensterstart anpassen */
  if(i>1) /* Falls um mehr als eine Zeile gescrollt werden muss, */
    sw_ohne_refresh(W_AKT); /* komplettes Fenster neu anzeigen */
  else if(i == 1) /* Sonst kann das Scrollen mit der Funktion text_down */
    text_down(0); /* erledigt werden */
  setz_cursor(W_AKT);  /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       fuehre Move nach unten aus (do_down)
*  --------
*
*  Beschreibung : Der Cursor wird intern um ein Zeichen nach unten bewegt.
*                 Bewegt sich dabei der Cursor aus dem Fenster, so wird
*                 das Fenster gescrollt.
*
*****************************************************************************/

void do_down()
{
  /* *** interne Daten und Initialisierung *** */
  register int hilf,  /* Zum Einlesen einer Tastenkombination */
	       i=0;   /* Zaehler, wie weit gescrollt werden muss */

  nodelay (akt_winp->winp,TRUE); /* Funktion taste soll -1 liefern, falls */
  do                             /* keine Taste gedrueckt ist.            */
  {
    if(down() /* Falls Cursor um eine Zeile nach unten bewegt werden konnte */
    && akt_winp->textline >= akt_winp->ws_line+akt_winp->dy) /* und Cursor */
      i++; /* dadurch Fenster verlaesst, Scrollzaehler erhoehen */
  }while ((hilf=taste(akt_winp->winp)) == aktcode); /* Tastenkombination */
  /* einlesen. Falls diese gleich der letzten, Aktion wiederholen */
  lastcode = hilf;  /* Letzte Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Funktion taste soll wieder auf Tasten- */
				  /* druck warten */
  akt_winp->ws_line += i; /* Fensterinhalt um i Zeilen nach oben */
  if(i>1)  /* Falls mehr als eine Zeile gescrollt werden muss, */
    sw_ohne_refresh(W_AKT); /* ganzes Fenster neu zeichnen */
  else
    if(i == 1)         /* Sonst Scrolling mit Funktion text_up erledigen */
      text_up(0);
  setz_cursor(W_AKT);       /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       Tablaenge setzen (do_settab)
*  --------
*
*  Beschreibung : Die Tablaenge wird neu eingestellt. Ist die Eingabe leer,
*                 so wird die Tablaenge nicht veraendert.
*
*****************************************************************************/

void do_settab()
{
  /* *** interne Daten *** */
  char dummy[100],  /* String fuer Ausgabeaufbereitung */
       num_str[6];  /* String fuer Eingabe */
  int  num;         /* Ergebnis der Umwandlung von num_str in integer */

  sprintf(dummy,PROMPT_TABLEN,akt_winp->tablen);
  print_stat(dummy);            /* Prompt ausgeben */
  read_stat(num_str,6,GS_NUM);  /* Eingabe lesen (nur Ziffern (GS_NUM)) */
  clear_stat();                 /* Statuszeile loeschen */
  if((num = atoi(num_str)) > 0) /* Eingabe in Integer umwandeln */
    akt_winp->tablen = num;     /* Falls Wert groesser 0, als neue Tab- */
  setz_cursor(W_AKT);                /* laenge merken, Cursor positionieren */
}

/*****************************************************************************
*
*  Funktion       eine Tabposition zurueckspringen (do_backtab)
*  --------
*
*  Beschreibung : Abhaengig vom Autoindent- und Insertmodus wird auf den
*                 Backtab-key reagiert:
*                       AUTOINDENT: Tab richtet sich nach darueberliegender
*                                   Zeile.
*                       INSERT:     feste Tabweite zurueckspringen
*
*****************************************************************************/

void do_backtab()
{
  /* *** interne Daten *** */
  register int diff,  /* Entfernung zur naechsten Tabgrenze (nur benutzt, */
		      /* falls Autoindent abgeschaltet) */
	       nsc;   /* Spaltennummer der Tabgrenze links des Cursors */

  if(akt_winp->autoindflag && up()) /* Falls Autoindent eingeschaltet, und */
  { /* Cursor eine Zeile nach oben bewegt werden konnte, */
    /* naechste Tabgrenze links vom Cursor nsc zuweisen. */
    if ((nsc = akt_winp->tablen * ((akt_winp->screencol-1)/akt_winp->tablen))
    >= fastll (akt_winp->alinep->text)) /* Ist nsc hinter Zeilenende, */
      akt_winp->screencol = nsc; /* screencol auf nsc setzen. */
    else                  /* Ist nsc in der Zeile, dann ein Wort nach links */
      word_left();
    down();   /* Anschliessed in die urspruengliche Zeile zurueckkehren */
  }
  else  /* Falls Autoindent ausgeschaltet ist, Entfernung zur naechsten */
	/* links vom Cursor gelegenen Tab-Grenze berechnen */
    for(diff = (akt_winp->screencol-1)%akt_winp->tablen + 1;diff>0;diff--)
      left(); /* So oft dann den Cursor nach links bewegen */

  if(akt_winp->ws_col > akt_winp->screencol) /* Falls Cursor dadurch */
  {                                          /* links vom Fenster, */
    akt_winp->ws_col = akt_winp->screencol;  /* Aktuelle Spalte zur ersten */
    sw_ohne_refresh(W_AKT); /* sichtbaren Spalte machen, Fensterinhalt neu zeigen */
  }
  setz_cursor(W_AKT); /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       Tab einfuegen (do_tab)
*  --------
*
*  Beschreibung : Abhaengig vom Autoindent- und Insertmodus wird auf den
*                 Tab-key reagiert:
*                       AUTOINDENT: Tab richtet sich nach darueberliegender
*                                   Zeile.
*                       INSERT:     Spaces werden eingefuegt.
*                 Achtung: Es wird vorausgesetzt, da· word_right() nur
*                          vorwaerts geht, die Distanz zur aktuellen Spalte
*                          also immer positiv ist!
*
*****************************************************************************/

void do_tab()
{
  /* *** interne Daten und Initialisierung *** */
  register int old_sc,  /* Zwischenspeicher fuer Cursorspalte */
	       dsc = 0, /* Entfernung, die durch Tab geskipt wird */
	       anz_ins; /* Anzahl der eingefuegten Blanks */
  char         scrolled = FALSE; /* Flag, ob enter_char scrollte */

  if(akt_winp->autoindflag && up()) /* Bei Autoindent eine Zeile hoch */
  {
    old_sc = akt_winp->screencol; /* falls das klappt, Spalte merken */
    word_right();                 /* ein Wort nach rechts */
    down();                       /* und wieder in alte Zeile */
    dsc = akt_winp->screencol - old_sc; /* "Streckenlaenge" merken */
    akt_winp->screencol = old_sc; /* Screencol wieder auf alten Wert */
  }
  /* falls normaler Tab oder bei autoindent nicht weitergegangen werden */
  if(!dsc) /* konnte, Entfernung zur naechsten festen Tabgrenze berechnen */
    dsc = akt_winp->tablen - akt_winp->screencol%akt_winp->tablen;
  if(akt_winp->insflag) /* Falls Insert-Mode aktiv, dsc Blanks einfuegen */
  {
    if((anz_ins = insert(dsc)) < dsc) /* Falls weniger als dsc chars ein- */
    { /* gefuegt werden konnten, eingefuegte Zeichen loeschen */
      mdelete(anz_ins);
      print_err("Zeile zu lang! ");  /* und Fehlermeldung ausgeben */
      return;
    }
    else
      while(anz_ins--)   /* Eingefuegte Zeichen mit Blanks belegen */
	enter_char(' ', &scrolled, scrolled?PUT:INSERT, FALSE);
  }
  else  /* Falls im Overwrite-Modus, dsc Zeichen nach rechts bewegen */
    while(dsc--)
      right();
  if(akt_winp->ws_col + akt_winp->dx <= akt_winp->screencol)
  { /* Falls Cursor ausserhalb des Fensters, Fensterinhalt anpassen */
    /* Aktuelle Spalte soll in der Mitte des Fensters stehen */
    akt_winp->ws_col = akt_winp->screencol - akt_winp->dx/2;
    sw_ohne_refresh(W_AKT); /* Anschliessend Fensterinhalt neu anzeigen */
  }
  setz_cursor(W_AKT);  /* Cursor an seine Position plazieren */
}

/*****************************************************************************
*
*  Funktion       Page Up ausfuehren (do_pgup)
*  --------
*
*  Beschreibung : Der interne Cursor wird um eine Seite hochbewegt. Dann
*                 wird die Fensterposition angepasst und der Fensterinhalt
*                 erneut dargestellt.
*
*****************************************************************************/

void do_pgup()
{
  /* *** interne Daten *** */
  register int hilf;  /* Zum Einlesen einer Tastenkombination */

  nodelay (akt_winp->winp,TRUE);  /* Funktion taste soll -1 liefern, */
  do                              /* falls keine Taste gedrueckt ist */
    if((akt_winp->ws_line -= screen_up()) < 0) /* Neuen Fensterstart */
      akt_winp->ws_line = 0;                   /* berechnen          */
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Naechste Tasten- */
  /* kombination lesen. Ist sie gleich der letzten, Aktion wiederholen */
  lastcode = hilf; /* zuletzt gelesene Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Fkt. Taste soll auf Taste warten */
  show_win(W_AKT);                 /* Text im Fenster neu anzeigen     */
}

/*****************************************************************************
*
*  Funktion       Page Down ausfuehren (do_pgdn)
*  --------
*
*  Beschreibung : Der interne Cursor wird um eine Seite nach unten bewegt.
*                 Dann wird die Fensterposition angepasst und der
*                 Fensterinhalt erneut dargestellt. Steht der Cursor in der
*                 letzten Textzeile, so wird diese in die Bildschirmmitte
*                 plaziert.
*
*****************************************************************************/

void do_pgdn()
{
  /* *** interne Daten *** */
  register int hilf;  /* Zum Einlesen einer Tastenkombination */

  nodelay (akt_winp->winp,TRUE);  /* Funktion taste soll -1 liefern, */
  do                              /* falls keine Taste gedrueckt ist */
    akt_winp->ws_line += screen_down(); /* Cursor um eine Seite nach unten */
				      /* bewegen und Fensterstart anpassen */
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Naechste Tasten- */
  /* kombinationo einlesen. Falls gleich der letzten, Aktion wiederholen */
  lastcode = hilf;                    /* letzte Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Fkt. taste soll auf Taste warten */
  show_win(W_AKT);                  /* Text im Fenster neu anzeigen     */
}

/*****************************************************************************
*
*  Funktion       leere Zeile vor aktueller einfuegen (do_open)
*  --------
*
*  Beschreibung : vor der aktuellen Zeile wird eine Zeile eingefuegt, die dann
*                 zur aktuellen wird.
*
*****************************************************************************/

void do_open()
{
  /* *** interne Daten *** */
  int old_sc;  /* Zwischenspeicher fuer alte Cursorspalte          */

  if(akt_winp->maxline < MAX_ANZ_LINES-1) /* Nur neue Zeile einfuegen, */
  {      /* wenn dadurch maximale Zeilenzahl nicht ueberschritten wird */
    check_buff();  /* evtl. noch im Puffer befindliche Daten in Text kopieren */
    if(akt_winp->maxline >= 0)   /* Enthaelt Text mindestens eine Zeile ? */
    {                     /* Ja, dann muss auf Autoindent geachtet werden */
      akt_winp->alinep = akt_winp->alinep->prev; /* da up nicht vor die erste */
      akt_winp->textline--; /* Zeile geht, muss man "zu Fuss" eine Zeile hoch.    */
      old_sc = akt_winp->screencol;    /* Zeile wird dahinter eingefuegt, */
      akt_winp->screencol = MAXLENGTH; /* also screencol aufs Ende        */
      koppel_line(ADAPT_COORS);  /* Zeile in Textstruktur einfuegen      */
      if (akt_winp->autoindflag) /* Bei Autoindent stellt man sich ueber */
	indent_line(FALSE);           /* den Anfang der Zeile darunter   */
      else
	akt_winp->screencol = old_sc;

      if(akt_winp->screencol < akt_winp->ws_col /* Wenn Cursor links oder */
      || akt_winp->ws_col+akt_winp->dx < akt_winp->screencol) /* rechts   */
      { /* ausserhalb des Fensters steht, wird die aktuelle Spalte zur    */
	akt_winp->ws_col = akt_winp->screencol; /* ersten sichtbaren      */
	sw_ohne_refresh(W_AKT);  /* Fensterinhalt neu anzeigen                 */
      }
      else /* Steht Cursor innerhalb des Fensters, wird mit text_down eine */
	text_down(akt_winp->textline - akt_winp->ws_line); /* Zeile auf    */
    }   /* dem Bildschirm eingefuegt */
    else  /* War der Text leer, wird lediglich eine neue Zeile erzeugt */
      koppel_line(IGNORE_COORS); /* Marker und lastpos sind im leeren Text egal */
    setz_cursor(W_AKT);      /* Cursor an richtige Position setzen */
  }
  else                /* Wuerde der Text durch eine weitere Zeile zu lang, */
    print_err(T_SIZE_ERRTEXT);     /* Fehlermeldung ausgeben */
}

/*****************************************************************************
*
*  Funktion       leere Zeile hinter aktueller einfuegen (do_hopen)
*  --------
*
*  Beschreibung : hinter der aktuellen Zeile wird eine Zeile eingefuegt, die dann
*                 zur aktuellen wird.
*
*****************************************************************************/

void do_hopen()
{
  /* *** interne Daten und Initialisierung *** */
  int hilfs  = akt_winp->shellflag, /* Zwischenspeicher fuer shellflag */
      hilfi  = akt_winp->insflag,   /* Zwischenspeicher fuer insflag   */
      old_sc = akt_winp->screencol; /* Zwischenspeicher fuer screencol */

  check_buff(); /* evtl. noch im Puffer stehende Daten in Text uebertragen */
  akt_winp->screencol = fastll(akt_winp->alinep->text); /* Ans Zeilenende */
  akt_winp->insflag = TRUE; /* so wird auf jeden Fall neue Zeile eingefuegt */
  akt_winp->shellflag = FALSE;    /* keine Zeile an Shell schicken */
  if(!do_newline())    /* Zeile einfuegen und testen, ob das geklappt hat */
    akt_winp->screencol = old_sc; /* Falls nein, Cursor wieder in alte Spalte */
  akt_winp->insflag = hilfi;      /* Insflag und shellflag restaurieren */
  akt_winp->shellflag = hilfs;
  setz_cursor(W_AKT);         /* Damit Flags in Kopf richtig angezeigt werden */
}

/*****************************************************************************
*
*  Funktion       newline ausfuehren (do_newline)
*  --------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : TRUE: Es konnte noch eine Zeile eingefuegt
*                                  werden.
*
*  Beschreibung : Die aktuelle Zeile wird evtl. gesplittet und in der
*                 Textstruktur eine neue Zeile angelegt. Falls dabei der
*                 Cursor aus dem Fenster laeuft, wird der Bildschirm ge-
*                 scrollt.
*
*****************************************************************************/

int do_newline()
{
  /* *** interne Daten und Initialisierung *** */
  char *cmd,           /* An SHELL zu uebergebende Kommandozeile */
       swflag = FALSE; /* Zeigt an, ob Fensterinhalt neu gezeigt werden muss */
  int  old_sc;         /* Zwischenspeicher fuer Cursorspalte */

  if (akt_winp->insflag) /* Falls im Insert-Mode, dann */
    if (new_line())      /* Zeile einfuegen und testen, ob das geklappt hat */
    { /* Testen, ob Fenster gescrollt werden muss */
      if (akt_winp->textline-akt_winp->ws_line >= akt_winp->dy)
      {
	akt_winp->ws_line++; /* wenn ja, Nummer der ersten Zeile + 1 */
	text_up(0);          /* und Fensterinhalt scrollen */
      }
      else /* Wenn nicht gescrollt werden muss, eine Zeile einfuegen */
	text_down(akt_winp->textline-akt_winp->ws_line);
      if(akt_winp->autoindflag)    /* Wenn Autoindent aktiv ist,       */
	indent_line(TRUE);         /* neue Zeile korrekt einruecken    */
				   /* nur die Zeile neu gezeigt werden */
      /* Dann wird getestet, ob der Cursor */
      if(akt_winp->ws_col + akt_winp->dx <= akt_winp->screencol /* rechts */
      || akt_winp->screencol < akt_winp->ws_col) /* oder links vom */
      {                                          /* Fenster steht  */
	akt_winp->ws_col = akt_winp->screencol;  /* Wenn ja, dann wird */
	swflag = TRUE; /* aktuelle Spalte zur ersten sichtbaren, und   */
      }            /* das Fenster wird als neu zu zeichnend markiert   */
      else         /* Wenn der Cursor innerhalb des Fensters blieb,    */
	lineout(akt_winp->textline - akt_winp->ws_line); /* dann muss  */
    }                          /* die neue Textzeile angezeigt werden  */
    else       /* Wenn keine neue Zeile eingefuegt werden konnte, dann */
    {
      print_err (T_SIZE_ERRTEXT); /* Fehlermeldung ausgeben */
      return (FALSE);             /* und Funktion abbrechen */
    }
  else                  /* OVERWRITE-modus */
  {
    if (!down())        /* Cursor eine Zeile nach unten bewegen */
      return(FALSE);    /* Klappte das nicht, Funktion abbrechen */
    else                /* Klappte es, Cursor an Zeilenanfang stellen */
      bol();
    if (akt_winp->textline-akt_winp->ws_line >= akt_winp->dy) /* Testen, */
    { /* ob Cursor nach unten aus dem Fenster gegangen ist */
      akt_winp->ws_line++; /* Wenn ja, Fensterstart anpassen */
      text_up(0);          /* und Fensterinhalt scrollen */
    }
    /* Wenn Cursor links vom Rand steht, dann muss die */
    /* erste Spalte sichtbar sein                      */
    if(akt_winp->ws_col)
    { /* Ist das nicht so, wird der Fensterinhalt als neu zu zeichnend   */
      swflag = TRUE;        /* markiert und Spalte 0 als erste sichtbare */
      akt_winp->ws_col = 0; /* markiert. */
    }
  }

     /*** Testen, ob Zeile an SHELL uebergeben werden muss *** */

  if(akt_winp->shellflag && (cmd = akt_winp->alinep->prev->text))
  { /* Das ist der Fall, wenn shellflag gesetzt ist und die aufgespaltene */
    old_sc = akt_winp->screencol; /* Zeile nicht leer ist. Dann wird die  */
    up();  /* Cursorspalte gemerkt und der Cursor eine Zeile hochbewegt.  */
    if(akt_winp->maxline < MAX_ANZ_LINES-1) /* Platz fuer weitere Zeile ? */
    {
      koppel_line(ADAPT_COORS);  /* Zeile einfuegen, so dass Shellausgabe */
			     /* nicht mit evtl. Zeilenrest vermischt wird */
      if(to_shell(cmd))   /* nur wenn auch etwas eingefuegt werden konnte */
      {                    /* eine Zeile nach Ende der Ausgabe anspringen */
	gotox(akt_winp->block.e_line+1); /* Steht Cursor anschliessend    */
	/* unter dem Fenster, Fensterposition anpassen */
	if(akt_winp->textline >= akt_winp->ws_line + akt_winp->dy)
	  akt_winp->ws_line = akt_winp->textline - akt_winp->dy/2;
	swflag = TRUE;        /* veraenderter Bildschirminhalt */
      }
      else                     /* Wenn keine Shellausgabe vorhanden ist, */
	del_line(ADAPT_COORS); /* eingefuegte Leerzeile wieder loeschen  */
    }
    else                       /* konnte keine Zeile mehr einfuegen */
      pe_or(T_SIZE_ERRTEXT);   /* also Fehlermeldung ausgeben */
    akt_winp->screencol = old_sc; /* Cursorspalte restaurieren */
  }
  if(swflag)            /* Wenn Fensterinhalt als zu restaurierend gekenn- */
    sw_ohne_refresh(W_AKT);  /* zeichnet, Fensterinhalt neu anzeigen */
  setz_cursor(W_AKT);        /* Cursor an richtige Position setzen */
  return (TRUE);
}

/*****************************************************************************
*
*  Funktion       loesche eine Zeile (do_delline)
*  --------
*
*  Beschreibung : Die aktuelle Zeile wird aus der Textstruktur geloescht
*                 und der Bildschirm aktualisiert.
*
*****************************************************************************/

void do_delline()
{
  /* *** interne Daten und Initialisierung *** */
  register int i=0,  /* Zaehler fuer geloeschte Zeilen */
	       hilf, /* Zum Einlesen einer Tastenkombination */
	       ret;  /* Zwischenspeicher fuer Rueckgabewert von del_line */

  nodelay (akt_winp->winp,TRUE); /* Funktion taste soll -1 liefern, falls */
  do                             /* keine Taste gedrueckt ist. */
  {
    save_delline();              /* zu loeschende Zeile abspeichern */
    if((ret = del_line(ADAPT_COORS)) != NO_LINE_DEL) /* Zeile loeschen, */
      i++;    /* Rueckgabewert von del_line merken und Zaehler anpassen */
  }while ((hilf=taste(akt_winp->winp)) == aktcode); /* Naechste Tasten- */
  /* kombination einlesen. Falls gleich der letzten, Aktion wiederholen */
  lastcode = hilf; /* Zuletzt gelesene Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Fkt. taste soll auf Taste warten */

  if(i == 1 && ret == LAST_LINE_DEL) /* Wenn nur die letzte Zeile geloescht */
  {  /* wurde, dann diese Zeile auf Bildschirm loeschen, Rahmen reparieren */
    wmove(akt_winp->winp,akt_winp->maxline+2-akt_winp->ws_line,1);
    wclrtoeol(akt_winp->winp);
    mvwaddch(akt_winp->winp,akt_winp->maxline+2-akt_winp->ws_line,akt_winp->dx + 1,REST_CHAR);
    setz_cursor(W_AKT);
  }
  if(i) /* Wenn mindestens eine Zeile geloescht wurde, testen ob Cursor */
  {     /* nach oben aus dem Fenster gewandert ist.                     */
    if(akt_winp->textline < akt_winp->ws_line) /* Wenn ja, dann warum ? */
      if(akt_winp->textline == -1) /* Wenn wegen "Text leer", dann      */
	akt_winp->ws_line = 0;     /* ws_line auf 0 setzen              */
      else                         /* Sonst aktuelle Zeile zur ersten   */
	akt_winp->ws_line = akt_winp->textline; /* sichtbaren machen    */
    show_win(W_AKT); /* Anschliessend Fensterinhalt anzeigen                 */
  }
} 

/*****************************************************************************
*
*  Funktion       Zeilenverknuepfung ausfuehren (do_join)
*  --------
*
*  Beschreibung : Mittels der Funktion join werden die aktuelle und die
*                 nachfolgende Zeile verknuepft. Bei der VerknÅpfung wird
*                 zwischen dem ersten non-Whitespace der nachfolgenden Zeile
*                 und dem letzten Zeichen der aktuellen Zeile genau ein
*                 Space eingefÅgt. Falls bei diesen Aktionen ein Fehler
*                 auftritt, wird gegebenfalls eine Fehlermeldung ausgegeben.
*
*****************************************************************************/ 

void do_join()
{
  /* *** interne Daten *** */
  char first_of_2nd_is_blank,  /* Flag, ob nachfolgende Zeile mit WS beginnt */
       second_is_empty,        /* Flag, ob nachfolgende Zeile leer ist       */
       dw,                     /* Flag, ob Leerraum zu lîschen ist           */
       first_is_blank;         /* Flag, ob aktuelle Zeile leer ist           */

  if (akt_winp->maxline > 0      /* Geht nur, wenn mehr als eine Zeile */
  && akt_winp->alinep->next != akt_winp->dummyp) /* und aktuelle Z. */
  {                              /* nicht letzte Zeile ist */
    second_is_empty = !akt_winp->alinep->next->text;
    first_of_2nd_is_blank = (second_is_empty ||
			     *(akt_winp->alinep->next->text) == ' ');
    dw = (!second_is_empty && strlen(akt_winp->alinep->next->text) >= 2
	  && *(akt_winp->alinep->next->text+1) == ' ');
    first_is_blank = !akt_winp->alinep->text;
    do_eol();            /* ZunÑchst ein Zeichen rechts vom Zeilenende */
    switch (join(ADAPT_COORS))   /* Verknuepfung ausfuehren, Ergebnis testen */
    {
      case J_TOOLONG : print_err(L_SIZE_ERRTEXT); /* Zeile wurde zu lang,    */
		       break;                     /* Meldung ausgeben        */
      case J_OK      : if(first_of_2nd_is_blank   /* Falls nÑchste Zeile     */
		       && !second_is_empty)       /* nicht leer ist und      */
						  /* mit einem Blank beginnt */
		       {
			 do_right();      /* ein Blank dazwischen lassen */
			 if (dw)
			    do_del_word();   /* Text ranziehen */
		       }
		       else               /* Sonst Blank einfÅgen */
		       { /* geht gut, da bei join mindestens ein Zeichen
			    Reserve. Aber nur einfÅgen, wenn erste Zeile 
			    nicht leer war ! */
			 if (!first_is_blank)
			 {
			   insert(1);  /* Platz am Zeilenende lÑ·t */
			   enter_char(' ', (char*) 0, akt_winp->insflag, FALSE);
			 }
		       }
		       show_win(W_AKT); /* Alles OK, Fensterinhalt neu anzeigen */
    }
  }
}

/*****************************************************************************
*
*  Funktion       Springe bestimmte Zeile an (do_goto)
*  --------
*
*  Beschreibung : Nach Einlesen einer Zeilennummer wird, falls moeglich,
*                 die aktuelle Zeile entsprechend gesezt.
*
*****************************************************************************/

void do_goto()
{
  /* *** interne Daten *** */
  char number[10]; /* String fuer einzulesende Zeilennummer */
  int numi;        /* Eingelesene Zeilennummer als Integer  */

  print_stat(PROMPT_LINENUMB);
  read_stat(number,9,GS_NUM);     /* Zeilennummer einlesen (nur Ziffern) */
  clear_stat();                   /* Statuszeile wieder loeschen */

  /* String in int wandeln und Bereichspruefung durchfuehren */
  if((numi = atoi(number)) > 0 && numi <= akt_winp->maxline+1)
  { /* Wenn Zeilennummer im korrekten Bereich, dann */
    akt_winp->lastline = akt_winp->textline; /* aktuelle Position als  */
    akt_winp->lastcol = akt_winp->screencol; /* letzte Position merken */
    gotox(numi-1);                     /* gewuenschte Zeile anspringen */
    if((akt_winp->ws_line = numi-akt_winp->dy/2) < 0) /* Zeile in der Mitte */
      akt_winp->ws_line = 0; /* plazieren. Falls das nicht geht, weil       */
      /* Zeilennummer zu klein, ws_line entsprechend anpassen               */
    sw_ohne_refresh(W_AKT);  /* Fensterinhalt neu anzeigen */
  }
  setz_cursor(W_AKT);        /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       beende editor (do_ende)
*  --------
*
*  Beschreibung : Nach einer Rueckfrage wird der Editor evtl. beendet.
*                 Nach dem letzten Speichern geaenderte Files werden ab-
*                 gespeichert. Danach wird das Config-File geupdatet.
*
*****************************************************************************/

void do_ende()
{
  /* Meldung ausgeben und J/N-Abfrage vornehmen */
  if(!did_anything_change() || ja_nein(PROMPT_WARNSAVE))
    /* Will User wirklich beenden, alle geaenderten Dateien sichern */
    if(save_all() || ja_nein(PROMPT_EXITANYWY)) /* Bei Fehler Abfrage */
    { /* Wenn Speichern klappte, oder Abfrage mit J beantwortet wurde, */
      write_config();  /* wird die Konfigurationsdatei gespeichert, */
      alles_frei();    /* aller reservierter Speicher freigegeben   */
      ende(0, TRUE);   /* und der Editor beendet.                   */
    }
  setz_cursor(W_AKT); /* Will der User doch nicht abbrechen, wird der Cursor */
}                /* an seine korrekte Position gesetzt. */

/******************************************************************************
*
* Funktion     : Loadfile ausfÅhren (ex_load)
* --------------
*
* Beschreibung : Das in der globalen Variablen spezifizierte loadfile wird
*                ausgefÅhrt. Dazu wird der Inhalt der Datei in eine tempo-
*                rÑre Datei kopiert (mit system("copy...")). An die temporÑre
*                Datei wird eine Zeile angehÑngt, die "exit" enthÑlt.
*                Dann wird command.com Åber spawnlp aufgerufen, als Parameter
*                wird "<"+<Name der temporÑren Datei> Åbergeben. Die Shell
*                liest also die Kommandos aus der temporÑren Datei und wird
*                anschlie·end durch exit beendet. Anschlie·end wird die
*                temprÑre Datei gelîscht und der Return-Wert von spawnlp
*                behandelt.
*
******************************************************************************/

void ex_load()
{
  int sys_ret;      /* RÅckgabewert der system-Funktion */
  int ex_st;

  if(access(loadfile,4) == 0)  /* Lese-Zugriff auf Loadfile ? */
  {
    sys_ret = system(loadfile);
    if(sys_ret == -1)
      fprintf(stderr,NO_LOAD_ERRTEXT);  /* Falls system nicht klappte, */
    ex_st=sys_ret;       /* Fehlermeldung ausgeben und ende */
  }
  else
  {
    fprintf(stderr,"Loadfile %s nicht gefunden !\n",loadfile);
    ex_st=1;
  }
#ifdef MOUSE
#ifdef OS2
  mouse_active = FALSE;
  DosWaitThread (&mouse_ThreadID, DCWW_WAIT);
#else
  set_mouse_int(0); /* Mausroutine maskieren */
#endif
#endif
#ifndef OS2
  *(long*)(27*4) = old_int; /* Vektor des Break-Interruptes restaurieren */
#endif
  exit (ex_st);
}

/*****************************************************************************
*
*  Funktion       beende editor mit loadfile (do_endemit)
*  --------
*
*  Beschreibung : Nach einer Rueckfrage wird der Editor evtl. beendet.
*                 Nach dem letzten Speichern geaenderte Files werden ab-
*                 gespeichert. Danach wird das Config-File geupdatet
*                 und das Load-File ausgefuehrt.
*
*****************************************************************************/

void do_endemit()
{
  /* Funktion der Abfragen s.o. */
  if(!did_anything_change() || ja_nein(PROMPT_WARNSAVE))
    if(save_all() || ja_nein(PROMPT_EXITANYWY))
    {
      write_config();  /* Konfigurationsdatei abspeichern */
      alles_frei();    /* Saemtlichen reservierten Speicherplatz freigeben */
      clear();         /* Bildschirm loeschen */
      refresh();
      endwin();        /* Curses "abschalten" */
      ex_load();       /* Loadfile ausfÅhren, Programm wird beendet */
    }
  setz_cursor(W_AKT); /* Will User doch nicht beenden, Cursor wieder plazieren */
}


/*****************************************************************************
*
*  Funktion       verlasse editor (quit)
*  --------
*
*  Beschreibung : Das Programm wird (nach Rueckfrage) beendet.
*
*****************************************************************************/

void quit()
{
  if (!did_anything_change() || ja_nein (PROMPT_WARNQUIT))
  { /* Sicherheitsabfrage */
    write_config();  /* Wenn User wirklich beenden will, Config-Datei */
    alles_frei();    /* speicher, Speicherplatz freigeben, */
    ende(0, TRUE);   /* Editor beenden */
  }
  setz_cursor(W_AKT);     /* Falls doch nicht beendet werden soll, Cursor */
}                    /* wieder an richtige Position stellen */

/*****************************************************************************
*
*  Funktion       quit mit Ausfuehren des load-Files (quitmit)
*  --------
*
*  Beschreibung : Nach einer positiv beantworteten Rueckfrage wird die
*                 Config-Datei gespeichert und der Editor mit dem
*                 Load-File ueberlagert.
*
*****************************************************************************/

void quitmit()
{
  if (!did_anything_change() || ja_nein (PROMPT_WARNQUIT))
  { /* Sicherheitsabfrage */
    write_config(); /* Falls mit J geantwortet wurde, Config-Datei */
    alles_frei();   /* speichern, Speicherplatz freigeben, */
    clear();        /* Bildschirm loeschen */
    refresh();
    endwin();       /* Curses "abschalten" */
    ex_load();
  }
  setz_cursor(W_AKT); /* Wollte User doch nicht beenden, Cursor plazieren */
}
