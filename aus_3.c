/****************************************************************/
/*                                                              */
/*      MODUL:  aus_3.c                                         */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - do_control (Kontrollzeichen in Text einfg.)   */
/*              - hndl_insert (Insert-Teil von do_inschar)      */
/*              - hndl_overwrite (Overwrite-Teil von do_inschar)*/
/*              - do_inschar (Zeichen in Text einfuegen)        */
/*              - do_tog_tkm (Tabkomprimierungsmodus togglen)   */
/*              - do_tog_bak (Backupfileerzeugungsmodus togglen)*/
/*              - do_tog_ai (Autoindentmodus togglen)           */
/*              - do_movewin (Fensterbewegung kontrollieren)    */
/*              - do_toggle_size (Fenster auf gespeicherte Gr.) */
/*              - do_sizewin (Fenstergroessenanpassung kontr.)  */
/*              - do_swnext (Zum "Nachfolger" des akt. Fensters)*/
/*              - do_swprev (Zum Vorgaenger des akt. Fensters)  */
/*              - do_swnum (Window absolut anspringen)          */
/*              - do_repeat (Funktion mehrmals ausfuehren)      */
/*              - do_blstart (Blockanfang setzen)               */
/*              - do_blnormend (Blockende (normal) setzen)      */
/*              - do_blrechtend (Blockende (Rechteck) setzen)   */
/*              - do_blunmark (Blockmarkierung loeschen)        */
/*              - do_blweg (Block loeschen)                     */
/*              - do_blcopy (Block kopieren)                    */
/*              - do_blmove (Block verschieben)                 */
/*              - do_blcut (Block in PASTE-Puffer kopieren)     */
/*              - do_blpaste (PASTE-Puffer in Text einfuegen)   */
/*              - do_blindent (Block einruecken)                */
/*              - do_blread (Block einlesen)                    */
/*              - do_blwrite (Block schreiben)                  */
/*              - do_goblend (Zum Blockende gehen)              */
/*              - do_goblanf (Zum Blockanfang gehen)            */
/*              - do_bltofil (Block an Filter uebergeben)       */
/*              - do_toghbl (Blockhervorhebungsmodus togglen)   */
/*              - do_shelltog (Shellflag invertieren)           */
/*              - do_setmarker (Marker setzen)                  */
/*              - do_jumpmarker (Marker anspringen)             */
/*              - do_lastpos (Letzte Position anspringen)       */
/*              - do_swname (Window gemaess Name suchen)        */
/*              - do_newname (Datei unter neuem Namen speichern)*/
/*              - do_macro (Macro definieren/ausfuehren)        */
/*              - do_restline (geloeschte Zeile wiederherstelen)*/
/*              - auswertung (Tastendruck auswerten)            */
/****************************************************************/

#include "defs.h"
#include "keys.h"

void do_inschar();

extern char backupflag,highblockflag,clear_buff,bufflag;
extern int schreib_file(),to_shell(),save_all();
extern short int letter,lastcode,aktcode,mc_index,taste();
extern int save_delline(),rest_delline();
extern int tst_overlap();
extern char bufflag,*loadfile,regexflag;
extern bzeil_typ *save_normal(),*save_rechteck();
extern block_typ global_block,*dup_block();
extern puff_typ macro[],puff_feld[];
extern int ks_index;
extern short int *keystack,*e_keystack,newwgetch();
extern WINDOW *status;
extern marker_typ marker[];
extern char *on_off[], /* Hilfstexte */
		       /* fuer Togglen der globalen Flags */
	    helpflag;  /* Flag: Hilfstexte anzeigen       */

extern void do_refresh(), do_bol(),do_eol(),do_halfup(),do_halfdn(),do_delete(),
	    do_backspace(),do_home(),do_nothome(),do_insovr(),do_textbeginn(),
	    do_eot(),do_del_word(),do_wleft(),do_wright(),do_right(),do_left(),
	    do_up(),do_down(),do_pgup(),do_pgdn(),do_newline(),do_delline(),
	    ueberschreiben(),do_schreib_file(),quit(),do_help(),laden(),
	    do_win_zu(),do_goto(),do_ende(),do_find(),do_replace(),
	    do_underline(),do_z_hoch(),do_z_runter(),do_z_oben(),
	    do_z_mitte(),do_z_unten(),do_deleol(),do_repfr(),do_repfr(),
	    do_join(),do_open(),do_tab(),do_settab(),do_hopen(),
	    do_backtab(),do_endemit(),quitmit(),do_find(), do_repfr(),
	    do_replace(),do_matchpar(),do_middle();


/*****************************************************************************
*
*  Funktion       Kontrollzeichen in Text (do_control)
*  --------
*
*  Beschreibung : Es wird ein Zeichen mit getch() eingelesen und ueber die
*                 Funktion do_inschar in den Text plaziert.
*
*****************************************************************************/

void do_control()
{
  /* *** interne Daten, Initialisierung *** */

  if(letter = newwgetch(akt_winp->winp)) /* Ein Zeichen lesen. Nur wenn es */
    do_inschar();                   /* nicht '\0' ist, wird es eingefuegt. */
}


/*****************************************************************************
*
*  Funktion       Insert-Teil von do_inschar behandeln (hndl_insert)
*  --------
*
*  Paramater    : restn     :
*                   Typ         : int *
*                   Wertebereich: Pointer auf Integer
*                   Bedeutung   : Anzahl der Zeichen, die noch einge-
*                                 fuegt werden muessen
*
*  Ergebnis     :
*                   Typ         : int
*                   Wertebereich: TRUE, FALSE
*                   Bedeutung   : TRUE: hat geklappt
*                                 FALSE: hat nicht geklappt
*
*  Beschreibung : Es wird versucht, Platz fuer die einzufuegenden Zeichen
*                 zu schaffen. Gelingt es nicht, ein einziges Zeichen ein-
*                 zufuegen, wird eine neue Zeile eingefuegt. Je nachdem ob
*                 die aktuelle Zeile am Ende aufgespalten wurde oder nicht
*                 wird mit dem Einfuegen in der aktuellen oder der neuen
*                 Zeile fortgefahren.
*
*****************************************************************************/

int hndl_insert(restn)
int *restn;
{
  /* *** interne Daten *** */
  int hilf;

  /* Anzahl eingefuegter Zeichen merken und von restn abziehen */
  *restn -= (hilf = insert (*restn));
  if (!hilf)        /* wenn keine Zeichen eingefuegt werden konnten   */
  {
    if (!new_line())     /* neue Zeile einfuegen */
    {
      print_err(T_SIZE_ERRTEXT); /* klappte das nicht, Meldung aus- */
      return(FALSE);             /* geben und Funktion beenden      */
    }
    if (akt_winp->autoindflag)   /* Eventuell aufgespaltene Zeile   */
      indent_line();             /* korrekt einruecken              */
    if (akt_winp->alinep->text)  /* neue Zeile nicht leer?  */
    {
      up();                 /* in alte Zeile, da dort noch Platz */
      eol();                        /* ans Ende */
      if(insert(1))         /* wirklich noch Platz (kann nur  */
      {
	sw_ohne_refresh(W_AKT);
	(*restn)--;         /* Fehler ergeben bei unterstr.Zchn.) ? */
      }                     /* wenn nicht, schlaegt enter_char */
    }                       /* fehl und do_newline wird aufgerufen */
    else                    /* Wenn neue Zeile doch leer, testen ob */
    {                       /* sich Cursor aus Fenster bewegt hat. */
      if(akt_winp->ws_col > akt_winp->screencol) /* gegebenenfalls */
	akt_winp->ws_col = akt_winp->screencol;  /* anpassen */
      (*restn)--;           /* Neue Zeile ist leer, also kann ein */
      sw_ohne_refresh(W_AKT);    /* Zeichen eingefuegt werden. */
    } /* Fensterinhalt muss aber wegen eingefuegter Zeile neu */
  }   /* angezeigt werden.                                    */
  return(TRUE);
}

/*****************************************************************************
*
*  Funktion       Overwrite-Teil von do_inschar behandeln (hndl_overwrite)
*  --------
*
*  Paramater    : restn     :
*                   Typ         : int *
*                   Wertebereich: Pointer auf Integer
*                   Bedeutung   : Anzahl der Zeichen, die noch einge-
*                                 setzt werden muessen
*
*  Ergebnis     :
*                   Typ         : int
*                   Wertebereich: TRUE, FALSE
*                   Bedeutung   : TRUE: hat geklappt
*                                 FALSE: hat nicht geklappt
*
*  Beschreibung : Es wird getestet, ob Platz fuer die zu schreibenden Zeichen
*                 vorhanden ist. Steht man am Zeilenende, wird versucht, in
*                 die naechste Zeile zu gehen. Ist das nicht moeglich, wird
*                 falls die maxlimale Zeilenanzahl noch nicht erreicht ist,
*                 eine Zeile angehaengt. In der Zeile geht man an den Anfang.
*                 Unterscheidet sich eines der zu ueberschreibenden Zeichen
*                 von dem dazugeh”rigen einzusetzenden Zeichen in der Laenge
*                 (eins unterstrichen, das andere nicht), dann werden alle
*                 zu berschreibenden Zeichen gel”scht und fr die neuen
*                 Zeichen mit insert() Platz geschaffen.
*
*****************************************************************************/

int hndl_overwrite(restn)
int *restn;
{
  /* *** interne Daten und Initialisierung *** */
  char swflag = FALSE; /* Flag, ob Fenster neu zu zeichnen ist */
  char u_diff=FALSE;   /* Flag: Einzutragendes Zeichen und zu berschrei- */
		       /* bendes Zeichen unterschiedlichen Unterstrcih-Mode */
  int  anz_del=0,      /* Anzahl l”schbarer Zeichen */
       old_sc,         /* Zwischenspeicher Screencol */
       old_tc;         /* Zwischenspeicher Textcol */

  if(akt_winp->screencol == MAXLENGTH) /* Steht man hinter Zeilenende? */
  {
    if(!down())      /* Eine Zeile runter. Klappt das nicht, dann */
    {                /* testen, ob neue Zeile angehaengt werden kann */
      if(akt_winp->maxline >= MAX_ANZ_LINES-1)
      {
	print_err(T_SIZE_ERRTEXT); /* wenn nein, Meldung ausgeben und raus */
	return(FALSE);
      }
      koppel_line(); /* neue Zeile anhaengen. */
      swflag = TRUE; /* Fensterinhalt ist neu zu zeichnen */
    }
    else
      bol();           /* An den Anfang der Zeile springen */
    if(akt_winp->ws_col)    /* Wenn erste Spalte nicht sichtbar, */
    {                       /* dann sichtbar machen */
      akt_winp->ws_col = 0;
      swflag = TRUE;        /* Fensterinhalt muss neu gezeichnet werden */
    }
    if(swflag)
      sw_ohne_refresh(W_AKT);
  }

  fill_buff(); /* Zeile falls noch nicht drin, in Puffer kopieren */
  old_tc = akt_winp->textcol;   /* Textcol und Screencol merken */
  old_sc = akt_winp->screencol;
  do
  {
    if(akt_winp->underflag ^ ul_char()) /* falls Zeichen verschiedene */
					/* Laengen haben */
      u_diff = TRUE;  /* Merken, daá 2 Zeichen unterschiedlich waren */
    anz_del++;        /* Anzahl zu l”schender Zeichen inkrementieren */
  } while (--(*restn) && right()); /* Solange wiederholen, bis alle */
		       /* Zeichen geprft oder Zeilenende erreicht */
  akt_winp->textcol = old_tc;    /* Textcol und Screencol restaurieren */
  akt_winp->screencol = old_sc;

  if(u_diff)  /* Hatten 2 Zeichen unterschiedliche L„nge ? */
  {
    mdelete(anz_del);       /* Zu berschreibende Zeichen loeschen */
    insert(anz_del);        /* platz fuer neue Zeichen schaffen */
  }
  return(TRUE);
}

/*****************************************************************************
*
*  Funktion       Zeichen in Text einfuegen (do_inschar)
*  --------
*
*  Beschreibung : Entsprechend dem insflag wird das in letter stehende Zeichen
*                 in den Text eingefuegt oder ueber den Text geschrieben.
*
*****************************************************************************/

void do_inschar()
{
  /* *** interne Daten und Initialisierung *** */
  short    int input [INS_BUFF_LEN]; /* lokaler Eingabepuffer */
  int          restn; /* Anzahl noch einzufuegender Zeichen   */
  register int n=0,   /* Index in lokalen Eingabepuffer       */
	       nmax,  /* Anzahl eingelesener Zeichen          */
	       hilf;  /* Zum Einlesen einer Tastenkombination */

  nodelay (akt_winp->winp,TRUE); /* Tastatur soll -1 liefern, wenn keine */
  do                             /* Taste gedrueckt wurde                */
    input [n++] = letter;        /* Zeichen in Eingabepuffer uebernehmen */
  while (n<INS_BUFF_LEN && (hilf=taste(akt_winp->winp)) == mc_index+1);
  /* Wenn Puffer noch nicht voll, Tastenkombination lesen. Wenn es ein */
  /* Zeichen ist, Aktion wiederholen.                                  */

  nodelay (akt_winp->winp,FALSE); /* Fkt. taste soll auf Taste warten */
  lastcode = hilf; /* Letzte gelesene Tastenkombination merken */
  nmax = restn = n; /* Variablen anhand des "Pufferfuellstandes" initial. */
  do
  {
    if (akt_winp->insflag)      /* Nur im Insert-Mode einfuegen */
    {
      if(!hndl_insert(&restn))  /* Einfuegung durchfuehren */
	return;
    }
    else                          /* Sonst Ueberschreiben durchfuehren */
      if(!hndl_overwrite(&restn)) /* Klappte Einfuegen oder Ueberschreiben */
	return;                   /* nicht, dann Funktion verlassen */
    while (enter_char((char) input [nmax-n]) /* Zeichen in Zeile einsetzen */
    && --n>restn); /* Index in input anpassen */
  } while (restn); /* Falls noch Zeichen einzufuegen sind, Aktion wiederholen */
  setz_cursor(W_AKT);   /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       Tab-Komprimierungs-modus toggeln (do_tog_tkm)
*  --------
*
*  Beschreibung : Der Status des Tab-Komprimierungs-modus wird invertiert.
*                 Auáerdem wird der Text als ge„ndert markiert, damit er
*                 abgespeichert wird. Das ist erforderlich, da sonst die
*                 Datei evtl. das alte Format beibeh„lt.
*
*****************************************************************************/

void do_tog_tkm()
{
  akt_winp->tabflag ^= TRUE;   /* Tabflagstatus togglen */
  akt_winp->changeflag = TRUE; /* Text soll auf jeden Fall gespeichert werden */
  setz_cursor(W_AKT);          /* Cursor wieder an richtige Position */
}

/*****************************************************************************
*
*  Funktion       Backup-Funktion toggeln (do_tog_bak)
*  --------
*
*  Beschreibung : Erzeugung eines .bak-Files wird aktiviert bzw. deaktiviert.
*
*****************************************************************************/

void do_tog_bak()
{
  print_stat(PROMPT_BACKUP);   /* Meldung ueber aktuellen Status */
  print_stat(on_off[backupflag ^= TRUE]);  /* in der Statuszeile ausgeben    */
  sleep(2);                                /* Warten, damit User Status lesen kann */
  clear_stat();                            /* Statuszeile wieder loeschen */
  setz_cursor(W_AKT);                           /* Cursor wieder an richtige Stelle */
}

/*****************************************************************************
*
*  Funktion       Autoindent-modus toggeln (do_tog_ai)
*  --------
*
*  Beschreibung : Der Status des Autoindent-modus wird invertiert.
*
*****************************************************************************/

void do_tog_ai()
{
  akt_winp->autoindflag ^= TRUE;  /* Autoindentflag togglen */
  setz_cursor(W_AKT);                  /* Cursor wieder an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       Patternmatching-Flag toggeln (do_togregex)
*  --------
*
*  Beschreibung : Der Status des Patternmatching-Modus wird invertiert.
*
*****************************************************************************/

void do_togregex()
{
  print_stat(PROMPT_REGEX);    /* Meldung ueber aktuellen Status */
  print_stat(on_off[regexflag ^= TRUE]);   /* in der Statuszeile ausgeben    */
  sleep(2);                                /* Warten, damit User Status lesen kann */
  clear_stat();                            /* Statuszeile wieder loeschen */
  setz_cursor(W_AKT);                      /* Cursor wieder an richtige Stelle */
}

/*****************************************************************************
*
*  Funktion       Fensterbewegung kontrollieren (do_movewin)
*  --------
*
*  Beschreibung : Es werden die Ecken des Fensters angezeigt. Anschliessend
*                 wird eine Nutzereingabe ausgewertet, aus der die Bewe-
*                 gunsrichtung entnommen wird. Beendet der Benutzer die
*                 Bewegung, so wird do_refresh() aufgerufen, um das Fenster
*                 an der neuen Position zu zeigen.
*
*****************************************************************************/

void do_movewin()
{
  /* *** interne Daten und Initialisierung *** */
  register short int c,  /* Zum Einlesen einer Taste */
		     d;  /* Zum Einlesen einer '\0'.. Sequenz */
  int step = SW_STEP,    /* Schrittweite beim Verschieben */
      i;                 /* Zaehler fuer Verschieben mit step != 1 */

  werase (akt_winp->winp);  /* Fensterinhalt auf Bildschirm loeschen */
  wrefresh(akt_winp->winp); /* damit kein Fehler beim Verschieben passiert */
  cpwins2stdscr(); /* Alle Fenster ausser akt. nach stdscr kopieren */
  do
  {
    mvprintw(LINES-1,0,PROMPT_WINMOVE,step);
    eckenhw();                 /* Ecken des Fensters markieren         */
    c = newwgetch(stdscr);     /* Ein Zeichen von Tastatur / Macro lesen */
    eckenhw();                 /* Ecken vor Verschieben loeschen       */
    if(c == 's')               /* Bei 's' Schrittweite anpassen */
      step = step == SW_STEP ? 1 : SW_STEP; /* war's 1, dann SW_STEP und umgekehrt */
    else                       /* Sonst <step> mal in angegebene Richtung */
#ifdef OS2  /* Bei OS/2 kein zweites Zeichen lesen! */
    {
      d = c;
#else
      if(c==(short int) '\0')              /* "Escape"-Kombination ? */
      {
	d = newwgetch(stdscr); /* Dann n„chstes Zeichen lesen */
#endif
	for(i=0;i<step;i++)      /* verschieben */
	  switch (d)
	  {
	    case KEY_RIGHT : win_right(1); break;
	    case KEY_LEFT  : win_left(1);  break;
	    case KEY_UP    : win_up(1);    break;
	    case KEY_DOWN  : win_down(1);  break;
	  }
      } /* Achtung!!! Diese Klammer geh”rt zum else fr OS2, zu if sonst */
  } while (c != END_KEY);      /* Solange wiederholen, bis beendende Taste */
  erase();refresh();           /* gedrueckt wurde (RETURN) */
  mvwin (akt_winp->winp,akt_winp->y,akt_winp->x); /* Fenster verschieben */
  do_refresh();                   /* Alle Fenster neu zeichnen              */
}

/*****************************************************************************
*
*  Funktion       Fenster auf gespeicherte Groesse bringen (do_toggle_size)
*  --------
*
*  Beschreibung : Die in der Windowstruktur gespeicherten alten Groessen
*                 und Fensterposition werden mit den aktuellen vertauscht.
*                 Die Cursorposition wird eventuell angepasst. Danach wird
*                 ein do_refresh() ausgefuehrt.
*
*****************************************************************************/

void do_toggle_size()
{
  toggle_size();   /* Alte Fensterkoordinaten mit neuen besetzen und umgekehrt */
  do_refresh();    /* Alle Fenster neu zeichnen */
}

/*****************************************************************************
*
*  Funktion       Fenstergroessenanpassung kontrollieren (do_sizewin)
*  --------
*
*  Beschreibung : Die Ecken des Fensters werden angezeigt. Dann wird eine
*                 Nutzereingabe ausgewertet, aus der die Bewegungs-
*                 richtung der rechten unteren Fensterecke entnommen wird.
*                 Wird die Groessenanpassung beendet, werden alle Fenster
*                 refresht (do_refresh()).
*
*****************************************************************************/

void do_sizewin()
{
  /* *** interne Daten *** */
  register short int c, /* Zum Einlesen einer Taste */
		     d; /* Zum Einlesen einer '\0'.. Sequenz */
  int step = SW_STEP,   /* Schrittweite beim Groessenveraendern */
      i;                /* Zahler beim Veraendern mit step != 1 */

  werase (akt_winp->winp);  /* Fensterinhalt auf Bildschirm loeschen,     */
  wrefresh(akt_winp->winp); /* damit keine Fehler beim Anpassen auftreten */
  cpwins2stdscr(); /* Alle Fenster ausser akt. nach stdscr kopieren */
  do
  {
    mvprintw(LINES-1,0,PROMPT_WINSIZE,step);
    eckenhw();              /* Fenstereckpunkte merkieren */
    c = newwgetch(stdscr);  /* Zeichen von Tastatur / Macro einlesen */
    eckenhw();              /* Fenstereckpunkte vor Verschieben loeschen */
    if(c == 's')            /* Bei 's' Schrittweite aendern : */
      step = step == SW_STEP ? 1 : SW_STEP; /* war step 1, dann SW_STEP und */
    else                                    /* umgekehrt                    */
#ifdef OS2  /* Bei OS/2 kein zweites Zeichen lesen! */
      d = c;
#else
      if(c==(short int) '\0')              /* "Escape"-Kombination ? */
      {
	d = newwgetch(stdscr); /* Dann n„chstes Zeichen lesen */
#endif
	for(i=0;i<step;i++)   /* <step> mal in angegebene Richtung vergroessern */
	  switch (d)
	  {
	    case KEY_RIGHT : size_right(1); break;
	    case KEY_LEFT  : size_left(1);  break;
	    case KEY_UP    : size_up(1);    break;
	    case KEY_DOWN  : size_down(1);  break;
	  }
#ifndef OS2
      }
#endif
  } while (c != END_KEY);   /* Solange wiederholen, bis beendende Taste */
  erase();                  /* gedrueckt wurde (RETURN) */
  refresh();                /* Fensterinhalt loeschen */
  delwin(akt_winp->winp);   /* Fenster mit Curses loeschen und neu anlegen */
  akt_winp->winp=newwin(akt_winp->dy+2,akt_winp->dx+2,akt_winp->y,akt_winp->x);
  init_win();               /* Fensterattribute setzen (raw etc.) */

  /* Falls Cursor jetzt ausserhalb des Fensters steht, Cursorposition anpassen */
  if (akt_winp->ws_line+akt_winp->dy <= akt_winp->textline)
    gotox(akt_winp->ws_line+akt_winp->dy-1);
  if (akt_winp->ws_col+akt_winp->dx <= akt_winp->screencol)
    akt_winp->screencol = akt_winp->ws_col+akt_winp->dx-1;
  do_refresh();  /* Alle Fenster neu zeichnen */
}

/*****************************************************************************
*
*  Funktion       Zum Nachfolger des aktuellen Fensters (do_swnext)
*  --------
*
*  Beschreibung : Das Fenster, das hinter dem Dummyelement steht, wird
*                 hinter dem aktuellen Fenster eingefuegt. Das Element hinter
*                 dem aktuellen Fenster wird dann zum aktuellen Fenster.
*
*****************************************************************************/

void do_swnext()
{
  /* *** interne Daten *** */
  win_typ *hilf,  /* Zeiger fuer erstes Fester in Liste */
	  *dummy; /* Zeiger fuer leeres Element in der Fensterliste */

  if (akt_winp->next->next == akt_winp) /* Test, ob nur ein Fenster */
  {
    print_err(PROMPT_ONEWINDOW); /* Wenn ja, Meldung und Ende */
    return;
  }
  kopf(W_NOTAKT);   /* Altes Fenster als inaktiv markieren */
  rahmen(W_NOTAKT);
  wrefresh(akt_winp->winp);

  check_buff();              /* Sonst evtl. Pufferinhalt in Text eintragen */
  dummy = akt_winp->next;    /* dummy auf leeres Element der Liste setzen  */
  hilf = dummy->next;        /* hilf auf erstes Fenster in Liste setzen    */
  dummy->next = hilf->next;
  dummy->prev = hilf;        /* Dummyfenstereintrag vor hilf einkoppeln,   */
  dummy->next->prev = dummy; /* da aktuelles Fenster immer am Listenende   */
  akt_winp->next = hilf;     /* stehen muss.                               */
  hilf->prev = akt_winp;
  hilf->next = dummy;
  akt_winp = hilf;           /* hilf wird zum aktuellen Fenster            */
  show_win(W_AKT);           /* Aktuellen Fensterinhalt neu zeichnen       */
}

/*****************************************************************************
*
*  Funktion       Zum Vorgaenger des aktuellen Fensters (do_swprev)
*  --------
*
*  Beschreibung : Das aktuelle Fenster, das vor dem Dummyelement steht, wird
*                 hinter dem Dummy-Element eingefuegt. Das Element vor dem
*                 aktuellen Fenster wird dann zum aktuellen Fenster. Man
*                 kommt also ueber diese Funktion in das Fenster, das un-
*                 mittelbar vor dem aktuellen Fenster geoeffnet wurde.
*
*****************************************************************************/

void do_swprev()
{
  /* *** interne Daten *** */
  win_typ *hilf,    /* zeigt auf Vorgaenger des aktuell werdenden Fensters */
	  *old_akt; /* Zeiger fuer Fenster, das beim Aufruf aktuell war */

  if (akt_winp->next->next == akt_winp) /* Test, ob nur ein Fenster da    */
  {
    print_err(PROMPT_ONEWINDOW); /* Wenn ja, Meldung und raus */
    return;
  }
  kopf(W_NOTAKT);   /* Altes Fenster als inaktiv markieren */
  rahmen(W_NOTAKT);
  wrefresh(akt_winp->winp);

  check_buff();                          /* Pufferinhalt in Text sichern   */
  akt_winp = (old_akt = akt_winp)->prev; /* old_akt und akt_winp setzen    */
  akt_winp->next = old_akt->next;        /* Aktuelles Fenster wird mit dem */
  hilf = akt_winp->prev;                 /* vor ihm stehenden vertauscht   */
  akt_winp->prev = old_akt;
  old_akt->prev = hilf;
  hilf->next = old_akt;
  akt_winp->next->prev = akt_winp;
  old_akt->next = akt_winp;
  show_win(W_AKT);                        /* Fensterinhalt neu zeichnen     */
}

/*****************************************************************************
*
*  Funktion       Window absolut anspringen (do_swnum)
*  --------
*
*  Beschreibung : Der Nutzer kann eine Fensternummer eingeben. Falls die
*                 Nummer existiert, wird das Fenster mit dieser Nummer
*                 zum aktuellen Fenster.
*
*****************************************************************************/

void do_swnum()
{
  /* *** interne Daten *** */
  char num_str [21];  /* String fuer einzulesende Fensternummer */
  int  num;           /* eingelesene Nummer als Integer         */

  print_stat (PROMPT_WINDOWNUM);
  read_stat (num_str,20,GS_NUM);   /* Nummer einlesen, nur 2 Ziffern erlaubt */
  clear_stat();                    /* Statuszeile wieder loeschen            */
  check_buff();                    /* Pufferinhalt in Text uebernehmen       */
  kopf(W_NOTAKT);   /* Altes Fenster als inaktiv markieren */
  rahmen(W_NOTAKT);
  wrefresh(akt_winp->winp);

  /* String in Integer umwandeln. Test, ob Nummer == 0 oder aktuelle Fnstnr. */
  if ((num = atoi (num_str)) && num != akt_winp->wini)
    if(make_akt_win(num)) /* Wenn alles in Ordnung, versuchen, gewuenschtes  */
      show_win(W_AKT);         /* Fenster zum aktuellen zu machen. Klappte das,   */
    else                  /* Fensterinhalt neu zeichnen, sonst Meldung       */
    {
      pe_or (PROMPT_WINNMNFND);
      rahmen(W_AKT); /* Doch wieder aktiv, kopf kommt von setz_cursor */
      setz_cursor(W_AKT);
    }
  else                    /* Fehler bei Nummer, dann nur Cursor plazieren    */
    setz_cursor(W_AKT);
}

/*****************************************************************************
*
*  Funktion       Funktion mehrmals ausfuehren (do_repeat)
*  --------
*
*  Beschreibung : Der User muss die Anzahl der Wiederholungen eingeben.
*                 Anschliessend,falls die eingegebene Anzahl groesser 0 war,
*                 wird ein Kommando eingelesen, welches dann von der Funktion
*                 taste so oft zurueckgegeben wird, wie der User spezifizierte.
*
*****************************************************************************/

void do_repeat()
{
  /* *** interne Daten *** */
  char zahl_str[6];  /* String fuer Anzahl der Wiederholungen */
  int  i;            /* Stringinhalt als Integer */

  if(ks_index < MACRO_NEST_DEPTH - 1) /* Testen, ob noch ein Puffer frei */
  {                                   /* Wenn ja, Wiederholungsanzahl einlesen */
    print_stat(PROMPT_REPEAT);
    read_stat(zahl_str,5,GS_NUM);
    clear_stat();                     /* Statuszeile wieder loeschen */
    if ((i=atoi(zahl_str)) > 0)       /* String nach Integer, muss > 0 sein */
    {
      /* Mit get_comstr wird eine Befehlsfolge von der Tastatur eingelesen */
      /* und im ersten freien Puffer (ks_index+1) abgelegt.                */
      get_comstr(&puff_feld[ks_index+1].begin,&puff_feld[ks_index+1].end);
      if(puff_feld[ks_index+1].begin)    /* keine Leereingabe? */
      {
	if(ks_index >=0)              /* Wenn schon ein Puffer aktiv, dann  */
	  puff_feld[ks_index].current = keystack; /* Position darin merken  */
	keystack = puff_feld[++ks_index].begin; /* Position im neuen setzen */
		    /* Puffer soll nach Ausfuehrung der Wiederholung wieder */
	puff_feld[ks_index].free_flag = TRUE;         /* freigegeben werden */
	e_keystack = puff_feld[ks_index].end; /* Pufferende merken          */
	puff_feld[ks_index].anz_rep = i - 1;  /* Anzahl der Wdh. eintragen  */
      }
    }
    setz_cursor(W_AKT);  /* Cursor an seine richtige Position setzen */
  }
  else      /* falls kein Puffer mehr frei, Fehlermeldung ausgeben */
  {
    clear_buff = TRUE;  /* beim naechsten Aufruf von newwgetch() Puffer loeschen */
    print_err(PROMPT_RECURSION);
  }
}

/*****************************************************************************
*
*  Funktion       Blockanfang an aktueller Position setzen (do_blstart)
*  --------
*
*  Beschreibung : An der aktuellen Cursorposition wird - falls zulaessig, d.h.
*                 falls der Blockanfang vor dem Blockende liegt oder noch
*                 kein Blockende definiert wurde - der Blockstart festgelegt.
*                 War schon ein Blockende vorhanden, so wird show_win(W_AKT)
*                 aufgerufen.
*
*****************************************************************************/

void do_blstart()
{
  akt_winp->block.s_line = akt_winp->textline;  /* Zeile und Spalte als Block- */
  akt_winp->block.s_col = akt_winp->screencol;  /* eintragen */
  if(highblockflag)                    /* Falls Block gehighlighted sein soll, */
    show_win(W_AKT);                        /* markierten Block anzeigen */
}

/*****************************************************************************
*
*  Funktion       Blockende (normal) an aktueller Position setzen (do_blnormend)
*  --------
*
*  Beschreibung : An der aktuellen Cursorposition wird - falls zulaessig, d.h.
*                 falls der Blockanfang vor dem Blockende liegt oder noch
*                 kein Blockanfang definiert wurde - das Blockende festgelegt.
*                 War schon ein Blockanfang vorhanden, so wird show_win(W_AKT)
*                 aufgerufen.
*
*****************************************************************************/

void do_blnormend()
{
  akt_winp->block.e_line = akt_winp->textline; /* Zeile und Spalte als Block- */
  akt_winp->block.e_col = akt_winp->screencol; /* ende eintragen              */
  akt_winp->block.typ = BT_NORMAL;             /* Blocktyp eintragen          */
  if(highblockflag)  /* Falls Block gehighlighted dargestellt werden soll,    */
    show_win(W_AKT);      /* Fenstrinhalt neu darstellen */
}

/*****************************************************************************
*
*  Funktion       Blockende (Rechteck) an aktueller Position setzen (do_blrechtend)
*  --------
*
*  Beschreibung : An der aktuellen Cursorposition wird - falls zulaessig, d.h.
*                 falls der Blockanfang vor dem Blockende liegt oder noch
*                 kein Blockanfang definiert wurde - das Blockende festgelegt.
*                 War schon ein Blockanfang vorhanden, so wird show_win(W_AKT)
*                 aufgerufen.
*
*****************************************************************************/

void do_blrechtend()
{
  akt_winp->block.e_line = akt_winp->textline;  /* Aktuelle Zeile und Spalte */
  akt_winp->block.e_col = akt_winp->screencol;  /* als Blockende eintragen   */
  akt_winp->block.typ = BT_RECHTECK;            /* Blocktyp eintragen        */
  if(highblockflag) /* Falls Block gehighlighted dargestellt werden soll,    */
    show_win(W_AKT);     /* Fenstrinhalt neu darstellen */
}

/*****************************************************************************
*
*  Funktion       Blockmarkierung loeschen (do_blunmark)
*  --------
*
*  Beschreibung : Die Blockmarkierungen werden geloescht und ein show_win(W_AKT)
*                 durchgefuehrt, um eventuelle Blockmarkierungen auf dem
*                 Schirm zu loeschen.
*
*****************************************************************************/

void do_blunmark()
{
  if(block_defined())   /* Wenn ein Block markiert ist         */
    if(tst_overlap())   /* und dieser im Fenster sichtbar ist, */
    {                   /* Blockgrenzen loeschen und           */
      akt_winp->block.e_line = akt_winp->block.s_line = -1;
      show_win(W_AKT);       /* Fensterinhalt neu zeichnen          */
    }
    else    /* Ist der Block nicht im Fenster, dann nur die Grenzen loeschen */
      akt_winp->block.e_line = akt_winp->block.s_line = -1;
}

/*****************************************************************************
*
*  Funktion       Block loeschen (do_blweg)
*  --------
*
*  Beschreibung : Falls ein Block markiert ist, wird in Abhaengigkeit vom Typ
*                 der Block geloescht, die Cursor- und Bildschirmposition an-
*                 gepasst und evtl. der Bildschirm neu gezeichnet.
*
*****************************************************************************/

void do_blweg()
{
  if (block_defined())  /* Nur ausfuehren, wenn ein Block definiert ist */
  {
    /* In laenge wird die Differenz der letzten und der ersten Zeile */
    /* eingetragen */
    akt_winp->block.laenge = akt_winp->block.e_line-akt_winp->block.s_line;
    if (akt_winp->block.typ == BT_RECHTECK)   /* Abhaengig vom Blocktyp wird */
      del_rechteck();                         /* entweder del_rechteck oder  */
    else                                      /* del_normal aufgerufen       */
    {
      del_normal();
      if (akt_winp->textline < akt_winp->ws_line)  /* Falls Cursor durch das  */
	akt_winp->ws_line = akt_winp->textline;    /* Loeschen des Blocks     */
      if (akt_winp->screencol < akt_winp->ws_col)  /* ausserhalb des Fensters */
	akt_winp->ws_col = akt_winp->screencol;    /* steht, Fenster anpassen */
    }   /* Anschliessend die Blockgrenzen loeschen */
    akt_winp->block.e_line = akt_winp->block.s_line = -1;
    show_win(W_AKT); /* Fensterinhalt neu zeichnen */
  }
}

/*****************************************************************************
*
*  Funktion       Block kopieren (do_blcopy)
*  --------
*
*  Beschreibung : Falls ein Block markiert ist, wird in Abhaengigkeit vom Typ
*                 der Block in einen Puffer kopiert und dieser dann an der
*                 aktuellen Position eingefuegt.  Danach wird der Puffer frei-
*                 gegeben und evtl. der Bildschirm neu gezeichnet.
*
*****************************************************************************/

void do_blcopy()
{
  /* *** interne Daten *** */
  int ok;  /* Zwischenspeicher fuer Rueckgabewert der Insert-Funktionen */

  if (block_defined())  /* Nur ausfuehren, falls ein Block markiert ist */
  {
    if (akt_winp->block.typ == BT_RECHTECK)     /* Abhaengig vom Blocktyp */
    {                                           /* entweder save_rechteck */
      akt_winp->block.bstart = save_rechteck(); /* und ins_rechteck auf-  */
      ok = ins_rechteck(&akt_winp->block);      /* rufen oder             */
    }
    else
    {
      akt_winp->block.bstart = save_normal();   /* save_normal und ins_normal */
      ok = ins_normal(&akt_winp->block);        /* aufrufen */
    }
    block_free(akt_winp->block.bstart);         /* Blocktext freigeben        */
    show_win(W_AKT);                                 /* Fensterinhalt neu anzeigen */
    if(!ok)                                     /* Falls beim Einfuegen ein   */
      print_err(B_SIZE_ERRTEXT);                /* Fehler auftrat, Meldung    */
  }
}

/*****************************************************************************
*
*  Funktion       Block verschieben (do_blmove)
*  --------
*
*  Beschreibung : Falls ein Block markiert ist, wird er kopiert, geloescht
*                 und an der aktuellen Cursorposition wieder eingefuegt.
*
*****************************************************************************/

void do_blmove()
{
  /* *** interne Daten *** */
  int old_sc,  /* Zwischenspeicher fuer Spalte falls Einfuegen nicht klappt */
      old_tl;  /* Zwischenspeicher fuer Zeile falls Einfuegen nicht klappt */

  if (block_defined()) /* Nur ausfuehren, wenn ein Block maekiert ist */
  {
    if (akt_winp->block.typ == BT_RECHTECK)  /* Blocktyp ermitteln */
    {
      akt_winp->block.bstart = save_rechteck();  /* Blocktext abspeichern    */
      del_rechteck();                            /* Block im Text loeschen   */
      if(!ins_rechteck(&akt_winp->block))        /* Block an aktueller       */
      {                                          /* Position einfuegen       */
	old_sc = akt_winp->screencol;            /* klappt das nicht, dann   */
	old_tl = akt_winp->textline;             /* Position merken,         */
	akt_winp->screencol = akt_winp->block.s_col; /* zum Anfang des alten */
	gotox(akt_winp->block.s_line);  /* Blocks gehen und dort wieder hin. */
	ins_rechteck(&akt_winp->block); /* Einfuegen an alter position muss  */
	akt_winp->screencol = old_sc;   /* funktionieren. Cursorposition     */
	gotox(old_tl);                  /* restaurieren                      */
	print_err(B_SIZE_ERRTEXT);      /* Fehlermeldung ausgeben            */
      }
      else
	show_win(W_AKT); /* Klappte Einfuegen, dann Fensterinhalt neu anzeigen */
    }
    else                                /* Normaler Block: */
      if (akt_winp->maxline + 2 < MAX_ANZ_LINES) /* Test ob genug Zeilen frei */
      {
	akt_winp->block.bstart = save_normal(); /* Wenn ja, Blocktext merken, */
	del_normal();                           /* Block im Text loeschen     */
	ins_normal(&akt_winp->block);           /* An aktueller Pos. einfuegen*/
	if (akt_winp->textline < akt_winp->ws_line)  /* Falls Cursor durch das  */
	  akt_winp->ws_line = akt_winp->textline;    /* Verschieben des Blocks  */
	if (akt_winp->screencol < akt_winp->ws_col)  /* ausserhalb des Fensters */
	  akt_winp->ws_col = akt_winp->screencol;    /* steht, Fenster anpassen */
	show_win(W_AKT);                             /* Fensterinhalt neu anzeigen */
      }
      else       /* Waren nicht genuegend Zeilen frei, Fehlermeldung ausgeben */
	print_err(B_SIZE_ERRTEXT);
    block_free(akt_winp->block.bstart);   /* Blocktext freigeben */
  }
}

/*****************************************************************************
*
*  Funktion       Block cutten (do_blcut)
*  --------
*
*  Beschreibung : Falls ein Block markiert ist, wird in Abhaengigkeit vom Typ
*                 der Block in einen Puffer, den sogenannten PASWTE-Puffer,
*                 kopiert. Dieser kann mit der Paste-Funktion ino einem belie-
*                 bigen Fenster eingefuegt werden.
*
*****************************************************************************/

void do_blcut()
{
  if (block_defined())      /* Nur ausfuehren, falls ein Block definiert ist */
  {
    if(global_block.bstart) /* Steht schon ein Block im Paste-Puffer, muss */
      block_free(global_block.bstart);       /* dieser freigegeben werden. */
    if (akt_winp->block.typ == BT_RECHTECK)  /* abhaengig vom Blocktyp die */
      akt_winp->block.bstart = save_rechteck(); /* entsprechende Funktion zum */
    else                                        /* Abspeichern des Blockes    */
      akt_winp->block.bstart = save_normal();   /* verwenden                  */
    show_win(W_AKT);                                 /* Fenster neu anzeigen       */
    memcpy(&global_block,&akt_winp->block,sizeof(block_typ)); /* Daten des akt. */
    print_stat(PROMPT_CUT); /* Blocks kopieren  */
    sleep(1);                           /* Meldung ausgeben und 2 Sek. warten */
    clear_stat();                       /* Meldung wieder loeschen */
    setz_cursor(W_AKT);                      /* Cursor an richtige Position */
  }
}

/*****************************************************************************
*
*  Funktion       Block pasten (do_blpaste)
*  --------
*
*  Beschreibung : Falls mit der cut-Funktion ein Block in den PASTE-Puffer ko-
*                 piert wurde, wird dieser im aktuellen Fenster an der Cursor-
*                 position eingefuegt.
*
*****************************************************************************/

void do_blpaste()
{
  /* *** interne Daten *** */
  block_typ *hilf;  /* Zeiger fuer duplizierten Paste-Block */
  int       ok;     /* Rueckgabewert der Insert-Funktionen */

  if (global_block.bstart)              /* Nur, wenn Paste-Puffer belegt */
  {
    if (akt_winp->maxline == -1)        /* Falls Text leer, Eine Zeile anlegen, */
      koppel_line(IGNORE_COORS);        /* da sonst Dummy ueberschrieben wird.  */
    /* Marker und lastpos sind im leeren Text egal */
    hilf = dup_block(&global_block);    /* Blocktext duplizieren */
    if (global_block.typ == BT_RECHTECK) /* Abhaengig vom Typ ins_rechteck */
      ok = ins_rechteck(hilf);           /* oder ins_normal aufrufen       */
    else
      ok = ins_normal(hilf);
    if(ok) /* Wenn ins_normal bzw. ins_rechteck geklappt haben, */
      show_win(W_AKT);  /* Fensterinhalt neu anzeigen */
    else   /* Klappte des Einfuegen nicht, Fehlermeldung ausgeben. */
      print_err(B_SIZE_ERRTEXT);
    block_free(hilf->bstart);     /* Blocktext des aktuellen Blocks freigeben */
    free (hilf);                  /* Hilfsblockstruktur freigeben */
  }
}

/*****************************************************************************
*
*  Funktion       Block einruecken (do_blindent)
*  --------
*
*  Beschreibung : Falls ein Block markiert wurde, werden alle Zeilen ab der
*                 Blockstartzeile bis zur Blockendezeile um einen vom Nutzer
*                 einzugebenden Wert eingerueckt. Dabei entscheidet das Vor-
*                 zeichen ueber die Einrueckungsrichtung.
*
*****************************************************************************/

void do_blindent()
{
  /* *** interne Daten *** */
  int  weite;         /* Eingelesene Weite als Integer */
  char weit_str[10];  /* Eingelesene Weite als String  */

  if (block_defined())  /* Nur ausfuehren, falls ein Block markiert ist */
  {
    print_stat(PROMPT_ASKINDENT);
    read_stat(weit_str,9,GS_ANY);  /* Weite als String einlesen              */
    clear_stat();                  /* Statuszeile wieder loeschen            */
    setz_cursor(W_AKT);                 /* Cursor wieder an richtige Position     */
    if (weite = atoi(weit_str))    /* String in int wandeln und auf 0 testen */
      if (indent_block(weite))     /* Weite != 0, dann Block einruecken      */
	show_win(W_AKT);                /* und Fensterinhalt neu anzeigen         */
  }
  else                             /* War kein Block markiert, Meldung ausgeben */
    print_err(PROMPT_NOBLOCK);
}

/*****************************************************************************
*
*  Funktion       Block einlesen (do_blread)
*  --------
*
*  Beschreibung : Eine vom Nutzer anzugebende Datei wird an der aktuellen
*                 Cursorposition in den Text eingefuegt und als Block mar-
*                 kiert.
*
*****************************************************************************/

void do_blread()
{
  /* *** interne Daten *** */
  char name[MAXLENGTH+1];  /* Array fuer einzulesenden Dateinamen */
  FILE *f;                 /* Filepointer fuer zu lesende Datei */

  print_stat(PROMPT_FILENAME);
  read_stat(name,MAXLENGTH,GS_ANY);  /* Dateiname einlesen */
  clear_stat();                      /* Statuszeile wieder loeschen */
  setz_cursor(W_AKT);                     /* Cursor an alte Position */
  if(name[0])                        /* Nur ausfuehren, wenn Name vorhanden */
    if (f=fopen(name,"r"))           /* Datei zum Lesen oeffnen */
    {
      akt_winp->block.typ = BT_NORMAL;     /* Blocktyp eintragen */
      if(lies_block(&akt_winp->block,f))   /* Block aus Datei lesen */
      {                                    /* Klappte das, dann Block in */
	if(!ins_normal(&akt_winp->block))  /* Text einfuegen. Falls Fehler */
	  print_err(B_SIZE_ERRTEXT);       /* beim Einfuegen, dann Meldung */
	else                               /* Sonst Fensterinhalt neu */
	  show_win(W_AKT);                      /* anzeigen */
	block_free(akt_winp->block.bstart);/* Blocktext freigeben */
	fclose (f);
      }
      else                                 /* lies_block lieferte FALSE, */
	print_err(PROMPT_BLOCKEMPT);       /* Meldung, dass Block leer */
    }
    else                                   /* Falls Datei nicht geoeffnet */
      print_err(PROMPT_FILENOTFD);         /* werden konnte, Meldung */
}

/*****************************************************************************
*
*  Funktion       Block schreiben (do_blwrite)
*  --------
*
*  Beschreibung : Falls ein Block markiert ist, wird dieser in einer vom
*                 Nutzer anzugebenden Datei abgespeichert. Dabei wird ge-
*                 testet, ob die Datei bereits existiert. Falls ja, wird der
*                 Nutzer gefragt, ob er die Datei ueberschreiben moechte.
*
*****************************************************************************/

void do_blwrite()
{
  /* *** interne Daten *** */
  char name[MAXLENGTH+1]; /* Array fuer einzulesenden Filenamen */
  FILE *f;                /* Pointer fuer Datei, in die Block geschrieben */
			  /* werden soll. */
  if (block_defined())    /* Nur, wenn Block markiert ist */
  {
    print_stat(PROMPT_FILENAME);
    read_stat(name,MAXLENGTH,GS_ANY); /* Filenamen einlesen */
    clear_stat();                     /* Statuszeile wieder loeschen */
    if(name[0])                       /* Name leer ? */
    {
      setz_cursor(W_AKT);                  /* Nein, Cursor plazieren */

      /* Falls Datei schon existiert, Sicherheitsabfrage vornehmen, ob    */
      /* der User die Datei ueberschreiben moechte. Existiert die Datei   */
      /* und der User moechte sie nicht ueberschreiben, wird die Funktion */
      /* beendet.                                                         */

      if(!access(name,0) && !ja_nein(PROMPT_FILEEXIST))
      {
	setz_cursor(W_AKT);
	return;
      }
      if (f = fopen (name,"w"))  /* Datei oeffnen */
      {                          /* Klappte das, Block mit save_... aus */
	if (akt_winp->block.typ == BT_RECHTECK) /* Text ausschneiden */
	  akt_winp->block.bstart = save_rechteck();
	else
	  akt_winp->block.bstart = save_normal();
	schr_block(akt_winp->block.bstart,f); /* Block in offene Datei */
	fclose (f);                  /* schreiben und Datei schliessen */
	block_free (akt_winp->block.bstart);    /* Blocktext freigeben */
      }
      else     /* Konnte Datei nicht geoeffnet werden, Fehlermeldung */
      {
	print_stat (PROMPT_ERRWRITE);
	pe_or(name);
      }
    }
    setz_cursor(W_AKT);
  }
}

/*****************************************************************************
*
*  Funktion       Zum Blockende gehen (do_goblend)
*  --------
*
*  Beschreibung : Falls der Blockanfang markiert ist, wird der Cursor dorthin
*                 gesetzt.
*
*****************************************************************************/

void do_goblend()
{
  if(akt_winp->block.e_line != -1)           /* Ist Blockende gesetzt ? */
  {
    akt_winp->lastline = akt_winp->textline; /* Ja, dann aktuelle Position */
    akt_winp->lastcol = akt_winp->screencol; /* als letzte Position merken */
    gotox(akt_winp->block.e_line);           /* Blockende anspringen */
    akt_winp->screencol = akt_winp->block.e_col;
    adapt_screen(1);                         /* Fensterinhalt evtl. anpassen */
  }
}

/*****************************************************************************
*
*  Funktion       Zum Blockanfang (do_goblanf)
*  --------
*
*  Beschreibung : Falls der Blockanfang markiert ist, wird der Cursor dorthin
*                 gesetzt.
*
*****************************************************************************/

void do_goblanf()
{
  if(akt_winp->block.s_line != -1)           /* Blockanfang definiert ? */
  {
    akt_winp->lastline = akt_winp->textline; /* Aktuelle Position als   */
    akt_winp->lastcol = akt_winp->screencol; /* letzte Position merken  */
    gotox(akt_winp->block.s_line);           /* Blockanfang anspringen  */
    akt_winp->screencol = akt_winp->block.s_col;
    adapt_screen(1);                         /* Evtl. Fensterinhalt anpassen */
  }
}

/*****************************************************************************
*
*  Funktion       Blockhervorhebungsmodus togglen (do_toghbl)
*  --------
*
*  Beschreibung : Das highblockflag wird getoggled. Dadurch wird die Darstel-
*                 lung eines markierten Blocks beeinflusst (gehighlighted oder
*                 nicht gehighlighted).
*
*****************************************************************************/

void do_toghbl()
{
  print_stat(PROMPT_BLKHILGHT); /* Meldung ueber den neuen Zustand */
  print_stat(on_off[highblockflag ^= TRUE]); /* ausgeben */
  sleep(1);
  /* Wenn ein Block markiert ist und dieser im Fenster sichtbar ist, */
  /* wird der Fensterinhalt neu dargestellt.                         */
  if(block_defined() && tst_overlap())
    sw_ohne_refresh(W_AKT);
  else         /* anderenfalls wird ein sleep ausgefuehrt, damit in bei- */
    sleep(1);  /* den Faellen die Meldung gleichlange zu lesen ist.      */
  clear_stat();
  setz_cursor(W_AKT);
}

/*****************************************************************************
*
*  Funktion       Block an Filter uebergeben (do_bltofil)
*  --------
*
*  Beschreibung : Falls ein Block markiert ist, wird dieser an ein vom Nutzer
*                 anzugebendes Filterprogramm uebergeben. Die Ausgabe des Fil-
*                 ters wird ueber eine Pipe eingelesen und als Block in den
*                 Text eingefuegt.
*
*****************************************************************************/

void do_bltofil()
{
  if (block_defined() && bl_to_fil())
  {
    if (akt_winp->textline < akt_winp->ws_line)  /* Falls Cursor durch das  */
      akt_winp->ws_line = akt_winp->textline;    /* Filtern des Blocks      */
    if (akt_winp->screencol < akt_winp->ws_col)  /* ausserhalb des Fensters */
      akt_winp->ws_col = akt_winp->screencol;    /* steht, Fenster anpassen */
    show_win(W_AKT);
  }
}

/*****************************************************************************
*
*  Funktion       Shellflag invertieren (do_shelltog)
*  --------
*
*  Beschreibung : Das shellflag der aktuellen Windowstruktur wird invertiert.
*
*****************************************************************************/

void do_shelltog()
{
  akt_winp->shellflag ^= TRUE;
  setz_cursor(W_AKT);      /* Kopfzeile aktualisieren */
}


/*****************************************************************************
*
*  Funktion       Marker setzen (do_setmarker)
*  --------
*
*  Beschreibung : Falls der Text nicht leer ist, wird der Nutzer nach der
*                 Markernummer (0-9) gefragt. Fuer den angegebenen Marker
*                 werden dann Position und Fensternummer gespeichert.
*
*****************************************************************************/

void do_setmarker()
{
  /* *** interne Daten *** */
  int m;   /* Nummer des gewuenschten Markers */

  if(akt_winp->maxline >= 0)  /* Text nicht leer ? */
  {
    if(helpflag)
      print_stat(PROMPT_MARKER);
    m = newwgetch(status) - '0'; /* Markernummer einlesen */
    if(helpflag)
      clear_stat();
    if(m >= 0 && m <= 9)         /* Bereich ueberpruefen */
    {
      marker[m].col = akt_winp->screencol; /* aktuelle Position und */
      marker[m].line = akt_winp->textline; /* Fensternummer eintragen */
      marker[m].window = akt_winp->wini;
      setz_cursor(W_AKT);
    }
    else                         /* Bereichspruefung schlug fehl */
      print_err(PROMPT_ERRMARKER);
  }
}

/*****************************************************************************
*
*  Funktion       Marker anspringen (do_jumpmarker)
*  --------
*
*  Beschreibung : Der Nutzer wird nach der Nummer des anzuspringenden Markers
*                 gefragt. Existiert der Marker, oder das zu dem Marker gehoe-
*                 rende fenster nicht mehr, so wird eine Meldung ausgegeben.
*                 Sonst wird das zum Marker gehoerige Fenster zum aktuellen
*                 Fenster, und der Cursor wird richtig positioniert.
*
*****************************************************************************/

void do_jumpmarker()
{
  /* *** interne Daten *** */
  int     m;        /* Nummer des gewuenschten Markers */
  win_typ *old_win; /* Zwischenspeicher fuer beim Aufruf aktuelles Fenster */

  if(helpflag)
    print_stat(PROMPT_MARKER);
  m = newwgetch(status) - '0';  /* Markernummer einlesen */
  if(helpflag)
    clear_stat();
  if(m >= 0 && m <= 9)          /* Bereichspruefung fuer Markernummer */
    if(marker[m].window != -1)  /* Ist Marker gesetzt ? */
    {
      kopf(W_NOTAKT);   /* Altes Fenster als inaktiv markieren */
      rahmen(W_NOTAKT);
      wrefresh(akt_winp->winp);

      check_buff();             /* Pufferinhalt evtl. in Text eintragen */
      old_win = akt_winp;
      if(make_akt_win(marker[m].window)) /* zum Fenster gehen, in dem */
      {                                  /* der Marker gesetzt wurde  */
	akt_winp->lastcol  = akt_winp->screencol; /* aktuelle Position */
	akt_winp->lastline = akt_winp->textline; /*  als letzte merken */

	/* Steht der Marker hinter dem Textende, dann zur letzten Zeile  */
	/* gehen. Andernfalls die vom Marker angegebene Zeile anspringen */
	/* Falls Text leer, geht gotox richtig auf dummyp.               */
	gotox(akt_winp->maxline <= marker[m].line?akt_winp->maxline:marker[m].line);
	/* Falls Text leer, dann Spalte 0, sonst richtige Markerspalte */
	akt_winp->screencol = akt_winp->maxline>=0 ? marker[m].col : 0;
	if(old_win != akt_winp) /* Falls nicht aktuelles Fenster, dann */
	  show_win(W_AKT);      /* Fensterkomplett neu zeichnen        */
	else
	  rahmen(W_AKT);        /* Sonst Fenster wieder aktiv */
	adapt_screen(1);        /* Fensterinhalt anpassen              */
      }
      else /* Fenster konnte nicht angesprungen werden, Meldung ausgeben */
	print_err(PROMPT_STALEMARK);
    }
    else
      print_err(PROMPT_EMPTYMARK);
  else
    print_err(PROMPT_ERRINPUT);
}

/*****************************************************************************
*
*  Funktion       Letzte Position anspringen (do_lastpos)
*  --------
*
*  Beschreibung : Es wird die letzte Position vor einem Find/Replace/Goto/
*                 lastpos/Marker angesprungen. Diese Position ist Fenster-
*                 intern.
*
*****************************************************************************/

void do_lastpos()
{
  /* *** interne Daten und Initialisierung *** */
  int hilf = akt_winp->textline; /* Zwischenspeicher fuer Zeilennummer */

  if(akt_winp->lastline != -1)   /* Letzte Position gesetzt ? */
  {
    /* aktuelle Spalte als letzte Spalte merken, letzte Spalte zur */
    /* aktuellen machen                                            */
    swap_int(&akt_winp->screencol,&akt_winp->lastcol);
    gotox(akt_winp->lastline);  /* Letzte Zeile anspringen */
    akt_winp->lastline = hilf;  /* Vorher aktuelle Zeile wird Letzte Z. */
    adapt_screen(1);            /* Fensterinhalt anpassen */
  }
}

/*****************************************************************************
*
*  Funktion       Window gemaess Name anspringen (do_swname)
*  --------
*
*  Beschreibung : Der Nutzer kann einen Dateinamen eingeben. Falls die
*                 Nummer existiert, wird das Fenster mit diesem Namen
*                 zum aktuellen Fenster.
*
*****************************************************************************/

void do_swname()
{
  /* *** interne Daten und Initialisierung *** */
  char filename[MAXLENGTH+1]; /* Array fuer einzulesenden Filenamen    */
  int  wn = akt_winp->wini;   /* Zwischenspeicher fuer akt. Fensternr. */

  print_stat (PROMPT_FILENAME);
  read_stat (filename,MAXLENGTH,GS_ANY);  /* Filenamen einlesen */
  clear_stat();
  if(!filename[0])                        /* Filename leer ? */
  {
    setz_cursor(W_AKT);                        /* Dann verlassen */
    return;
  }
  kopf(W_NOTAKT);   /* Altes Fenster als inaktiv markieren */
  rahmen(W_NOTAKT);
  wrefresh(akt_winp->winp);

  check_buff();               /* sonst Pufferinhalt evtl. in Text eintragen, */
  if (sw_name(filename))      /* versuchen, gewuenschtes Fenster zum aktu- */
  {                           /* ellen zu machen. */
    if(akt_winp->wini == wn)  /* War es das aktuelle Fenster ? */
    {
      rahmen(W_AKT);          /* Fenster wird evtl. wieder aktiv */
      setz_cursor(W_AKT);          /* Dann nur Cursor setzen */
    }
    else                      /* Sonst komplettes Fenster neu zeichnen */
      show_win(W_AKT);
  }
  else                        /* Fenster nicht gefunden */
    /* Abfragen, ob Datei geladen werden soll */
    if(ja_nein(PROMPT_NEWWINDOW))
    {
      kopf(W_NOTAKT);   /* Altes Fenster als inaktiv markieren */
      rahmen(W_NOTAKT);
      wrefresh(akt_winp->winp);

      if (koppel_win())   /* Soll Datei neu geladen werden, */
      {                   /* dann neues Fenster erzeugen */
	akt_winp->filename = save_text(filename); /* Filenamen eintragen */
	if (!lies_file())  /* Datei in neues Fenster einlesen */
	{
	  gb_win_frei();   /* Klappte das nicht, Fenster wieder loeschen, */
	  rahmen(W_AKT);   /* Fenster highlighten */
	  setz_cursor(W_AKT);   /* und Cursor an richtige Position */
	}
	else               /* Sonst Fensterwerte initialisieren */
	  open_window();
      }
      else                 /* koppel_win klappte nicht */
      {
	pe_or("Zu viele Fenster! ");
	rahmen(W_AKT); /* Altes Fenster wieder aktiv */
	setz_cursor(); /* Cursor an richtige Position */
      }
    }
    else                   /* Fenster nicht gefunden, Datei soll aber */
    {
      rahmen(W_AKT);       /* Fenster highlighten */
      setz_cursor(W_AKT);       /* auch nicht neu geladen werden */
    }
}

/*****************************************************************************
*
*  Funktion       Datei unter neuem Namen abspeichern (do_newname)
*  --------
*
*  Beschreibung : Der Dateiname der Datei im aktuellen Fenster wird intern
*                 geaendert und der Text unter dem neuen Namen abgespeichert.
*
*****************************************************************************/

void do_newname()
{
  /* *** interne Daten *** */
  char name[MAXLENGTH+1];  /* Array fuer einzulesenden neuen Namen */

  print_stat(PROMPT_FILENAME);
  read_stat(name,MAXLENGTH,GS_ANY); /* Dateinamen einlesen */
  clear_stat();
  if(!name[0])                      /* Leerer Name, dann raus */
  {
    pos_cursor();
    return;
  }
  free(akt_winp->filename);         /* Sonst alten Namen freigeben       */
  akt_winp->attribs = STD_FATTR;    /* Attribute und read_only-Status    */
  akt_winp->read_only = FALSE;      /* initialisieren                    */
  akt_winp->filename = save_text(name); /* Neuen Namen abspeichern       */
  schreib_file();                   /* Datei unter neuem Namen speichern */
  setz_cursor(W_AKT);
}

/*****************************************************************************
*
*  Funktion       Macro definieren/ausfuehren (do_macro)
*  --------
*
*  Beschreibung : Nach Eingabe eines Grossbuchstabens kann der Benutzer
*                 einen Macro definieren, indem er die dem Macro zuzu-
*                 ordnenden Tasten drueckt. Beendet wird die Macrodefinition
*                 durch Druck der ESCAPE-Taste (get_comstr()).
*                 Nach Eingabe eines Kleinbuchstabens wird der entsprechende
*                 Macro - falls definiert - aufgerufen.
*
*****************************************************************************/

void do_macro()
{
  /* *** interne Daten *** */
  short int mnum;  /* Zum einlesen des Macrobuchstabens */

  if(helpflag)
    print_stat(PROMPT_MACRO);
  mnum = newwgetch(status);  /* Macrobuchstaben einlesen */
  if(helpflag)
    clear_stat();
  if(mnum>='A' && mnum<='Z') /* Macro definieren ? */
  {
    mnum -= 'A';
    line_free(macro[mnum].begin); /* vorherigen Macrotext freigeben */
    get_comstr(&macro[mnum].begin,&macro[mnum].end); /* und neuen einlesen */
  }
  else                       /* Macro ausfuehren */
    exec_macro(mnum);
  setz_cursor(W_AKT);
}

/*****************************************************************************
*
*  Funktion       Geloeschte Zeile wiederherstellen (do_restline)
*  --------
*
*  Beschreibung : Mittels rest_delline() wird die zuletzt geloeschte Zeile
*                 bzw. eine Leerzeile vor der aktuellen Zeile eingefuegt.
*
*****************************************************************************/

void do_restline()
{
  if(akt_winp->maxline < MAX_ANZ_LINES-1) /* noch Platz da? */
  {
    rest_delline();
    show_win(W_AKT);
  }
  else                /* Kein Platz mehr, Fehlermeldung ausgeben */
    print_err(T_SIZE_ERRTEXT);
}

/*****************************************************************************
*
*  Funktion       Alle modifizierten Files loeschen (do_saveall)
*  --------
*
*  Beschreibung : Ruft save_all auf, um alle geaenderten Dateien zu sichern.
*                 Anschliessend wird ein Refresh ausgefuehrt.
*
*****************************************************************************/

void do_saveall()
{
  save_all();
  show_win(W_AKT);
}

void (*funktion [])() = {do_refresh, do_bol,do_eol,do_halfup,do_halfdn,do_delete,
			 do_backspace,do_home,do_nothome,do_insovr,do_textbeginn,
			 do_eot,do_del_word,do_wleft,do_wright,do_right,do_left,
			 do_up,do_down,do_pgup,do_pgdn,do_newline,do_delline,
			 ueberschreiben,do_schreib_file,quit,do_control,do_help,
			 do_movewin,do_sizewin,do_swnext,do_swprev,do_swnum,
			 laden,do_win_zu,do_goto,do_ende,do_find,do_replace,
			 do_underline,do_z_hoch,do_z_runter,do_z_oben,
			 do_z_mitte,do_z_unten,do_deleol,do_toggle_size,
			 do_repfr,do_repeat,do_repfr,do_join,do_open,do_tog_ai,
			 do_tab,do_settab,do_tog_tkm,do_blstart,do_blnormend,
			 do_blrechtend,do_blunmark,do_blweg,do_blcopy,
			 do_blmove,do_blcut,do_blpaste,do_blindent,do_blread,
			 do_blwrite,do_goblend,do_goblanf,do_toghbl,do_bltofil,
			 do_setmarker,do_jumpmarker,do_lastpos,do_hopen,
			 do_swname,do_newname,do_backtab,do_tog_bak,do_macro,
			 do_restline,do_shelltog,do_endemit,quitmit,do_delete,
			 do_togregex, do_matchpar, do_middle, do_saveall,
			 do_inschar };

/*****************************************************************************
*
*  Funktion       Tastendruck auswerten (auswertung)
*  --------
*
*  Parameter    : t         :
*                   Typ          : int
*                   Wertebereich : 0-MAX_COMM_INDEX
*                   Bedeutung    : Nummer des Befehls. Ist t == BUCHSTABE,
*                                  dann steht der Buchstabe in letter.
*
*  Beschreibung : Anhand des Kommandos wird eine Funktion aufgerufen, die
*                 den Befehl ausfuehrt.
*
*****************************************************************************/

void auswertung (t)
int t;
{
  hide_show(MOUSE_HIDE);  /* Maus vor Ausfhrung der Operation verstecken */
  (*funktion[t])();
  hide_show(MOUSE_SHOW);  /* Maus nach der Operation wieder anzeigen */
}
