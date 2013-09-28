/****************************************************************/
/*                                                              */
/*      MODUL:  main.c                                          */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - pe_or (Fehlertext ausgeben ohne refresh)      */
/*              - print_err (Fehlertext ausgeben)               */
/*              - wart_ret (warte auf RETURN)                   */
/*              - no_mem_err (Fehlermeldung, wenn kein Speicher)*/
/*              - reserve_mem (Speicher reservieren)            */
/*              - save_text (Textzeile sichern)                 */
/*              - line_free (Zeile freigeben)                   */
/*              - revcpy (String von hinten kopieren)           */
/*              - fwdcpy (String von vorne kopieren)            */
/*              - swap_int (zwei integer vertauschen)           */
/*              - ja_nein (Lese JA/NEIN Abfrage)                */
/*              - set_lines_cols (Zeilen- und Spaltenz.ermitteln*/
/*              - check_env (Environment checken)               */
/*              - catchsig (Signal abfangen)                    */
/*              - init (initialisieren)                         */
/*              - ende (Editor beenden)                         */
/*              - save_all (alle geaenderten Texte abspeichern) */
/*              - alles_frei (alle Speicherbereiche freigeben)  */
/*              - do_wildcard_expansion (Wildcards expandieren) */
/*              - hndl_params (Parameter verarbeiten)           */
/*              - main (Hauptprogramm)                          */
/*                                                              */
/****************************************************************/

#define aktdef  /* Damit in defs.h nicht lokales als extern deklariert wird */

#ifdef OS2
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include <os2.h>
#else
#pragma inline
#include <dir.h>
#endif
#include "defs.h"
#include "regex.h"
#include <signal.h>

extern char *getenv(),*sd_line,helpflag;
#undef getch()
extern int read_config();
#ifdef MOUSE
extern int mouse_jn; /* Variable fÅr Maus in ja_nein */
#ifdef OS2
extern char mouse_active;
extern short int mouse_handle;
extern TID mouse_ThreadID, mouse_jn_ThreadID;
#endif /* OS2 */
#endif /* MOUSE */

void wart_ret(), ende(int, char);

/* *** globale Daten und Initialisierung *** */
win_typ    *akt_winp;            /* Aktuelles Fenster                     */
WINDOW     *status;              /* Statusfenster                         */
short int  aktcode;              /* Aktueller Befehlstastencode           */
short int  *keystack,            /* Zeiger auf aktuelles Pufferzeichen    */
	   *e_keystack,          /* Zeiger auf Pufferende                 */
	   mc_index;             /* Anzahl der Tastenkombinationen-1      */
int        ks_index = -1;        /* Index des aktuellen Puffers           */
int        blockattr,            /* Attribut zum Highlighten eines Blocks */
	   ersetzaddr,           /* Attribut fuer zu ersetzenden String   */
	   def_tab = STD_TAB;    /* Laenge eines Tabulatorsprungs         */
char       highblockflag = TRUE, /* Sollen Bloecke gehighlighted werden ? */
	   backupflag=TRUE,      /* Es sollen .bak-Files erzeugt werden   */
	   regexflag=TRUE,       /* Es soll Patternmatching angew. werden */
	   def_aiflag = FALSE;   /* Normalerweise kein Autoindent         */
char       *conffile = PF_CONFIG,/* Pfad der Config-Datei                 */
	   *loadfile = PF_LOAD;  /* Pfad des Loadfiles                    */
block_typ  global_block;         /* Paste-Puffer fuer globalen Block      */
marker_typ marker[ANZ_MARKER];   /* Feld fuer alle Marker                 */
puff_typ   macro[ANZ_MACROS];    /* Feld fuer alle Macros                 */
char       clear_buff = FALSE,   /* soll Puffer bei naechsten newwgetch() */
				 /* geloescht werden? */
	   *tasten_inf = PF_TASTEN; /* Name des Tastenbelegungsfiles      */
#ifdef OS2
HMTX       sem_handle;           /* Handle fÅr Semaphor, der Zusammelspiel*/
#else                            /* von Tastatur uns Maus synchronisiert  */
long       old_int;              /* Zwischenspeicher Vektor des Break-Ints*/
#endif

/*****************************************************************************
*
*  Funktion       Fehlertext ausgeben ohne setz_cursor() (pe_or)
*  --------
*
*  Parameter    : fehlertext:
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Auszugebender Fehlertext
*
*  Beschreibung : Der Fehlertext wird mit print_stat() ausgegeben, anschlie-
*                 ssend wird ein wart_ret ausgefuehrt, dann wird das Status-
*                 fenster geloescht.
*
*****************************************************************************/

void pe_or(fehlertext)
char *fehlertext;
{
  print_stat(fehlertext);  /* Text im Statusfenster anzeigen */
  wart_ret();              /* Auf RETURN warten              */
  clear_stat();            /* Statuszeile wieder loeschen    */
}

/*****************************************************************************
*
*  Funktion       Fehlertext ausgeben (print_err)
*  --------
*
*  Parameter    : fehlertext:
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Auszugebender Fehlertext
*
*  Beschreibung : Der Fehlertext wird mit pe_or ausgegeben, anschliessend
*                 wird der Cursor wieder positioniert.
*
*****************************************************************************/

void print_err(fehlertext)
char *fehlertext;
{
  pe_or(fehlertext);    /* Fehlermeldung ausgeben und aus RETURN warten */
  if(akt_winp->winp)    /* nur wenn schon ein window angefordert wurde  */
    setz_cursor(W_AKT);      /* s. koppel_win() */
}

/*****************************************************************************
*
*  Funktion       warte auf RETURN (wart_ret)
*  --------
*
*  Beschreibung : Der Text " Bitte RETURN druecken..." wird in der
*                 Statuszeile ausgegeben, anschliessend wird auf RETURN
*                 gewartet.
*
*****************************************************************************/

void wart_ret()
{
  /* *** interne Daten *** */
  char dummy; /* "String", der mit read_stat eingelesen wird */

  print_stat (PROMPT_ENTER);
  read_stat(&dummy,0,GS_ANY); /* String der Laenge 0 einlesen */
  clear_stat();               /* Statuszeile wieder loeschen  */
}

/*****************************************************************************
*
*  Funktion       "Kein Speicher" - Fehlermeldung (no_mem_err)
*  --------
*
*  Beschreibung : Eine Fehlermeldung wird ausgegeben und nach einer Rueck-
*                 frage die Dateien gespeichert und das Programm beendet.
*
*****************************************************************************/

void no_mem_err()
{
  win_typ *w = akt_winp->next->next; /* Zum Freigeben der Fenster */

  while(w!=akt_winp)  /* Alle WINDOW-Strukturen aller Fenster freigeben */
  {                   /* um Speicherplatz zu gewinnen */
    delwin(w->winp); /* Damit Fenster nicht nochmals von delwin freigegeben */
    w->winp = NULL;  /* wird, Pointer auf NULL setzen */
    w = w->next;     /* Zum nÑchsten Fenster */
  }
  clear_stat();
  print_stat(PROMPT_OUTOFMEM);
  if(ja_nein(PROMPT_SAVE)) /* Abfrage */
    save_all();                  /* Alle geaenderten Dateien sichern */
  ende(1, TRUE);  /* Editor beenden, Rueckgabewert 1 */
}

/*****************************************************************************
*
*  Funktion       Speicher reservieren (reserve_mem)
*  --------
*
*  Parameter    : laenge    :
*                   Typ          : int
*                   Wertebereich : 0 - MAX_INT
*                   Bedeutung    : Laenge des zu reservierenden Speicher-
*                                  bereiches in Bytes
*
*  Ergebnis     :
*                   Typ          : char*
*                   Wertebereich : Pointer auf Speicherbereich
*                   Bedeutung    : Pointer auf reservierten Speicherbereich
*
*  Beschreibung : Es wird versucht, ueber malloc den angeforderten Speicher-
*                 bereich zu allozieren. Falls dies fehlschlaegt, wird
*                 eine Fehlermeldung ausgegeben, alle Files gesichert und
*                 das Programm abgebrochen.
*
*****************************************************************************/

char *reserve_mem (laenge)
int laenge;
{
  /* *** interne Daten *** */
  register char *hilf; /* Rueckgabewert der Funktion */

  if (hilf = malloc (laenge)) /* Speicher anfordern */
    return (hilf); /* klappte, Adresse zurueckgeben */
  else             /* Sonst Fehlermeldung und Ende  */
    no_mem_err();
}

/*****************************************************************************
*
*  Funktion       Textzeile sichern (save_text)
*  --------
*
*  Parameter    : txt       :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Zu sichernder String
*
*  Ergebnis     :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Pointer auf gesicherten String
*
*  Beschreibung : Fuer den uebergebenen String wird, falls der String
*                 eine Laenge ungleich 0 hat und txt != NULL ist, Speicher
*                 alloziert, der uebergebene String wird dorthin kopiert
*                 und der Pointer auf den Bereich zurueckgegeben.
*                 Ist die Stringlaenge 0 oder txt NULL, so wird ein
*                 NULL-Pointer zurueckgegeben.
*
*****************************************************************************/

char *save_text(txt)
register char *txt;
{
  /* *** interne Daten *** */
  register int len; /* Laenge des abzuspeichernden Textes */

  if (!txt) /* Falls String leer, NULL-Pointer zurueckgeben */
    return (NULL);
  return ((len=strlen(txt))?strcpy(reserve_mem(len+1),txt):NULL);
}

/*****************************************************************************
*
*  Funktion       Zeile freigeben (line_free)
*  --------
*
*  Parameter    : txt       :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Freizugebende Zeile
*
*  Beschreibung : Falls der Pointer nicht gleich NULL ist, wird der Spei-
*                 cherbereich, auf den er zeigt, freigegeben.
*
*****************************************************************************/

void line_free(txt)
char *txt;
{
  if(txt)
    free(txt);
}

/*****************************************************************************
*
*  Funktion       String von hinten kopieren (revcpy)
*  --------
*
*  Parameter    : s1         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf Speicherbereich
*                   Bedeutung    : Platz, an den zu kopierender String
*                                  kopiert wird
*                 s2         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : zu kopierender String
*
*  Ergebnis     :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : s1
*
*  Beschreibung : s2 wird, von hinten beginnend, nach s1 kopiert.  Dies ist
*                 zum Beispiel nuetzlich, wenn s2 und s1 gemeinsame Speicher-
*                 bereiche haben und s2 nach s1 beginnt.
*
*****************************************************************************/

char *revcpy(s1,s2)
register char *s1,*s2;
{
  /* *** interne Daten und Initialierung *** */
  register char *s2e = s2 + strlen(s2); /* Zeiger auf Stringende */

  for(s1+=s2e-s2;s2e >= s2;*s1-- = *s2e--);
  return(s1);
}

/*****************************************************************************
*
*  Funktion       String vorwaerts kopieren (fwdcpy)
*  --------
*
*  Parameter    : s1         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf Speicherbereich
*                   Bedeutung    : Platz, an den zu kopierender String
*                                  kopiert wird
*                 s2         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : zu kopierender String
*
*  Ergebnis     :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : s1
*
*  Beschreibung : s2 wird, von vorne beginnend, nach s1 kopiert.
*                 Diese Funktion wird bei ueberlappenden Strings benoetigt,
*                 da strcpy keine Garantie fuer die Kopierrichtung gibt.
*
*****************************************************************************/

char *fwdcpy(s1,s2)
register char *s1,*s2;
{
  if(!s2)                       /* wie strcpy */
  {
    if(s1)
      *s1 = '\0';
  }
  else
    while (*s1++ = *s2++);
  return(s1);
}

/*****************************************************************************
*
*  Funktion       Zwei Integer vertauschen (swap_int)
*  --------
*
*  Parameter    : a         :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer
*                   Bedeutung    : zu vertauschender Integer
*
*               : b         :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer
*                   Bedeutung    : zu vertauschender Integer
*
*  Beschreibung : Die Integer, auf die die Pointer a und b zeigen, werden
*                 mittels einer Hilfsvariable vertauscht.
*
******************************************************************************/

void swap_int(a,b)
int *a,*b;
{
  /* *** interne Daten *** */
  int hilf; /* Hilfsvariable zum Vertauschen */

  hilf = *a;
  *a = *b;
  *b = hilf;
}

/*****************************************************************************
*
*  Funktion       Lese JA/NEIN Abfrage (ja_nein)
*  --------
*
*  Parameter    : s         :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Text der Abfrage
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : Ergebnis der Abfrage: ja = TRUE
*                                                        nein = FALSE
*
*  Beschreibung : s wird in der Statuszeile ausgegeben, danach ein Zeichen,
*                 terminiert durch RETURN eingelesen. Ist das Zeichen nicht
*                 in "JjNn" wird die Abfrage wiederholt.  Es wird FALSE
*                 zurueckgegeben, falls das Zeichen 'n' oder 'N' war,
*                 ansonsten ist der Returnwert TRUE.
*
*****************************************************************************/

int ja_nein (s)
char *s;
{
  /* *** interne Daten *** */
  short int jn; /* einzulesendes Zeichen */

  print_stat (s);           /* Uebergebenen String ausgeben   */
  print_stat(PROMPT_YES_NO);  /* Zusatz anhaengen               */
#ifdef MOUSE
  mouse_jn_init(TRUE);      /* Maus fÅr ja_nein aktivieren    */
#endif
  nodelay(status,TRUE); /* Fenster wg. Maus auf NODELAY schalten */
  do                        /* Solange ein Zeichen lesen, bis */
  { jn = newwgetch(status); /* es eins von j,J,y,Y,n,N ist        */
#ifdef OS2
    if (jn == -1)
      DosSleep (10);
#endif
  } while (!strchr ("jny,YJN",(char)jn)
#ifdef MOUSE
	 && mouse_jn == NO_KLICK
#endif
				);
  nodelay(status,FALSE);  /* Wieder DELAY einschalten */
#ifdef MOUSE
  mouse_jn_init(FALSE);     /* Maus fÅr ja_nein deaktivieren  */
#endif
  clear_stat();             /* Statuszeile wieder loeschen    */

  if (jn == 'n' || jn == 'N'
#ifdef MOUSE
 || mouse_jn == KLICK_RIGHT
#endif
			   )
    return (FALSE);
  return (TRUE);
}

#ifdef OWN_CURSES
/******************************************************************************
*
* Funktion     : Zeilen- und Spaltenzahl ermitteln (set_lines_cols)
* --------------
*
* Parameter    : modus       :
*                  Typ          : char *
*                  Wertebereich : Pointer auf ASCII-Zeichenkette
*                  Bedeutung    : Aus der Umgebungsvariablen EHPINIT
*                                 extrahierter Teil
*
* Beschreibung : Der aus der Umgebungsvariable stammende Teil hinter
*                dem Bezeichner "screen" mu· die Syntax <Spalten>x<Zeilen>
*                haben. Das 'x' darf auch groa geschrieben sein.
*                Ist der Åbergebene String syntaktisch korrekt so werden die
*                Curses-Variablen LINES und COLS entsprechend gesetzt.
*
******************************************************************************/

void set_lines_cols(modus)
char *modus;
{
  char *zeil_str; /* Zeiger auf Tokenteil, der Zeilenzahl angibt */
  int  lines,     /* Anzahl der Bildschirmzeilen */
       cols;      /* Anzahl der Bildschirmspalten */

  if(!(zeil_str = strchr(modus,'x')) && !(zeil_str = strchr(modus,'X')))
    return;  /* Falsches Format, Bildschirmgrî·e wird nicht verÑndert */
  *zeil_str++ = '\0'; /* 'x' bzw. 'X' lîschen, String in 2 Teile teilen */
  cols = atoi(modus); /* Erste Zahl steht fÅr die Spaltenangabe */
  if(cols != 40 && cols != 80 && cols != 100 && cols != 132) /* Es werden nur die */
    return;            /* Spaltenzahlen 40, 80, 100 und 132 unterstÅtzt */
  lines = atoi(zeil_str); /* Zweite Zahl des Strings steht fÅr Zeilenangabe */
  LINES = lines; /* Curses-Variablen LINES und COLS setzen */
  COLS  = cols;
}
#endif

/*****************************************************************************
*
*  Funktion       Environment checken (check_env)
*  --------
*
*  Beschreibung : Die Umgebung wird nach der Variablen EHPINIT abgesucht.
*                 Wird sie gefunden, so wird der Inhalt analysiert.
*                 Folgende Token haben folgenden Effekt:
*                   deftab <n>   : Default-Tablaenge auf <n> setzen
*                   nohelp       : Hilfstexte defaultmaessig ausgeschaltet
*                   autoind      : Autoindentmodus defaultmaessig eingesch.
*                   noshowblock  : Blockhervorhebung defaultmaessig eing.
*                   conffile <f> : Name des Config-Files auf <f> setzen
*                   loadfile <f> : Name des Load-Files auf <f> setzen
*                   nobak        : Defaultmaessig keine .bak-Files
*                   screen <mode>: Bildschirmgrî·e in Zeilen und Spalten
*                   keys <f>     : Name des Tastaturbeschreibungsfiles = <f>
*
****************************************************************************/

void check_env()
{
  /* *** interne Daten *** */ 
  char *ehp_init, /* Zeiger auf Inhalt der Umgebungsvariable     */
       *konv,     /* Zeiger zur Konvertierung in Kleinbuchstaben */
       *token;    /* Zeiger auf aktuellen Parameter              */
  int  i;         /* Integerwert der Tablaenge                   */

  if(ehp_init = getenv("EHPINIT"))
    while(token = strtok(ehp_init," "))
    {
      ehp_init = NULL;  /* fuer strtok */
      for (konv = token; *konv; konv++)
	*konv = tolower (*konv); /* Token in Kleinbuchstaben konvertieren */
      if(!strcmp("deftab",token))
      {
	if(!(token = strtok(ehp_init," ")))
	  break;        /* war letzter token */
	if((i = atoi(token)) > 0 && i < MAXLENGTH)
	{
	  def_tab = i;
	  continue;
	}
      }
      if(!strcmp("nohelp",token))
	helpflag = FALSE;
      else
	if(!strcmp("autoind",token))
	  def_aiflag = TRUE;
	else
	  if(!strcmp("noshowblock",token))
	    highblockflag = FALSE;
	  else
	    if(!strcmp("conffile",token))
	    {
	      if(!(token = strtok(ehp_init," ")))
		break;        /* war letzter token */
	      conffile = token;
	    }
	    else
	      if(!strcmp("loadfile",token))
	      {
		if(!(token = strtok(ehp_init," ")))
		  break;        /* war letzter token */
		loadfile = token;
	      }
	      else
		if(!strcmp("nobak",token))
		  backupflag = FALSE;

#ifdef OWN_CURSES /* Bildschirmgrî·e lÑ·t sich nur mit dem eigenen Curses
		     anpassen ! */
		else
		  if(!strcmp("screen",token))
		  {
		    if(!(token = strtok(ehp_init," ")))
		      break;        /* war letzter token */
		    set_lines_cols(token); /* Bildschirmgrî·e setzen */
		  }
#endif
		  else
		    if(!strcmp("keys", token))
		    {
		      if(!(token = strtok(ehp_init," ")))
			break;      /* war letzer Token */
		      tasten_inf = strcpy(reserve_mem (strlen(token)),token);
		    }
		    else
		      if(!strcmp("noregex", token))
			regexflag = FALSE;
    }
}

/******************************************************************************
*
* Funktion     : Signal abfangen (catchsig)
* --------------
*
* Beschreibung : Diese Funktion wird beim Auftreten des Break-Interruptes
*                (Nummer 27) aufgerufen. Es wird das clear_buff-Flag gesetzt,
*                wodurch beim nÑchsten Aufruf von newwgetch alle Puffer und
*                Macros abgebrochen werden.
*
******************************************************************************/

void catchsig(sig)
int sig;
{
  /* Um ein waehrend der Abarbeitung dieser Routine auftretendes */
  /* Signal abzufangen, wird das Signal auf SIG_IGN umgeleitet   */

  signal(sig,SIG_IGN);
#ifdef OS2
  if(sig == SIGINT || sig == SIGQUIT || sig == SIGBREAK)
#else
  if(sig == SIGINT || sig == SIGTERM)
#endif
    clear_buff = TRUE;  /* Macro- und Repeatpuffer loeschen */
  else
  {
    if(ja_nein(PROMPT_FATAL))
      save_all();
    ende(sig, TRUE);     /* Speicherbereiche werden automat. v. Unix freigegeben */
  }
  signal(sig, catchsig); /* Adresse dieser Funktion wieder eintragen */
}

/*****************************************************************************
*
*  Funktion       initialisieren (init)
*  --------
*
*  Beschreibung : Die CURSES-Funktionen initscr(),noecho() und raw() werden
*                 aufgerufen, um CURSES zu initialisieren, echo abzuschalten,
*                 und die Abfrage von einzelnen Zeichen zu ermoeglichen.
*                 Dann wird das Dummyelement der Windowliste initialisiert.
*                 Es ist _w_i_c_h_t_i_g, daa check_env von initscr aufgerufen wird.
*                 In check_env werden nÑmlich evtl. die Variablen LINES
*                 und COLS angepaat, was vor der Bildschirminitalisierung
*                 stattfinden mua. Ebenfalls in initscr kînnen sich diese
*                 beiden Werte nochmals Ñndern.
*
*****************************************************************************/

void init()
{
  /* *** interne Daten *** */
  int i;    /* Fuer Eingabe der Terminaltypen */

  /* Pruefen, ob IO des Prozesses umgelenkt wurde */
  if(!isatty(0) || !isatty(1) || !isatty(2))
  {
    fputs(PROMPT_IO_REDIR,stderr);
    exit(1);
  }

#ifdef OS2
  set_os2_raw (TRUE);       /* !!! mu· erste Aktion sein !!! */
#endif
  check_env();              /* Umgebungsvariable bearbeiten */
  initscr();                /* Curses "einschalten" */
  noecho();
  raw(); /* Zeichen direkt, keine Signale verarbeiten */
  nonl(); /* \r soll nicht in \n umgewandelt werden */
  status = newwin(1,COLS,LINES-1,0); /* Statusfenster oeffnen */
  lies_tasten();        /* Tastaturbelegung laden */
  akt_winp = (win_typ*) reserve_mem (sizeof (win_typ)); /* Dummyfenster */
  akt_winp->next = akt_winp->prev = akt_winp;           /* allozieren   */
  akt_winp->filename = NULL; /* damit print_err testen kann, ob es */
  akt_winp->winp = NULL;     /* schon ein Fenster gibt */

  /* Terminal stellt zu ersetzende Begriffe blinkend dar, */
  /* Bloecke halbhell.                                    */
  ersetzaddr = A_BLINK;
  blockattr = A_BOLD;

  global_block.e_line = global_block.s_line = -1;
  global_block.bstart = NULL;   /* PASTE-Puffer leer */
  for(i=0;i<ANZ_MARKER;i++)     /* Marker loeschen   */
    marker[i].window = marker[i].line = marker[i].col = -1;
  for(i=0;i<ANZ_MACROS;i++)     /* Macros loeschen   */
    macro[i].begin = macro[i].end = NULL;
#ifdef MOUSE
  init_mouse();
#endif

  signal (SIGINT, catchsig); /* kommt wohl nicht durch, da CTRL-c im raw- */
#ifdef OS2
  signal (SIGBUS, catchsig); /* Modus nicht als Signal verarbeitet wird.  */
  signal (SIGQUIT, catchsig);
  signal (SIGBREAK, catchsig);
  /* Jetzt mu· der Semaphor erzeugt werden, der verhindert, da· sich von der
     Maus und von der Tastatur empfangene Kommandos behindern. Dieser Sema-
     phor wird als "owned" geîffnet, mu· also erst "released" werden, damit
     ein Kommando akzeptiert werden kann. */
  DosCreateMutexSem (NULL, &sem_handle, DC_SEM_SHARED, TRUE);
#else
  signal (SIGABRT, catchsig);
  signal (SIGTERM, catchsig);
  signal (SIGSEGV, catchsig);
#endif
/*  re_set_syntax (RE_INTERVALS); */
}

/*****************************************************************************
*
*  Funktion       editor beenden (ende)
*  --------
*
*  Parameter    : r         :
*                   Typ          : int
*                   Wertebereich : -MAX_INT - +MAX_INT
*                   Bedeutung    : Rueckgabewert des Programms
*
*                 wait_mouse: (Nur OS/2)
*                   Typ          : char
*                   Wertebereich : FALSE, TRUE
*                   Bedeutung    : TRUE:  Warte auf die Beendigung des
*                                         Mausthreads
*                                  FALSE: Nicht auf Mausthread warten.
*
*  Beschreibung : Die Fenster werden mittels endwin() geschlossen und danach
*                 exit aufgerufen.
*
*****************************************************************************/

void ende(r, wait_mouse)
int r;
char wait_mouse;
{
  clear();      /* Bildschirm loeschen */
  refresh();
  endwin();     /* Curses beenden */
#ifdef MOUSE
#ifdef OS2
  mouse_active = FALSE;  /* Maus nicht mehr benîtigt */
  /* Durch Setzen von mouse_active=FALSE beendet sich der Mouse-Thread
     selbstÑndig und die Maus wird geschlossen. Hier mu· dann nur noch
     auf die Terminierung des Threads gewartet werden. */
  if (wait_mouse)
  {
    DosWaitThread (&mouse_ThreadID, DCWW_WAIT);
    if (mouse_jn_ThreadID) /* Wurde schon einmal ja_nein aufgerufen? */
      DosWaitThread (&mouse_jn_ThreadID, DCWW_WAIT);
  }
#else
  set_mouse_int(0); /* Mausroutine maskieren */
#endif
#endif
#ifndef OS2
  *(long*)(27*4) = old_int; /* Vektor des Break-Interruptes restaurieren */
#endif
  exit(r);
}


/*****************************************************************************
*
*  Funktion       pruefen, ob Texte geaendert wurden
*  --------
*
*  Ergebnis     : TRUE: Es wurde mindestens eine Datei geaendert
*                 FALSE: Alle Dateien sind ungeaendert
*
*  Beschreibung : Es wird ueber alle bestehenden Fenster iteriert und
*                 das changeflag geprueft. Ist eines TRUE, so wird
*                 sofort mit TRUE zurueckgekehrt.
*
*****************************************************************************/

int did_anything_change()
{
  win_typ *p = akt_winp->next->next;

  while (p != akt_winp->next)
  {
    if (p->changeflag)
      return TRUE;
    p = p->next;
  }
  return FALSE;
}

/*****************************************************************************
*
*  Funktion       alle geaenderten Texte abspeichern (save_all)
*  --------
*
*  Ergebnis     : TRUE: alle Dateien konnten abgespeichert werden.
*                 FALSE: nicht alle Dateien ...
*
*  Beschreibung : Die zu den Fenstern gehoerigen Files werden abgespeichert,
*                 sofern sie veraendert wurden.
*
*****************************************************************************/

int save_all()
{
  /* *** interne Daten und Initialisierung *** */
  int     no_errflag = TRUE;   /* Zeigt Fehler beim Abspeichern an */
  win_typ *d = akt_winp->next; /* Zeiger auf Dummy-Fenster         */

  check_buff();
  akt_winp = d->next;
  while (akt_winp != d)
  {
    if(akt_winp->changeflag && !schreib_file())
      no_errflag = FALSE; /* schreib_file lieferte Fehler */
    akt_winp = akt_winp->next;
  }
  akt_winp = d->prev; /* akt_winp restaurieren */
  return(no_errflag);
}

/*****************************************************************************
*
*  Funktion       Text und Fenster freigeben (alles_frei)
*  --------
*
*  Beschreibung : Der fuer Texte und Fenster allozierte Speicherplatz wird
*                 freigegeben.
*
*****************************************************************************/

void alles_frei()
{
  while(akt_winp != akt_winp->next)     /* noch ein Fenster da? */
  {
    free_text();  /* Text des Fensters freigeben */
    delwin(akt_winp->winp); /* Fenster fuer Curses freigeben */
    gb_win_frei(); /* gb_win_frei loescht Fenster aus Liste und */
  }                /* macht akt_winp zu ->prev */
  if(global_block.bstart)     /* globalen Block evtl. freigeben */
    block_free(global_block.bstart);
  line_free(sd_line);      /* zuletzt geloeschte Zeile freigeben */
  free(akt_winp);          /* dummy auch freigeben */
}

#ifndef OS2
/*****************************************************************************
*
*  Funktion       Wildcards expandieren (do_wildcard_expansion)
*  --------
*
*  Parameter    : argc         :
*                   Typ          : int *
*                   Wertebereich : Zeiger auf Integer
*                   Bedeutung    : Zeiger auf bisherige Anzahl der Parameter
*
*                 argv         :
*                   Typ          : char **[]
*                   Wertebereich : Pointer auf Pointer auf Array von Pointer
*                                  auf Parametern
*                   Bedeutung    : 1. Parameter: Programmname
*                                  restliche Parameter: zu expandierende
*                                  Dateinamen
*
*  Beschreibung : Mittels der Funktionen findfirst und findnext werden
*                 die als Parameter angegebenen Dateinamenmuster zu
*                 vollstÑndigen Dateinamen expandiert. argc aud argv
*                 werden auf das entstehende Erbebnis gesetzt, das keine
*                 Wildcards mehr enthÑlt.
*
*****************************************************************************/

void do_wildcard_expansion(argc, argv)
int *argc;
char **argv[];
{
  int ende,               /* Kein weiteres File zum Pattern ?        */
      complete_length,    /* Summe aller StringlÑngen                */
      new_argc,           /* Neue Parameteranzahl                    */
      new_argv_index,     /* Index in neues Argumentarray            */
      argv_index;         /* Index in argv, zÑhlt die Dateiparameter */
  struct ffblk file_info; /* Struktur fÅr findfirst und findnext     */
  struct filenamelist     /* FÅr die Konstruktion der neuen Liste    */
  {
    char                *name;
    struct filenamelist *next;
  } *p, *q, *first;       /* Laufzeiger und Listenstart              */
  char **new_argv;        /* Neues Feld fÅr Parameter                */

  if (*argc > 1) /* Nur etwas machen, wenn auch Parameter angegeben wurden */
  {
    /* ZunÑchst ein Dummy-Element fÅr die Namenliste anlegen: */
    first = p = (struct filenamelist *) reserve_mem (sizeof (struct filenamelist));
    p->next = NULL;
    argv_index = 0;
    new_argc = 1; /* Zumindest der Kommandoname */
    complete_length = strlen ((*argv) [0]); /* LÑnge des Kommandos ! */
    while (++argv_index < *argc)
    {
      ende = findfirst ((*argv) [argv_index], &file_info, 0);
      if (ende)
      { /* öberhaupt keinen passenden Namen, dann neue Datei! */
	p->next =
	  (struct filenamelist *) reserve_mem (sizeof (struct filenamelist));
	p = p->next;
	p->next = NULL;
	p->name = strcpy ((char*) reserve_mem (strlen ((*argv) [argv_index])),
			  (*argv) [argv_index]);
	complete_length += strlen ((*argv) [argv_index]) + 1;
	new_argc++;
      }
      else
	while (!ende)
	{
	  p->next = 
	    (struct filenamelist *) reserve_mem (sizeof (struct filenamelist));
	  p = p->next;
	  p->next = NULL;
	  p->name = strcpy ((char*) reserve_mem(strlen (file_info.ff_name)+1),
			    file_info.ff_name);
	  complete_length += strlen (file_info.ff_name) + 1;
	  new_argc++;
	  ende = findnext (&file_info);
	}
    }

    /* Jetzt die aufgebaute Liste in ein Array vom Typ char *[] verwandeln */
    /* Achtung: Letzter Pointer mu· NULL-Pointer sein! */
    new_argv = (char **) reserve_mem ((new_argc+1) * sizeof (char*));
    new_argv[0] = (*argv)[0];
    new_argv_index = 1;
    p = first->next;
    while (p)
    {
      new_argv [new_argv_index++] = p->name;
      p = p->next;
    }
    new_argv [new_argv_index] = NULL; /* Am Ende der Argumentliste mu· ein NULL-Pointer stehen */
    *argc = new_argc;
    *argv = new_argv;
    /* Jetzt temporÑre Liste freigeben! */
    p = first;
    while (p)
    {
      q = p->next;
      free (p);
      p = q;
    }
  }
}
#endif



/*****************************************************************************
*
*  Funktion       parameter verarbeiten (hndl_params)
*  --------
*
*  Parameter    : argc         :
*                   Typ          : int
*                   Wertebereich : 1 - maxint
*                   Bedeutung    : Anzahl der Parameter
*
*                 argv         :
*                   Typ          : char **
*                   Wertebereich : Pointer auf Array von Pointer auf
*                                  Parametern
*                   Bedeutung    : 1. Parameter: Programmname
*                                  restliche Parameter: zu edierende Dateien
*
*                 anz_files    :
*                   Typ          : int
*                   Wertebereich : 0 - maxint
*                   Bedeutung    : Anzahl der schon eingelesenen Dateien
*
*  Beschreibung : Die Kommandozeilenparameter werden als Filenamen interpre-
*                 tiert und in den Editor geladen.
*                 Konnten keine Files geladen werden, so wird das Programm
*                 abgebrochen.
*
*****************************************************************************/

void hndl_params(argc,argv,anz_files)
int argc,anz_files;
char **argv;
{
#ifdef OS2                  /* OS/2 stellt _wildcard zur VerfÅgung, */
  _wildcard (&argc, &argv); /* um Patterns zu expandieren */
#else
  do_wildcard_expansion (&argc, &argv);
#endif
  if(argc>1)   /* Wurden Parameter angegeben ? */
  {
    while(--argc) /* Fuer jeden Parameter ein Fenster oeffnen */
    {
      if (koppel_win())
      {
	akt_winp->filename = save_text(argv[argc]);
	if(!lies_file()) /* und versuchen, die Datei zu laden */
	{
	  free(akt_winp->dummyp);
	  gb_win_frei(); /* klappt das nicht, Fenster wieder schliessen */
	}
	else
	{
	  anz_files++;   /* Sonst Anzahl der geladenen Files erhoehen */
	  open_window(); /* und Fenster initialisieren */
	}
      }
      else /* Konnte Fenster nicht geoeffnet werden, Fehlermeldung */
      {
	print_err(W_COUNT_ERRTEXT);
	break;
      }
    }
    if(!anz_files) /* Wenn keines der angegebenen Files geladen werden  */
    {              /* konnte, Fehlermeldung ausgeben und Editor beenden */
      print_stat(PROMPT_NO_FILE);
      wart_ret();
      ende(0, TRUE);
    }
    setz_cursor(W_AKT);  /* falls letzte Datei nicht geladen werden konnte, */
  }                 /* wird kein setz_cursor gemacht */
  else              /* Wurden keine Parameter angegeben und konnte    */
    if(!anz_files && !laden()) /* kein File geladen werden, versuchen */
      ende(0, TRUE);   /* Dateinamen vom User zu holen. Falls Fehler, Ende */
}


/*****************************************************************************
*
*  Funktion       main (main)
*  --------
*
*  Parameter    : argc         :
*                   Typ          : int
*                   Wertebereich : 1 - maxint
*                   Bedeutung    : Anzahl der Parameter
*
*  Parameter    : argv         :
*                   Typ          : char **
*                   Wertebereich : Pointer auf Array von Pointer auf
*                                  Parametern
*                   Bedeutung    : 1. Parameter: Programmname
*                                  restliche Parameter: zu edierende Dateien
*
*  Beschreibung : main() startet den Editor und enthaelt die Hauptschleife, die
*                 einen Befehl holt und diesen ausfuehrt, bis das Programm
*                 beendet werden soll.
*
*****************************************************************************/

int main (argc,argv)
int argc;
char **argv;
{
  init();  /* Tastaturbelegung laden, Variablen initialisieren, Umgebungs- */
	   /* variable bearbeiten, Signale abfangen, Curses initialisieren */
  hndl_params(argc,argv,read_config(argc)); /* Config-Datei lesen und Para-*/
					    /* meter auswerten             */
  while (1)
    auswertung (aktcode = taste(akt_winp->winp)); /* Tastenkombination lesen */
}                                                 /* und auswerten           */
