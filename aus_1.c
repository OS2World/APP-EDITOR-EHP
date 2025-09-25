/****************************************************************/
/*                                                              */
/*      MODUL:  aus_1.c                                         */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - ueberschreiben (Neuen Text in altes Fenster)  */
/*              - do_win_zu (Fenster schliessen)                */
/*              - laden (Datei laden und Fenster oeffnen)       */
/*              - do_schreib_file (Datei abspeichern)           */
/*              - do_help (Hilfstexte an/aus)                   */
/*              - do_refresh (refreshe Bildschirm)              */
/*              - do_z_runter (Text und Cursor eine Z. runter)  */
/*              - do_z_hoch (Text und Cursor eine Zeile hoch)   */
/*              - do_z_mitte (aktuelle Zeile in Fenstermitte)   */
/*              - do_z_unten (aktuelle Zeile an Fensterende)    */
/*              - do_z_oben (aktuelle Zeile an Fensteranfang)   */
/*              - do_bol (gehe an Zeilenanfang)                 */
/*              - do_eol (gehe an Zeilenende)                   */
/*              - do_halfup (gehe halbe Seite hoch)             */
/*              - do_halfdn (gehe halbe Seite runter)           */
/*              - do_delete (loesche aktuelles Zeichen)         */
/*              - do_backspace (loesche Zeichen links)          */
/*              - do_home (Cursor an Bildschirmanfang)          */
/*              - do_middle (Cursor an Bildschirmmitte)         */
/*              - do_nothome (Cursor an Bildschirmende)         */
/*              - do_underline (Underlinemodus togglen)         */
/*              - do_insovr (Schreibmodus togglen)              */
/*              - do_matchpar (passennde Klammer finden)        */
/*                                                              */
/****************************************************************/

#define aus1_def
#include "defs.h"

extern char backupflag,highblockflag,clear_buff,bufflag;
extern int schreib_file(),to_shell();
extern short int letter,lastcode,aktcode,taste();
extern int save_delline(),rest_delline();
extern int tst_overlap(),do_find(),do_replace(),do_repfr();
extern char bufflag,*loadfile;
extern bzeil_typ *save_normal(),*save_rechteck();
extern block_typ global_block,*dup_block();
extern puff_typ macro[],puff_feld[];
extern int ks_index;
extern short int *keystack,*e_keystack,newwgetch();
extern WINDOW *status;
extern marker_typ marker[];
void do_refresh();

/* *** interne Daten und Initialisierung *** */
char *on_off[] = 
#ifdef GERMAN
		 {"ausgeschaltet.","eingeschaltet."}; /* Hilfstexte */
#else
		 {"turned off.","turned on."}; /* Hilfstexte */
#endif
				 /* fuer Togglen der globalen Flags */
char helpflag=TRUE;              /* Flag: Hilfstexte anzeigen       */

/****************************************************************************
*
*  Funktion       Neuen Text in altes Fenster (ueberschreiben)
*  --------
*
*  Beschreibung : Fragt den User nach dem Filenamen der Datei und liest,
*                 falls die Datei geladen werden konnte, diese ein.
*                 Gelang dies nicht, wird das Fenster geschlossen.
*
*****************************************************************************/

void ueberschreiben()
{
  /* *** interne Daten und Initialisierung *** */
  char    name[61],                 /* Name der zu ladenden Datei */
	  dummy[MAXLENGTH+1];       /* String fuer Fehlermeldung  */
  int     wn = akt_winp->wini,      /* Variable fuer aktuelle Fensternummer */
	  ws;                       /* Fensternummer des Fensters, in dem Datei
				       eventuell schon enthalten ist */

  if(!akt_winp->changeflag || ja_nein(PROMPT_WARNLOAD))
  {
    print_stat(PROMPT_FILENAME);
    read_stat(name,60,GS_ANY); /* Filename nach name einlesen (alle Zeichen erlaubt) */
    clear_stat();              /* Kommandozeile wieder loeschen */
    if(!name[0])               /* Falls Leereingabe, Funktion beenden */
    {
      setz_cursor(W_AKT);
      return;
    }
    bufflag = FALSE;            /* Pufferinhalt verwerfen */
    akt_winp->filename[0]='\0'; /* leere Filenamen gibt es nicht */
    free_text();                /* also wird aktuelles Fenster durch sw_name */
    if (sw_name(name))          /* keinesfalls gefunden. sw_name checkt, */
    {                           /* ob Datei in einem Fenster vorhanden */
      sprintf(dummy,PROMPT_ALRDYLDD, akt_winp->wini);
      if (!ja_nein(dummy))      /* Eingabe J/N, ob trotzdem ueberladen */
      {
	ws = akt_winp->wini;    /* Nummer des jetzt aktuellen Fensters merken */
	make_akt_win(wn);       /* Zum alten Fenster gehen */
	werase(akt_winp->winp); /* Fenster auf Bildschirm loeschen */
	delwin(akt_winp->winp); /* Altes Fenster mit Curses loeschen */
	gb_win_frei();          /* Window aus Liste auskoppeln */
	make_akt_win(ws);       /* Wieder zum gefundenen Fenster gehen */
	do_refresh();           /* Jetzt aktuellen Bildschirm zeichnen */
	return;
      }
      else                      /* Wenn trotzdem berladen werden soll, */
	make_akt_win(wn);       /* dann wieder zum alten Fenster gehen  */
    }
    line_free(akt_winp->filename); /* Filenamen deallokieren */
    akt_winp->filename = save_text (name); /* Speicher fuer Filenamen holen */
					   /* und Filename merken */
    if (!lies_file ())          /* Datei einlesen. Falls das nicht klappt, */
    {
      werase(akt_winp->winp);   /* Fenster auf Bildschirm loeschen */
      delwin(akt_winp->winp);   /* Fenster freigeben */
      gb_win_frei ();           /* und aus Liste auskoppeln */
      if(akt_winp->next == akt_winp)    /* kein Fenster mehr ? */
      {
	write_config();         /* Dann Config-File schreiben */
	ende(0, TRUE);          /* und Programm beenden */
      }
      do_refresh();             /* Sonst kompletten Schirm neu aufbauen */
    }
    else                /* laden funktionierte */
    {
      akt_winp->ws_line = akt_winp->ws_col = 0; /* Erste Zeile und Spalte zeigen */
      akt_winp->textcol = akt_winp->screencol = 0; /* Cursor nach oben links */
      akt_winp->block.s_line = akt_winp->block.e_line = -1; /* kein Block markiert */
      sw_ohne_refresh(W_AKT);                        /* Fenster darstellen */
    }
  }
  setz_cursor(W_AKT);                /* Cursor plazieren und refresh ausfuehren */
}

/****************************************************************************
*
*  Funktion       Fenster schliessen (do_win_zu)
*  --------
*
*  Parameter    : wait_mouse:
*                 Typ         : char
*                 Wertebereich: TRUE, FALSE
*                 Bedeutung   : Rufe im Falle, daá das zu schlieáende
*                               Fenster das letzte offene war, ende()
*                               mit wait_mouse als 2. Parameter auf.
*
*  Beschreibung : Nach einer Rueckfrage wird ggf. der dem Fenster assoziierte
*                 Text freigegeben und das Fenster geschlossen.
*
*****************************************************************************/

void do_win_zu(wait_mouse)
char wait_mouse;
{
  if(!akt_winp->changeflag ||     /* Hat sich Text nicht geaendert ? */
     (akt_winp->changeflag &&     /* oder falls doch, will der User abbrechen ? */
     ja_nein(PROMPT_WARNCLOSE)))
  {
    bufflag = FALSE;              /* Pufferinhalt verwerfen */
    free_text();                  /* Text des aktuellen Fensters freigeben */
    werase(akt_winp->winp);       /* Fenster fuer Curses loeschen */
    delwin(akt_winp->winp);
    gb_win_frei();                /* Fenster aus Liste auskoppeln */
    if(akt_winp->next == akt_winp)      /* wenn keine Textfiles mehr, */
    {
      write_config();                   /* Config-File schreiben und Editor */
      ende(0, wait_mouse);              /* beenden */
    }
    do_refresh();                       /* Sonst ganzen Schirm neu aufbauen */
  }
  setz_cursor(W_AKT);             /* und Cursor an richtige Position */
}

/****************************************************************************
*
*  Funktion       laden (laden)
*  --------
*
*  Ergebnis     : TRUE, falls File geladen werden konnte, sonst FALSE
*  Beschreibung : Fragt den User nach dem Filenamen der Datei.
*                 Falls schon ein Fenster mit dieser Datei existiert, wird
*                 der User gefragt, ob wirklich neu geladen werden soll.
*                 Wenn ja, dann oeffnet die Funktion, falls die Datei
*                 geladen werden konnte, dafuer ein Fenster.
*
*****************************************************************************/

int laden()
{
  /* *** interne Daten und Initialisierung *** */
  char    name[61],            /* Name der zu ladenden Datei */
	  dummy[MAXLENGTH+1];  /* String fuer Fehlermeldung */
  int     wn = akt_winp->wini; /* Nummer des aktuellen Fensters */
	  /* um entscheiden zu koennen, ob Fenster neu gezeichnet werden muss */

  check_buff();                 /* Puffer evtl. zurueckschreiben */
  print_stat(PROMPT_FILENAME);
  read_stat(name,60,GS_ANY);    /* Filenamen einlesen (max. 60 zeichen) */
  clear_stat();                 /* Kommandozeile wieder loeschen */
  if(!name[0])                  /* Abbruch, falls leerer Name */
  {
    setz_cursor(W_AKT);
    return (FALSE);
  }
  rahmen(W_NOTAKT); /* Rahmen "unhighlighten", damit falls anderes Fenster */
  kopf(W_NOTAKT);   /* zum aktiven wird, nicht 2 gehighlightet */
  if(akt_winp->next != akt_winp) /* Schon ein Fenster auf ? */
    wrefresh(akt_winp->winp);
  if (sw_name(name))            /* Versuchen, ein Fenster mit diesem Datei- */
  {                             /* namen zu finden */
    sprintf(dummy,PROMPT_ALRDYLDD,akt_winp->wini);
    if (!ja_nein(dummy))        /* Falls gefunden, fragen, ob trotzdem neu laden */
    {
      show_win(W_AKT);           /* Falls nein, gefundenes Fenster zeichnen */
      return (TRUE);
    }
  }
  if (koppel_win())             /* Neues Fenster in Fensterliste einhaengen */
  {                             /* Falls das klappt, Speicherplatz fuer File- */
    akt_winp->filename = save_text (name); /* namen holen und merken */
    akt_winp->block.bstart = (bzeil_typ*) NULL;
    if (!lies_file())           /* Datei einlesen */
    {                           /* klappt das nicht, neu eingehaengtes Fenster */
      gb_win_frei();            /* wieder auskoppeln */
      rahmen(W_AKT);            /* Altes Fenster wieder optisch aktivieren */
      setz_cursor(W_AKT);
      return(FALSE);
    }
    open_window();              /* Klappte Einlesen, dann Fenster auf Schirm */
    return(TRUE);               /* oeffnen, Koordinaten initialisieren usw. */
  }
  if(wn != akt_winp->wini)      /* Falls das aktuelle Fenster ein anderes ist */
    show_win(W_AKT);             /* als vorher, dann neu zeichnen */
  print_err(PROMPT_NOWINDOW); /* Falls koppel_win nicht klappte, Error */
  return(FALSE);
}

/*****************************************************************************
*
*  Funktion       Datei abspeichern (do_schreib_file)
*  --------
*
*  Beschreibung : Die Datei im aktuellen Fenster wird mit der Funktion
*                 schreib_file abgespeichert. Danach wird die Funktion
*                 setz_cursor aufgerufen, damit in der unteren Rahmenzeile
*                 der Text GEAENDERT verschwindet.
*
*****************************************************************************/

void do_schreib_file()
{
  schreib_file(); /* Aktuellen Text abspeichern */
  setz_cursor(W_AKT);  /* Cursor wieder an richtige Stelle */
}

/*****************************************************************************
*
*  Funktion       Hilfstexte an/aus (do_help)
*  --------
*
*  Beschreibung : Die Hilfstextoption wird an- bzw. ausgeschaltet
*
*****************************************************************************/

void do_help()
{
  print_stat(PROMPT_HELPTEXT); /* Wenn helpflag TRUE ist, wird "eingeschaltet" */
  print_stat(on_off[helpflag ^= TRUE]); /* ausgegeben, sonst "ausgeschaltet" */
  sleep(2); /* 2 Sekunden warten, damit User die Meldung lesen kann. */
  clear_stat();  /* Statuszeile wieder loeschen */
  setz_cursor(W_AKT); /* und Cursor wieder an richtige Position */
}

/*****************************************************************************
*
*  Funktion       refreshe Bildschirm (do_refresh)
*  --------
*
*  Beschreibung : Der Bildschirm wird geloescht, alle Fenster, Rahmen und
*                 Kopfzeilen neu gezeichnet und danach setz_cursor() aufge-
*                 rufen.
*
*****************************************************************************/

void do_refresh()
{
  /* *** interne Daten und Initialisierung *** */
  win_typ *oldwin = akt_winp; /* Zeiger auf Fenster, das bei Aufruf der */
			      /* Funktion aktuell war. */

  check_buff();         /* Evtl. Pufferinhalt in Text schreiben */
  werase(curscr);       /* Gesamten Bildschirm loeschen */
  for(akt_winp=akt_winp->next->next; akt_winp != oldwin->next; akt_winp=akt_winp->next)
  { /* Alle Fenster durchlaufen */
    sw_ohne_refresh(akt_winp == oldwin ? W_AKT : W_NOTAKT); /* Fensterinhalt neu ausgeben */
    kopf(akt_winp == oldwin ? W_AKT : W_NOTAKT); /* Kopf des Fensters ausgeben */
#ifdef OWN_CURSES
    wnoutrefresh(akt_winp->winp); /* Fensterinhalt anzeigen */
#else
    wrefresh(akt_winp->winp);
#endif
  } /* noch nicht in physikalischen Bildschirm uebertragen */
  akt_winp = oldwin; /* Das Fester, das vorher aktuelle war, wieder zum */
  pos_cursor();      /* aktuellen machen, Cursor positionieren */
#ifdef OWN_CURSES
  doupdate();       /* und virtuellen Bildschirm in physikalischen uebertragen */
#endif
}

/*****************************************************************************
*
*  Funktion       Text und Cursor eine Zeile runter (do_z_runter)
*  --------
*
*  Beschreibung : Der Text wird mitsamt Cursor um eine Zeile nach unten
*                 bewegt.
*
*****************************************************************************/

void do_z_runter()
{
  /* *** interne Daten und Initialisierung *** */
  int   i=0;      /* Zaehler fuer Wiederholungen */
  short int hilf; /* Zum Einlesen einer Tastenkombination */

  nodelay (akt_winp->winp,TRUE); /* taste soll -1 liefern, falls keine */
  do                             /* Taste gedrueckt ist. */
    i++;
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Falls der gleiche */
  /* Code noch mal im Puffer, ganze Aktion wiederholen */
  lastcode = hilf;  /* Zuletzt gelesene Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Funktion taste soll auf Taste warten */

  if(akt_winp->maxline >= 0 && akt_winp->ws_line > 0)
  { /* Falls Text nicht leer und noch nicht die erste Textzeile zu sehen ist, */
    if(i==1) /* Nur einmel ? */
    {
      akt_winp->ws_line--; /* Nummer der ersten sichtbaren Zeile dekrementieren */
      text_down(0);        /* Text ab Zeile 0 (ganzes Fenster) um 1 nach unten  */
      if(akt_winp->textline >= akt_winp->ws_line + akt_winp->dy)
	up(); /* Stand Cursor in letzter Schirmzeile, Cursor 1 Zeile hoch */
      setz_cursor(W_AKT);        /* Cursor an richtige Position */
    }
    else /* mehrere Wiederholungen */
    {
      /* Fensterinhalt scrollen. Falls zuweit, dann letzte Zeile anzeigen */
      if((akt_winp->ws_line -= i) < 0)
	akt_winp->ws_line = 0;
      /* Ist der Cursor aus dem Fenster gerutscht, dann wird er */
      /* in die letzte Fensterzeile gestellt. Das kann nicht    */
      /* hinter dem Textende sein, da er sonst nicht aus dem    */
      /* Fenster gerutscht waere.                               */
      if(akt_winp->textline >= akt_winp->ws_line+akt_winp->dy)
	gotox(akt_winp->ws_line+akt_winp->dy-1);
      show_win(W_AKT); /* Fensterinhalt darstellen */
      setz_cursor(W_AKT); /* Cursor an richtige Position stellen */
    }
  }
}

/*****************************************************************************
*
*  Funktion       Text und Cursor eine Zeile hoch (do_z_hoch)
*  --------
*
*  Beschreibung : Der Text wird mitsamt Cursor um eine Zeile nach oben
*                 bewegt.
*
*****************************************************************************/

void do_z_hoch()
{
  /* *** interne Daten und Initialisierung *** */
  int   i=0;      /* Zaehler fuer Wiederholungen */
  short int hilf; /* Zum Einlesen einer Tastenkombination */

  nodelay (akt_winp->winp,TRUE); /* taste soll -1 liefern, falls keine */
  do                             /* Taste gedrueckt ist. */
    i++;
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Falls der gleiche */
  /* Code noch mal im Puffer, ganze Aktion wiederholen */
  lastcode = hilf;  /* Zuletzt gelesene Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Funktion taste soll auf Taste warten */

  if(akt_winp->ws_line < akt_winp->maxline)
  { /* Wenn noch nicht die letzte Textzeile zu sehen ist, */
    if(i==1) /* Nur einmel ? */
    {
      akt_winp->ws_line++; /* dann Nummer der ersten sichtbaren Zeile erhoehen */
      text_up(0);          /* gesamten Fenstertext um 1 Zeile nach oben */
      if(akt_winp->textline < akt_winp->ws_line) /* Falls Cursor in oberster */
	down();            /* Zeile, dann Cursor um 1 Zeile nach unten bewegen */
      setz_cursor(W_AKT);       /* Cursor an richtige Position setzen */
    }
    else /* mehrere Wiederholungen */
    {
      /* Fensterinhalt scrollen. Falls zuweit, dann letzte Zeile anzeigen */
      if((akt_winp->ws_line += i) > akt_winp->maxline)
	akt_winp->ws_line = akt_winp->maxline;
      /* Ist der Cursor aus dem Fenster gerutscht, dann wird er */
      /* in die erste Fensterzeile gestellt.                    */
      if(akt_winp->textline < akt_winp->ws_line)
	gotox(akt_winp->ws_line);
      show_win(W_AKT); /* Fensterinhalt darstellen */
      setz_cursor(W_AKT); /* Cursor an richtige Position stellen */
    }
  }
}

/*****************************************************************************
*
*  Funktion       aktuelle Zeile in Fenstermitte (do_z_mitte)
*  --------
*
*  Beschreibung : Die aktuelle Zeile wird in die Fenstermitte bewegt.
*
*****************************************************************************/

void do_z_mitte()
{
  /* Erste sichtbare Zeile wird die, die einen halben Bildschirm (dy/2)  */
  /* ueber der aktuellen Zeile liegt. Sollte dadurch ws_line kleiner als */
  /* 0 werden (textline zu klein), wird die erster Textzeile zur ersten  */
  /* sichtbaren Zeile gemacht */
  if((akt_winp->ws_line = akt_winp->textline - akt_winp->dy/2)<0)
    akt_winp->ws_line = 0;
  show_win(W_AKT); /* Fensterinhalt anzeigen */
}

/*****************************************************************************
*
*  Funktion       aktuelle Zeile an Fensterende (do_z_unten)
*  --------
*
*  Beschreibung : Die aktuelle Zeile wird an das Fensterende bewegt.
*
*****************************************************************************/

void do_z_unten()
{
  /* Erste sichtbare Zeile wird aktuelle Zeile abzueglich einer Bild-   */
  /* schirmlaenge. Ist das resultat kleiner 0 (textline zu klein), wird */
  /* die erste Textzeile zur ersten sichtbaren Zeile */
  if((akt_winp->ws_line = akt_winp->textline - akt_winp->dy + 1)<0)
    akt_winp->ws_line = 0;
  show_win(W_AKT); /* Fensterinhalt darstellen */
}

/*****************************************************************************
*
*  Funktion       aktuelle Zeile an Fensteranfang (do_z_oben)
*  --------
*
*  Beschreibung : Die aktuelle Zeile wird an den Fensteranfang bewegt.
*
*****************************************************************************/

void do_z_oben()
{
  if(akt_winp->textline >=0) /* Falls Text nicht leer, */
  {
    akt_winp->ws_line = akt_winp->textline; /* wird aktuelle Zeile zur */
    show_win(W_AKT); /* erster sichtbaren, Fensterinhalt wird neu dargestellt */
  }
}

/*****************************************************************************
*
*  Funktion       Gehe an Zeilenanfang (do_bol)
*  --------
*
*  Beschreibung : Der Cursor wird an den Zeilenanfang bewegt.
*
*****************************************************************************/

void do_bol()
{
  bol();                    /* Cursor an Zeilenanfang stellen */
  if(akt_winp->ws_col)      /* Wenn erste Spalte nicht auf Bildschirm, */
  {
    akt_winp->ws_col = 0;   /* Dann erste Spalte zur ersten sichtbaren */
    show_win(W_AKT);             /* machen, Fensterinhalt neu darstellen    */
  }
  setz_cursor(W_AKT);  /* Cursor an richtige Position */
}

/*****************************************************************************
*
*  Funktion       Gehe an Zeilenende (do_eol)
*  --------
*
*  Beschreibung : Der Cursor wird an das Zeilenende bewegt.
*
*****************************************************************************/

void do_eol()
{
  eol(); /* Cursor intern ans Zeilenende setzen */
  /* Falls Cursorposition rechts vom Bildschirm oder */
  if(akt_winp->ws_col + akt_winp->dx <= akt_winp->screencol
  || akt_winp->ws_col > akt_winp->screencol) /* links vom Bildschirm */
  { /* dann Position anpassen */
    /* Falls screencol < dx, dann kann erste Spalte dargestellt werden. */
    /* Sonst wird die aktuelle Spalte zur letzten sichtbaren Spalte. */
    akt_winp->ws_col = (akt_winp->screencol < akt_winp->dx) ?
    0 : akt_winp->screencol - akt_winp->dx + 1;
    show_win(W_AKT); /* Fensterinhalt darstellen */
  }
  setz_cursor(W_AKT); /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       Gehe halbe Seite hoch (do_halfup)
*  --------
*
*  Beschreibung : Der Cursor wird um eine halbe Seite nach oben bewegt.
*
*****************************************************************************/

void do_halfup()
{
  /* *** interne Daten *** */
  register int hilf;   /* zum Einlesen einer Tastenkombination */

  nodelay (akt_winp->winp,TRUE); /* taste soll -1 liefern, falls keine */
  do                             /* Taste gedrueckt ist. */
    if((akt_winp->ws_line -= half_up()) < 0) /* Cursor hochbewegen und */
      akt_winp->ws_line = 0; /* neuen Fensterstart berechnen */
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Falls der gleiche */
  /* Code noch mal im Puffer, ganze Aktion wiederholen */
  lastcode = hilf;  /* Zuletzt gelesene Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Funktion taste soll auf Taste warten */
  show_win(W_AKT);                 /* Text im Fenster neu anzeigen     */
  setz_cursor(W_AKT);               /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       Gehe halbe Seite runter (do_halfdn)
*  --------
*
*  Beschreibung : Der interne Cursor wird um eine halbe Seite nach unten bewegt.
*                 Dann wird die Fensterposition angepasst und der
*                 Fensterinhalt erneut dargestellt. Steht der Cursor in der
*                 letzten Textzeile, so wird diese in die Bildschirmmitte
*                 plaziert.
*
*****************************************************************************/

void do_halfdn()
{
  /* *** interne Daten *** */
  register int hilf;   /* Zum Einlesen einer Tastenkombination */

  nodelay (akt_winp->winp,TRUE); /* taste soll -1 liefern, falls keine */
  do                             /* Taste gedrueckt ist. */
    akt_winp->ws_line += half_down(); /* Cursor und Fensterstart anpassen */
  while ((hilf=taste(akt_winp->winp)) == aktcode); /* Falls noch mal der */
  /* gleiche Code im Tastaturpuffer, Aktion wiederholen */
  lastcode = hilf;   /* letzte Tastenkombination merken */
  nodelay (akt_winp->winp,FALSE); /* Fkt. taste soll wieder auf Taste warten */
  show_win(W_AKT);                  /* Text im Fenster neu anzeigen     */
  setz_cursor(W_AKT);                /* Cursor an richtige Position setzen */
}

/*****************************************************************************
*
*  Funktion       loesche aktuelles Zeichen (do_delete)
*  --------
*
*  Beschreibung : Das unter dem Cursor stehende Zeichen wird geloescht.
*
*****************************************************************************/

void do_delete()
{
  if (is_last())    /* steht man am / hinterm Zeilenende ? */
    do_join();
  else
    if (delete())   /* Zeichen, auf dem der Cursor steht loeschen */
    {               /* klappt das, wird die aktuelle Zeile neu angezeigt */
      lineout(akt_winp->textline-akt_winp->ws_line);
      setz_cursor(W_AKT); /* und der Cursor an die richtige Position gesetzt */
    }
}

/*****************************************************************************
*
*  Funktion       loesche Zeichen links (do_backspace)
*  --------
*
*  Beschreibung : Das links vom Cursor stehende Zeichen wird geloescht.
*
*****************************************************************************/

void do_backspace()
{
  if(backspace())  /* Das Zeichen links vom Cursor wird geloescht */
  {                /* Klappt das, wird gecheckt, ob Cursor links vom Schirm */
    if(akt_winp->screencol < akt_winp->ws_col)
      text_right(); /* Falls ja, Text um eine Spalte nach rechts bewegen */
    lineout(akt_winp->textline-akt_winp->ws_line); /* Zeile neu anzeigen */
    setz_cursor(W_AKT); /* cursor an richtige Position setzen */
  }
  else /* es ging nicht nach links. Das ist sicher, da delete in backspace */
  {    /* nur dann einen Fehler liefert, wenn Abschluánull gel”scht werden */
       /* soll, aber das kann hier nicht auftreten. */
    if (akt_winp->textline > 0) /* geht es eine Zeile hoch ? */
    {
      do_up();
      do_join();
    }
  }
}

/*****************************************************************************
*
*  Funktion       Cursor an Bildschirmanfang (do_home)
*  --------
*
*  Beschreibung : Der Cursor wird in die erste Zeile des Bildschirms
*                 bewegt.
*
*****************************************************************************/

void do_home()
{
  gotox(akt_winp->ws_line); /* Cursor intern auf erste Bildschirmzeile setzen */
  setz_cursor(W_AKT);            /* Cursor auf Bildschirm plazieren */
}

/*****************************************************************************
*
*  Funktion       Cursor an Bildschirmmitte (do_middle)
*  --------
*
*  Beschreibung : Der Cursor wird in die mittlere Zeile des Fensters
*                 bewegt.
*
*****************************************************************************/

void do_middle()
{
  gotox(akt_winp->ws_line+akt_winp->dy/2); /* Cursor intern auf Mitte setzen */
  setz_cursor(W_AKT);                      /* Cursor auf Bildschirm plazieren */
}

/*****************************************************************************
*
*  Funktion       Cursor an Bildschirmende (do_nothome)
*  --------
*
*  Beschreibung : Der Cursor wird in die letzte Zeile des Bildschirms
*                 bewegt.
*
*****************************************************************************/

void do_nothome()
{
  gotox(akt_winp->ws_line + akt_winp->dy -1); /* Cursor in letzte Bild- */
  setz_cursor(W_AKT); /* schirmzeile. Anschliessend Cursor auf Schirm plazieren */
}

/*****************************************************************************
*
*  Funktion       Underlinemodus toggeln (do_underline)
*  --------
*
*  Beschreibung : Die unterstrichene Schriftart wird aktiviert/deaktiviert.
*
*****************************************************************************/

void do_underline()
{
  akt_winp->underflag ^= TRUE;  /* Flag fuer Unterstreichung togglen */
  setz_cursor(W_AKT);                /* Kopfzeile aktualisieren */
}

/*****************************************************************************
*
*  Funktion       Schreibmodus toggeln (do_insovr)
*  --------
*
*  Beschreibung : Der Schreibmodus wird invertiert.
*
*****************************************************************************/

void do_insovr()
{
  akt_winp->insflag ^= TRUE;    /* Insertflag togglen */
  setz_cursor(W_AKT);                /* Kopfzeile aktualisieren */
}

/*****************************************************************************
*
*  Funktion       Passende Klammer aufsuchen (do_matchpar)
*  --------
*
*  Beschreibung : Es wird in der Umgebung nach einer ”ffnenden oder
*                 schlieáenden Klammer gesucht (rund, eckig,
*                 geschweift). Zu der gefundenen Klammer wird die
*                 passende gesucht (bei ”ffnender wird vorw„rts,
*                 bei schlieáender rckw„rts gesucht).
*                 Wird eine passende Klammer gefunden, wird der
*                 Cursor dorthin positioniert und der Bildschirm
*                 angepaát. Andernfalls bleibt der Cursor an seiner
*                 aktuellen Position.
*
*****************************************************************************/

void do_matchpar()
{
  /* *** lokale Daten *** */
  int  old_sc = akt_winp->screencol,  /* Spaltennummer beim Aufruf */
       old_tl = akt_winp->textline;   /* Zeilennummer beim Aufruf  */

  if (find_next_par())  /* Klammer gefunden ? */
  {
    if (search_match_par ())
      adapt_screen (1);             /* Falls Zeile der gefundenen Klammer */
    else                            /* auáerhalb des Bildschirms steht,   */
    {                               /* anpassen.                          */
      akt_winp->screencol = old_sc; /* Falls nicht gefunden, Cursor wieder*/
      akt_winp->textline  = old_tl; /* an die Stelle von vor dem Aufruf   */
      beep ();
    }
  }
  else
     beep();
}
