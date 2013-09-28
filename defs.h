/* *********************** Include-Datei DEFS.H ******************** */
/* Diese Datei enthÑlt fÅr alle Moduln von EHP wichtige Definitionen */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "curses.h"
#include <sys/stat.h>
#include <stdlib.h>

#define sname(x) (strrchr((x),'\\')?strrchr((x),'\\')+1:x)

#define MAX_HEIGHT (LINES-1) /* Eine Zeile mua fÅr Statusfenster freibleiben */
#define MAX_WIDTH (COLS)  

#define START_HEIGHT MAX_HEIGHT /* Anfangsgroessen eines Fensters */
#define START_WIDTH MAX_WIDTH
#define START_X 0
#define START_Y 0

#ifndef OWN_CURSES
#define A_BLINK     _STANDOUT
#define A_BOLD      _STANDOUT
#define A_STANDOUT  _STANDOUT
#define A_UNDERLINE _STANDOUT
#define nodelay(window, b)
#endif
#define BORDER_HOR  'ƒ'
#define BORDER_VER  '≥'
#define CORNER_UL   '⁄'
#define CORNER_UR   'ø'
#define CORNER_LL   '¿'
#define CORNER_LR   'Ÿ'
#define REST_CHAR   BORDER_VER+256*A_STANDOUT   /* Zeichen zum Restaurieren des */
#define REST_ARR_UP ''+256*A_STANDOUT /* linken und rechten Fensterrandes */
#define REST_ARR_DN ''+256*A_STANDOUT 

#define W_AKT     0             /* FÅr modus-Flag der Funktionen kopf */
#define W_NOTAKT  1             /* und rahmen */

#define STD_TAB 8               /* Standard-Tablaenge */
#define SW_STEP 5               /* Schrittweite fuer grosse Schritte */
				/* bei size_win und move_win */

#define BUFFSIZE  500           /* Groesse von Textpuffern */
#define MAXLENGTH 500           /* Maximale Zeilenlaenge */
#define MAX_ANZ_LINES 2147483647/* Maximale Zeilenanzahl */
#define ANZ_WIN   2147483647    /* Maximale Fensteranzahl */
#define INS_BUFF_LEN 3*MAXLENGTH  /* Falls Insert mit unterstrichenen Zeichen */

#define INSERT 1                /* modes fuer fastcharout */
#define PUT 2

#define MAX_COMM_LEN 10         /* Defines fuer Taste    */

#define F_NONE       0          /* defines fuer Wiederholfunktion Suche/ */
#define F_FIND       1          /* Ersetze                               */
#define F_REPLACE    2

#define J_OK         0          /* defines fuer Rueckgabewerte der JOIN-Fkt. */
#define J_LASTLINE   1
#define J_TOOLONG    2

#define IGNORE_COORS 0          /* defines fuer Modi von del_line() und koppel_line() */
#define ADAPT_COORS  1

#define AP_INSERT 0             /* defines fuer adapt_pos() */
#define AP_DELETE 1

#ifdef OS2
#define MOUSE_MASK        126   /* Linken und rechten Mausknopf inkl. Bewegung testen */
#define MOUSE_BUT1          4
#define MOUSE_BUT2         16
#define MOUSE_BUT3         64
#define MOUSE_BUT1_MOVE     2
#define MOUSE_BUT2_MOVE     8
#define MOUSE_BUT3_MOVE    32
#else
#define MOUSE_MASK         10   /* Linken und rechten Mausknopf testen */
#define MOUSE_BUT1          1
#define MOUSE_BUT2          2
#define MOUSE_BUT3          4
#endif
#define MOUSE_HIDE          1   /* Mauszeiger verstecken */
#define MOUSE_SHOW          2   /* Mauszeiger anzeigen */
#define NO_KLICK            1   
#define KLICK_RIGHT         2   /* Konstanten fÅr Funktion ja_nein */
#define KLICK_LEFT          3
#define MOUSE_KOPF          1   /* Maus steht in Kopfzeile */
#define MOUSE_CLOSE         2   /* Knopf zum Schlieaen des Fensters */
#define MOUSE_TOG_SIZE      3   /* Groesse wechseln */
#define MOUSE_SIZE          4   /* Groesse veraendern */
#define MOUSE_TEXT          5   /* Maus im Textbereich */
#define MOUSE_RAHMEN        6
#define MOUSE_SCROLL_UP     7   /* Text nach oben scrollen */
#define MOUSE_SCROLL_DOWN   8   /* Text nach unten scollen */
#define MOUSE_SCROLL_RIGHT  9   /* Text nach rechts scrollen */
#define MOUSE_SCROLL_LEFT  10   /* Text nach links scrollen */

#define GS_NUM 1                /* define fuer newgetstr: nur Ziffern */
#define GS_ANY 2                /* alle moeglichen Zeichen */
#define EC_SCROLL_WIDTH 10      /* Bildschirmverschiebung bei Erreichen */
				/* des rechten Randes                   */
#define MACRO_NEST_DEPTH 20     /* max. Verschachtelungstiefe der Macros */
#define MAX_MACRO_LEN MAXLENGTH /* max. Anzahl Zeichen pro Macro */
#define ANZ_MACROS 26
#define ANZ_MARKER 10
#define STD_FATTR S_IWRITE | S_IREAD /* Standard-Dateiattribute */

#define PF_TASTEN "tasten.inf" /* Pfad fÅr Tastenbelegungsdatei */
#define PF_CONFIG "config.ehp" /* Pfad fÅr Standardkonfigurationsdatei */
#ifdef OS2
#define STD_SHELL "cmd.exe"
#define PF_LOAD   "load.cmd"   /* default load-file */
#else
#define STD_SHELL "command.com"
#define PF_LOAD   "load.bat"   /* default load-file */
#endif

#define LAST_LINE_DEL 2
#define OTHER_LINE_DEL 1
#define NO_LINE_DEL 0

#define BT_RECHTECK 0           /* Blocktypen */
#define BT_NORMAL 1

#define B_IN_BLOCK 1                    /* Rueckgabewerte von in_block() */
#define B_FIRST_LINE 2
#define B_LAST_LINE 4
#define B_BEFORE 8
#define B_AFTER 16
#define B_LINE 32
#define B_FIRST_CHAR 64
#define B_RIGHT 128

#ifdef GERMAN
#define T_SIZE_ERRTEXT   "Text wÅrde zu lang werden!"
#define L_SIZE_ERRTEXT   "Zeile wÅrde zu lang werden!"
#define B_SIZE_ERRTEXT   "Text oder Zeile wÅrden zu lang werden!"
#define W_COUNT_ERRTEXT  "Kein Fenster mehr frei, weitere Dateien werden ignoriert!"
#define NO_LOAD_ERRTEXT  "Load-File konnte nicht ausgefuehrt werden!\n"
#define PROMPT_SEARCH    "Suchbegriff : "
#define PROMPT_REPLACE   "Ersetzungsbegriff : "
#define PROMPT_ERRREGEX  "Fehler beim öbersetzen des Suchmusters"
#define PROMPT_FOPTIONS  "Optionen ([<Zahl>] [r] [i] [w] [b|e] [x|n]) : "
#define PROMPT_ROPTIONS  "Optionen ([<Zahl>] [r] [i] [w] [b|e] [x|n] [g] [a] [u]) : "
#define PROMPT_NOTFOUND  "Suchbegriff nicht gefunden!"
#define PROMPT_FOUNDN    "Konnte den Suchbegriff nur %d mal finden!"
#define PROMPT_ERREPLACE "Konnte nicht ersetzen, Zeile zu lang!"
#define PROMPT_FEMPTY    "Text leer! Kann nichts suchen!"
#define PROMPT_REMPTY    "Text leer! Kann nichts ersetzen!"
#define PROMPT_REPLYN    "Ersetzen (Ja/Nein/Abbruch)? "
#define PROMPT_SEARCHING "Suche laeuft!"
#define PROMPT_HELPTEXT  "Hilfstexte "
#define PROMPT_BACKUP    "Sicherheitskopieoption "
#define PROMPT_REGEX     "RegulÑre AusdrÅcke "
#define PROMPT_BLKHILGHT "Blockhervorhebung "
#define PROMPT_WORKING   "Bearbeitung lÑuft"
#define PROMPT_OUTOFMEM  "Nicht genug Speicher ! "
#define PROMPT_ENTER     " Bitte RETURN drÅcken..."
#define PROMPT_YES_NO    " (J/N) ? "
#define PROMPT_NO_FILE   "Konnte kein File laden!"
#define PROMPT_REPEAT    "Anzahl der Wiederholungen : "
#define PROMPT_KEYREAD   "Tastenbelegungsdatei nicht lesbar!"
#define PROMPT_KEYLENGTH "Tastenbelegungsdatei (LÑngenangabe) fehlerhaft!"
#define PROMPT_KEYCOMM   "Tastenbelegungsdatei (Kommandos) fehlerhaft!"
#define PROMPT_KEYCOMLEN "Befehlskombination in Tastenbelegungsdatei zu lang!"
#define PROMPT_KEYHELP   "Tastenbelegungsdatei (Hilfstexte) fehlerhaft!"
#define PROMPT_KEYHELPEM "Tastenbelegungsdatei fehlerhaft (Hilfstexte leer)!"
#define PROMPT_RECURSION "Max. Verschachtelungstiefe Åberschritten, Puffer werden gelîscht!"
#define PROMPT_COMMAND   "Kommandos eingeben (<RETURN> = Ende, CTRL-c <c> = <c> einfÅgen): "
#define PROMPT_FILENAME  "Dateiname : "
#define PROMPT_TABLEN    "Neue TabulatorlÑnge (dafault: %d): "
#define PROMPT_LINENUMB  "Welche Zeilennummer? "
#define PROMPT_WINDOWNUM "Fensternummer: "
#define PROMPT_CUT       "Block wurde im PASTE-Puffer gespeichert"
#define PROMPT_ASKINDENT "Wie viele Spalten (negativ = nach links) ? "
#define PROMPT_NOBLOCK   "Kein Block markiert!"
#define PROMPT_ERRWRITE  "Schreibfehler bei Datei "
#define PROMPT_MARKER    "Welcher Marker (0-9)? "
#define PROMPT_MACRO     "A-Z: Macro def.; a-z: Macro ausf.;  "
#define PROMPT_SAVE      "Dateien sichern"
#define PROMPT_SAVING    "Satei %s wird abgespeichert..."
#define PROMPT_LOADING   "Satei %s wird geladen..."
#define PROMPT_NOWINDOW  "Kein Fenster mehr frei! "
#define PROMPT_LINETOLNG "Zeile zu lang! "
#define PROMPT_ONEWINDOW "Nur ein Fenster geîffnet !"
#define PROMPT_BLOCKEMPT "Block leer!"
#define PROMPT_FILENOTFD "Datei nicht gefunden !"
#define PROMPT_ERRMARKER "Inkorrekte Eingabe, es wurde kein Marker gesetzt!"
#define PROMPT_STALEMARK "Zum Marker gehîriges Fenster existiert nicht mehr!"
#define PROMPT_EMPTYMARK "GewÅnschter Marker noch nicht belegt!"
#define PROMPT_ERRINPUT  "Inkorrekte Eingabe!"
#define PROMPT_TMPOPEN   "TemporÑre Datei konnte nicht geîffnet werden!"
#define PROMPT_ERRSHELL  "Shell lieferte Fehlercolde zurÅck !"
#define PROMPT_TMPNOTFND "TemporÑre Datei nicht gefunden (evtl. kein Speicher)!"
#define PROMPT_ERROPNCFG "Konnte Config-Datei nicht zum Schreiben îffnen!"
#define PROMPT_ERRWRTCFG "Schreibfehler in Config-Datei; Datei wird gelîscht!"
#define PROMPT_ERRDELETE "Lîschen was nicht mîglich!"
#define PROMPT_MACROEMPT "Macro %c ist leer!"
#define PROMPT_FILETOLNG "Datei zu lang, konnte nicht ganz gelesen werden!"
#define PROMPT_WARNLOAD  "Text geÑndert, aber nocht nicht gespeichert! Wirklich Åberladen"
#define PROMPT_ALRDYLDD  "Datei schon in Fenster %s! Trotzdem laden"
#define PROMPT_WARNCLOSE "énderungen noch nicht gespeichert! Fenster wirklich schlie·en"
#define PROMPT_WARNSAVE  "GeÑnderte Dateien werden gespeichert! Wirklich beenden"
#define PROMPT_EXITANYWY "Trotzdem beenden"
#define PROMPT_WARNQUIT  "Dateien werden nicht gespeichert! Wirklich beenden"
#define PROMPT_FILEEXIST "Datei existiert bereits! öberschreiben"
#define PROMPT_NEWWINDOW "Nicht gefunden! Datei in neues Fenster laden"
#define PROMPT_FATAL     "Fataler Fehler (Bus Error)! GeÑnderte Dateien speichern"
#define PROMPT_ASKNEW    "Datei %s nicht lesbar! Neu anlegen"
#define PROMPT_LINE      "Zeile"
#define PROMPT_COLUMN    "Spalte"
#define PROMPT_MODIFIED  "GEéNDERT"
#define PROMPT_WRTPROTCT "SCHREIBSCHUTZ"
#define PROMPT_WINDOW    "Fenster"
#define PROMPT_WINDOWEMP "L E E R"
#define PROMPT_INSERT    "Einfg"
#define PROMPT_OVERWRITE "öber"
#define PROMPT_UNDERLINE "Unter"
#define PROMPT_ERRUNLINK "Loeschen war nicht moeglich!"
#define PROMPT_ERRTMPOPN "TemporÑre Datei konnte nicht geîffnet werden!"
#define PROMPT_EMPTFILT  "Filterausgabe war leer, Block wurde geloescht!"
#define PROMPT_FILTNAME  "Name des Filters: "
#define PROMPT_FLTNMDFLT "Name des Filters (Default ist jetzt %s): "
#define PROMPT_WINMOVE   "Pfeile: bewege Fenster; s: Schrittweite Ñndern (jetzt:%d); RETURN: Ende  "
#define PROMPT_WINSIZE   "Pfeile: Ñndere Grî·e; s: Schrittweite Ñndern (jetzt:%d); RETURN: Ende  "
#define PROMPT_ERRCONF   "Inkorrektes Dateiformat, Config-Datei wird gelîscht!"
#define PROMPT_WRTPROT   "%s schreibgeschÅtzt! Zum Speichern umbenennen!"
#define PROMPT_KEYLONG   "Maximale LÑnge Åberschritten, letzte Eingabe ignoriert!"
#define PROMPT_FILTINSRT "Filterausgabe konnte nicht eingefuegt werden!"
#define PROMPT_SHELINSRT "Konnte Shell-Ausgabe nicht einfuegen!"
#define PROMPT_INDNTLONG "Zeile zu lang, konnte nicht einruecken!"
#define PROMPT_WINNMNFND "Window mit dieser Nummer nicht gefunden !"
#define PROMPT_IO_REDIR  "Ein- und Ausgabe von EHP dÅrfen nicht umgelenkt werden!\n"
#else
#define T_SIZE_ERRTEXT   "Text would grow too long!"
#define L_SIZE_ERRTEXT   "Line would grow too long!"
#define B_SIZE_ERRTEXT   "Text od line would grow too long!"
#define W_COUNT_ERRTEXT  "No window free, further files are ignored!"
#define NO_LOAD_ERRTEXT  "Unable to execute load-file!\n"
#define PROMPT_SEARCH    "Find : "
#define PROMPT_REPLACE   "Replace with : "
#define PROMPT_ERRREGEX  "Error in compiling regular search expression"
#define PROMPT_FOPTIONS  "Options ([<number>] [r] [i] [w] [b|e] [x|n]) : "
#define PROMPT_ROPTIONS  "Options ([<Zahl>] [r] [i] [w] [b|e] [x|n] [g] [a] [u]) : "
#define PROMPT_NOTFOUND  "Searchstring not found!"
#define PROMPT_FOUNDN    "Searchstring found only %d times!"
#define PROMPT_ERREPLACE "Could not replace, line too long!"
#define PROMPT_FEMPTY    "Text empty! Nothing to find!"
#define PROMPT_REMPTY    "Text empty! Nothing to replace!"
#define PROMPT_REPLYN    "Replace (Yes/No/Abort)? "
#define PROMPT_SEARCHING "Searching!"
#define PROMPT_HELPTEXT  "Helptexts "
#define PROMPT_BACKUP    "Creation of .bak-Files "
#define PROMPT_REGEX     "Regular expressions "
#define PROMPT_BLKHILGHT "Blockhighlight "
#define PROMPT_WORKING   "Working"
#define PROMPT_OUTOFMEM  "Out of memory ! "
#define PROMPT_ENTER     " Please hit RETURN..."
#define PROMPT_YES_NO    " (Y/N) ? "
#define PROMPT_NO_FILE   "No file loadable!"
#define PROMPT_REPEAT    "Times to repeat : "
#define PROMPT_KEYREAD   "Keyboardfile unreadable!"
#define PROMPT_KEYLENGTH "Keyboardfile (length information) erroneous!"
#define PROMPT_KEYCOMM   "Keyboardfile (commands) erroneous!"
#define PROMPT_KEYCOMLEN "Command in keyboardfile too long!"
#define PROMPT_KEYHELP   "Keyboardfile (helptexts) erroneous!"
#define PROMPT_KEYHELPEM "Keyboardfile erroneous (helptexts empty)!"
#define PROMPT_RECURSION "Recursion too deep, buffers are deleted!"
#define PROMPT_COMMAND   "Enter commands (<RETURN> = end, CTRL-c <c> = insert <c>): "
#define PROMPT_FILENAME  "Filename : "
#define PROMPT_TABLEN    "New tablength (dafault: %d): "
#define PROMPT_LINENUMB  "Which linenumber? "
#define PROMPT_WINDOWNUM "Windownumber: "
#define PROMPT_CUT       "Block stored in PASTE-buffer"
#define PROMPT_ASKINDENT "How many columns (negativ = to left) ? "
#define PROMPT_NOBLOCK   "No block marked!"
#define PROMPT_ERRWRITE  "Error writing file "
#define PROMPT_MARKER    "Which marker (0-9)? "
#define PROMPT_MACRO     "A-Z: define macro; a-z: execute macro;  "
#define PROMPT_SAVE      "Save files"
#define PROMPT_SAVING    "Saving file %s..."
#define PROMPT_LOADING   "Loading file %s..."
#define PROMPT_NOWINDOW  "No window available! "
#define PROMPT_LINETOLNG "Line too long! "
#define PROMPT_ONEWINDOW "Only one window open!"
#define PROMPT_BLOCKEMPT "Block empty!"
#define PROMPT_FILENOTFD "File not found!"
#define PROMPT_ERRMARKER "Erroneous imput, no marker is set!"
#define PROMPT_STALEMARK "Window belonging to marker ceased to exist!"
#define PROMPT_EMPTYMARK "Marker not yet defined!"
#define PROMPT_ERRINPUT  "Invalid input!"
#define PROMPT_TMPOPEN   "Error opening tamporary file!"
#define PROMPT_ERRSHELL  "Shell returned errorcode!"
#define PROMPT_TMPNOTFND "Temporary file not found (perhaps no memory)!"
#define PROMPT_ERROPNCFG "Error opening config-file for writing!"
#define PROMPT_ERRWRTCFG "Error writing config-file; removing file!"
#define PROMPT_ERRDELETE "Removing failed!"
#define PROMPT_MACROEMPT "Macro %c is empty!"
#define PROMPT_FILETOLNG "File too long, couldn't be read completely!"
#define PROMPT_WARNLOAD  "Text modified but not saved! Really overwrite"
#define PROMPT_ALRDYLDD  "File already in window %d! Load anyway"
#define PROMPT_WARNCLOSE "Modifications not saved! Really close window"
#define PROMPT_WARNSAVE  "Modified files are saved! Really exit"
#define PROMPT_EXITANYWY "Exit anyway"
#define PROMPT_WARNQUIT  "Files are not saved! Really exit"
#define PROMPT_FILEEXIST "File already exists! Overwrite"
#define PROMPT_NEWWINDOW "Not found! Load file in new window"
#define PROMPT_FATAL     "Fatal error (bus error)! Save modified files"
#define PROMPT_ASKNEW    "File %s not readable! Create it"
#define PROMPT_LINE      "line"
#define PROMPT_COLUMN    "column"
#define PROMPT_MODIFIED  "MODIFIED"
#define PROMPT_WRTPROTCT "WRITEPROTECTED"
#define PROMPT_WINDOW    "window"
#define PROMPT_WINDOWEMP "E M P T Y"
#define PROMPT_INSERT    "Insert"
#define PROMPT_OVERWRITE "Overwrt"
#define PROMPT_UNDERLINE "Under"
#define PROMPT_ERRUNLINK "Deletion failed!"
#define PROMPT_ERRTMPOPN "Unable to open temporary file!"
#define PROMPT_EMPTFILT  "Filteroutput was empty, deleting block!"
#define PROMPT_FILTNAME  "Name of filter: "
#define PROMPT_FLTNMDFLT "Name of filter (current default %s): "
#define PROMPT_WINMOVE   "Arrow-keys: move window; s: toggle stepwidth (now:%d); RETURN: quit "
#define PROMPT_WINSIZE   "Arrow-keys: change size; s: toggle stepwidth (now:%d); RETURN: quit "
#define PROMPT_ERRCONF   "Erroneous fileformat, deleting config-file!"
#define PROMPT_WRTPROT   "%s write-protected! Rename for saving!"
#define PROMPT_KEYLONG   "Maximum length exceeded, ignoring last input!"
#define PROMPT_FILTINSRT "Unable to insert filter output!"
#define PROMPT_SHELINSRT "Unable to insert shell output!"
#define PROMPT_INDNTLONG "Line too long, unable to indent!"
#define PROMPT_WINNMNFND "Window with this number not found!"
#define PROMPT_IO_REDIR  "In- and output of EHP must not be redirected!\n"
#endif

typedef struct s1
{
  char      *text;
  struct s1 *prev,*next;
} zeil_typ;

typedef struct s5
{
  char      *text;
  struct s5 *next;
} bzeil_typ;

typedef struct s3
{
  char      typ;
  int       s_line,s_col,e_line,e_col,laenge;
  bzeil_typ *bstart;
} block_typ;

typedef struct s2
{
  char      *filename;
  int       ws_col,ws_line,textline,textcol,screencol,maxline,wini,x,y,dx,dy;
  int       ax,ay,adx,ady,tablen,lastcol,lastline;
  unsigned  short int attribs;
  char      shellflag,changeflag,insflag,underflag,autoindflag,tabflag,read_only;
  block_typ block;
  zeil_typ  *alinep,*dummyp;
  WINDOW    *winp;
  struct s2 *prev,*next;
} win_typ;

typedef struct s6{
  int window,line,col;
}marker_typ;

typedef struct s8{
  short int *begin,*end,*current,anz_rep;
  char free_flag;
}puff_typ;

typedef struct s4{
  short int blen;
  short int befehl[MAX_COMM_LEN];
  char  *info_text;
}comm_typ;

#ifndef aktdef
extern win_typ *akt_winp;
extern char *reserve_mem();
extern char *save_text();
#endif
