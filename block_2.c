/****************************************************************/
/*                                                              */
/*      MODUL:  block_2.c                                       */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - block_free (Blocktext freigeben)              */
/*              - block_defined (Testen, ob Block definiert)    */
/*              - nl_blockadapt (Anpassung Block nach newline)  */
/*              - dl_blockadapt (Anpassung Block nach delline)  */
/*              - insdel_blockadapt (Anpssg. nach insert/delete)*/
/*              - lies_block (Block aus Datei einlesen)         */
/*              - schr_block (Block in Datei schreiben)         */
/*              - get_f_name (Filternamen einlesen)             */
/*              - bl_to_fil (Block an Filter uebergeben)        */
/*              - to_shell (Zeile an Shell uebergeben)          */
/****************************************************************/

#include "defs.h"
#include <process.h>

extern char *getenv(),*mktemp(),space,*reserve_mem(),*fastzeichen();
extern marker_typ marker[];
extern bzeil_typ *save_rechteck(),*save_normal();

/*****************************************************************************
*
*  Funktion       aktuellen Blocktext freigeben (block_free)
*  --------
*
*  Parameter    : blockstart :
*                   Typ          : bzeil_typ*
*                   Wertebereich : Pointer auf Blockzeilenliste
*                   Bedeutung    : Freizugebender Blocktext
*
*  Beschreibung : Die Speicherplatz fuer die uebergebene Liste und den in
*                 ihr enthaltenen Text wrd freigegeben.
*
*****************************************************************************/

void block_free(blockstart)
bzeil_typ *blockstart;
{
  /* *** interne Daten *** */
  register bzeil_typ *hilf;  /* Zeiger auf naechste Blockzeile */

  while(blockstart)
  {
    hilf = blockstart->next;
    line_free(blockstart->text); /* Speicher der Zeile freigeben */
    free(blockstart);            /* Zeilenstruktur freigeben     */
    blockstart = hilf;
  }
}

/*****************************************************************************
*
*  Funktion       Testen, ob Block definiert ist (block_defined)
*  --------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : TRUE = Block ist definiert
*                                  FALSE = Block ist nicht definiert
*
*  Beschreibung : Falls Bockanfang und Blockende definiert sind und der Block-
*                 anfang vor dem Blockende liegt, wird TRUE zurueckgegeben.
*                 Sonst wird FALSE zurueckgegeben.
*
*****************************************************************************/

int block_defined()
{
  /* *** interne Daten und Initialisierung *** */
  register block_typ *bl = &akt_winp->block; /* Aktueller Block */

  return(bl->e_line != -1 && bl->s_line != -1 && (bl->e_line > bl->s_line
	 || (bl->s_line == bl->e_line && bl->e_col > bl->s_col)));
}

/*****************************************************************************
*
*  Funktion       Anpassung des Blocks nach newline (nl_blockadapt)
*  --------
*
*  Beschreibung : Falls ein Block definiert ist, werden, abhaengig von der
*                 Cursorposition relativ zum Block, die Blockkoordinaten,
*                 die letzte Position und die Marker angepasst.
*                 Handelt es sich um einen normalen Block, so koennen unter
*                 Umstaenden auch die col-Koordinaten angepasst werden.
*
*****************************************************************************/

void nl_blockadapt()
{
  /* *** interne Daten und Initialisierung *** */
  register int i, /* Zaehler fuer Schleife bei Markeranpassung              */
	       ib = in_block(akt_winp->textline,akt_winp->screencol);
		  /* ib gibt die Position des Cursors relativ zum Block an  */

  if(block_defined()) /* Blockkoordinaten nur aendern, falls Block markiert */
    if(ib & B_IN_BLOCK  /* Wenn der Cursor im Block steht oder der Block    */
    || (akt_winp->block.typ == BT_RECHTECK  /* rechteckig ist und der       */
    && ib & B_LINE                          /* Cursor in einer Blockzeile   */
    && !(ib & B_BEFORE) && !(ib & B_AFTER))) /* und nicht vor oder hinter   */
					    /* dem Block steht              */
    {                                       /* dann wird die Blockendzeile  */
      akt_winp->block.e_line++;             /* inkrementiert.               */
      /* Stand der Cursor in der letzten Zeile eines normalen Blocks, dann  */
      /* wird die Blockendspalte angepasst.                                 */
      if(akt_winp->block.typ == BT_NORMAL)
      {
	if(ib & B_LAST_LINE)
	  akt_winp->block.e_col -= akt_winp->screencol;
	/* Steht man auf dem ersten Zeichen eines normalen Blocks, dann */
	/* wird die Startzeile und die Startspalte angepasst.           */
	if(ib & B_FIRST_CHAR)
	{
	  akt_winp->block.s_line++;
	  akt_winp->block.s_col -= akt_winp->screencol;
	}
      }
    }
    else /* Steht der Cursor nicht im Block, wird getestet, ob er davor steht */
      if(ib & B_BEFORE)
      {
	akt_winp->block.e_line++; /* Wenn ja, werden sowohl Blockstart- als   */
	akt_winp->block.s_line++; /* auch -endzeile angepasst.                */
	/* Ist Cursorzeile erste Blockzeile, dann Startspalte anpassen,       */
	/* Ist Cursorzeile letzte Blockzeile, dann Endspalte anpassen.        */
	if(akt_winp->block.typ == BT_NORMAL)
	{
	  if (ib & B_FIRST_LINE)
	    akt_winp->block.s_col -= akt_winp->screencol;
	  if (ib & B_LAST_LINE)
	    akt_winp->block.e_col -= akt_winp->screencol;
	}
      }

  /* lastpos anpassen */
  if(akt_winp->textline < akt_winp->lastline)  /* Cursor vor lastpos ? */
    akt_winp->lastline++;           /* dann Zeile von lastpos anpassen */
  if(akt_winp->textline == akt_winp->lastline  /* Cursor in lastposzeile */
  && akt_winp->screencol <= akt_winp->lastcol) /* vor lastpos, dann      */
  { /* Zeile und Spalte von lastpos anpassen */
    akt_winp->lastline++;
    akt_winp->lastcol -= akt_winp->screencol;
  }

  /* Marker anpassen */
  for(i=0;i<ANZ_MARKER;i++)
    if(marker[i].window == akt_winp->wini)  /* richtiges Fenster ? */
    {
      if (akt_winp->textline < marker[i].line) /* Cursor vor Markerzeile */
	marker[i].line++;
      if(akt_winp->textline == marker[i].line  /* Cursor in Markerzeile */
      && akt_winp->screencol <= marker[i].col) /* vor Marker, dann      */
      { /* Markerzeile und -spalte anpassen */
	marker[i].line++;
	marker[i].col -= akt_winp->screencol;
      }
    }
}

/*****************************************************************************
*
*  Funktion       Anpassung des Blocks nach del_line (dl_blockadapt)
*  --------
*
*  Beschreibung : Falls ein Block definiert ist, werden, abhaengig von der
*                 Cursorposition relativ zum Block die Blockkoordinaten,
*                 die letzte Position und die Marker angepasst.
*                 Handelt es sich um einen normalen Block, so koennen unter
*                 Umstaenden auch die col-Koordinaten angepasst werden.
*
*****************************************************************************/

void dl_blockadapt()
{
  /* *** interne Daten *** */
  register int i,  /* Zaehler fuer Schleife bei Markeranpassung */
	       ib; /* Position des Cursors relativ zum Block    */

  if(block_defined())  /* Nur anpassen, wenn ein Block markiert ist */
  {
    /* berechnen, wo Cursor relativ zum Block steht und testen, ob  */
    /* er davor steht.                                              */
    if((ib = in_block(akt_winp->textline,akt_winp->screencol)) & B_BEFORE)
    {
      akt_winp->block.e_line--;   /* Ja, dann Endzeile -1 */
      if(ib & B_FIRST_LINE)       /* In erster Zeile vor Block ? */
      {                           /* ja, dann bei normalem Block Anfangs- */
	if(akt_winp->block.typ == BT_NORMAL) /* spalte auf 0 */
	  akt_winp->block.s_col = 0;
      }
      else                        /* Steht man in einer Zeile vor dem Block, */
	if (!(ib & B_LINE))       /* dann Startzeile -1 */
	  akt_winp->block.s_line--;
    }
    else                          /* Wenn man nicht davor steht: */
      if(ib & B_LINE)             /* Steht man in einer Blockzeile ? */
      {
	akt_winp->block.e_line--; /* ja, dann Endzeile -1 */
	if(akt_winp->block.typ == BT_NORMAL) /* Bei normalem Block */
	{                         /* evtl. Spalten anpassen        */
	  if(ib & B_LAST_LINE)    /* falls man in der letzten Blockzeile steht */
	  {
	    check_buff();         /* Pufferinhalt muss evtl. in Text */
	    /* Endspalte wird hinter letzte Spalte der darueberliegenden */
	    /* Zeile gesetzt                                             */
	    akt_winp->block.e_col = fastll(akt_winp->alinep->prev->text);
	  }
	  if(ib & B_FIRST_LINE)   /* Steht man in der ersten Zeile, wird */
	    akt_winp->block.s_col = 0; /* Startspalte auf 0 gesetzt */
	}
      }

    /* Testen, ob der Block komplett geloescht wurde, und falls ja, dann */
    /* Start- und Endzeile auf -1 setzen (Block unmarkieren) und Fenster-*/
    /* inhalt neu zeichnen.                                              */
    if (akt_winp->block.e_line < akt_winp->block.s_line
    || (akt_winp->block.e_line == akt_winp->block.s_line
    && akt_winp->block.e_col <= akt_winp->block.s_col))
      akt_winp->block.s_line = akt_winp->block.e_line = -1;
  }
  /* jetzt marker und lastpos anpassen */
  if(akt_winp->textline < akt_winp->lastline)
    akt_winp->lastline--;
  for(i=0;i<ANZ_MARKER;i++)
    if(marker[i].window == akt_winp->wini && akt_winp->textline < marker[i].line)
      marker[i].line--;
}

/*****************************************************************************
*
*  Funktion       Anpassung des Blocks nach insert/delete (insdel_blockadapt)
*  --------
*
*  Parameter    : offset    :
*                   Typ          : int
*                   Wertebereich : 0 - MAXLENGTH-1
*                   Bedeutung    : Anzahl der Einfuegungen/Loeschungen
*
*  Beschreibung : Falls ein Block definiert ist, werden, abhaengig von der
*                 Cursorposition relativ zum Block, bei einem normalen Block
*                 die Blockkoordinaten angepasst. Desweiteren werden evtl.
*                 die letzte Position und die Marker angepasst.
*
*****************************************************************************/

void insdel_blockadapt(offset)
int offset;
{
  /* interne Daten und Initialisierung *** */
  register int i,  /* Schleifenzaehler fuer Markeranpassung          */
	   obd,    /* Ergebnis des Tests, ob ein Block definiert ist */
	   sc = akt_winp->screencol, /* Alte Cursorspalte            */
	   ib;     /* Position des Cursors relativ zum Block         */

  ib = in_block(akt_winp->textline,sc);

  /* Testen, ob ein Block markiert ist, und wenn ja, ob es ein normaler ist */
  if ((obd=block_defined()) && akt_winp->block.typ == BT_NORMAL)
  {
    /* Falls eine Spalte zu weit angepasst wird, d.h. anschliessend links vom  */
    /* Cursor steht, dann wird die Spalte auf die Cursorspalte gesetzt.        */

    if (ib & B_FIRST_LINE && ib & (B_BEFORE | B_FIRST_CHAR))
    { /* Steht man in der ersten Zeile vor oder auf dem  Blockanfang? */
      if((akt_winp->block.s_col+=offset)<sc)  /* Ja, dann Startspalte anpassen */
	akt_winp->block.s_col = sc;
      if (ib & B_LAST_LINE)                   /* Liegt in der Zeile auch */
	if((akt_winp->block.e_col+=offset)<sc)/* das Blockende, dann End-*/
	  akt_winp->block.e_col = sc;         /* spalte auch anpassen    */
    }
    else                                      /* Nicht in der ersten Blockzeile */
      if (ib & B_LAST_LINE && ib & B_IN_BLOCK)/* Letzte Bockzeile und im Block? */
	if((akt_winp->block.e_col+=offset)<sc)/* Ja, dann Endspalte anpassen    */
	  akt_winp->block.e_col = sc;
  }
  /* checken, ob einzeiliger Block nun nicht mehr existiert */
  if (obd && akt_winp->block.e_line == akt_winp->block.s_line && akt_winp->block.e_col <= akt_winp->block.s_col)
    akt_winp->block.s_line = akt_winp->block.e_line = -1;

  /* jetzt marker und lastpos anpassen */
  /* nur bis col = screencol  (bei delete wichtig) */
  if(akt_winp->textline == akt_winp->lastline && sc <= akt_winp->lastcol)
    /* Wenn man nicht auf der letzten Position steht oder eingefuegt wurde */
    /* (nicht geloescht, offset > 0), dann wird der offset auf die Spalte  */
    /* der letzten Position addiert. Gelangt man dadurch nach links der    */
    /* aktuellen Position, wird die Spalte der letzten Position auf die    */
    /* aktuelle Spalte gesetzt. Gleiches gilt fuer die Marker.             */
    if((akt_winp->lastcol != sc || offset > 0) && (akt_winp->lastcol += offset) < sc)
      akt_winp->lastcol = sc;

  for(i=0;i<ANZ_MARKER;i++)
    if(marker[i].window == akt_winp->wini && akt_winp->textline == marker[i].line && sc <= marker[i].col)
      if((marker[i].col != sc || offset > 0) && (marker[i].col += offset) < sc)
	marker[i].col = sc;
}

/*****************************************************************************
*
*  Funktion       Block einlesen (lies_block)
*  --------
*
*  Parameter    : bl      :
*                   Typ          : block_typ*
*                   Wertebereich : Pointer auf Blockstruktr
*                   Bedeutung    : Spaeter der eingelesene Block
*
*               : f          :
*                   Typ          : FILE*
*                   Wertebereich : Pointer auf Datei
*                   Bedeutung    : Datei, aus der Text geladen werden soll
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : TRUE: Es konnten Zeichen gelesen werden
*
*  Beschreibung : Aus der uebergebenen Datei wird der Text geladen und in
*                 die Blocktextliste der uebergebenen Blockstruktur ge-
*                 schrieben.
*                 Evtl. vorkommende Tabs werden expandiert.
*
*****************************************************************************/

int lies_block(bl,f)
block_typ *bl;
FILE *f;
{
  /* *** interne Daten *** */
  char buff[3*MAXLENGTH+2],  /* Puffer fuer Zeile aus Datei */
       buff2[3*MAXLENGTH+2], /* Puffer fuer in Block zu     */
			     /* uebernehmende Zeile         */
       nlflag;               /* Zeigt an, ob die letzte ge- */
			     /* lesene Zeile vollstaendig   */
			     /* war ('\n' am Ende)          */
  int  len,    /* Laenge der eingelesenen Zeile      */
       in,     /* Index in Eingabepuffer buff        */
       out,    /* Index in Ausgabepuffer buff2       */
       sc,     /* Laenge von buff2 im screencol-Mass */
       max;    /* Laenge der laengsten Zeile bei     */
	       /* rechteckigem Block, sonst Laenge   */
	       /* der letzten Blockzeile             */
  register bzeil_typ *p,     /* aktuelle Blockzeile  */
		     *p_old; /* vorige Blockzeile    */

  bl->s_col = akt_winp->screencol;  /* Aktuelle Position als */
  bl->s_line = akt_winp->textline;  /* Blockstart merken     */
  bl->laenge = -1;                  /* Laenge noch nicht bekannt */
  bl->bstart = p = p_old = (bzeil_typ*) reserve_mem (sizeof (bzeil_typ));
  p->text = NULL;
  p->next = NULL;
  out = sc = max = 0;

  while (fgets(buff,3*MAXLENGTH + 1,f))  /* Zeile aus Datei einlesen */
  {
    if(buff[(len = strlen(buff))-1] == '\n') /* Schloss Zeile mit '\n' ab ? */
    {
      nlflag = TRUE;
      buff[--len] = '\0'; /* '\n' streichen */
    }
    else
      nlflag = FALSE;
    in = 0;
    while(in < len)  /* Alle Zeichen des eingelesenen Strings durchgehen */
    {
      if (buff[in] == '_' && buff[in+1] == '' && buff[in+2])
      {                            /* Fand man ein unterstrichenes Zeichen, */
	buff2[out++] = buff[in++]; /* dann Unterstrich und Backspace ueber- */
	buff2[out++] = buff[in++]; /* nehmen                                */
      }
      if(buff[in] == '\t')         /* Ein Tab wird zu Spaces expandiert     */
	tab_in_buff(buff2,&sc,&out);
      else                         /* Alle anderen Zeichen normal in Aus-   */
      {                            /* gebapuffer kopieren.                  */
	buff2[out++] = buff[in];
	sc++;
      }
      in++;
      if(sc > max || bl->typ == BT_NORMAL) /* bei normalem Block ist max    */
	max = sc;                          /* laenge der letzten Zeile      */
      if(sc == MAXLENGTH)       /* Ist die Ausgabezeile voll, */
      {
	buff2[out] = '\0';      /* dann Puffer abschliessen   */
	sc = out = 0;
	p->text = save_text(buff2); /* Zeile in Blocktextstruktur merken */
	p->next = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ)); /* neue Zeile */
	p_old = p;
	p = p->next;
	p->next = NULL;
	bl->laenge++; /* eine Zeile mehr im Block */
      }
    }

    /* Eingelesene Zeile jetzt in Blocktextstruktur ablegen, */
    /* falls die eingelesene Zeile durch ein Newline abge-   */
    /* schlossen wurde (nlflag).                             */
    if(nlflag && !(!sc && len))         /* falls gerade volle Zeile abge- */
    {                                   /* speichert wurde, nicht noch    */
      buff2[out] = '\0';                /* einmal abspeichern             */
      sc = out = 0;
      p->text = save_text(buff2);
      p->next = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ));
      p_old = p;
      p = p->next;
      p->next = NULL;
      bl->laenge++; /* Eine Zeile mehr im Block */
    }
  }

  if(p_old != p)                        /* wenn eine Zeile zuviel alloziert */
  {                                     /* diese freigeben */
    free(p);
    p_old->next = NULL;
  }
  /* Endspalte des Blockes ausrechnen: Dazu wird bei einem rechteckigen */
  /* Block die Laenge der laengsten Zeile auf die Startspalte addiert,  */
  /* bei einem normalen Block ist es lediglich die Laenge der letzten   */
  /* Zeile.                                                             */
  bl->e_col = (bl->typ == BT_RECHTECK ? bl->s_col + max : max);
  if(bl->laenge != -1)  /* Endzeile ist Startzeile + Laenge */
  {
    bl->e_line = bl->s_line + bl->laenge;
    akt_winp->changeflag = TRUE;
    return(TRUE);
  }
  bl->s_line = bl->e_line = -1; /* War der Block leer, Block unmarkieren, */
  return(FALSE);                /* FALSE zurueckgeben                     */
}

/*****************************************************************************
*
*  Funktion       Schreib Block auf Platte (schr_block)
*  --------
*
*  Parameter    : p          :
*                   Typ          : bzeil_typ*
*                   Wertebereich : Pointer auf Textliste
*                   Bedeutung    : abzuspeichernder Text
*
*               : f          :
*                   Typ          : FILE*
*                   Wertebereich : Pointer auf Datei
*                   Bedeutung    : Datei, in der Text gesichert werden soll
*
*  Beschreibung : Der aktuelle Block wird in die zu dem uebergebenen File-
*                 pointer gehoerige Datei geschrieben.
*
*****************************************************************************/

void schr_block(p,f)
register bzeil_typ *p;
FILE      *f;
{
  check_buff();   /* Pufferinhalt evtl. in Text uebernehmen */
  while(p)
  {
    if (p->text)
      put_zeile(p->text,f); /* Zeile in Datei schreiben, Tabs evtl. kompr. */
    putc('\n',f);
    p = p->next;
  }
}

/*****************************************************************************
*
*  Funktion       Filternamen einlesen (get_f_name)
*  --------
*
*  Parameter    : old_fil     :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : alter Filtername (min. 61 Bytes Puffer)
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE: Filtername vorhanden
*                                  FALSE: kein Filter gewaehlt
*
*  Beschreibung : Ein Filtername wird eingelesen. Ist dessen Laenge Null, so
*                 wird der urspruengliche Filtername beibehalten. Ist dann jedoch
*                 kein alter Filtername vorhanden, wird FALSE zurueckgegeben.
*
*****************************************************************************/

int get_f_name(old_fil)
char *old_fil;
{
  /* *** interne Daten *** */
  char command[61]; /* String zur Eingabe des Filternamens */

  if(!*old_fil) /* Existiert noch kein Filtername, auf jeden Fall fragen */
    strcpy(command,PROMPT_FILTNAME);
  else          /* Sonst den Default mit angeben */
    sprintf(command,PROMPT_FLTNMDFLT,old_fil);
  print_stat(command);
  read_stat(command,60,GS_ANY);  /* Filtername einlesen */
  clear_stat();
  setz_cursor(W_AKT);
  if (*command)
    strcpy(old_fil,command);
  return(*old_fil != '\0');
}

/*****************************************************************************
*
*  Funktion       Block an Filter uebergeben (bl_to_fil)
*  --------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE: Block wurde ersetzt
*                                  FALSE: Text wurde nicht veraendert
*
*  Beschreibung : Es werden die Namen fr zwei tempor„re Dateien generiert.
*                 In die erste wird der Block geschrieben. Dann wird mit
*                 der Funktion system das gewnschte Filterprogramm aufge-
*                 rufen. Die Eingabe wird dabei aus der ersten tempor„ren
*                 Datei gelesen, die Ausgabe wird in die zweite tempor„re
*                 Datei geschrieben. Nachdem system zurckkehrt, wird aus
*                 der zweiten tempor„ren Datei der Block gelesen und in den
*                 Text eingefgt.
*                 Tritt ein Fehler auf, so wird eine Fehlermeldung ausgegeben.
*
*****************************************************************************/

int bl_to_fil()
{
  /* *** interne Daten und Initialisierung *** */
  /* filt_name ist der Name des Filterprogramms */
  static char filt_name[256],
	      filt_name_needs_init = TRUE;
  char        dummy[80],   /* Zum Zusammensetzen des Kommandos */
	      fn_in [12],  /* Name der Temor„rren Datei fr Eingabe */
	      fn_out [12]; /* Name der tempor„ren Datei fr Eingabe */
  FILE        *f;          /* Pointer auf Datei */
  int         ret,         /* Returnwert der Blockfunktionen */
	      old_sc,      /* Alte Cursorposition Spalte */
	      old_tl,      /* Alte Cursorposition Zeile  */
	      error=FALSE; /* Flag, ob Fehler aufgetreten ist */
  block_typ   hilf;  /* Block, der aus der Datei gelesen wird              */

  if (filt_name_needs_init)
  {
     *filt_name = '\0';
     filt_name_needs_init = FALSE;
  }

  mktemp(strcpy(fn_in,"tmpinXXXXXX")); /* Namen fr tempor„re */
  mktemp(strcpy(fn_out,"tmpotXXXXXX")); /* Dateien erzeugen */

  if(!get_f_name(filt_name))  /* Filtername einlesen */
    error = TRUE;
  else
  {
    if(!(f = fopen(fn_in,"w")))  /* Tempor„re Datei zum Schreiben ”ffnen */
    {
      print_err(PROMPT_ERRTMPOPN);
      error = TRUE; /* Fehlermeldung ausgeben */
    }
    else
    {
      print_stat(PROMPT_WORKING);
      /* Blocktext aus dem Text kopieren */
      akt_winp->block.bstart = akt_winp->block.typ == BT_NORMAL? save_normal():save_rechteck();
      schr_block(akt_winp->block.bstart,f); /* und in die Ausgabe-Datei schreiben */
      fclose (f);                           /* Ausgabedatei schlieáen */
      block_free(akt_winp->block.bstart);   /* Block freigeben */

      sprintf(dummy,"%s <%s >%s",filt_name,fn_in,fn_out); /* Kommando erstellen */
      if(system(dummy) == -1) /* Shell starten */
      {
	clear_stat();
	print_err(PROMPT_ERRSHELL);
	error = TRUE;
      }
      else /* Fehler beim Ausfhren des Filters */
      {
	/* Shell-Ausgabe wird als normaler Block eingefuegt */
	if(!(f = fopen(fn_out,"r")))  /* Ausgabedatei zum Lesen ”ffnen */
	{
	  print_err(PROMPT_TMPNOTFND);
	  error = TRUE;
	}
	else
	{
	  old_tl = akt_winp->textline;   /* Cursorposition merken */
	  old_sc = akt_winp->screencol;
	  gotox(akt_winp->block.s_line); /* Cursor an Blockanfang, da lies_block */
	  akt_winp->screencol = akt_winp->block.s_col; /* Blockstart setzt */
	  hilf.typ = akt_winp->block.typ;/* muss fuer lies_block gesetzt werden */
	  ret = lies_block(&hilf,f);     /* Block aus Datei lesen */
	  gotox(old_tl);                 /* Cursorposition restaurieren */
	  akt_winp->screencol = old_sc;
	  fclose (f);                    /* Datei schliessen */
	} /* end of else kein Fehler beim ™ffnen der 2. tempor„ren Datei */
	clear_stat();                  /* filter fertig */
      } /* end of else kein Fehler beim Ausfhren des Filters */
    } /* end of else kein Fehler beim ”ffnen der ersten tempor„ren Datei */
  } /* end of else kein Fehler bei get_f_name */

  unlink(fn_in);  /* tempor„re Dateien l”schen */
  unlink(fn_out);

  if(error)         /* Trat bisher ein Fehler auf, dann Funktion verlassen */
    return(FALSE);

  /* Jetzt Block aus Text loeschen. Dabei werden Marker, Cursor und letzte */
  /* Position automatisch angepasst. */
  if (akt_winp->block.typ == BT_RECHTECK)
    del_rechteck();
  else
    del_normal();
  if(ret) /* Block nicht leer? */
  {
    old_tl = akt_winp->textline;   /* Cursorposition merken */
    old_sc = akt_winp->screencol;
    gotox(akt_winp->block.s_line);       /* Block muss an Position des alten */
    akt_winp->screencol = akt_winp->block.s_col; /* Blocks eingefuegt werden */
    ret = akt_winp->block.typ == BT_NORMAL?ins_normal(&hilf):ins_rechteck(&hilf);
    block_free(hilf.bstart);   /* eingefuegten Block freigeben */

    /* Wurde der Block vor der Cursorposition eingefgt, dann Cursorzeile */
    /* um die L„nge des Blocks (in Zeilen) erh”hen */
    if(old_tl > akt_winp->block.s_line)
      old_tl += akt_winp->block.laenge;
    gotox(old_tl);             /* Cursorposition restaurieren  */
    akt_winp->screencol = old_sc;
    if(!ret) /* Konnte Block nicht eingefuegt werden, dann   */
    {        /* Fehlermeldung ausgeben und Block unmarkieren */
      pe_or(PROMPT_FILTINSRT);
      akt_winp->block.s_line = akt_winp->block.e_line = -1;
    }
    return(TRUE);  /* alter Block ist auf jeden Fall geloescht */
  }
  else      /* bei leerem Block sind s_line und e_line korrekt auf -1 */
  {
    akt_winp->block.e_line = akt_winp->block.s_line = -1;
    print_err(PROMPT_EMPTFILT);
    return(TRUE);
  }
}

/*****************************************************************************
*
*  Funktion       Zeile an Shell uebergeben (to_shell)
*  --------
*
*  Parameter    : line      :
*                   Typ          : char *
*                   Wertebereich : Pointer auf ASCII-Zeichenkette
*                   Bedeutung    : an Shell zu uebergebende Kommandozeile
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE, FALSE
*                   Bedeutung    : TRUE : Alles funktionierte
*                                  FALSE: es wurde keine Ausgabe eingefuegt
*
*  Beschreibung : line wird in eine tempor„re Datei geschrieben. Dahinter
*                 wird das Kommando exit plaziert. Anschlieáend wird eine
*                 Shell gestartet, die die Eingabe aus dieser tempor„ren
*                 Datei liest. Die Shellausgabe wird in eine weitere tempo-
*                 r„re Datei geschrieben. Diese wird anschlieáend zum Lesen
*                 ge”ffnet und der Inhalt als Block in den Text eingefgt.
*
*****************************************************************************/

int to_shell(line)
char *line;
{
  /* *** interne Daten und Initialisierung *** */
  FILE *f;          /* Filepointer fuer Tempor„re Dateien */
  int  ret = TRUE;  /* Returnwert der Funktion            */
  char *shell_name, /* Name des Kommandointerpreters      */
       dummy[80],   /* String zum Zusammenbasteln der Kommandozeile */
       fn_in [12],  /* Name der Temor„rren Datei fr Eingabe */
       fn_out[12];  /* Name der tempor„ren Datei fr Eingabe */

  mktemp(strcpy(fn_out,"tmpotXXXXXX"));

  sprintf(dummy,"%s >%s",line, fn_out); /* Kommando erstellen */
  print_stat(PROMPT_WORKING);
  if(system(dummy) == -1) /* Shell starten */
  {
    clear_stat();
    print_err(PROMPT_ERRSHELL);
  }

  /* Shell-Ausgabe wird als normaler Block eingefuegt */
  if(!(f = fopen(fn_out,"r")))  /* Ausgabedatei zum Lesen ”ffnen */
  {
    clear_stat();
    print_err(PROMPT_TMPNOTFND);
    ret = FALSE;
  }
  else
  {
    akt_winp->block.typ = BT_NORMAL;
    if(!lies_block(&akt_winp->block,f))        /* Block aus Datei lesen */
      ret = FALSE;  /* keine shell-ausgabe */
    else            /* Lesen aus Datei klappte */
      if(!ins_normal(&akt_winp->block))       /* Block einfuegen       */
      { /* klappte das Einfuegen nicht, Block unmarkieren und raus     */
	clear_stat(); /* Statuszeile loeschen */
	ret = FALSE;
	akt_winp->block.s_line = akt_winp->block.e_line = -1;
	pe_or(PROMPT_SHELINSRT);
      }
    block_free(akt_winp->block.bstart);     /* Block freigeben    */
    clear_stat();  /* shell fertig */
  } /* End of else kein Fehler beim ™ffnen der tmpot-Datei */
  unlink(fn_in);  /* Tempor„re Dateien wieder l”schen */
  unlink(fn_out);
  if (f)
    fclose(f);
  return(ret);
}


