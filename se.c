/****************************************************************/
/*                                                              */
/*      MODUL:  se.c                                            */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - ask_replace (fragen, ob ersetzen)             */
/*              - upcase (Buchstabe in Grossbuchstabe wandeln)  */
/*              - instr (String in Text?)                       */
/*              - find (Begriff im Text suchen)                 */
/*              - str_to_num (String in Zahl wandeln)           */
/*              - o_to_vals (Optionen in Werte wandeln)         */
/*              - do_repfr (Latztes Suchen/Ersetzen wiederh.)   */
/*              - do_find (Suche ausfuehren)                    */
/*              - trag_e_ein (Ersetzungsbegriff eintragen)      */
/*              - flash_word (Wort zum Blinken bringen)         */
/*              - unflash_word (Wort "entblinken")              */
/*              - do_replace (Ersetzen ausfuehren)              */
/*              - find_next_par (nÑchste Klammer in Cursorum    */
/*                gebung finden)                                */
/*              - search_match_par (passende Klammer finden)    */
/*                                                              */
/****************************************************************/

#include "defs.h"
#include <ctype.h>
#include "regex.h"

extern char *fastzeichen(int), *fastzeile(zeil_typ*);
extern char space, regexflag;
extern WINDOW *status;
extern int ersetzaddr;

void do_find(), do_replace();

/* *** globale Daten und Initialisierung *** */
static char sbegriff[256],    /* Suchbegriff                                 */
	    ebegriff[256],    /* Ersetzungsbegriff                           */
	    optionen[256];    /* String fuer Optioneneingabe                 */
int         repeatflag=FALSE, /* Ist aktueller Vorgang eine Wiederholung ?   */
	    last_func=F_NONE; /* Art des letzten Vorgangs (Suchen, Ersetzen) */

unsigned char upcase();

/*****************************************************************************
*
*  Funktion       Fragen, ob ersetzt werden soll (ask_replace)
*  --------
*
*  Ergebnis     :
*                   Typ          : int char
*                   Wertebereich : J/N/A
*                   Bedeutung    : Eingelesenes Zeichen (upcase)
*
*  Beschreibung : Es wird ein Prompt im Statusfenster ausgegeben, das
*                 der User durch Eingabe eines Zeichens beantworten kann.
*                 Es sind nur die Zeichen j,J,y,Y,n,N,a,A zugelassen. Der
*                 Rueckgabewert ist das Zeichen, jedoch auf jeden Fall
*                 als Grossbuchstabe. (y,Y werden nach j,J konvertiert)
*
*****************************************************************************/

int ask_replace()
{
  /* *** interne Daten *** */
  int antw; /* zum Einlesen eines Zeichens */

  print_stat(PROMPT_REPLYN);
  do
    antw = newwgetch(status); /* Zeichen vom Tastatur / Macro lesen */
  while (!strchr ("jnJNyYAa",antw));
  clear_stat();
  if (strchr ("yY", antw)) /* Beantwortung mit Y(es) zulassen */
    antw = 'J';
  return(upcase(antw));
}

/*****************************************************************************
*
*  Funktion       Buchstabe in Grossbuchstabe wandeln (upcase)
*  --------
*
*  Parameter    : c         :
*                   Typ          : unsigned char
*                   Wertebereich : (char) 0 - (char) 255
*                   Bedeutung    : Umzuwandelndes Zeichen
*
*  Ergebnis     :
*                   Typ          : unsigned char
*                   Wertebereich : (char) 0 - (char) 255
*                   Bedeutung    : umgewandeltes Zeichen
*
*  Beschreibung : Falls das Zeichen ein Buchstabe ist, wird es in einen
*                 Grossbuchstaben umgewandelt.
*
*****************************************************************************/

unsigned char upcase (c)        /* eee klappt nicht bei Umlauten und anderen
				   fremdsprachigen Zeichen, z.B. Ç und ê */
register unsigned char c;
{
  return (isalpha(c)?toupper(c):c);
}

/*****************************************************************************
*
*  Funktion       Text in String ? (instr)
*  --------
*
*  Parameter    : begriff   :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : String, nach dem gesucht wird
*
*                 zeile     :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Start des Strings, in dem gesucht wird.
*                                  Beachte: Suche beginnt an durch screencol
*                                  bestimmter Position
*
*                 ignore    :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE : Gross-/Kleinschreibung ignorieren
*
*                 word      :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE : Nur ganze Woerter finden
*
*                 richtung  :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE : Vorwaerts suchen
*                                  FALSE: Rueckwaerts suchen
*
*                 pat_buf   :
*                   Typ          : struct re_pattern_buffer *
*                   Wertebereich : Pointer auf regex-Puffer
*                   Bedeutung    : öbersetztes Suchpattern oder NULL,
                                  falls regex nicht verwendet werden soll.
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : LÑnge des gematchten Textes
*                                  0 bedeutet: nicht gefunden
*
*  Beschreibung : Die uebergebene Zeile wird auf den uebergebenen Suchstring
*                 begriff untersucht. Dabei wird, falls das ignore-Flag TRUE
*                 ist, die Gross-/Kleinschreibung ignoriert. In diesem Fall
*                 mu· der Suchbegriff komplett in Gro·buchstaben vorliegen.
*                 Im Fall des
*                 Pattern-Matchings mit regex wird bei ignore-Flag=TRUE
*                 vorausgesetzt, da· im Åbersetzten pat_buf die translate-
*                 table auf eine entsprechende Umsetzung von Klein- auf
*                 Gro·buchstaben eingetragen ist.
*                 Ist das Word-Flag gesetzt, so werden nur Suchbegriffe gefun-
*                 den, die rechts und links entweder Zeilenanfang bzw. Zeilen-
*                 ende oder einen Worttrenner haben.
*                 zeile mu· auf einen String ohne Unterstreichungen zeigen.
*                 akt_winp->screencol wird angepa·t und steht bei einem
*                 Treffer auf dessen Anfang. Wird kein Vorkommen gefunden,
*                 so ist screencol undefiniert, jedoch innerhalb der Zeile.
*
*****************************************************************************/

int instr(begriff,zeile,ignore,word,richtung,pat_buf)
register char *begriff;
char *zeile;
int  ignore,word,richtung;
struct re_pattern_buffer *pat_buf;
{
  /* *** interne Daten und Initialisierung *** */
  register int  beg_sc,               /* Zwischenspeicher Cursorspalte  */
		len,                  /* Laenge des Suchbegriffs        */
		index=0,              /* Index in aktuellen Suchbegriff */
		match_len,            /* LÑnge des Matches (Return-Wert)*/
		match_pos,            /* Position des gefundenen Strings*/
		fr_ret;               /* Rueckgabewert von fastright    */
  struct re_registers regs;           /* Register fÅr Matching mit regu-*/
				      /* lÑren AusdrÅcken               */
  register char *beg_start,           /* Position des Suchbegriffs in   */
				      /* der Zeile (nur bei word)       */
		*in_zeile;            /* Zeiger in String zeile         */

  if (!word)    /* Es muss kein abgeschlossenes Wort gefunden werden */
  {
    if (pat_buf)               /* Matching mit regulÑren AusdrÅcken ? */
    {
      match_pos = re_search (pat_buf, zeile, strlen (zeile),
			     akt_winp->screencol, 
			     richtung ? strlen (zeile)-akt_winp->screencol
				      : -akt_winp->screencol-1,
			     &regs);
      if (match_pos != -1)
      {
	match_len = re_match (pat_buf, zeile, strlen (zeile),
			      match_pos, &regs); /* MusterlÑnge bestimmen */
	akt_winp->screencol = match_pos; /* Position des Matches */
      }
      else
	match_len = 0;
      return match_len;
    }
    else /* Kein Pattern, normaler Text, dann zeichenweise vergleichen */
    {
      in_zeile = & (zeile [akt_winp->screencol]); /* Startposition bestimmen */
      len = strlen (begriff);
      do
      {
	if (ignore? /* Wird Gross-/Kleinschreibung ignoriert, dann upcase */
	(upcase(*in_zeile) != begriff[index++]):(*in_zeile != begriff[index++]))
	{
	  if ((in_zeile -= index-richtung) < zeile) /* Nicht gefunden, ein Zeichen  */
	  {                                     /* weiter rechts/links mit der  */
	    index = 0;                          /* Suche starten. Zeile zuende, */
	    break;                              /* dann do-while abbrechen      */
	  }
	  index = 0;      /* Im Suchbegriff wieder vorne beginnen */
	  if (!richtung)  /* Falls rueckwaerts gesucht wird, nicht mehr nach */
	    continue;     /* rechts, da Position 0 getestet werden muss.     */
	}


	if(!*++in_zeile) /* ein Zeichen nach rechts gehen, wenn am Zeilenende,*/
	  break;         /* dann Schleife abbrechen. */
      } while (index < len); /* Solange suchen, bis Suchbegriff gefunden wurde */
      if (index == len)     /* Wenn Suchbegriff gefunden wurde: */
      {                               /* Zum Anfang des Suchbegriffs gehen */
	/* Falls Suchbegriff am Zeilenende, */
	akt_winp->screencol = (in_zeile-zeile)-len;
	return len;                   /* klappte fastright nicht, also 1 */
      }                               /* weniger nach links */
      else                  /* Wurde Suchbegriff nicht gefunden, NULL zurueck */
	return (0);
    }
  }
  else          /* Es soll ein abgeschlossenes Wort gefunden werden */
  {
    while (1)
    {
      /* Durch rekursiven Aufruf Begriff zunaechst normal in der Zeile */
      /* suchen. Wird er nicht gefunden, return(NULL), sonst testen,   */
      /* ob Wortgrenzen vorhanden                                      */
      if (match_len = instr(begriff,zeile,ignore,FALSE,richtung,pat_buf))
      {
	beg_sc = akt_winp->screencol;
	if (!beg_sc || wortende(zeile [beg_sc-1])) /* Wortende oder */
						   /* Zeilenende links ? */
	  if (!zeile [beg_sc+match_len] || wortende(zeile [beg_sc+match_len]))
	  /* Wenn hinter Suchbegriff Zeilen- oder Wortende */
	    return (match_len);   /* dann gefunden ! */
	/* Es lag kein Wortende vor, Suche ein Z. weiter rechts/links */
	if(richtung)
	{
	  if (!zeile [++akt_winp->screencol]) /* Zeilenende ? */
	    return 0;
	}
	else
	  if (!akt_winp->screencol--)         /* Zeilenanfang ? */
	  {
	    akt_winp->screencol++;  /* Screencol wieder auf Zeilenanfang */
	    return 0;
	  }
      }
      else
	return (0); /* Begriff in aktueller Zeile nicht gefunden  */
    } /* Falls kein Wortende gefunden, nach erneutem Vorkommen suchen */
  }
}

/*****************************************************************************
*
*  Funktion       Begriff im Text suchen (find)
*  --------
*
*  Parameter    : begriff   :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Suchbegriff
*
*                 richtung  :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE : vorwaerts , FALSE : rueckwaerts
*
*                 ignore    :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : Gross-/Kleinschreibung ignorieren
*
*                 word      :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : Nur ganze Woerter finden
*
*                 start_line:
*                   Typ          : int
*                   Wertebereich : 0-MAX_ANZ_LINES
*                   Bedeutung    : Nummer der Zeile, in der die Suche
*                                  gestartet wurde.
*
*                 start_col :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : Nummer der Spalte, in der die Suche
*                                  gestartet wurde.
*
*                 count     :
*                   Typ          : int*
*                   Wertebereich : Pointer auf Integer
*                   Bedeutung    : Wenn die Zeile, in der die Suche begann,
*                                  schon durchsucht wurde, ist *count 0,
*                                  sonst 1.
*
*                 akt_regex :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : Gibt an, ob mit regex oder ohne gesucht
*                                  werden soll.
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : -1-MAXINT
*                   Bedeutung    : LÑnge des gefundenen Matches
*                                  0 bedeutet: Nicht gefunden
*                                 -1 bedeutet: Fehler bei PatternÅbersetzung
*
*  Beschreibung : Es wird ab der aktuellen Position im Text nach dem ueber-
*                 gebenen Begriff gesucht. Dabei werden die Parameter richtung,
*                 ignore und word beruecksichtigt. Kommt man in der Zeile hinter
*                 der aktuellen an und count ist 0, so wird die Suche beendet.
*                 Findet man den Suchbegriff in der Zeile, in der die Suche
*                 gestartet wurde, hinter der Startspalte, so wird 0
*                 zurueckgegeben.
*                 Die Suche innerhalb einer Zeile wird durch die Funktion instr
*                 durchgefuehrt.
*                 Beim ersten Aufruf von find() werden die statischen Kompo-
*                 nenten, die im Zusammenhang mit dem Pattern-Matching mit
*                 regex stehen, initialisiert.
*
******************************************************************************/

int find(begriff,richtung,ignore,word,start_line,start_col,count,
	 akt_regex)
register char *begriff;
int  richtung,ignore,word,start_line,start_col,*count,akt_regex;
{
  /* *** interne Daten und Initialisierung *** */
#define INIT_REGEX_BUF_SIZE 100               /* Initiale Puffergrî·e    */
  char *zeile,                                /* Zeiger auf Puffer, in   */
					      /* dem Inhalt der aktuellen*/
					      /* Zeile ohne Unterstrei-  */
					      /* chung steht.            */
       *begr_p,                               /* Pointer in Suchbegriff  */
       *comp_err=(char*) NULL;                /* Flag, ob beim öbersetzen*/
					      /* des Patterns ein Fehler */
					      /* auftrat                 */
  register int  blen = strlen(begriff),       /* Laenge des Suchbegriffs */
		old_sc = akt_winp->screencol, /* Alte Cursorspalte       */
		old_line=akt_winp->textline,  /* Alte Cursorzeile        */
		nsl=start_line+2*richtung-1;  /* erste Zeile, die nicht  */
					      /* zweimal durchsucht wer- */
					      /* den darf.               */
  int           fl,      /* Laenge der aktuellen Zeile im screencol-Mass */
		match_len, /* LÑnge des aktuellen Matches in der Zeile   */
		i;       /* ZÑhler zur Initialisierung der transl.-Table */
  static char   *old_begriff = NULL;   /* letzter Suchbegriff        */
  static struct re_pattern_buffer buf; /* Puffer fÅr Handling regex  */
  static char   upper_translate [256], /* Translate table upper case */
		fastmap_table   [256]; /* Fastmap fÅr schnelle Suche */

  if (ignore)              /* Gro·/Klein ignorieren? Dann alles gro· */
    for (begr_p = begriff; *begr_p; begr_p++)
      *begr_p = upcase (*begr_p);
  if (akt_regex)
  {
    if (!old_begriff)  /* Initialisierung der static-Komponenten nîtig! */
    {
      old_begriff = strcpy (reserve_mem (strlen (begriff)+1), begriff);
      buf.buffer = reserve_mem (INIT_REGEX_BUF_SIZE);
      buf.allocated = INIT_REGEX_BUF_SIZE;
      buf.fastmap   = fastmap_table;   /* öbersetzungstabelle fÅr öbersetzung */
      for (i=0; i<256; i++)            /* von Klein- auf Gro·buchstaben       */
	upper_translate [i] = upcase(i);  /* initialisieren                   */

      buf.translate = ignore ? upper_translate : (char*) 0;
      comp_err = re_compile_pattern (begriff, strlen (begriff), &buf);
    }
    buf.translate = ignore ? upper_translate : (char*) 0;
    if (strcmp (old_begriff, begriff)) /* Suchbegriff hat sich geÑndert */
    { 
      free (old_begriff);
      old_begriff = strcpy (reserve_mem (strlen (begriff)+1), begriff);
      comp_err = re_compile_pattern (begriff, strlen (begriff), &buf);
    }
    if (comp_err)
    {
      print_err(comp_err);
      /* NeuÅbersetzung beim nÑchsten Aufruf erzwingen: */
      strcpy (old_begriff, ""); /* find() wird nie mit leerem Begriff */
      return -1;                /* aufgerufen! */
    }
  }
  if (!start_line && !richtung)
    nsl=akt_winp->maxline+1;
  else
    if (start_line == akt_winp->maxline && richtung)
      nsl = -1;

  /* Falls Cursor auf oder hinter dem Zeilenende steht und rueckwaerts
     gesucht werden soll, Cursor auf das Zeilende setzen, falls mit
     regulÑren AusdrÅcken gesucht werden soll, da man dann die LÑnge
     des Matches nicht voraussagen kann. Andernfalls Cursor um die
     LÑnge des Suchbegriffs vor das Zeilenende setzen, da das die erste
     mîgliche Trefferposition ist. Bei VorwÑrtssuche und Cursor hinter
     dem Zeilenende Cursor auf Anfang der nÑchsten Zeile setzen. Falls 
     screencol negativ (kann durch rÅckwÑrts Skippen des Suchbegriffs 
     auftreten), dann ans Ende der darÅberliegenden Zeile. */
  /* Unterstreichung beseitigen */
  zeile = fastzeile (akt_winp->alinep);

  if(akt_winp->screencol >= (fl = strlen(zeile)) && !richtung)
  {
    akt_winp->screencol = fl - (akt_regex ? 1 : blen);
    if (akt_winp->screencol < 0) /* in diesem Fall wird zwar kein Match */
      akt_winp->screencol = 0;   /* gefunden, aber es gibt auch keinen Fehler */
  }
  else
    if (richtung && akt_winp->screencol >= strlen (zeile))
    {
      if (!down())
	gotox(0);
      zeile = fastzeile (akt_winp->alinep);
      akt_winp->screencol = 0;
    }
    else
      if(akt_winp->screencol < 0)
      {
	if(!up())
	  gotox(akt_winp->maxline);
	zeile = fastzeile (akt_winp->alinep);
	akt_winp->screencol =  strlen (zeile) - (akt_regex ? 1 : blen);
      }
  do
  {
    /* leere Zeilen werden Åbersprungen */
    if(strlen (zeile))
    { /* Zeile mit der Funktion instr durchsuchen */
      if (match_len=instr(begriff,zeile,ignore,word,richtung,
	  akt_regex ? &buf : (struct re_pattern_buffer *) NULL))
	/* Wenn Teil der Zeile schonmal durchsucht, testen, ob gefundener Teil*/
	/* im schon durchsuchten Teil liegt.                                  */
	if(akt_winp->textline == start_line &&
	(richtung ? akt_winp->screencol >= start_col
		  : akt_winp->screencol <= start_col) && !*count)
	  break; /* Wenn ja, ist Ergebnis ungueltig, abbrechen */
	else
	  return (match_len);   /* Begriff gefunden */
    }
    /* Zur naechsten zu durchsuchenden Zeile gehen */
    akt_winp->alinep = richtung?akt_winp->alinep->next:akt_winp->alinep->prev;
    if((akt_winp->textline += 2*richtung-1) < 0)
      akt_winp->textline = akt_winp->maxline + 1; /* alinep steht auf dummyp */
    else
      if(akt_winp->textline > akt_winp->maxline)
	akt_winp->textline = -1;        /* alinep steht auf dummyp */

    /* Bei vorwaertssuchen ab erster Spalte beginnen, bei Rueckwaertssuchen */
    /* blen Spalten vor Zeilenende, da Suchbegriff nicht spaeter anfangen   */
    /* kann.                                                                */
    zeile = fastzeile (akt_winp->alinep);
    akt_winp->screencol = richtung?0:strlen(zeile)-(akt_regex?1:blen);
  }while(akt_winp->textline != nsl || (*count)--); /* ganzen Text durchlaufen */
  gotox (old_line);             /* falls nicht gefunden, an alte Position */
  akt_winp->screencol = old_sc;
  return (0);
}

/*****************************************************************************
*
*  Funktion       In einem String enthaltene Zahl berechnen (str_to_num)
*  --------
*
*  Parameter    : string    :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : String, in dem die Zahl enthalten ist
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : 0-MAXINT
*                   Bedeutung    : Im String enthaltene Zahl als int
*
*  Beschreibung : Zuerst wird die erste im String vorkommende Ziffer gesucht.
*                 Dann wird die Ziffer in einen Integer umgewandelt. Bei
*                 jeder weiteren gefundenen Ziffer wird der bisherige Wert
*                 mit 10 multipliziert und der Wert der gefundenen Ziffer
*                 addiert. Sobald das Stringende oder keine Ziffer gefuden
*                 wird, bricht die Funktion ab.
*
*****************************************************************************/

int str_to_num(string)
register char *string;
{
  /* *** interne Daten *** */
  register int hilf=0; /* vorlaeufiges Ergebnis */

  while (*string && (*string < '0' || *string > '9'))
    string++;   /* erste Ziffer suchen */
  if (!*string) /* Keine Ziffer gefunden, -1 zurueck */
    return (-1);
  while (isdigit(*string))
    hilf = 10*hilf + *string++ - '0';
  return (hilf);
}

/*****************************************************************************
*
*  Funktion       Optionen in Werte wandeln (o_to_vals)
*  --------
*
*  Parameter    : optionen  :
*                   Typ          : char*
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Auszuwertender String
*
*               : richtung  :
*                   Typ          : int*
*                   Wertebereich : Pointer auf Integer
*                   Bedeutung    : TRUE:vorwaerts, FALSE:rueckwaerts
*
*               : anzahl    :
*                   Typ          : int*
*                   Wertebereich : Pointer auf integer
*                   Bedeutung    : Anzahl der Wiederholungen
*
*               : ignore    :
*                   Typ          : int*
*                   Wertebereich : Pointer auf integer
*                   Bedeutung    : Gross-/Kleinschreibung ignorieren
*
*               : word      :
*                   Typ          : int*
*                   Wertebereich : Pointer auf integer
*                   Bedeutung    : Nur ganze Worte finden
*
*               : anf_ende  :
*                   Typ          : int*
*                   Wertebereich : Pointer auf Integer
*                   Bedeutung    : Soll vom Anfang oder vom Ende gesucht/
*                                  ersetzt werden ?
*
*  Beschreibung : Anhand des Optionenstrings werden die Parameter gesetzt.
*                 anzahl wird ueber str_to_num ermittelt, alle anderen
*                 ueber die Funktion strchr.
*                 Im Sonderfall, da· vom Anfang oder vom Ende gesucht werden
*                 soll, wird anf_ende auf TRUE gesetzt und der interne Cursor
*                 (screencol/textline) wird auf die entsprechende Position
*                 gesetzt.
*
*****************************************************************************/

void o_to_vals(optionen,richtung,anzahl,ignore,word,anf_ende,akt_regex)
char *optionen;
int  *richtung,*anzahl,*ignore,*word,*anf_ende,*akt_regex;
{
  /* Wenn der aktuelle Vorgang eine Wiederholung ist */
  /* oder der User keine Anzahl angegeben hat, wird  */
  /* die Anzahl auf 1 gesetzt.                       */
  if (repeatflag || (*anzahl = str_to_num(optionen))==-1)
    *anzahl = 1;

  *richtung = !strchr(optionen,'r') && !strchr(optionen,'R');
  *ignore   =  strchr(optionen,'i') ||  strchr(optionen,'I');
  *word     =  strchr(optionen,'w') ||  strchr(optionen,'W');
  if (strchr(optionen,'x') ||  strchr(optionen,'X'))
    *akt_regex = TRUE;
  else
    if (strchr(optionen,'n') ||  strchr(optionen,'N'))
      *akt_regex = FALSE;
  *anf_ende =  FALSE;         /* default: nicht von Anfang oder Ende */
  if (strchr(optionen,'b') || strchr(optionen,'B'))
  {
    if(!repeatflag) /* bei repeat nicht mehr vom anfang an */
    {
      *anf_ende = TRUE;
      gotox(0);                /* Zeile 0, Spalte 0 aufsuchen */
      akt_winp->screencol = 0;
    }
    *richtung = 1; /* Bei Suchen vom anfang immer vorawerts */
  }
  else
    if (strchr(optionen,'e') || strchr(optionen,'E'))
    {
      if(!repeatflag)
      {
	*anf_ende = TRUE;         /* letzte Zeile, letzte Spalte aufsuchen */
	gotox(akt_winp->maxline);
	akt_winp->screencol = fastll(akt_winp->alinep->text)-1;
      }
      *richtung = 0;      /* Wenn vom Ende aus gesucht werden soll, dann  */
    }                     /* auf jeden Fall rueckwaerts.                  */
}

/*****************************************************************************
*
*  Funktion       Letztes Suchen/Ersetzen wiederholen (do_repfr)
*  --------
*
*  Beschreibung : Falls schon eine Suche bzw. Ersetzung stattgefunden hat,
*                 wird diese mit den gleichen Optionen, jedoch anzahl=1
*                 wiederholt.
*
*****************************************************************************/

void do_repfr()
{
  if (last_func != F_NONE)
  {
    repeatflag = TRUE;
    if (last_func == F_FIND)
      do_find();
    else
      do_replace();
    repeatflag = FALSE;
  }
}

/*****************************************************************************
*
*  Funktion       Suchen ausfuehren (do_find)
*  --------
*
*  Beschreibung : Falls die Funktion nicht durch do_repfr() aufgerunfen wurde,
*                 werden Suchbegriff und Optionen eingelesen. Anschliessend
*                 wird mit der Funktion o_to_vals der Optionenstring in die
*                 Parameter zerteilt und anzahl mal find() aufgerufen.
*
*****************************************************************************/

void do_find()
{
  /* *** interne Daten und Initialisierung *** */
  int          richtung,   /* Flag, ob vorawerts gesucht wird                */
	       anzahl,     /* Anzahl der Suchwiederholungen                  */
	       alt_anz,    /* Zwischenspeicher fuer Wiederholungszahl        */
	       ignore,     /* Flag, ob Gross-/Kleinschreibung ignoriert wird */
	       word,       /* Flag, ob nur ganze Woerter gefunden werden     */
	       anf_ende,   /* Flag, ob vom Anfang oder vom Ende gesucht wird */
	       akt_regex,  /* fÅr diese Suche reg. Ausdr. verwenden ?        */
	       match_len,  /* zeigt die LÑnge des gefundenen Musters an      */
	       count=1;    /* Zeigt an, ob die Zeile hinter der Anfangszeile */
			   /* schon einmal durchsucht wurde.                 */
  register int start_line, /* Zeile, in der die Suche begann                 */
	       start_col;  /* Spalte, in der die Suche begann                */
  char         dummy[256]; /* String fuer zusammengeflickte Fehlermeldung    */

  if(akt_winp->maxline >= 0) /* Wenn Text leer, dann auch nicht suchen */
  {
    last_func = F_FIND;      /* Fuer do_repfr Funktionstyp merken */
    akt_regex = regexflag;   /* zunÑchst globales Flag Åbernehmen */
    if (!repeatflag)         /* Handelt es sich um eine Wiederholung, dann   */
    {                        /* wird der Suchbegriff nicht erneut eingelesen */
      print_stat(PROMPT_SEARCH);
      read_stat(sbegriff,255,GS_ANY);
      clear_stat();
    }
    if(strlen(sbegriff)) /* Bei leerem Suchbegriff nichts unternehmen */
    {
      if (!repeatflag) /* Optionen nur einlesen, wenn es keine Wiederholung ist */
      {
	print_stat(PROMPT_FOPTIONS);
	read_stat(optionen,255,GS_ANY);
	clear_stat();
      }
      akt_winp->lastline = akt_winp->textline; /* Aktuelle Position als letzte */
      akt_winp->lastcol = akt_winp->screencol; /* Position merken              */

      /* Optionen auswerten */
      o_to_vals(optionen,&richtung,&anzahl,&ignore,&word,&anf_ende,&akt_regex);

      start_line = akt_winp->textline;
      if (!anf_ende) /* Wenn nicht vom Anfang oder vom Ende gesucht werden soll,*/
	akt_winp->screencol += 2*richtung-1;      /* Dann von einer Pos. weiter */
      start_col=akt_winp->screencol;              /* rechts/links suchen        */
      check_buff();       /* Evtl. Puffer zurueck, da auch alinep->text wichtig */

      /* Wenn der Cursor beim Vorwaertssuchen am Zeilenende steht oder beim */
      /* Rueckwaertssuchen am Zeilenanfang, dann wird die Nummer der Start- */
      /* zeile und der Startspalte angepasst. */
      if (richtung ? fastzeichen(start_col) == &space : akt_winp->screencol == -1)
      {
	if((start_line += 2*richtung-1) < 0)
	  start_line = akt_winp->maxline;
	else
	  if(start_line > akt_winp->maxline)
	    start_line = 0;
	start_col=richtung?0:MAXLENGTH;  /* Fall rueckwaerts suchen, dann */
      }                                  /* Zeilenende als Suchbeginn markieren */
      if ((alt_anz = anzahl) > 0)
      {
	print_stat(PROMPT_SEARCHING);
	/* Suchbegriff suchen bis nicht mehr zu finden oder gewuenschte Anzahl */
	while ((match_len = find(sbegriff,richtung,ignore,word,start_line,
				start_col, &count, akt_regex)) > 0
	       && --anzahl)
	  akt_winp->screencol += richtung ? match_len : -1; /* begriff skippen */
	  /* Beachte: Hierdurch kann screencol < 0 werden. Dieser Fall mu·
	     dann in find() berÅcksichtigt werden. */
	clear_stat();
	adapt_screen(match_len>=0?match_len:0); /* anschliessend Bildschirm anpassen */
	if (anzahl) /* Wurde nicht die geforderte Anzahl gefunden ? */
	{
	  if (alt_anz == anzahl) /* Keinmal ? */
	  {
	    if (!anf_ende)
	      akt_winp->screencol--;
	    strcpy(dummy,PROMPT_NOTFOUND);
	  }
	  else
	  { /* match_len == -1 kann hier nicht auftreten, da der Fehler
	       des SuchmusterÅbersetzens sofort beim ersten Aufruf
	       auftritt. */
	    akt_winp->screencol -= richtung ? match_len : -1; /* Skippen rueckgaengig machen */
	    sprintf (dummy,PROMPT_FOUNDN,alt_anz-anzahl);
	  }
	  pe_or(dummy);
	}
      }
    }
    setz_cursor(W_AKT);
  }
  else
    print_err(PROMPT_FEMPTY);
}

/*****************************************************************************
*
*  Funktion       Ersetzungsbegriff eintragen (trag_e_ein)
*  --------
*
*  Parameter    : ebegriff  :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : Einzutragender Begriff
*
*                 elen      :
*                   Typ          : int
*                   Wertebereich : 0 - MAX_INT
*                   Bedeutung    : Laenge des Ersetzungsbegriffs
*
*  Beschreibung : Der uebergebene Ersetzungsbegriff wird mit der Funktion
*                 enter_char() ab der aktuellen Position in den Text ein-
*                 getragen. Dabei wird der Insert-Mode abgeschaltet.
*
*****************************************************************************/

void trag_e_ein(ebegriff,elen)
register char *ebegriff;
register int elen;
{
  /* *** interne Daten und Initialisierung *** */
  register int i;                        /* Schleifenzaehler */

  for (i=0;i<elen;i++)
    enter_char(*ebegriff++, (char*) 0, PUT);
}

/*****************************************************************************
*
*  Funktion       Wort zum Blinken bringen (flash_word)
*  --------
*
*  Parameter    : laenge    :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : Laenge des zum Bilnek zu bringenden Worts
*
*  Beschreibung : Ab der aktuellen Cursorposition werden laenge Zeichen zum
*                 blinken gebracht, d.h. mit dem Ersetzungsattribut.
*                 versehen.
*
*****************************************************************************/

void flash_word(laenge)
register int laenge;
{
  while (laenge--)
    waddch(akt_winp->winp,winch(akt_winp->winp) | 256*ersetzaddr);
}

/*****************************************************************************
*
*  Funktion       Wort "entblinken" (unflash_word)
*  --------
*
*  Parameter    : laenge    :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : Laenge des zu "entblinkenden" Wortes
*
*  Beschreibung : Ab der aktuellen Cursorposition wird bei laenge Zeichen
*                 das Attribut fuer Blinken (ersatzaddr) geloescht.
*
*****************************************************************************/

void unflash_word(laenge)
register int laenge;
{
  while (laenge--)
    waddch(akt_winp->winp,winch(akt_winp->winp) & ~(256*ersetzaddr));
}

/*****************************************************************************
*
*  Funktion       Ersetzen ausfuehren (do_replace)
*  --------
*
*  Beschreibung : Falls die Funktion nicht von do_repfr aufgerufen wurde,
*                 kann der Benutzer Such-, Ersetzungsbegriff und Optionen
*                 eingeben. Dann wird ab der aktuellen Position aus der Such-
*                 begriff gesucht und je nach Option mit oder ohne Abfrage
*                 ersetzt. Die Suche kann vorwaerts und rueckwaerts, nur
*                 nach ganzen worten oder unter Ignorierung der Gross-/Klein-
*                 schreibnug durchgefuehrt werden. Zur Suche wird die Funktion
*                 find() benutzt, um den Begriff zu ersetzen die Funktion
*                 trag_e_ein().
*
*****************************************************************************/

void do_replace()
{
  /* *** interne Daten und Initialisierung *** */
  register int oldu = akt_winp->underflag, /* Zwischenspeicher */
	       elen,        /* Laenge des Ersetzungsbegriffs                  */
	       ulflag,      /* Flag, ob Ersetzungsbegriff nach aktuellem      */
			    /* Modus oder nach dem ersten Zeichen des gefun-  */
			    /* denen Suchbegriffs unterstrichen werden soll   */
	       allflag;     /* Flag, ob alle vorkommenden Suchstrings ersetzt */
			    /* werden sollen.                                 */
  int          richtung,    /* Flag, ob vorawerts gesucht wird                */
	       anzahl,      /* Anzahl der Ersetzungswiederholungen            */
	       ignore,      /* Flag, ob Gross-/Kleinschreibung ignoriert wird */
	       word,        /* Flag, ob nur ganze Woerter gefunden werden     */
	       match_len=0, /* LÑnge des gefundenen Begriffs, Initialisierung,*/
			    /* da am Ende fÅr adapt_screen benutzt.           */
	       alt_anz,     /* Zwischenspeicher fuer Wiederholungszahl        */
	       diff,
	       dl,          /* Dummy, da Anf/Ende-Information unnîtig         */
	       globalflag,
	       ret,         /* Rueckgabewert von ask_replace                  */
	       start_line=akt_winp->textline, /* Suchstart Zeile              */
	       start_col=akt_winp->screencol, /* Suchstart Spalte             */
	       akt_regex,   /* Flag, ob mit regulÑren AusdrÅcken gesucht wird */
	       count=1;     /* Zeigt an, ob die Zeile hinter der Anfangszeile */
			    /* schon einmal durchsucht wurde.                 */
  char         dummy[256];

  if(akt_winp->maxline >= 0) /* Wenn Text leer, nichts suchen */
  {
    last_func = F_REPLACE; /* Funktionstyp fuer evtl. Wiederholung merken */
    akt_regex = regexflag; /* Globales Regex-Flag Åbernehmen */
    check_buff(); /* Evtl. Puffer zurueckschreiben, da alinep->text gebraucht */
    if (!repeatflag)       /* wird. */
    {                      /* Bei Wiederholung keinen neuen Suchbegriff,  */
      print_stat(PROMPT_SEARCH); /* keinen neuen Ersetzungsbegriff und */
      read_stat(sbegriff,255,GS_ANY); /* keine neuen Optionen einlesen    */
      clear_stat();
      if(!sbegriff[0])    /* falls leer, raus */
      {
	setz_cursor(W_AKT);
	return;
      }
      print_stat(PROMPT_REPLACE);
      read_stat(ebegriff,255,GS_ANY);
      clear_stat();
      print_stat(PROMPT_ROPTIONS);
      read_stat(optionen,255,GS_ANY);
      clear_stat();
    }
    /* Laengendifferenz zwischen Such- und Ersetzungsbegriff berechnen */
    elen = strlen(ebegriff);
    akt_winp->lastline = akt_winp->textline; /* aktuelle Position als  */
    akt_winp->lastcol = akt_winp->screencol; /* letzte Position merken */

    /* Optionenstring auswerten */
    o_to_vals(optionen,&richtung,&anzahl,&ignore,&word,&dl,&akt_regex);
    alt_anz = anzahl;
    globalflag = strchr(optionen,'g') || strchr(optionen,'G');
    allflag = strchr(optionen,'a') || strchr(optionen,'A'); /* alle? */
    ulflag = strchr(optionen,'u') || strchr(optionen,'U'); /* unterstr. nach modus?*/

    /* Falls Cursor hinter Zeilenende oder vor Zeilenanfang steht, */
    /* naechste Suchposition als Startposition eintragen.          */
    if (richtung && fastzeichen(start_col) == &space)
    {
      if((start_line += 2*richtung-1) < 0) /* Bei Textanfang oder */
	start_line = akt_winp->maxline;    /* Textende wraparound */
      else
	if(start_line > akt_winp->maxline)
	  start_line = 0;
      start_col=richtung?0:MAXLENGTH-1; /* Fall rueckwaerts suchen, dann Zeilenende als Suchbeginn markieren */
    }

    /* Suchbegriff entweder anzahl mal oder bei gesetztem allflag so oft */
    /* es geht suchen und ersetzen.                                      */
    print_stat(PROMPT_SEARCHING);
    while ((allflag || anzahl--)
	  && (match_len = find(sbegriff,richtung,ignore,word,
			       start_line,start_col,&count,
			       akt_regex)))
    {
      diff = elen - match_len;   /* VerlÑngerung der Zeile */
      if (!globalflag)  /* muss Abfrage vorgenommen werden ? */
      {
	clear_stat();
	adapt_screen(match_len); /* Ja, dann Bildschirm anpassen */
	flash_word(match_len);   /* Suchbegriff highlighten */
	setz_cursor(W_AKT);      /* und refreshen */
	if((ret = ask_replace()) != 'J') /* soll nicht ersetzt werden ? */
	{
	  if(ret == 'A')    /* Abbruch? */
	  {
	    setz_cursor(W_AKT); /* Highlight aus, aber noch kein refresh */
	    unflash_word(match_len);
	    anzahl = -1;    /* Meldung am Ende unterdruecken */
	    break;
	  }
	  setz_cursor(W_AKT);             /* Highlight aus */
	  unflash_word(match_len);
	  akt_winp->screencol+=match_len; /* Suchbegriff skippen */
	  setz_cursor(W_AKT);
	  wrefresh(akt_winp->winp);       /* Fenster refreshen   */
	  print_stat(PROMPT_SEARCHING);
	  continue;
	}
	else
	{
	  setz_cursor(W_AKT);             /* Highlight aus */
	  unflash_word(match_len);
	}
      }
      if(fastll(akt_winp->alinep->text) + diff > MAXLENGTH)
      {
	clear_stat();
	print_err(PROMPT_ERREPLACE); /* Zeile wÅrde zu lang werden */
	if (globalflag && (allflag || anzahl))
	  print_stat(PROMPT_SEARCHING);
      }
      else
      {
	/* falls erstes Zeichen des Wortes unterstrichen, wird gesamtes Wort
	   unterstrichen, wenn ulflag = FALSE, sonst wird je nach underline-
	   modus unterstrichen */
	if(!ulflag)
	  if(ul_char())
	    akt_winp->underflag = TRUE;
	  else
	    akt_winp->underflag = FALSE;
	mdelete(match_len);
	insert(elen); /* Platz fuer neuen (weil evtl. underlined) */
	trag_e_ein(ebegriff,elen); /* Ersetzungsbegriff eintragen */
	check_buff(); /* Puffer zurueckschreiben, da alinep->text gebraucht */
      }
      if(!globalflag) /* Wenn Abgefragt wurde, dann Bildschirm */
      {               /* korrigieren */
	lineout(akt_winp->textline - akt_winp->ws_line);
	wrefresh(akt_winp->winp); /* veraenderte Zeile restaurieren */
	if (allflag || anzahl)    /* wird nochmal gesucht? Dann prompten */
	  print_stat(PROMPT_SEARCHING);
      }
    } /* of while */
    clear_stat();

    /* Wenn nicht alle vorkommenden Suchbegriffe gefunden werden sollten */
    /* und nicht abgebrochen wurde, testen ob alle gewuenschten Er-      */
    /* setzungen durchgefuehrt werden konnten.                           */
    if (!allflag && anzahl != -1)
    {
      if (alt_anz-1 == anzahl)
	strcpy(dummy,PROMPT_NOTFOUND);
      else
	sprintf (dummy,PROMPT_FOUNDN,alt_anz-anzahl-1);
      print_err(dummy);
    }
    if (globalflag) /* Wenn keine Abfrage stattfand, dann jetzt Bildschirm */
      adapt_screen(match_len); /* anpassen */
    show_win(W_AKT); 
  }
  else
    print_err(PROMPT_REMPTY);
  akt_winp->underflag = oldu; /* underflag restaurieren */
}

/*****************************************************************************
*
*  Funktion       NÑchste Klammer in Cursorumgebung finden (find_next_par)
*  --------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE <=> Klammer in Umgebung gefunden
*
*  Beschreibung : In der Umgebung des Cursors wird im aktuellen Fenstertext
*                 nach einer runden, eckigen oder geschweiften Klammer
*                 gesucht. Dabei wird "gleichzeitig" vorwÑrts und
*                 rÅckwÑrts gesucht, um die Klammer zu finden, die am
*                 nÑchsten in CursornÑhe steht. Wird eine Klammer gefunden,
*                 so wird textline/screencol auf die gefundene Klammer
*                 gesetzt. Ansonsten bleiben beide unverÑndert.
*
*****************************************************************************/

int find_next_par ()
{
  /* *** lokale Daten *** */
  char forw  = TRUE,   /* Flag, ob noch vorwÑrts gesucht wird  */
       back  = TRUE,   /* Flag, ob noch rÅckwÑrts gesucht wird */
       found = FALSE,  /* Flag, ob Klammer gefunden wurde */
       *par_str = "(){}[]", /* matchbare Klammern */
       *zeile;         /* Zeiger auf Zeilenpuffer (ohne Untersteichung) */
  int  f_tl,           /* Zeilennummer VorwÑrtssuche */
       b_tl,           /* Zeilennummer RÅckwÑrtssuche */
       diff,           /* Abstand zum Cursor in erster Suchzeile */
       len,            /* LÑnge der ersten Suchzeile */
       sc;             /* Spalte in Suchzeile */
  zeil_typ *f_tp,      /* Zeile VorwÑrtssuche */
	   *b_tp;      /* Zeile RÅckwÑrtssuche */

  if (akt_winp->maxline != -1) /* Wenn Text leer, dann nichts machen. */
  {
    sc = akt_winp->screencol;
    f_tl = b_tl = akt_winp->textline;
    f_tp = b_tp = akt_winp->alinep;

    while ((forw || back) && !found)
    {
      if (forw)
      {
	zeile = fastzeile (f_tp); /* Zeile von Unterstreichung befreien */
	if (sc) /* sc>0, tritt nur bei erster durchsuchter Zeile auf: */
	{
	  diff = 0;
	  len = strlen (zeile);
	  if (sc >= len) /* Falls Cursor hinter Zeilenende, dann auf */
	    sc = len-1;  /* Zeilenende setzen */
	  while ((sc-diff >= 0 || sc+diff < len) && !found)
	  {
	    if (sc+diff < len)
	      found = strchr (par_str, zeile [sc+diff]) ? TRUE : FALSE;
	    if (found) sc = sc + diff + 1; /* es wird noch 1 abgezogen */
	    if (!found && sc-diff >= 0)
	    {
	      found = strchr (par_str, zeile [sc-diff]) ? TRUE : FALSE;
	      if (found) sc = sc - diff + 1; /* es wird noch 1 abgezogen */
	    }
	    diff++;
	  }
	}
	else /* Nicht erste Zeile, dann nur in einer Richtung durchsuchen */
	  while (zeile [sc] && !found)
	    found = strchr (par_str, zeile [sc++]) ? TRUE : FALSE;
	if (!found)
	{
	  f_tp = f_tp->next; /* in nÑchste Zeile gehen */
	  if (f_tp == akt_winp->dummyp) /* Am Ende? Dann nicht weiter vw. */
	    forw = FALSE;
	  else
	    f_tl++;  /* Zeilennummer mitzÑhlen */
	}
	else /* Bei Fund textline, screencol und alinep auf Fundzeile setzen */
	{
	  akt_winp->textline = f_tl;
	  akt_winp->alinep   = f_tp;
	  akt_winp->screencol= sc-1;
	}
      }
      if (!found && back)
      {
	b_tp = b_tp->prev;
	if (b_tp != akt_winp->dummyp)  /* Nicht am Anfang ? */
	{
	  b_tl--;    /* Zeilennummer mitzÑhlen */
	  zeile = fastzeile (b_tp); /* Unterstreichung raus */
	  sc = strlen (zeile)-1;
	  while (sc >= 0 && !found)
	    found = strchr (par_str, zeile [sc--]) ? TRUE : FALSE;
	}
	else
	  back = FALSE;
	if (!found) /* Bei Fund Spalte in sc merken */
	  sc = 0;   /* in erste Spalte fÅr nÑchste Zeile VorwÑrtssuche */
	else  /* Bei Fund textline, screencol und alinep auf Fundzeile setzen */
	{
	  akt_winp->alinep   = b_tp;
	  akt_winp->textline = b_tl;
	  akt_winp->screencol= sc+1;
	}
      }
    }
  }
  return found;
}

/*****************************************************************************
*
*  Funktion       Passende Klammer finden (search_match_par)
*  --------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE: Passende Klammer gefunden
*                                  FALSE: Keine passende Klammer gefunden
*
*  Beschreibung : Zu der Klammer an der Cursorposition (dies ist eine
*                 Vorbedingung fÅr die Funktion) wird die korrespondierende
*                 gesucht. Die Suchrichtung ist durch die Richtung der
*                 Klammer vorgegeben.
*                 Wird keine passende Klammer gefunden, so wird FALSE
*                 zurÅckgegeben. Ansonsten TRUE, und der Cursor steht
*                 auf der Position der gefundenen Klammer. Es wird jedoch
                 keine Refresh oder eine Anpassung des Bildschirmaus-
*                 schnitts vorgenommen.
*
*****************************************************************************/

int search_match_par ()
{
  char found = FALSE,  /* Flag, ob passende Klammer gefunden */
       vorwaerts=TRUE, /* Flag, in welche Richtung gesucht werden soll */
       *zeile,         /* zu durchsuchende aktuelle Zeile */
       par_open,       /* Zeichen der îffnenden Klammer */
       par_close;      /* Zeichen der schlie·enden Klammer */
  int  par_count = 0,  /* ZÑhler fÅr îffnende und schlie·ende Klammern */
       sc = akt_winp->screencol,   /* Spalte der 1. Klammer */
       tl = akt_winp->textline;    /* Zeilennummer der 1. Klammer */
  zeil_typ *tp = akt_winp->alinep; /* Zeilenstruktur der 1. Klammer */

  /* Zuerst ermitteln, ob îffnende/schlie·ende Klammer */
  switch (par_open = *fastzeichen(akt_winp->screencol))
  {
    case '}': par_close = par_open;
	      par_open = '{';      
	      vorwaerts = FALSE;   
	      break;               
    case ')': par_close = par_open;
	      par_open = '(';       
	      vorwaerts = FALSE;    
	      break;                
    case ']': par_close = par_open;
	      par_open = '[';       
	      vorwaerts = FALSE;    
	      break;                
    case '{': par_close = '}'; break;
    case '(': par_close = ')'; break;
    case '[': par_close = ']'; break;
  }

  /* Die Klammer, auf der der Cursor steht, wird mitgezÑhlt. Dadurch
     wird die Zahl sofort != 0. */
  while (!found && tp != akt_winp->dummyp)
  {
    zeile = fastzeile (tp);
    if (sc >= strlen (zeile)) /* Falls Cursor hinter Zeilenende, dann auf */
      sc = strlen (zeile)-1;  /* Zeilenende setzen */
    while (!found && (vorwaerts ? zeile [sc] : sc >= 0))
    {
      if (zeile [sc] == par_close) par_count--;
      if (zeile [sc] == par_open) par_count++;
      found = !par_count; /* Gefunden, wenn ZÑhler wieder auf 0 */
      if (!found)
	vorwaerts ? sc++ : sc--;
    }
    if (!found)  /* nicht in Zeile gefunden, dann nÑchste Zeile */
    {
      if (vorwaerts)
      { tp = tp->next; tl = tl++; sc = 0; }
      else
      { tp = tp->prev; tl = tl--; sc = MAXLENGTH; }
    }
  }
  if (found)
  {
    akt_winp->screencol = sc;
    akt_winp->textline = tl;
    akt_winp->alinep = tp;
  }

  return found;
}
