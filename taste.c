/****************************************************************/
/*                                                              */
/*      MODUL:  taste.c                                         */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - lies_tasten (Tastenbelegunsfile einlesen)     */
/*              - search_next (suche naechsten passenden Bef.)  */
/*              - taste (Tastendruck auswerten)                 */
/*              - newwgetch (Taste von Keyboard || Puffer lesen)*/
/*              - exec_macro (Macro ausfuehren)                 */
/*              - newgetstr (String mittels newgetch einlesen)  */
/*              - get_comstr (Kommandostring einlesen)          */
/*                                                              */
/****************************************************************/

#ifdef OS2
#define INCL_DOSSEMAPHORES
#include <os2.h>
#else
#define _ANSI_SOURCE
#endif

#include "defs.h"
#include "keys.h"
#include <ctype.h>

extern char helpflag;
extern int ks_index;
extern short int *keystack,*e_keystack;  /* puffer-pointer */
extern short int mc_index;
extern WINDOW *status;
extern char clear_buff, /* aus main.c */
	    *tasten_inf; /* Name des Tastenbelegungsfiles aus main.c */
extern int do_macro(),do_control(),do_newline(),(*funktion[])();  /* funktionsarray aus auswertung.c f. get_comstr */
extern puff_typ macro[];
#ifdef OS2
extern HMTX sem_handle;
#endif

/* *** globale Daten und Initialisierung *** */
static short int eingabe[MAX_COMM_LEN]; /* Bisher eingelesene Zeichen          */
static short int index=0,               /* Index in eingabe                    */
		 bef_index=0;           /* aktuelle moegliche Tastenkombination*/
puff_typ  puff_feld[MACRO_NEST_DEPTH];  /* Macropuffer                         */
comm_typ  *comm_feld;                   /* Tastenkombinationen                 */
short int letter,                       /* Buchstabe, falls Befehl T_BUCHSTABE */
	  lastcode = -1,                /* Letzte Tastenkombination            */
	  newwgetch();

#ifndef OWN_CURSES
/*****************************************************************************
*
*  Funktion       Warnton ausgeben (beep)
*  --------
*
*  Beschreibung : Es wird ein CTRL-G ausgegeben, was einen Warnton erzeugt.
*
*****************************************************************************/

void beep()
{
   printf("");  /* Beep ausgeben */
}
#endif

/*****************************************************************************
*
*  Funktion       Tastenbelegungsfile einlesen (lies_tasten)
*  --------
*
*  Beschreibung : Liest die Tastenbelegungsdatei in das Feld comm_feld ein.
*
*****************************************************************************/

void lies_tasten()
{
  /* *** interne Daten *** */
  FILE               *f;     /* Filepointer fuer Tastenbelegungsdatei */
  register short int i,      /* Zaehler fuer Kombinationsnummer       */
		     j;      /* Zaehler fuer Indexfeld                */
  register int       n_info; /* Anzahl einzulesender Strings          */
  short int          *index; /* Feld fuer Stringindices               */
  char               *hilf,  /* Zeiger auf Hilfstring                 */
		     dummy [256]; /* Zum einlesen der Hilfstring      */

  if (!(f = fopen (tasten_inf,"rb"))) /* Tastebelegungsdatei oeffnen */
  {
    print_stat(PROMPT_KEYREAD);
    wart_ret();
    ende(1, TRUE);
  }
  if(fread(&mc_index,sizeof(short int),1,f) < 1) /* Anzahl Kommandos lesen */
  {
    print_stat(PROMPT_KEYLENGTH);
    wart_ret();
    ende(1, TRUE);
  }
  comm_feld = (comm_typ*)  reserve_mem((mc_index+1) * sizeof(comm_typ));
  index     = (short int*) reserve_mem((mc_index+1) * sizeof(short int));
  for (n_info=i=0;i<=mc_index;i++)  /* Alle Kombinationen einlesen */
    if (fread (&comm_feld[i].blen,sizeof(short int),1,f) < 1)
    { /* Laenge der Kombination einlesen hat nicht geklappt */
      print_stat(PROMPT_KEYCOMM);
      wart_ret();
      ende(1, TRUE);
    }
    else /* Laenge Einlesen hat geklappt, dann eigentliche Kombination lesen */
    {
      if (comm_feld[i].blen > MAX_COMM_LEN)
      { /* Befehl seltsamerweise zu lang */
	print_stat(PROMPT_KEYCOMLEN);
	wart_ret();
	ende(1, TRUE);
      }
      fread (comm_feld[i].befehl,sizeof(short int),comm_feld[i].blen,f);
      fread (&index[i],sizeof(short int),1,f); /* Stringindex lesen */
      comm_feld[i].info_text = NULL;
      if(index[i] > -1)
	for (n_info++,j=0;j<i;j++)  /* Testen, ob Stringindex schon in */
	  if (index[j] == index[i]) /* Indextabelle enthalten ist      */
	  {
	    n_info--;   /* Nur einen weiteren String einlesen,  */
	    break;      /* falls neuer Index                    */
	  }
    }

  /* Jetzt die noetige Anzahl Hilfstrings einlesen */
  for (i=0;i<n_info;i++)
  {
    /* Einen String einlesen und testen, ob er mit '\n' terminiert ist */
    if (!fgets (dummy,255,f) || !(hilf = strchr(dummy,'\n')))
    {
      print_stat(PROMPT_KEYHELP);
      wart_ret();
      ende(1, TRUE);
    }
    *hilf = '\0';  /* '\n' ueberschreiben */
    if (!(hilf = save_text (dummy))) /* Hilfstext im Speicher ablegen */
    {
      print_stat(PROMPT_KEYHELPEM);
      wart_ret();
      ende(1, TRUE);
    }
    for (j=0;j<=mc_index;j++) /* Bei allen Tastenkombinationen, die den */
      if (index[j] == i) /* aktuellen Stringindex aufweisen, Zeiger auf */
	comm_feld[j].info_text = hilf; /* Hilfstext setzen              */
  }
  fclose (f);
}

/*****************************************************************************
*
*  Funktion       suche naechsten passenden Befehl (search_next)
*  --------
*
*  Parameter    : eingabe   :
*                   Typ          : short int[]
*                   Wertebereich : Pointer auf integer
*                   Bedeutung    : bisher eingelesene Tastencodes
*
*               : index     :
*                   Typ          : short int
*                   Wertebereich : 0..MAX_COMM_LEN
*                   Bedeutung    : index des zuletzt eingelesenen Zeichens
*
*               : bef_index :
*                   Typ          : short int
*                   Wertebereich : 0..mc_index
*                   Bedeutung    : aktuelle, nicht mehr passende Tasten-
*                                  kombination
*
*  Ergebnis     :
*                   Typ          : short int
*                   Wertebereich : -1,1..mc_index
*                   Bedeutung    : naechste passende Tastenkombination
*
*  Beschreibung : Sucht in comm_feld die naechste auf die bisher eingele-
*                 lesenen Zeichen passende Tastenkombination. Wird keine
*                 entsprechende Kombination gefunden, so wird -1 zurueck-
*                 gegeben.
*
*****************************************************************************/

short int search_next(eingabe,index,bef_index)
short int eingabe[];
register short int index,bef_index;
{
  /* *** interne Daten *** */
  register short int *inp, /* Zeiger in alte Eingabe            */
		     *bef; /* Zeiger in moeglichen "Kandidaten" */

  while(++bef_index <= mc_index) /* Alle weiteren Kombinationen testen */
  {
    for(inp = eingabe,bef = comm_feld[bef_index].befehl;
	(inp <=eingabe+index) && *inp == *bef;inp++,bef++);
    if(inp > eingabe + index) /* passende gefunden */
      return(bef_index);
  }
  return(-1);
}

/*****************************************************************************
*
*  Funktion       Tastendruck auswerten (taste)
*  --------
*
*  Parameter    :   w
*                   Typ          : WINDOW *
*                   Wertebereich : Pointer auf curses-Windowstruktur
*                   Bedeutung    : Fenster, in dem Befehl eingelesen werden
*                                  soll
*
*  Ergebnis     :
*                   Typ          : short int
*                   Wertebereich : 0..mc_index+1
*                   Bedeutung    : Interne Nummer der Tastenkombination
*
*  Beschreibung : Es wird eine Taste eingelesen und in der Tabelle nach
*                 einer Kombination beginnend mit dieser Taste gesucht.
*                 Wird keine gefunden, so wird mc_index+1 zurueckgegeben und
*                 die Taste in letter gespeichert.
*                 Wird die Kombination mit dieser Taste abgeschlossen, so
*                 wird die Nummer der gefundenen Kombination zurueckgegeben.
*                 Sonst wird das naechste Zeichen gelesen. Ergibt sich da-
*                 durch keine zulaessige Kombination, so werden alle Zeichen
*                 verworfen. Sonst wird wieder verglichen usw.
*                 Ist das Eingabefenster das Statusfenster, so wird der
*                 Cursor nach Anzeige eines Infotextes nicht positioniert.
*
*****************************************************************************/

short int taste(w)
WINDOW *w;
{
  /* *** interne Daten und Initialisierung *** */
  short int hilf;        /* Zwischenspeicher fuer letzte Tastenkombination */
  char      hgz = FALSE  /* Flag, ob ein Hilfstext angezeigt wurde         */
#ifdef MOUSE
	    , mouse_akt=FALSE  /* Flag, ob Maus aktiviert ist */
	    , macro_active     /* Flag, ob Macro aktiv        */
#endif
			    ;

  if (lastcode != -1)
  {
    hilf = lastcode; /* Falls eine Funktion ein "ungetchar" ausgefuehrt hat, */
    lastcode = -1;   /* wird der geungetcharte code zurueckgegeben.          */
    return (hilf);
  }
  index = bef_index = 0; /* Alle Kombinationen kommen in Frage */
  while(1)
  {
#ifdef MOUSE
    if(!mouse_akt && ks_index == -1 && !index) /* Falls kein Macro aktiv ist */
    { /* und noch kein Zeichen eingelesen wurde */
#ifdef OS2
      DosReleaseMutexSem (sem_handle);
#else
      set_mouse_int(MOUSE_MASK);  /* Vor Eingabe Maus aktivieren */
#endif
      mouse_akt = TRUE;
      hide_show (MOUSE_SHOW);
    }
#endif
    letter = eingabe[index] = newwgetch(w); /* Ein Zeichen von Tastatur/Macro */
#ifdef MOUSE
    if(mouse_akt)
    { /* Nach erstem gelesenen Zeichen Maus deaktivieren */
      mouse_akt = FALSE; /* damit Maus nicht zwischen ein Kommando funkt */
      hide_show (MOUSE_HIDE);
#ifdef OS2
      DosRequestMutexSem (sem_handle,-1);
#else
      set_mouse_int(0); 
#endif /* OS2 */
    }
#endif /* MOUSE */
    if(hgz)  /* wurde Hilfstext angezeigt? */
    {
      hgz = FALSE;
      clear_stat();                   /* Hilfstext wieder loeschen              */
      if(w != status)                 /* wenn nicht im Statusfenster eingelesen */
	pos_cursor();                 /* wurde, dann muss Cursor wieder positio-*/
    }                                 /* niert werden */
    if (letter == -1)                 /* War nodelay gesetzt und keine Taste    */
      if(!index)                      /* gedrueckt ? Falls erstes Zeichen */
	return (-1);                  /* eine Kombination, dann -1 zurÅck */
      else                            /* Sonst -1 ignorieren, da sonst evtl. */
	continue;                     /* Kombination "zerhackt" wÅrde */
    if(comm_feld[bef_index].befehl[index] != eingabe[index])
    {
      /* naechsten passenden Befehl suchen */
      if((bef_index = search_next(eingabe,index,bef_index)) < 0) /* keinen gefunden? */
	if(!index && (letter % 256))    /* normal char and not zero */
	{
	  index++;                      /* ein zeichen eingelesen */
	  return(bef_index = mc_index+1);
	}
	else                            /* Befehl verwerfen */
	{
	  index = bef_index = 0;
	  continue;
	}
    }
    if(++index == comm_feld[bef_index].blen) /* passte Zeichen, dann testen, */
      return(bef_index);                /* ob komplette Kombination gelesen. */
    if (helpflag && comm_feld[bef_index].info_text)
    { /* wenn Hilfstexte angezeigt werden sollen und die momentane Kom- */
      hgz = TRUE; /* bination einen hat, dann ausgeben */
      print_hilf (comm_feld[bef_index].info_text);
    }
  }
}

/*****************************************************************************
*
*  Funktion       Zeichen aus Puffer oder v. Tastatur lesen (newwgetch)
*  --------
*
*  Parameter    : w         :
*                   Typ          : WINDOW *
*                   Wertebereich : Pointer auf curses-Windowstruktur
*                   Bedeutung    : Fenster, in dem eingegeben werden soll
*
*  Ergebnis     :
*                   Typ          : short int
*                   Wertebereich : curses-Tastenwerte
*                   Bedeutung    : aktuelles Zeichen (s.u.)
*
*  Beschreibung : Diese Funktion liefert das naechste aktuelle Zeichen
*                 zurueck. Dies ist, falls noch Zeichen im Puffer sind, d.h.
*                 ks_index noch nicht -1 ist bzw. in Puffer 0 noch nicht das
*                 letzte Zeichen verarbeitet wurde, das aktuelle Zeichen im
*                 Puffer, andernfalls eine eingelesene Taste.
*                 Zur Erklaerung der Puffer:
*                   ks_index ist der Index des aktuellen Puffers. Ist der
*                   Puffer [ks_index] leer und ks_index>0, so wird in
*                   Puffer [ks_index-1] weitergelesen.
*                   Die Puffer sind in einer Tabelle vom Typ puff_typ[]
*                   gespeichert. Das Strukturelement begin stellt einen
*                   Zeiger auf das aktuelle Element dar, waehrend end auf
*                   das Pufferende zeigt.
*                   keystack ist ein Pointer in den aktuellen Puffer und
*                   zeigt auf das aktuelle Element (falls ks_index>=0);
*                   e_keystack zeigt auf das Ende des aktuellen Puffers.
*
*****************************************************************************/

short int newwgetch(w)
WINDOW *w;
{
  short int result;

  if(clear_buff) /* Wenn Puffer als zu loeschend markiert sind: */
  {
    while(ks_index > -1)  /* Macro beenden, evtl. freizugebende Strings freigeben */
    {
      if(puff_feld[ks_index].free_flag)
	line_free(puff_feld[ks_index].begin);
      ks_index--;
    }
    clear_buff = FALSE;  /* Flag zuruecksetzen */
  }
  if(ks_index != -1)  /* Wenn ein Puffer aktiv, dann aus Puffer lesen */
  {
    result = *keystack++; /* Dann  aktuelles Pufferzeichen zurueckgeben */
    /* Alle Zeiger auf nÑchstes Pufferzeichen setzen */
    while (ks_index >= 0 && (!keystack || keystack > e_keystack))
    {
      if(puff_feld[ks_index].anz_rep) /* War es eine Repeat-Funktion ? */
      {
	puff_feld[ks_index].anz_rep--;
	keystack = puff_feld[ks_index].begin; /* Selben Puffer nochmal */
	e_keystack = puff_feld[ks_index].end;
      }
      else /* Puffer war kein Repeat-Puffer, Puffer loeschen */
      {
	if(puff_feld[ks_index].free_flag)   /* falls free_flag gesetzt ist, Puffer freigeben */
	  line_free(puff_feld[ks_index].begin);
	/* keine Zeichen mehr im aktuellen Puffer; vielleicht noch im vorigen? */
	if(--ks_index >= 0)     /* noch nicht erster Puffer? */
	{
	  keystack = puff_feld[ks_index].current; /* im vorherigen Puffer weiter */
	  e_keystack = puff_feld[ks_index].end;
	}
      }
    }
  }
  /* Kein Puffer (mehr) aktiv, von Tastatur lesen */
  else
    result = wgetch(w);
  return result;
}


/*****************************************************************************
*
*  Funktion       Macro ausfuehren (exec_macro)
*  --------
*
*  Parameter    : mnum      :
*                   Typ          : short int
*                   Wertebereich : 'a' - 'a'+ANZ_MACROS
*                   Bedeutung    : Name des auszufuehrenden Macros
*
*  Beschreibung : Der gewuenschte Macro wird ausgefuehrt, indem auf den
*                 Pufferstapel ein neuer Puffer geschoben wird, in den der
*                 Macrotext kopiert wird. Falls das nicht der erste Puffer
*                 auf dem Stapel ist, wird die Position im alten Puffer
*                 in current abgelegt. Anschliessend wird keystack und
*                 ekeystack korrekt gesetzt.
*
*****************************************************************************/

void exec_macro(mnum)
register short int mnum;
{
  /* *** interne Daten *** */
  char dummy[80], /* Zum Zusammenflicken der Fehlermeldung */
       old_delay; /* Zwischenspeicher fuer nodelay-Zustand */

  if(mnum>='a' && mnum<='z')  /* exec macro */
    if(macro[mnum-='a'].begin) /* Macro auch nicht leer ? */
      if(ks_index < MACRO_NEST_DEPTH - 1) /* Verschachtelungstiefe OK ? */
      {
	if(ks_index >=0) /* falls schon ein Puffer aktiv, aktuelle Position */
	  puff_feld[ks_index].current = keystack;       /* im Puffer merken */

	/* Zeiger in neuen Puffer setzen */
	keystack = puff_feld[++ks_index].begin = macro[mnum].begin;
	e_keystack = puff_feld[ks_index].end = macro[mnum].end;
	puff_feld[ks_index].anz_rep = 0; /* Puffer ist nicht zu wiederholen */
	puff_feld[ks_index].free_flag = FALSE;  /* Puffer nach Ausfuehrung  */
      } /* nicht freigeben, da sonst Macrotext freigegeben wuerde */
      else /* zu grosse Verschachtelungstiefe */
      {
	clear_buff = TRUE;  /* beim naechsten Aufruf von newwgetch() Puffer loeschen */
#ifdef OWN_CURSES
	if(old_delay = akt_winp->winp->_dlyflag)
	  nodelay(akt_winp->winp,FALSE);  /* falls true eingestellt */
#endif
	print_stat(PROMPT_RECURSION);
	wart_ret();
	clear_stat();
#ifdef OWN_CURSES
	nodelay(akt_winp->winp,old_delay);
#endif
      }
    else /* Macro war leer */
    {
      sprintf(dummy,PROMPT_MACROEMPT,mnum+'a');
      print_err(dummy);
    }
}

/*****************************************************************************
*
*  Funktion       String von Tastatur einlesen (newgetstr)
*  --------
*
*  Parameter    : w         :
*                   Typ          : WINDOW *
*                   Wertebereich : Pointer auf curses-Windowstruktur
*                   Bedeutung    : Fenster, in dem Text eingelesen werden soll
*
*                 y         :
*                   Typ          : int
*                   Wertebereich : 0 - Anzahl der Zeilen im Window-1
*                   Bedeutung    : Zeile, in der eingelesen werden soll
*
*                 x         :
*                   Typ          : int
*                   Wertebereich : 0 - Anzahl der Spalten im Window-1
*                   Bedeutung    : Spalte, ab der eingelesen werden soll
*
*                 buff      :
*                   Typ          : char *
*                   Wertebereich : Pointer auf Speicherbereich
*                   Bedeutung    : Speicherbereich, in den Eingabe ge-
*                                  schrieben wird (muss min. max_anz + 1
*                                  Bytes lang sein)
*
*                 max_anz   :
*                   Typ          : int
*                   Wertebereich : 0 - MAX_INT
*                   Bedeutung    : Anzahl maximal einzulesender Zeichen
*                                  = 0 : wartet nur auf RETURN
*                 typ       :
*                   Typ          : int
*                   Wertebereich : GS_NUM, GS_ANY
*                   Bedeutung    : GS_NUM: nur Ziffern einlesen
*                                  GS_ANY: alle Zeichen zulassen
*
*  Beschreibung : Diese Funktion liest mittels newgetch einen String ein.
*                 Hierbei sind die Funktionstasten Backspace (loesche Zeichen
*                 links und bewege Cursor um eine Spalte nach links), Pfeil
*                 links (Cursor um eine Position nach links) und Pfeil rechts
*                 (Cursor um eine Position nach rechts) erlaubt. Maximal
*                 koennen max_anz Zeichen eingegeben werden; versucht man,
*                 mehr Zeichen einzugeben, ertoent ein Piepston und die Ein-
*                 gabe wird ignoriert.
*                 Da bei Backspace die Zeile bis zum Ende geloescht wird,
*                 kann unbeabsichtigt Information geloescht werden (s.u.).
*
*****************************************************************************/

void newgetstr(w,y,x,buff,max_anz,typ)
WINDOW *w;
int y,x,max_anz,typ;
char *buff;
{
  /* *** interne Daten und Initialisierung *** */
  short int tmp_inp;        /* Zum einlesen eines Zeichens          */
  int       akt_index = 0,  /* Index in Eingabepuffer               */
	    max_index = -1; /* Index fuer letztes Zeichen in Puffer */

  if(max_anz < 0)
    return;

  buff[akt_index] = '\0';  /* Pufferende markieren */
  do
  {
    wmove(w,y,x + akt_index); /* Cursor an gewuenschte Position */
    wrefresh(w);
    switch(tmp_inp = newwgetch(w))    /* echoing has to be disabled */
    {
      case '\010':          /* Backspace */
	if(akt_index > 0)
	{ /* Pufferrest eins nach links kopieren */
	  fwdcpy(&buff[akt_index-1],&buff[akt_index]);
	  akt_index--;
	  max_index--;
	  mvwaddstr(w,y,x,buff);
	  wclrtoeol(w); /* bis Zeilenende loeschen wg. evtl. ESC-codes */
	}
      break;
#ifndef OS2 /* Bei OS/2 ist alles in einem Zeichen enthalten */
      case '\0':            /* "Escape"-Sequenz */
	switch(newwgetch(w))
	{
#endif
	  case KEY_LEFT:        /* Pfeil links */
	    if(akt_index > 0)
	    akt_index--;
	    break;
	  case KEY_DC:          /* Delete char */
	    if(akt_index <= max_index)
	    {
	      fwdcpy(&buff[akt_index],&buff[akt_index+1]);
	      max_index--;
	      mvwaddstr(w,y,x,buff);
	      wclrtoeol(w); /* bis Zeilenende loeschen wg. evtl. ESC-codes */
	    }
	    break;
	  case KEY_RIGHT:       /* Pfeil rechts */
	    if(akt_index <= max_index)
	    akt_index++;
	    break;
#ifndef OS2
	  default:
	    beep();  /* Unbekannte "Escape"-Sequenz */
	}
	break;
#endif
      default:
	/* Bei allen anderen Zeichen ausser dem abschliessenden Zeichen   */
	/* Typ testen und falls korrekt und noch Platz im Puffer, Zeichen */
	/* eintragen */
	if(tmp_inp != END_KEY)
	  if(max_index < max_anz-1
	  && (typ & GS_NUM && isdigit(tmp_inp) || typ & GS_ANY))
	  {
	    max_index++;
	    revcpy(&buff[akt_index+1],&buff[akt_index]);  /* insert char */
	    buff[akt_index++] = tmp_inp;
	    mvwaddstr(w,y,x,buff);
	  }
	  else      /* Typ falsch oder Puffer voll */
	    beep();
      break;
    }
  } while(tmp_inp != END_KEY);
}

/*****************************************************************************
*
*  Funktion       Kommando-String einlesen (get_comstr)
*  --------
*
*  Parameter    : anf       :
*                   Typ          : short int **
*                   Wertebereich : Pointer auf Pointer auf short int
*                   Bedeutung    : Anfang des Kommandostrings
*
*                 end       :
*                   Typ          : short int **
*                   Wertebereich : Pointer auf Pointer auf short int
*                   Bedeutung    : Ende des Kommandostrings
*
*
*  Beschreibung: Es wird ein Kommandostring eingelesen, der mit RETURN ter-
*                miniert sein muss. Die maximale Laenge ist hierbei durch
*                MAX_MACRO_LEN vorgegeben. Anschliessend wird der eingele-
*                sene String mittels reallocmem und memcpy gesichert und
*                in *anf und *end Anfang bzw. Ende des Strings eingetragen.
*                Moechte man in den Kommandostring ein RETURN einfuegen, so
*                kann man dies mittels der Kombination CTRL-c RETURN, ein
*                CTRL-c laesst sicht durch CTRL-c CTRL-c einfuegen.
*                Hat der Eingabestring (ohne RETURN) die Laenge 0, so wird
*                ein NULL-Pointer zurueckgeliefert.
*
***************************************************************************/

void get_comstr(anf,end)
short int **anf,**end;
{
  /* *** interne Daten und Initialisierung *** */
  register short int bef, /* Zum Einlesen einer Tastenkombination     */
		     h;   /* Zum Einlesen des Macrozeichens           */
  short int          mbuff[MAX_MACRO_LEN+1];     /* Eingabepuffer     */
  register int       i=0, /* Index in Eingabepuffer mbuff             */
		     i2;  /* Zwischenspeicher fuer Index bzgl. Macros */

  print_stat(PROMPT_COMMAND);
  while(funktion[bef = taste(status)] != do_newline) /* code fuer newline ist ende */
  {
    if(i + index > MAX_MACRO_LEN) /* Noch Platz ? */
    {
      clear_stat();
      beep();
      pe_or(PROMPT_KEYLONG);
    }
    else                          /* Es war noch Platz */
      if(funktion[bef] == do_control) /* controlcode einfuegen? */
	mbuff[i++] = newwgetch(status); /* dann ein Zeichen lesen und rein */
      else
	if(funktion[bef] == do_macro && i+index+1 < MAX_MACRO_LEN)  /* Macro-Befehl? */
	{ /* zuerst Befehl sichern */
	  memcpy(&mbuff[i],eingabe,index * sizeof(short int));
	  i += index;
	  mbuff[i++] = newwgetch(status); /* Macrozeichen lesen und einfuegen */
	}
	else  /* Kein Macrobefehl */
	{     /* Befehlskombination (eingabe) in Puffer kopieren */
	  memcpy(&mbuff[i],eingabe,index * sizeof(short int));
	  i += index;
	}
    clear_stat();
    print_stat(PROMPT_COMMAND);
  }
  clear_stat();
  if(!i)  /* Eingabelaenge NULL?*/
    *anf = *end = NULL;
  else
  {
    *anf = (short int *)reserve_mem(i * sizeof(short int));
    *end = &(*anf)[i-1];  /* Zeiger auf Anfang und Ende setzen */
    memcpy(*anf,mbuff,i * sizeof(short int)); /* Kommandostring kopieren */
  }
}
