/****************************************************************/
/*                                                              */
/*      MODUL:  config.c                                        */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - save_read (aus Datei lesen)                   */
/*              - check_koors (Koordinaten ueberpruefen)        */
/*              - read_config (Config-Datei einlesen)           */
/*              - save_write (in Datei schreiben)               */
/*              - write_config (Config-Datei schreiben)         */
/*                                                              */
/****************************************************************/

#include <fcntl.h>
#include "defs.h"

#define SH_SIZ (sizeof (short int))

extern char backupflag,helpflag,highblockflag,regexflag,
	    *reserve_mem(),*conffile;
extern win_typ *akt_winp;
extern marker_typ marker[];
extern puff_typ macro[];

void write_config();
int wc_errflag = FALSE;  /* Zeigt an, ob beim Lesen oder Schreiben */
			 /* der Config-Datei ein Fehler auftrat    */

/*****************************************************************************
*
*  Funktion       Aus Datei lesen mit Fehlerbehandlung (save_read)
*  --------
*
*  Parameter    : f         :
*                   Typ          : int
*                   Wertebereich : Dateihandle
*                   Bedeutung    : Datei, aus der gelesen werden soll
*
*                 b         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf Speicherbereich
*                   Bedeutung    : Puffer, in den Daten gelesen werden
*
*                 n         :
*                   Typ          : int
*                   Wertebereich : 0 - MAX_INT
*                   Bedeutung    : Anzahl einzulesender Bytes
*
*  Beschreibung : Aus der Datei mit dem Handle f werden n Bytes in den
*                 Puffer b gelesen. Ist dies nicht moeglich, so wird eine
*                 Fehlermeldung ausgegeben und versucht, das Config-File
*                 zu loeschen. Gelingt dies nicht, so wird eine entspre-
*                 chende Meldung ausgegeben.
*                 Diese Funktion wird nur von read_config() aufgerufen.
*
*****************************************************************************/

void save_read(f,b,n)
int f,n;
char *b;
{
  if(read(f,b,n) < n)  /* Falls Lesen fehlschlaegt, Meldung ausgeben */
  {                    /* und Datei loeschen.                        */
    pe_or(PROMPT_ERRCONF);
    close (f);
    if(unlink(conffile) < 0)
      pe_or(PROMPT_ERRDELETE);
    write_config(); /* Wenigstens versuchen, was bisher gelesen wurde zu retten */
    ende(1, TRUE);  /* da noch nichts gemacht wurde, muss auch nichts gespeichert */
  }                 /* werden */
}

/*****************************************************************************
*
*  Funktion       Koordinaten ueberpruefen und ggf. anpassen (check_koors)
*  --------
*
*  Parameter    : dy        :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer-Variable
*                   Bedeutung    : Hoehe des Bereichs in Bildschirmzeilen
*
*                 dx        :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer-Variable
*                   Bedeutung    : Breite des Bereichs in Spalten
*
*                 y         :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer-Variable
*                   Bedeutung    : Anfangszeile
*
*                 x         :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer-Variable
*                   Bedeutung    : Anfangsspalte
*
*  Beschreibung :   Es wird ueberprueft, ob ein Fenster mit den uebergebenen
*                   Koordinaten vollstaendig auf dem Bildschirm dargestellt
*                   werden koennte (die Groesse eines Rahmens wird intern
*                   beruecksichtigt). Ist dies nicht der Fall, so werden
*                   die Koordinaten angepasst.
*
*****************************************************************************/

void check_koors(dy,dx,y,x)
int *dy,*dx,*y,*x;
{
  if(*dy > MAX_HEIGHT - 2)  /* -2 wg. rahmen */
    *dy = MAX_HEIGHT - 2;

  if(*y + *dy > MAX_HEIGHT-2 + START_Y)
    *y = MAX_HEIGHT - *dy - 2 + START_Y;

  if(*dx > MAX_WIDTH - 2)
    *dx = MAX_WIDTH - 2;

  if(*x + *dx > MAX_WIDTH - 2 + START_X)
    *x = MAX_WIDTH - *dx - 2 + START_X;
}

/*****************************************************************************
*
*  Funktion       Config-Datei einlesen (read_config)
*  --------
*
*  Parameter    : argc :
*                   Typ          : int
*                   Wertebereich : 1 - maxint
*                   Bedeutung    : Anzahl v. Parameter in der Kommandozeile
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : 0 - maxint
*                   Bedeutung    : Anzahl eingelesener Dateien
*
*  Beschreibung :  Es wird versucht, eine Config-Datei einzulesen.
*                  Existiert keine solche Datei, so wird 0 zurueckgegeben.
*                  Die Config-Datei hat folgenden Aufbau:
*                    char helpflag,backupflag,highblockflag;
*                    short int Anzahl Macros in Datei
*                    short int Anzahl short ints 1. Macro
*                     <entsprechend viele short ints>
*                    ... gleiches fuer alle anderen Macros
*                    short int Anzahl der Fenster
*                    <Anzahl der Fenster> Strukturen vom Typ win_typ
*                    Hierbei ist zu beachten, dass in dem Struktur-
*                    element filename die Laenge des Dateinamens in Byte
*                    steht. Der Filename selbst folgt der jeweiligen
*                    Struktur unmittelbar und ist nicht durch ein bes.
*                    Zeichen terminiert.
*                    Fuer jedes Fenster wird die entsprechende Datei
*                    geladen und ein Fenster geoeffnet.
*                    short int Anzahl Marker
*                    <Anzahl Marker> Strukturen vom Typ marker_typ
*                  Endet die Datei unerwartet, so wird eine Fehlermel-
*                  dung ausgegeben und das Programm abgebrochen.
*
*****************************************************************************/

int read_config(argc)
int argc;
{
  /* *** interne Daten und Initialisierung *** */
  int       i,          /* Z„hler fr Fenster, Macros etc.           */
	    fh,         /* Filehandle fuer Config-Datei              */
	    mnum=0,     /* Schleifenzaehler fuer Einlesen der Macors */
	    msizeb,     /* Macrogroesse in Bytes                     */
	    old_tl,     /* Zwischenspeicher fuer Cursorzeile         */
	    old_sc,     /* Zwischenspeicher fuer Cursorspalte        */
	    namelen;    /* Laenge des Filenamens des aktuellen Fstr. */
  short int msize,      /* Macrogroesse in short int gemessen        */
	    anz_win=0,  /* Anzahl der Fenster                        */
	    anz_marker, /* Anzahl der Marker                         */
	    anz_macros; /* Anzahl der Macros                         */
  win_typ   *old_next,  /* Zwischenspeicher der Verzeigerung des ak- */
	    *old_prev,  /* tuell eingelesenen Fensters, da abgespei- */
			/* cherte Verzeigerung jetzt falsch ist.     */
	    dummy;      /* Zum evtl. Skippen von Fenstereintraegen   */

  if(access(conffile,4))   /* Kann das Config-File gelesen werden ? */
    return(0);             /* kein Config-File */
  fh = open(conffile,O_RDONLY | O_BINARY);    /* Datei oeffnen */
  save_read(fh,&helpflag,1);       /* Die vier globalen Flags einlesen */
  save_read(fh,&backupflag,1);
  save_read(fh,&highblockflag,1);
  save_read(fh,&regexflag,1);

  save_read(fh,&anz_macros,SH_SIZ); /* Anzahl der Macros einlesen */
  for(i=anz_macros;i>0;i--)         /* Fuer alle Macros Groesse   */
  {                                 /* und Inhalt einlesen        */
    save_read(fh,&msize,SH_SIZ);
    if(msize >= 0)
      if(mnum < ANZ_MACROS)
      {
	if(!msize)
	  macro[mnum].begin = macro[mnum].end = NULL; /* leere Macros auf Null */
	else
	{
	  msizeb = msize * SH_SIZ;
	  save_read(fh,macro[mnum].begin = (short int *) reserve_mem(msizeb),msizeb);
	  macro[mnum].end = macro[mnum].begin + msize - 1;
	}
	mnum++;
      }
      else
	lseek(fh,(long)(msize * SH_SIZ),1); /* Zum naechsten Macro in Datei */
  }
  if(argc == 1)         /* no params - use old windows and markers */
  {
    save_read(fh,&anz_win,SH_SIZ); /* Fensteranzahl einlesen */
    for(i=anz_win;i>0;i--)
    {
      if(koppel_win())             /* Neues Fenster in Liste einhaengen */
      {
	old_next = akt_winp->next; /* Verzeigerung merken */
	old_prev = akt_winp->prev;
	save_read(fh,akt_winp,sizeof(win_typ)); /* Fensterdaten einlesen */
	akt_winp->block.bstart = (bzeil_typ*) NULL; /* Blockzeiger l”schen */
	akt_winp->next = old_next; /* Verzeigerung wieder korrigieren    */
	akt_winp->prev = old_prev;
	akt_winp->filename = reserve_mem((namelen = (int)akt_winp->filename)+1);
	save_read(fh,akt_winp->filename,namelen); /* Filenamen einlesen  */
	akt_winp->filename[namelen] = '\0'; /* Filenamen abschliessen    */
	old_tl = akt_winp->textline; /* Cursorposition zwischenspeichern */
	old_sc = akt_winp->screencol;
	if(!lies_file())             /* Dateitext einlesen */
	{                            /* klappt das nicht, Fenster wieder */
	  gb_win_frei();             /* freigeben und Fensteranzahl um   */
	  anz_win--;                 /* eins reduzieren.                 */
	  setz_cursor(W_NOTAKT);     /* lies_file macht kein setz_cursor */
	}
	else  /* Einlesen des Textes hat geklappt */
	{
	  /* Ueberpruefen, ob Fenster auf Bildschirm passt, ggf. anpassen */
	  check_koors(&akt_winp->dy,&akt_winp->dx,&akt_winp->y,&akt_winp->x);
	  check_koors(&akt_winp->ady,&akt_winp->adx,&akt_winp->ay,&akt_winp->ax);
	  if(old_tl > akt_winp->maxline)  /* nur wenn old_tl groesser maxline */
	  {                               /* kann auch ws_line zu gross sein  */
	    old_tl = akt_winp->maxline;
	    if(akt_winp->ws_line > akt_winp->maxline)
	      if((akt_winp->ws_line = akt_winp->maxline - akt_winp->dy + 1) < 0)
		akt_winp->ws_line = 0;
	  }
	  gotox(old_tl); /* Cursorposition restaurieren */

	  /* Wenn Cursorspalte zu gross, anpassen */
	  if((akt_winp->screencol = old_sc) > MAXLENGTH)
	    akt_winp->screencol = MAXLENGTH;
	  if(akt_winp->lastcol > MAXLENGTH)
	    akt_winp->lastcol = MAXLENGTH;

	  /* Falls Cursor ausserhalb des Fensters steht, */
	  /* Fensterinhalt anpassen.                     */
	  if(akt_winp->ws_line + akt_winp->dy <= akt_winp->textline)
	    akt_winp->ws_line = akt_winp->textline - akt_winp->dy + 1;
	  if(akt_winp->ws_col + akt_winp->dx <= akt_winp->screencol)
	    akt_winp->ws_col = akt_winp->screencol - akt_winp->dx + 1;
	  akt_winp->winp = newwin(akt_winp->dy+2,akt_winp->dx+2,akt_winp->y,akt_winp->x);
	  init_win();  /* Fenster mit Curses anlegen und initialisieren */
	  show_win(W_NOTAKT); /* Fensterinhalt auf Bildschirm darstellen */
	}
      }
      else  /* Falls kein Fenster mehr in Liste passte: */
      {
	print_err(W_COUNT_ERRTEXT);   /* Fehlermeldung ausgeben */
	while(i--)                    /* Alle weiteren Fenstereintraege skippen */
	{
	  save_read(fh,&dummy,sizeof(win_typ));
	  lseek(fh,(long)dummy.filename,1);
	}
	break;     /* keine weiteren Dateien lesen */
      }
    }

    rahmen(W_AKT); /* Cursor setzen, Rahmen highlighten */
    setz_cursor(W_AKT); 

    save_read(fh,&anz_marker,SH_SIZ); /* Anzahl der Marker einlesen */
    if(anz_marker > ANZ_MARKER)
      anz_marker = ANZ_MARKER;
    save_read(fh,marker,anz_marker * sizeof(marker_typ));
    for(i=0;i<anz_marker;i++)
      if(marker[i].col > MAXLENGTH)   /* falls EHP in der Zwischenzeit mit */
	  marker[i].col = MAXLENGTH;  /* einer Verringerung von MAXLENGTH  */
  }                                   /* neu kompiliert wurde              */
  close(fh);
  return(anz_win);
}

/*****************************************************************************
*
*  Funktion       In Datei schreiben mit Fehlerbehandlung (save_write)
*  --------
*
*  Parameter    : f         :
*                   Typ          : int
*                   Wertebereich : Dateihandle
*                   Bedeutung    : Datei, in die geschrieben werden soll
*
*                 b         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf Speicherbereich
*                   Bedeutung    : Puffer, aus dem Daten geschrieben werden
*
*                 n         :
*                   Typ          : int
*                   Wertebereich : 0 - MAX_INT
*                   Bedeutung    : Anzahl zu schreibender Bytes
*
*  Beschreibung : In die Datei mit dem Handle f werden n Bytes aus dem
*                 Puffer b gelesen. Ist dies nicht moeglich, so wird die
*                 Variable wc_errflag gesetzt.
*                 Ist wc_errflag schon gesetzt, wenn die Funktion aufge-
*                 rufen wird, so wird nichts in die Datei geschrieben.
*                 Diese Funktion wird nur von write_config() aufgerufen.
*
*****************************************************************************/
							
void save_write(f,b,n)
int f,n;
char *b;
{
  if(!wc_errflag && (write(f,b,n) < n)) /* liefert write Fehler,   */
    wc_errflag = TRUE;                  /* wird wc_errflag gesetzt */
}

/*****************************************************************************
*
*  Funktion       Config-Datei schreiben (write_config)
*  --------
*
*  Beschreibung : Das Config-File wird in dem in read_config beschriebenen
*                 Format abgespeichert. Trat dabei ein Fehler auf, so wird
*                 eine entsprechende Fehlermeldung ausgegeben und die Datei
*                 geloescht.
*
*****************************************************************************/

void write_config()
{
  /* *** interne Daten *** */
  int       fh,         /* Handle fuer Config-Datei               */
	    namelen,    /* Laenge des Filenamens fuer ein Fenster */
	    i;          /* Schleifenzaehler                       */
  short int msize,      /* Macrogroesse                           */
	    anz_macros, /* Anzahl der belegten Macros             */
	    anz_win,    /* Anzahl der Fenster                     */
	    anz_marker; /* Anzahl der belegten Marker             */
  char      *name;      /* Filename des zu schreibenden Fensters  */
  win_typ   *w;         /* Pointer fuer Schleife ueber Windows    */

  if((fh = open(conffile,O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,0666)) < 0)
  {
    print_err(PROMPT_ERROPNCFG);
    return;                /* kein Config-File */
  }

  wc_errflag = FALSE; /* Fehlerflag zuruecksetzen */

  /* globale flags schreiben */
  save_write(fh,&helpflag,1);
  save_write(fh,&backupflag,1);
  save_write(fh,&highblockflag,1);
  save_write(fh,&regexflag,1);

  /* macros schreiben */
  for(anz_macros = ANZ_MACROS; !macro[anz_macros-1].begin; anz_macros--);
  save_write(fh,&anz_macros,SH_SIZ);    /* falls hintere macros leer, nicht */
  for(i=0;i<anz_macros;i++)             /* schreiben */
  {
    msize = macro[i].begin
	    ? macro[i].end - macro[i].begin + 1
	    : 0;                                 /* size in short ints */
    save_write(fh,&msize,SH_SIZ);
    if(msize > 0)
      save_write(fh,macro[i].begin,msize * SH_SIZ); /* Inhalt schreiben */
  }

  /* windows schreiben */
  for(w=akt_winp->next->next,anz_win = 0;w != akt_winp->next;w=w->next,anz_win++);
  save_write(fh,&anz_win,SH_SIZ);  /* Anzahl der Fenster schreiben */
  for(w = akt_winp->next->next,i=anz_win;i>0;w = w->next,i--)
  {
    name = w->filename; /* Pointer auf Filename durch dessen Laenge ersetzen */
    w->filename = (char*) (namelen = strlen(w->filename));
    save_write(fh,w,sizeof(win_typ)); /* Fensterstruktur abspeichern */
    save_write(fh,name,namelen); /* Dateinamen abspeichern */
    w->filename = name;          /* Filenamen rekonstruieren */
  }

  /* marker schreiben */
  /* Zuerst Anzahl der belegten Marker ermitteln */
  for(anz_marker = ANZ_MARKER; marker[anz_marker-1].window == -1; anz_marker--);
  save_write(fh,&anz_marker,SH_SIZ); /* Anzahl der Marker schreiben */
  /* Belegten Anteil des Markerfeldes abspeichern */
  save_write(fh,marker,anz_marker * sizeof(marker_typ));

  close(fh);
  if(wc_errflag)  /* trat beim Abspeichern ein Fehler auf, dann loeschen */
  {
    print_err(PROMPT_ERRWRTCFG);
    if(unlink(conffile) < 0)
      print_err(PROMPT_ERRUNLINK);
  }
}
