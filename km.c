#include "curses.h"  /* includiert stdlib.h */
#include <stdio.h>
#include <string.h>

#define MAX_COMM_LEN 10  /* maximale Laenge einer Tastenkombination */
#define MAX_COMM_ANZ 200 /* maximale Anzahl Kommandos */

#ifdef GERMAN
#define PROMPT_NUMBERMISSING "Anzahl der Kombinationen nicht in Datei!\r\n"
#define PROMPT_TOOFEWCOMBS   "Nicht genÅgend Tastenkombinationen!\r\n"
#define PROMPT_ERROR         "Tastenbelegungsdatei fehlerhaft, Programm wird beendet!\r\n"
#define PROMPT_ERRWRITE      "Tastenfile nicht schreibbar!\r\n"
#define PROMPT_NOBAK         "Konnte keine Sicherungskopie des Tastenfiles anlegen. Datei wird Åberschieben.\r\n"
#define PROMPT_NUMBERCHANGE  "Welchen Nummer wollen Sie Ñndern (-1 fuer Ende)? "
#define PROMPT_OLDSTRING     "alter String: %s\r\n"
#define PROMPT_STRING        "String eingeben: "
#define PROMPT_OLDCOMB       "Alte Kombination (dezimal): "
#define PROMPT_COMMAND       "\r\nKommando: %s\n"
#define PROMPT_KEYS          "\r\nGeben Sie die Tastaturcodes ein: (/ fuer Ende)\r\n"
#define PROMPT_NEWCOMB       "Eingegebene Kombination (dezimal): "
#define PROMPT_CONFLICT      "Kombination ist nicht eindeutig, gerÑt in Konflikt mit Befehl %hd\r\nBisherige Kombination bleibt erhalten!\n\n\r"
#define PROMPT_KEYABORT      "Bitte eine Taste drÅcken (a=Abbruch)...\r\n"
#define PROMPT_KEY           "Bitte eine Taste drÅcken... \r\n"
#define PROMPT_MENU1         "1) Tastenkombinationen aendern\r\n"
#define PROMPT_MENU2         "2) Hilfstexte aendern\r\n"
#define PROMPT_MENU3         "3) Befehlsnamen anzeigen\r\n"
#define PROMPT_MENU4         "4) Ende (Tastaturdatei abspeichern)\r\n"
#define PROMPT_MENU5         "Welche Alternative wuenschen Sie (1-4) ? "
#else
#define PROMPT_NUMBERMISSING "Number of combinations not in file!\r\n"
#define PROMPT_TOOFEWCOMBS   "Too few combinations!\r\n"
#define PROMPT_ERROR         "Keyboardfile erroneous, program exits!\r\n"
#define PROMPT_ERRWRITE      "Error writing keyboard file\r\n"
#define PROMPT_NOBAK         "Error creating backup of keyboard file. File will be overwritten!\r\n"
#define PROMPT_NUMBERCHANGE  "Which number do you want to change (-1 for exit)? "
#define PROMPT_OLDSTRING     "old string: %s\r\n"
#define PROMPT_STRING        "Enter string: "
#define PROMPT_OLDCOMB       "Old combination (decimal): "
#define PROMPT_COMMAND       "\r\nCommand: %s\n"
#define PROMPT_KEYS          "\r\nEnter new combination: (/ is terminator)\r\n"
#define PROMPT_NEWCOMB       "Entered combination (decimal): "
#define PROMPT_CONFLICT      "Ambiguous combination, conflicts with number %hd\r\nPrevious combination remains unchanged!\n\n\r"
#define PROMPT_KEYABORT      "Please hit a key (a=abort)...\r\n"
#define PROMPT_KEY           "Please hit a key...\r\n"
#define PROMPT_MENU1         "1) Change key combinations\r\n"
#define PROMPT_MENU2         "2) Change helptexts\r\n"
#define PROMPT_MENU3         "3) Show command names\r\n"
#define PROMPT_MENU4         "4) Exit (save keyboard file)\r\n"
#define PROMPT_MENU5         "Enter your choice (1-4) : "
#endif

typedef struct s3
{
  short int blen;
  short int befehl[MAX_COMM_LEN];
  char  *info_text;
}comm_typ;

/* *** globale Daten *** */
comm_typ  comm_feld [MAX_COMM_ANZ];  /* Feld der Tastenkombinationen */
char      *it_feld[MAX_COMM_ANZ];    /* Pointertabelle fuer Hilfstrings */
short int anz,                       /* Anzahl der Tastenkombinationen */
	  n_info;                    /* Anzahl der Info-Strings */
char      *kom_names[MAX_COMM_ANZ] = /* Namen der Kommandos */
#ifdef GERMAN
	  { "Refresh","Zum Zeilenanfang","Zum Zeilenende","Halbe Seite hoch",
	    "Halbe Seite runter","Zeichen unter Cursor loeschen",
	    "Zeichen links vom Cursor loeschen","Zum Fensteranfang",
	    "Zum Fensterende","Einfuege-/Ueberschreibmodus aendern",
	    "Zum Dateianfang","Zum Dateiende","Wort loeschen",
	    "Wort nach links","Wort nach rechts","Nach rechts","Nach links",
	    "Nach oben","Nach unten","Seite hoch","Seite runter",
	    "Naechste Zeile","Zeile loeschen",
	    "Datei in aktuelles Fenster laden",
	    "Aktuelles Fenster abspeichern",
	    "Verlassen ohne Sichern ohne Loadfile",
	    "Controlcode einfuegen","Anzeigen der Hilfstexte ein-/ausschalten",
	    "Fenster bewegen","Fentsergroesse aendern","Zum naechsten Fenster",
	    "Zum vorherigen Fenster","Fenster per Nummer anspringen",
	    "Fenster oeffnen","Fenster schliessen",
	    "Zeile per Nummer anspringen",
	    "Verlassen mit Abspeichern ohne Loadfile","Suchen","Ersetzen",
	    "Unterstreichung an/aus","Fensterinhalt eine Zeile hoch",
	    "Fensterinhalt eine Zeile runter",
	    "Aktuelle Zeile zur obersten Fensterzeile machen",
	    "Aktuelle Zeile zur mittleren Fensterzeile machen",
	    "Aktuelle Zeile zur untersten Fensterzeile machen",
	    "Bis zum Zeilenende loeschen","Fenstergroesse wechseln",
	    "Suchen/Ersetzen wiederholen","Kommando(s) wiederholen",
	    "Suchen/Ersetzen wiederholen","Zeilen verknuepfen",
	    "Zeile vor aktueller einfuegen","Autoindent an/aus","Tabulator",
	    "Tabulatorlaenge festlegen","Tabkomprimierungsmodus an/aus",
	    "Blockanfang setzen","Blockende normaler Block setzen",
	    "Blockende rechteckiger Block setzen","Blockmarkierung loeschen",
	    "Block loeschen","Block kopieren","Block verschieben",
	    "Block in Paste-Puffer kopieren","Paste-Puffer in Text kopieren",
	    "Block einruecken","Block aus Datei lesen",
	    "Block in Datei schreiben","Zum Blockende","Zum Blockanfang",
	    "Blockhervorhebung an/aus","Block an Filter uebergeben",
	    "Marker setzen","Marker anspringen","Zur letzten Position",
	    "Zeile hinter der aktuellen einfuegen",
	    "Fenster per Dateiname anspringen",
	    "Datei im aktuellen Fenster umbenennen","Tabulator rueckwaerts",
	    "Sicherungskopie an/aus","Macro definieren/ausfuehren",
	    "Geloeschte Zeile wiederherstellen","Shell-Modus an/aus",
	    "Verlassen mit Abspeichern mit Loadfile",
	    "Verlassen ohne Abspeichern mit Loadfile",
	    "Aktuelles Zeichen loeschen", "RegulÑre AusdrÅcke an/aus",
	    "Passende Klammer finden", "Cursor in Bildschirmmitte",
	    "Alle geÑnderten Dateien sichern", "Absatz formatieren",
	    "Automatischer Zeilenumbruch an/aus" };
#else
	  { "refresh","to beginning of line","to end of line",
	    "half page up", "half page down","delete current character",
	    "delete character left to cursor","to top of window",
	    "to bottom of window","toggle insert/overwrite",
	    "to beginning of file","to end of file","delete word",
	    "word left","word right","one character right",
	    "one character left", "one line up","one line down","page up",
	    "page down", "new line", "delete line",
	    "load file in current window", "save current window",
	    "quit without save without loadfile", "insert control character",
	    "toggle helptext display", "move window","change windowsize",
	    "to next window", "to previous window",
	    "switch to window by number", "open window","close window",
	    "goto line by number", "quit with save without loadfile",
	    "find","replace", "toggle underline",
	    "move text one line up", "move text one line down",
	    "make current line top line", "make current line middle line",
	    "make current line lowest line", "delete to end of line",
	    "toggle windowsize", "repeat find/replace",
	    "repeat command sequence", "repeat find/replace",
	    "join lines", "insert line before current","toggle autoindent",
	    "tab", "set tablength","toggle tabcompression",
	    "mark beginning of block","mark end of normal block",
	    "mark end of rectangular block","unmark block",
	    "delete block","copy block","move block",
	    "cut block","paste block", "indent block","read block from file",
	    "write block to file","goto end of block" ,
	    "goto beginning of block", "toggle blockhighlighting",
	    "pass block to filter", "set marker","goto marker",
	    "goto last position", "insert line after current",
	    "switch to window by filename",
	    "rename file in current window","tab backwards",
	    "toggle creation of .bak-files","define/execute macro",
	    "undelete line","toggle shell-mode", 
	    "quit with save and loadfile", "quit without save with loadfile",
	    "delete current character", "regular expressions on/off",
	    "goto matching parenthesis", "cursor to middle of window",
	    "save all modified files", "reflow paragraph",
	    "toggle automatic linebreak mode" };
#endif

#ifndef OWN_CURSES
/******************************************************************************
*
* Funktion    : Fatale Fehlermeldung ausgeben (fatal)
* -------------
*
* Parameter   : s           :
*                 Typ          : char *
*                 Wertebereich : Pointer auf ASCII-Zeichenkette
*                 Bedeutung    : Auszugebende Fehlermeldung
*
* Bescheibung : Der bergene String s wird ausgegeben, das Programm mit dem
*               Fehlercode 1 abgebrochen.
*
******************************************************************************/

void fatal(s)
char *s;
{
#ifdef GERMAN
  printf("\r\nFATAL: %s\r\nProgramm wird abgebrochen\r\n",s);
#else
  printf("\r\nFATAL: %s\r\naborting program\r\n",s);
#endif
  exit(1);
}
#endif

/******************************************************************************
*
* Funktion     : Speicher reservieren (reserve_mem)
* --------------
*
* Parameter    : n           :
*                  Typ          : int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : Laenge des zu reservierenden Bereichs
*
* Ergebnis     :
*                  Typ          : char *
*                  Wertebereich : != NULL
*                  Bedeutung    : Zeiger auf reservierten Bereich
*
* Beschreibung : Ist noch genug Speicher frei, wird der gewuenschte Bereich
*                angefordert und der Pointer auf den Bereich zurueckgegeben.
*                Ansonsten wird die Curses-Fuktion fatal aufgerufen, die
*                zum Abbruch des Programms fuehrt.
*
******************************************************************************/

char *reserve_mem(n)
{
  char *p;

  if(p = malloc(n))
    return(p);
  else
#ifdef GERMAN
    fatal("Kein Speicherplatz mehr ");
#else
    fatal("Out of memory ");
#endif
}

/*****************************************************************************
*
*  Funktion       Hilfstexte-Array neu anordnen (rearrange_it)
*  --------
*
*  Beschreibung : Durch die Bearbeitung des Tastenbelegungsfiles kînnen
*                 Hilfetexte ÅberflÅssig geworden sein. Dadurch wÅrden
*                 bei unverÑndertem Ablegen von it_feld in der Datei
*                 beim nÑchsten Einlesen zu wenig Hilfstexte gelesen.
*                 Es wird also zunÑchst fÅr alle Hilfstexte geprÅft,
*                 ob sie noch von mindestens einem Kommando benîtigt werden.
*                 Ist dies der Fall, so werden sie als zu Åbernehmend
*                 markiert.
*                 Anschlie·end werden die als nicht zu Åbernehmend
*                 markierten Hilfstexte aus dem Array gelîscht und die
*                 weiter hinten stehenden werden entsprechend nach
*                 vorne verschoben.
*
*****************************************************************************/

void rearrange_it ()
{
  int  i, /* Laufindex fÅr Tastenkombinationen */
       j, k; /* Laufindex fÅr Hilfstexte */
  char *take_it = reserve_mem (n_info); /* String zu Åbernehmen ? */

  /* initialisiere take_it und shift: */
  for (j=0; j<n_info; j++)
    take_it [j] = FALSE;
  for (i=0; i<anz; i++)
    for (j=0; j<n_info; j++)
      if (comm_feld [i].info_text == it_feld [j])
	if (take_it [j])
	  break;
	else
	{
	  take_it [j] = TRUE;
	  break;
	}

  /* Jetzt das Feld zusammenschrumpfen */
  j = 0;
  while (j<n_info)
  {
    if (!take_it [j])           /* Hilfstext nicht Åbernehmen? */
    {
      n_info--;                 /* Eine Kombination weniger    */
      for (k=j; k<n_info; k++)  /* Rest ranziehen              */
      {
	it_feld [k] = it_feld [k+1];
	take_it [k] = take_it [k+1];
      }
    }
    else
      j++;
  }
}

/*****************************************************************************
*
*  Funktion       Tastenbelegung einlesen (lies_tasten)
*  --------
*
*  Beschreibung : Aus der Datei tasten.inf wird die Tastaturbelegung ge-
*                 laden. Dabei werden die Zeiger auf die Strings zunaechst
*                 als Indices in die Tabelle geladen, die Anzahl der aus
*                 der Datei zu ladenden Strings wird mitgezaehlt. Nachdem
*                 alle Kombinationen eingelesen sind, wird die Stringtabelle
*                 in das Array it_feld eingelesen.
*
*****************************************************************************/

void lies_tasten()
{
  FILE      *f;          /* Zeiger fuer Tastenbelegungsdatei */
  short int i,           /* Schleifenzaehler */
	    j,           /* Schleifenzaehler */
	    index[MAX_COMM_ANZ];  /* Zwischenspeicher fuer Hilfstextnummern */
  char      *hilf,       /* Hilfszeiger fuer Hilfstexte */
	    dummy [256]; /* String zum Einlesen der Hilfstexte */

  if (!(f = fopen ("tasten.inf","rb")))
  {
    anz = n_info = 0;
    return;
  }
  if(fread(&anz,sizeof(short int),1,f) != 1) /* Anzahl der Befehle einlesen */
  {
    printf(PROMPT_NUMBERMISSING);
    gets(0);
    endwin();
    exit(1);
  }
  anz++; /* In Datei steht letzter Index, also ist Anzahl 1 hoeher */
  for (n_info=i=0;i<anz;i++) /* Alle Kombinationen einlesen */
  {
    /* Zuerst die Laenge des aktuellen Befehls einlesen */
    if(fread (&comm_feld[i].blen,sizeof(short int),1,f) < 1)
    {
      printf(PROMPT_TOOFEWCOMBS);
      gets(0);
      endwin();
      exit(1);
    }
    /* Jetzt Befehl selber einlesen */
    fread (comm_feld[i].befehl,sizeof(short int),comm_feld[i].blen,f);
    fread (&index[i],sizeof(short int),1,f); /* Index fuer Hilfstext */
    comm_feld[i].info_text = NULL;
    if(index[i] > -1)  /* Ueberhaupt Hilfstext fuer diesen Befehl ? */
      for (n_info++,j=0;j<i;j++) /* Ja, dann checken, ob ein anderer */
	if (index[j] == index[i]) /* Befehl schon diesen String hat. */
	{
	  n_info--;     /* Nur einen weiteren String einlesen,  */
	  break;        /* falls neuer Index                    */
	}
  }
  for (i=0;i<n_info;i++)  /* Hilfstexte einlesen */
  {
    if (!fgets (dummy,255,f))
    {
      printf(PROMPT_ERROR);
      gets(0);
      endwin();
      exit(1);
    }
    dummy[strlen(dummy)-1] = '\0';              /* \n abschneiden */
    it_feld[i] = hilf = strcpy(reserve_mem(strlen(dummy)+1),dummy);
    for (j=0;j<=anz;j++)  /* Zeiger auf Hilfstext in comm_feld eintragen */
      if (index[j] == i)
	comm_feld[j].info_text = hilf;
  }
  fclose (f);
}

/*****************************************************************************
*
*  Funktion       Tastenbelegung in Datei schreiben (schreib_tasten)
*  --------
*
*  Beschreibung :
*
*****************************************************************************/

void schreib_tasten()
{
  FILE      *f;    /* Pointer fuer Ausgabedatei */
  short int i,     /* Schleifenzaehler fuer Infotexte */
	    j,     /* Schleifenzaehler fuer Kommandos und String */
	    index; /* Stringindex des aktuellen Hilfstextes */

  unlink ("tasten.bak"); /* Fehlercode wird ignoriert */
  if (rename ("tasten.inf", "tasten.bak"))
    printf(PROMPT_NOBAK);
  if (!(f = fopen ("tasten.inf","wb")))
  {
    printf(PROMPT_ERRWRITE);
    exit(1);
  }

  rearrange_it (); /* Es werden nicht mehr benîtigte Info-Texte aus dem
		      Array gelîscht und das Array entsprechend
		      zusammengeschrumpft. Das ist nîtig, damit nicht
		      beim Lesen der Datei zu wenig Texte eingelesen werden.*/
  anz--; /* Es wird der letzte Index in die Datei geschrieben */
  fwrite(&anz,sizeof(short int),1,f); /* Anzahl der Kombinationen schreiben */
  anz++;
  for(j=0;j<anz;j++) /* Alle Kombinationsstrings schreiben */
  {
    index = -1; /* Noch kein Info-String */
    for(i=0;i<n_info;i++) /* Auf welchen String zeigt die Kombination ? */
      if(it_feld[i] == comm_feld[j].info_text)
      {
	index = i; /* Stringindex merken */
	break;
      }
    /* Zuerst die Laenge der Kombination, dann die Kombination selber */
    /* und dann den Stringindex in die Datei schreiben */
    fwrite(&comm_feld[j].blen,sizeof(short int),1,f);
    fwrite(comm_feld[j].befehl,sizeof(short int),comm_feld[j].blen,f);
    fwrite(&index,sizeof(short int),1,f);
  }

  for(j=0;j<n_info;j++) /* Alle Infotexte schreiben */
    fprintf(f,"%s\n",it_feld[j]);
  fclose(f);
}

/*****************************************************************************
*
*  Funktion       Infotexte aendern (change_strings)
*  --------
*
*  Beschreibung : Der User gibt die Nummer des zu aendernden Strings ein.
*                 Daraufhin bekommt er den alten String angezeigt. Er gibt
*                 den neuen ein, der in it_feld an der Stelle des alten
*                 eingetragen wird.
*
*****************************************************************************/

void change_strings()
{
  short int index,       /* Nummer des Strings */
	    j;           /* Zaehler zum Durchlaufen des Kommandoarrays */
			 /* und der Stringtabelle */
  char      string[MAX_COMM_ANZ], /* Puffer fuer neuen String */
	    *old_text;   /* Zwischenspeicher fuer Adresse alter String */

  while(1)
  {
    for(j=0;j<n_info;j++) /* Hilfstexte anzeigen */
      printw("%d.:  %s\r\n",j,it_feld[j]);
    printw(PROMPT_NUMBERCHANGE); refresh();
    scanw("%hd\n",&index);  /* Stringnummer einlesen */

    if(index >= n_info)   /* Keinen neuen String anlegen */
      continue;
    if(index == -1)       /* Abbruch bei -1 */
      break;

    printw(PROMPT_OLDSTRING,it_feld[index]);
    printw(PROMPT_STRING);refresh();
    getstr(string);   /* Neuen String in Puffer einlesen */
    free(old_text = it_feld[index]); /* Alten freigeben, Adresse merken */
    /* Neuen String in dafuer allozierten Speicher kopieren und neue */
    /* Adresse in it_feld eintragen */
    it_feld[index] = strcpy(reserve_mem(strlen(string)+1),string);

    /* Zeigt der Stringzeiger eines anderen Kommandos auf den geaenderten */
    /* String, dann wird auch dieser Zeiger auf den neuen String gesetzt. */
    for(j=0;j<anz;j++)
      if(comm_feld[j].info_text == old_text)
	comm_feld[j].info_text = it_feld[index];
  }
}


/*****************************************************************************
*
*  Funktion       Curses initialisieren (init)
*  --------
*
*  Beschreibung : Curses wird aktiviert, das Terminal in den raw-mode
*                 gesetzt, die Sondertasten und das Scrolling zugelassen.
*
*****************************************************************************/

void init()
{
#ifdef OS2
  set_os2_raw (TRUE);       /* !!! mu· erste Aktion sein !!! */
#endif
  initscr();
  scrollok(stdscr,TRUE);
  nonl();
}

/*****************************************************************************
*
*  Funktion       Kommandostring aendern (change_kom)
*  --------
*
*  Parameter    : index     :
*                   Typ          : short int
*                   Wertebereich : 0-anz
*                   Bedeutung    : Nummer des zu aendernden Kommandos
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE: Kombination war eindeutig
*                                  FALSE: Kombination geriet in Konflikt
*
*  Beschreibung : Die alte Kombination wird angezeigt. Dann wird die neue
*                 Kombination in das Feld temp eingelesen. Anschliessend
*                 wird die neue Kombination auf ihre Eindeutigkeit getestet.
*
*****************************************************************************/

int change_kom(index)
short int index;
{
  short int temp[MAX_COMM_LEN], /* Puffer zum Einlesen der Kombination */
	    i,                  /* Schleifenzaehler */
	    j,                  /* Schleifenzaehler fuer Tasten in temp */
	    k;                  /* Schleifenzaehler fuer Eindeutigkeitscheck */
  char      string[MAX_COMM_ANZ]; /* Puffer fuer Eingabe Hilfstext */

  printw(PROMPT_OLDCOMB);
  for(i=0;i<comm_feld[index].blen;i++)
    printw("%d ",comm_feld[index].befehl[i]);

  printw(PROMPT_COMMAND, kom_names [index]);
  printw(PROMPT_KEYS);
  refresh();

  /* Befehl einlesen. Erreicht j den maximalen Index, so muss j noch   */
  /* inkrementiert werden, da die nachfolgende Schleife davon ausgeht, */
  /* dass j auch nach dem Lesen des / ionkrementiert wurde.            */
  j=0;
  noecho();
  raw();
  while((temp[j++] = getch()) != (short int) '/'
	 && (j < MAX_COMM_LEN || (j++,0)));
  echo();
  noraw();

  /* Eingegebene Kombination anzeigen: */
  printw(PROMPT_NEWCOMB);
  for(i=0;i<j-1;i++)
    printw("%d ",temp[i]);
  printw("\r\n");

  /* Befehl auf Eindeutigkeit checken */
  for(j--,i=0;i<anz;i++)       /* j auf '/' setzen */
    if(i != index) /* Bei aktueller Kombination nicht testen */
    {
      /* k auf das Ende des kuerzeren Kommandos setzen */
      /* Solange wiederholen, bis k<0 oder Kommandos ungleich */
      for(k=(comm_feld[i].blen > j ? j : comm_feld[i].blen)-1;
	  k >= 0 && comm_feld[i].befehl[k] == temp[k]; k--);
      if(k == -1)   /* ein String Teil des anderen oder Strings identisch? */
      {
	printw(PROMPT_CONFLICT,i);
	return(FALSE);
      }
    }
  printw("\r\n");
  /* Befehl eindeutig, dann Kombination im comm_feld uebernehmen */
  memcpy(comm_feld[index].befehl,temp,j * sizeof(short int));

  if((comm_feld[index].blen = j) > 1) /* Bei Kommandos, die laenger */
  { /* als ein Zeichen sind, wird nach einem Hilfstext gefragt.     */
    for(j=0;j<n_info;j++) /* Hilfstexte anzeigen */
      printw("%d.:  %s\r\n",j,it_feld[j]);
    printw("\r\nString-Index (-1 fuer keinen String): ");refresh();
    scanw("%hd",&j); /* Index fuer Hilfsstring zum Kommando einlesen */
    if(j != -1) /* Wird Hilfsstring gewuenscht ? */
    {
      if(j >= n_info)  /* Neuer String ? */
      {
	printw(PROMPT_STRING); refresh(); /* Dann einlesen */
	getstr(string);
	it_feld[j = n_info++] = strcpy(reserve_mem(strlen(string)+1),string);
      }
      comm_feld[index].info_text = it_feld[j]; /* Stringzeiger eintragen */
    }
  }
  else
    comm_feld[index].info_text = NULL; /* Kein String, dann NULL eintragen */
  return(TRUE);
}

/******************************************************************************
*
* Funktion     : Eingabe neuer Tastenkombinationen steuern (hndl_koms)
* --------------
*
* Beschreibung : Es wird nach der Nummer des zu aendernden Kommandos gefragt.
*                Gibt der User eine Zahl ungleich -1 an, so wird die ent-
*                sprechende Tastenkombination eingelesen. Ist die Nummer
*                zu groa, wird sie auf die Nummer der ersten nicht belegten
*                Kombination gesetzt.
*                Gibt der User -1 ein, so wird die Funktion beendet.
*
******************************************************************************/

void hndl_koms()
{
  short int index;

  do
  {
    printw(PROMPT_NUMBERCHANGE);
    refresh();
    scanw("%hd",&index);   /* Nummer der Kombination einlesen */
    if(index == -1)        /* Bei -1 Schleife abbrechen       */
      break;

    if(index >= anz)  /* Falls Nummer zu groa, eine Kombination mehr, */
    {                 /* neue Kombination einlesen */
      if(!change_kom(anz++)) /* Konflikt, dann anz wieder runter, da */
	anz--;               /* keine neue Kombination entstand */
    }
    else
      change_kom(index);
  } while (1);
}

/******************************************************************************
*
* Funktion     : Kommandonamen anzeigen (show_names)
* --------------
*
* Beschreibung : Die Kommandonamen werden angezeigt. Ist der Bildschirm
*                voll, wird ein Tastendruck erwartet.
*
******************************************************************************/

void show_names()
{
  int i;

  for (i=0;i<anz;i++)
  {
    if(!((i+1)%20))  /* Bildschirm voll ? */
    {
      printw(PROMPT_KEYABORT);
      refresh();
      noecho();
      raw();
      if(getch() == 'a')
      {
	echo();
	noraw();
	break; /* Bei a for-Schleife abbrechen */
      }
      echo();
      noraw();
    }
    printw("%3d : %s\r\n",i,kom_names[i]); /* Kommandonamen anzeigen */
  }
  refresh();
  if((i+1)%20) /* Zum Abschlu· nochmals auf Tastendruck warten */
  {
    printw(PROMPT_KEY);
    refresh();
    noecho();
    raw();
    getch();
    noraw();
    echo();
  }
}

/******************************************************************************
*
* Funktion     : Menue (menue)
* --------------
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : 1-4
*                  Bedeutung    : Gewaehlte Alternative
*
* Beschreibung : Die Alternativen werden angezeigt, der Nutzer mua eine
*                auswaehlen. Die Nummer der ausgewaehlten Alternative wird
*                als Integer zurueckgegeben.
*
******************************************************************************/

int menue()
{
  char c;
  
  printw(PROMPT_MENU1);
  printw(PROMPT_MENU2);
  printw(PROMPT_MENU3);
  printw(PROMPT_MENU4);
  printw(PROMPT_MENU5);
  refresh();
  noecho();
  raw();
  while((c = getch()) < '1' && c > '4');
  echo();
  noraw();
  printw("%c\r\n",c);
  refresh();
  return(c-'0');
}

/*****************************************************************************
*
*  Funktion       Hauptprogramm (main)
*  --------
*
*  Beschreibung : Curses wird initialisiert, die alte Tastaturbeledung aus
*                 der Datei tasten.inf gelesen. Dann kann der User Tasten-
*                 kombinationen aendern. Anschlieaend koennen die Hilfs-
*                 texte geaendert werden. Ist das abgeschlossen, wird die
*                 neue Tastaturbelegung in die Datei zurueckgeschrieben.
*
*****************************************************************************/

int main()
{
  int alt;

  init();
  lies_tasten(); /* Bisherigen Tastenkombinationen einlesen */
  do
  {
    switch(alt=menue()) /* Nutzereingabe lesen */
    {
      case 1:
	hndl_koms();   /* Tastenkombinationen aendern */
	break;
      case 2:
	change_strings(); /* Evtl. Stringinhalte aendern */
	break;
      case 3:
	show_names(); /* Namen der Komandos anzeigen */
	break;
    }
  } while (alt != 4); /* Solange, bis der User ENDE wuenscht */
  endwin();
  schreib_tasten(); /* Tastenbelegung wieder in Datei zurueckschreiben */
}
