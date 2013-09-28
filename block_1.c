/****************************************************************/
/*                                                              */
/*      MODUL:  block_1.c                                       */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - tst_overlap (Ueberlappt Block mit Fenster ?)  */
/*              - in_block (Test ob Zeichen im Block)           */
/*              - dup_block (Block duplizieren)                 */
/*              - shrink_line (Teil aus Zeile loeschen)         */
/*              - adjust_pair (Koordinaten anpassen)            */
/*              - adapt_pos (Cursorposition anpassen)           */
/*              - del_rechteck (rechteckigen Block loeschen)    */
/*              - del_normal (normalen Block loeschen)          */
/*              - save_part (Zeilenteil kopieren)               */
/*              - save_rechteck (rechteckigen Block speichern)  */
/*              - save_normal (normalen Block speichern)        */
/*              - ins_normal (normalen Block in Text einfuegen) */
/*              - ins_rechteck (rechteckigen Block einfuegen)   */
/*              - indent_block (Block ein-/ausruecken)          */
/****************************************************************/

#include "defs.h"

extern char *getenv(),*mktemp(),space,*reserve_mem(),*fastzeichen();
extern marker_typ marker[];

/*****************************************************************************
*
*  Funktion       Test, ob Block sich mit Fenster ueberlappt (tst_overlap)
*  --------
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : TRUE: Teil des Blockes ist in Fenster
*                                        sichtbar
*                                  FALSE: Block nicht sichtbar
*
*  Beschreibung :  Die Blockkoordinaten werden mit den Koordinaten
*                  des aktuellen Fensters verglichen. Ueberlappen sich die
*                  Bloecke, so wird TRUE, sonst FALSE zurueckgegeben.
*
******************************************************************************/

int tst_overlap()
{
  /* *** interne Daten und Initialisierung *** */
  register int xa = akt_winp->block.s_col,  /* Blockkoordinaten */
	       xe = akt_winp->block.e_col,
	       ya = akt_winp->block.s_line,
	       ye = akt_winp->block.e_line;

  if(!(ya<akt_winp->ws_line && ye<akt_winp->ws_line /* Block nicht ueber */
  || ya>=akt_winp->ws_line+akt_winp->dy    /* und nicht unter Ausschnitt */
  && ye>=akt_winp->ws_line+akt_winp->dy))
    if(akt_winp->block.typ == BT_RECHTECK)
    { /* Bei Rechteck koennen noch die Spalten geprueft werden */
      if(!(xa<akt_winp->ws_col && xe<akt_winp->ws_col      /* nicht links     */
      || xa>=akt_winp->ws_col+akt_winp->dx /* und nicht rechts vom Ausschnitt */
      && xe>=akt_winp->ws_col+akt_winp->dx))
	return(TRUE);                      /* also innerhalb */
    }
    else              /* normaler Block, kein Spaltentest  */
      return(TRUE);   /* kein genauer Test, aber gut genug */
  return(FALSE);
}

/*****************************************************************************
*
*  Funktion       Test, ob Zeichen im Block (in_block)
*  --------
*
*  Parameter    : y         :
*                   Typ          : int
*                   Wertebereich : 0-MAX_ANZ_LINES
*                   Bedeutung    : Zeile, in der Zeichen steht.
*
*               : x         :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : Spalte, in der Zeichen steht
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : 0 - B_IN_BLOCK | B_FIRST_LINE | B_LAST_LINE |
*                                  B_BEFORE | B_AFTER | B_LINE | B_FIRST_CHAR |
*                                  B_RIGHT
*                   Bedeutung    : B_IN_BLOCK: Zeichen befindet sich im Block
*                                  B_FIRST_LINE: Zeichen befindet sich in
*                                    der ersten Zeile des Blocks
*                                  B_LAST_LINE: Zeichen befindet sich in der
*                                    letzten Zeile des Blocks
*                                  B_BEFORE: Zeichen steht vor dem Block
*                                  B_AFTER: Zeichen steht hinter dem Block
*                                  B_LINE: Zeichen steht in einer Blockzeile
*                                  B_FIRST_CHAR: Zeichen ist erstes Zeichen
*                                    des Blocks
*                                  B_RIGHT: Zeichen steht in Blockzeile, jedoch
*                                    rechts vom Block.
*
*  Beschreibung : Die Funktion berechnet das Verhaeltnis der uebergebenen
*                 Position relativ zum Block. Dazu werden im Rueckgabewert
*                 bestimmte Bits mit bestimmten Bedeutungen gesetzt bzw.
*                 geloescht. Das geschieht, indem fuer jedes Bit die damit
*                 verbundene Bedingung ausgewertet und das Ergebnis mit dem
*                 Bitwert multipliziert wird. Das Ergebnis wird zum Rueck-
*                 gabewert hinzugeODERt.
*
*****************************************************************************/

int in_block(y,x)
register int y,x;
{
  /* *** interne Daten und Initialisierung *** */
  register block_typ *akt_bl = &akt_winp->block; /* aktueller Block */
  register int       ret=0;                      /* Rueckgabewert   */

  if(!block_defined())  /* Wenn kein Block markiert ist, dann */
    return(0);          /* 0 zurueckgeben */

  ret |= B_FIRST_LINE * (y == akt_bl->s_line);
  ret |= B_LAST_LINE  * (y == akt_bl->e_line);
  ret |= B_BEFORE     * (y <  akt_bl->s_line || (y == akt_bl->s_line && x < akt_bl->s_col));
  ret |= B_AFTER      * (y >  akt_bl->e_line || (y == akt_bl->e_line && x >= akt_bl->e_col));
  ret |= B_LINE       * (y >= akt_bl->s_line &&  y <= akt_bl->e_line);
  ret |= B_FIRST_CHAR * (y == akt_bl->s_line &&  x == akt_bl->s_col);

  if(akt_bl->typ == BT_RECHTECK)
  {
    ret |= B_RIGHT * ((ret & B_LINE) && x >= akt_bl->e_col);
    ret |= B_IN_BLOCK * ((ret & B_LINE)
	   && x >= akt_bl->s_col && x < akt_bl->e_col);
  }
  else
  {
    ret |= B_RIGHT * ((ret & B_LAST_LINE) && x >= akt_bl->e_col);
    ret |= B_IN_BLOCK * ((y == akt_bl->s_line && x >= akt_bl->s_col
	   && (y != akt_bl->e_line || x < akt_bl->e_col))
	   || (y > akt_bl->s_line && y < akt_bl->e_line)
	   || (y == akt_bl->e_line && x < akt_bl->e_col && (y != akt_bl->s_line
	   || x >= akt_bl->s_col)));
  }
  return(ret);
}

/*****************************************************************************
*
*  Funktion       Block duplizieren (dup_block)
*  --------
*
*  Parameter    : old_bl    :
*                   Typ          : block_typ *
*                   Wertebereich : Pointer auf block_typ
*                   Bedeutung    : zu kopierender Block
*
*  Ergebnis     :
*                   Typ          : block_typ *
*                   Wertebereich : Pointer auf block_typ
*                   Bedeutung    : Kopie des Blocks
*
*  Beschreibung : Die Funktion kopiert den uebegebenen Block und gibt einen
*                 Pointer auf die Kopie zurueck
*
*****************************************************************************/

block_typ *dup_block(old_bl)
block_typ *old_bl;
{
  /* *** interne Daten und Initialisierung *** */
  /* new_bl zeigt auf die Struktur des neuen Blockes */
  block_typ          *new_bl = (block_typ*) reserve_mem(sizeof(block_typ));
  register bzeil_typ *nz,  /* Aktuelle Zeile im neuen Block */
		     *az;  /* Aktuelle Zeile im alten Block */

  memcpy(new_bl,old_bl,sizeof(block_typ));   /* Blockstruktur duplizieren */
  /* Falls Block mindestens eine Zeile hat, Platz dafuer reservieren */
  if (az = old_bl->bstart)
    new_bl->bstart = nz = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ));
  else
    new_bl->bstart = NULL;
  while (az)  /* Alle Blockzeilen in Zeilenliste des neuen Blocks sichern */
  {
    nz->text = save_text(az->text);
    if (az->next)
      nz->next = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ));
    else
      nz->next = NULL;
    nz = nz->next;
    az = az->next;
  }
  return (new_bl);
}

/*****************************************************************************
*
*  Funktion       Teil aus Zeile loeschen (shrink_line)
*  --------
*
*  Parameter    : anf       :
*                   Typ          : int
*                   Wertebereich : 0 - MAXLENGTH-1
*                   Bedeutung    : Anfangsspalte
*
*                 ende      :
*                   Typ          : int
*                   Wertebereich : anf - MAXLENGTH-1
*                   Bedeutung    : Erstes nicht mehr zu loeschendes Zeichen
*
*  Beschreibung : Die Funktion loescht in der aktuellen Zeile den Bereich von
*                 anf bis ende (soweit moeglich).  Besteht die Zeile danach
*                 nur noch aus Spaces, so wird die Textpointer freigegeben
*                 ud durch NULL ersetzt (gemaess Konvention).
*
*****************************************************************************/

void shrink_line(anf,ende)
int anf,ende;
{
  /* *** interne Daten *** */
  int  len;    /* Laenge des verbleibenden Strings */
  char *anfp,  /* Pointer auf erstes nicht zu uebernehmendes Zeichen */
       *endep; /* Pointer auf letztes nicht zu uebernehmendes Zeichen */

  /* Wenn Zeichen wegfallen (anf!=ende), dann wird anfp der Zeiger */
  /* auf das entsprechende Zeichen zugewiesen. Liegt anfp hinter   */
  /* dem Zeilenende (anfp==&space), geschieht nichts.              */
  if(anf != ende && (anfp = fastzeichen(anf)) != &space)
  {
    /* Sonst wird der Zeiger auf das zu ende gehoerende Zeichen ermittelt.    */
    if((endep = fastzeichen(ende)) == &space) /* Liegt ende hinte Zeilenende, */
      *anfp = '\0';              /* so wird der String einfach abgeschnitten. */
    else                         /* Sonst wird der hintere zu uebernehmende   */
      fwdcpy(anfp,endep);        /* Teil direkt an den vorderen kopieret.     */

    /* Hat die Zeile eine Laenge ungleich 0 und besteht sie nicht nur aus */
    /* Blanks, dann wird dafuer der noetige Speicher alloziert.           */
    if(akt_winp->alinep->text 
    && (len=strlen(akt_winp->alinep->text)) && strspn(akt_winp->alinep->text," ") != len)
    {
      if(!(akt_winp->alinep->text = realloc(akt_winp->alinep->text,len+1)))
	no_mem_err();            /* Kein Speicher, Fehlermeldung ausgeben */
    }
    else                   /* Zeile leer oder nur Blanks, Zeile freigeben */
    {
      line_free(akt_winp->alinep->text);
      akt_winp->alinep->text = NULL;
    }
  }
}
 
/*****************************************************************************
*
*  Funktion       Koordinaten anpassen (adjust_pair)
*  --------
*
*  Parameter    : y         :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer-Variable
*                   Bedeutung    : anzupassende Zeilennummer
*
*                 x         :
*                   Typ          : int *
*                   Wertebereich : Pointer auf Integer-Variable
*                   Bedeutung    : anzupassende Spaltennummer
*
*                 modus     :
*                   Typ          : int
*                   Wertebereich : AP_INSERT, AP_DELETE
*                   Bedeutung    : AP_INSERT: Block wurde eingefuegt
*                                  AP_DELETE: Block soll geloescht werden
*
*  Beschreibung : Falls sich durch ein Loeschen des Blockes (modus == AP_DELETE)
*                 die uebergebenen Koordinaten veraendern wuerden, so
*                 werden diese entsprechend angepasst.
*                 Haben sich durch das Einfuegen des Blockes (modus == AP_INSERT)
*                 die uebergebenen Koordinaten veraendert, so werden diese
*                 entsprechend angepasst.
*
*****************************************************************************/

void adjust_pair(y,x,modus)
int *y,*x,modus;
{
  /* *** interne Daten und Initialisierung *** */
  int       ib = in_block(*y,*x);   /* Position relativ zum Block */
  block_typ *bl = &akt_winp->block; /* Zeiger auf aktuellen Block */

  if(bl->typ == BT_NORMAL)    /* Normaler Block ? */
  {
    if(ib & B_IN_BLOCK)      /* Position jetzt innerhalb des Blocks ? */
      if(modus == AP_DELETE) /* wird Block geloescht ? */
      {
	*y = bl->s_line; /* falls Koordinaten im Block, an Blockanfang setzen */
	*x = bl->s_col;
      }
      else                   /* Block wird eingefuegt */
      {
	*y += bl->e_line - bl->s_line; /* neue Position ist um Blockhoehe nach */
	if(ib & B_FIRST_LINE)          /* unten gerutscht.Falls Koor in erster */
	  *x += bl->e_col - bl->s_col; /* Blockzeile war, auch Spaltenposition */
      }                                /* anpassen */
    else                     /* Position war nicht innerhalb des Blocks */
      if(ib & B_AFTER)       /* Falls dahinter, dann */
	if(modus == AP_DELETE) /* beim Loeschen Blockhoehe subtrahieren */
	{ /* wenn letzte und erste Zeile zusammen zu lang, dann wird noetige */
	  *y -= bl->e_line - bl->s_line; /* anpassung in del_normal gemacht */
	  if(ib & B_LAST_LINE) /* Wenn Position in der letzten Zeile hinter */
	    if(ib & B_RIGHT)   /* dem Block liegt, dann X anpassen:         */
	      *x += bl->s_col - bl->e_col;
	    else               /* Wenn in letzter Zeile im Block, dann X    */
	      *x = bl->s_col;  /* auf Blockstart X setzen                   */
	}
	else                 /* Beim Einfuegen Blockhoehe auf Y addieren */
	  *y += bl->e_line - bl->s_line;
  }
  else           /* rechteckiger Block, y-Koordinaten werden nicht veraendert */
    if(ib & B_LINE)           /* Befindet sich Position in einer Blockzeile ? */
      if(modus == AP_DELETE)         /* Beim Loeschen falls rechts vom Block, */
      {                              /* Blockbreite von x-Koordinate abzie-   */
	if(ib & B_RIGHT)             /* hen.                                  */
	  *x -= bl->e_col-bl->s_col;
	else                         /* Falls innerhalb des Block, x-Koordina-*/
	  if(ib & B_IN_BLOCK)        /* te auf Blockanfang X setzen           */
	    *x = bl->s_col;
      }
      else                           /* Soll der Block eingefuegt werden und  */
	if(*x >= bl->s_col)          /* die Position liegt rechts der Start-  */
	  *x += bl->e_col - bl->s_col; /* spalte, dann Breite zu X addieren   */
}

/*****************************************************************************
*
*  Funktion       Positionen anpassen (adapt_pos)
*  --------
*
*  Parameter    : modus     :
*                   Typ          : int
*                   Wertebereich : AP_INSERT, AP_DELETE
*                   Bedeutung    : AP_INSERT: Block wurde eingefuegt
*                                  AP_DELETE: Block soll geloescht werden
*
*  Beschreibung : Entsprechend dem Modus werden Cursorposition, Marker und
*                 letzte Position mit der Funktion adjust_pair angepasst.
*
*****************************************************************************/

void adapt_pos(modus)
int modus;
{
  /* *** Interne Daten und Initialisierung *** */
  int i = akt_winp->textline; /* Zwischenspeicher fuer akt. Zeilennummer */

  /* Zuerst Cursorposition: */
  adjust_pair(&i,&akt_winp->screencol,modus); /* nicht &textline, damit */
  gotox(i);              /* textline und alinep nicht desynchronisieren */
  for(i=0;i<ANZ_MARKER;i++)  /* Alle Markerpositionen fuer aktuelles    */
    if(akt_winp->wini == marker[i].window)          /* Fenster anpassen */
      adjust_pair(&marker[i].line,&marker[i].col,modus);
  /* Letzte Position anpassen: */
  adjust_pair(&akt_winp->lastline,&akt_winp->lastcol,modus);
}

/*****************************************************************************
*
*  Funktion       rechteckigen Block loeschen (del_rechteck)
*  --------
*
*  Beschreibung : Der aktuelle Block wird aus der Textstruktur entfernt und
*                 evtl. die Cursorpositon angepasst.
*
*****************************************************************************/

void del_rechteck()
{
  /* *** Interne Daten und Initialisierung */
  zeil_typ  *old_line; /* Zwischenspeicher fuer Zeiger auf aktuelle Zeile */
  int       old_num,   /* Zwischenspeicher fuer aktuelle Zeilennummer     */
	    count;     /* Z„hler fr gel”schte Zeilen                     */
  block_typ *bl = &akt_winp->block; /* Zeiger auf aktuellen Block         */

  adapt_pos(AP_DELETE);   /* Cursor, Marker und letzte Position anpassen */
  old_num = akt_winp->textline;   /* Cursorposition erst jetzt speichern */
  old_line = akt_winp->alinep;

  /* Aus allen Blockzeilen den zum Block gehoerenden Teil loeschen
     Beachte: count ist n”tig, da das Kriterium textline<=bl_eline
	      nicht ausreicht, wenn Block sich bis in letzte Zeile
	      erstreckt. */
  count = bl->e_line - akt_winp->textline;
  for(gotox(bl->s_line); count-- >= 0; down())
    shrink_line(bl->s_col,bl->e_col);
  akt_winp->changeflag = TRUE;  /* Text als geaendert markieren */
  akt_winp->alinep = old_line;  /* Alte Cursorposition wieder   */
  akt_winp->textline = old_num; /* herstellen                   */
}

/*****************************************************************************
*
*  Funktion       normalen Block loeschen (del_normal)
*  --------
*
*  Beschreibung : Der aktuelle normale Block wird aus der Textstruktur
*                 entfernt und evtl. die Cursorpositon angepasst.
*
*****************************************************************************/

void del_normal()
{
  /* *** interne Daten und Initialisierung *** */
  int                anz_del, /* Anzahl zu loeschender Zeilen   */
		     len,     /* Laenge der neuen ersten Zeile  */
		     old_num, /* Aktuelle Zeilennummer          */
		     old_sc,  /* Zwischenspeicher Cursorspalte  */
		     fll;     /* Laenge der uebriggebliebenen   */
			      /* ersten Blockzeile              */
  register block_typ *bl = &akt_winp->block; /* aktueller Block */
  char               *ez_anf; /* Zeiger auf Blockanfang         */

  adapt_pos(AP_DELETE); /* Cursor, Marker und letzte Position anpassen */
  old_num = akt_winp->textline; /* Zeilennummer merken und zum Block- */
  gotox(bl->s_line);            /* start gehen */
  if(!bl->laenge)               /* Block innerhalb einer Zeile */
    shrink_line(bl->s_col,bl->e_col); /* Blockteil der Zeile loeschen */
  else
  {                          /* Block geht ueber mehrere Zeilen */
    anz_del = bl->laenge-1;  /* letzte Zeile nicht vollstaendig loeschen */
    if(bl->s_col)            /* erste Zeile teilweise loeschen */
    {
      /* Zeiger auf erstes Blockzeichen berechnen und testen, ob es hinter */
      /* dem Zeilenende liegt (== &space)                                  */
      if ((ez_anf = fastzeichen(bl->s_col)) != &space)
      {
	*ez_anf = '\0';  /* Zeile am Blockanfang abschneiden */
	/* Wenn neue Laenge der Zeile == 0 oder Zeile nur noch aus Blanks */
	/* besteht, dann Zeile freigeben. Sonst Platz neu allozieren      */
	if(akt_winp->alinep->text
	&& (len=strlen(akt_winp->alinep->text))
	&& strspn(akt_winp->alinep->text," ") != len)
	{
	  if(!(akt_winp->alinep->text = realloc(akt_winp->alinep->text,len + 1)))
	    no_mem_err(); /* Kein Speicherplatz, Fehlermeldung, Abbruch */
	}
	else
	{
	  line_free(akt_winp->alinep->text);
	  akt_winp->alinep->text = NULL;
	}
      }
      down();  /* In 2. Blockzeile gehen */
    }
    else            /* Wenn Block am Anfang einer Zeile beginnt, */
      anz_del++;    /* dann erste Zeile auch loeschen            */
    for(;anz_del;anz_del--)
      del_line(IGNORE_COORS);   /* blockkoors werden nicht veraendert */
    shrink_line(0,bl->e_col);   /* Blockteil der letzten Zeile loeschen */
    if(bl->s_col)               /* Wenn Block nicht am Anfang einer Zeile */
    {                           /* anfaengt, muss man die Zeilentruemmer */
      up();                     /* zusammenfuegen.                      */
      if(join(IGNORE_COORS)==J_TOOLONG) /* Klappte Join nicht ? */
      { /* Dann ans Zeilenende stellen und Anpassung wie bei newline   */
	/* vornehmen, da adapt_pos alles um eine Zeile zuviel anpasste */
	if(akt_winp->screencol > (fll=fastll(akt_winp->alinep->text))
	&& akt_winp->textline == old_num) /* Wenn Cursor in letzter Block- */
	  /* zeile hinter dem Block stand, dann Cursorspalte anpassen */
	  akt_winp->screencol -= fll;
	old_num++; /* Alte Cursorzeile sowieso anpassen */
	old_sc = akt_winp->screencol; /* Cursorspalte merken */
	akt_winp->screencol = fll;
	nl_blockadapt();              /* Anpassungen vornehmen */
	akt_winp->screencol = old_sc; /* Cursorspalte restaurieren */
      }
    }
  }
  akt_winp->changeflag = TRUE;  /* Text als geaendert markieren */
  gotox(old_num);               /* Alte Zeile aufsuchen         */
}

/*****************************************************************************
*
*  Funktion       Zeilenteil kopieren (save_part)
*  --------
*
*  Parameter    : z         :
*                   Typ          : bzeil_typ *
*                   Wertebereich : Pointer auf Blockzeile
*                   Bedeutung    : aktuelle Blockzeile
*
*               : anf       :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : erstes zu kopierendes Zeichen
*
*               : ende      :
*                   Typ          : int
*                   Wertebereich : 0-MAXLENGTH
*                   Bedeutung    : erstes nicht mehr zu kopierendes Zeichen
*
*  Beschreibung : Eine neue Blockzeile wird erzeugt und mit der aktuellen
*                 verknuepft. In die neue Zeile wird der entsprechende Zeilen-
*                 ausschnitt gespeichert (soweit moeglich). Ist der Ausschnitt
*                 leer, so wird ein NULL-Pointer im text-Element eingetragen.
*
*****************************************************************************/

bzeil_typ *save_part(z,anf,ende)
register bzeil_typ *z;
int anf,ende;
{
  char *anfp,               /* Zeiger auf erstes zu uebernehmendes Zeichen  */
       *endep,              /* Zeiger auf letztes zu uebernehmendes Zeichen */
       buff[3*MAXLENGTH+1]; /* Puffer fuer zu kopierenden Teil              */

  /* Ein neues Element in der Blockstruktur eintragen */
  z = z->next = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ));
  z->next = NULL;

  /* Pointer auf erstes zu uebernehmendes Zeichen ermitteln und testen, ob */
  /* es hinter dem Ende der aktuellen Zeile liegt.                         */
  if ((anfp = fastzeichen(anf)) == &space)
    z->text = NULL;     /* Liegt es dahinter, keinen Speicher reservieren */
  else    /* Sonst Ende ermitteln, testen, ob es hinter Zeilenende liegt. */
  {
    if ((endep = fastzeichen(ende)) == &space)              /* Dahinter ? */
      z->text = save_text(anfp);        /* dann alles ab anfp uebernehmen */
    else               /* Sonst zu uebernehmenden Teil in Puffer kopieren */
    {
      strncpy(buff,anfp,endep-anfp);
      buff[endep-anfp] = '\0';   /* Pufferende markieren */
      z->text = save_text(buff); /* Puffer in Blocktext uebernehmen */
    }
  }
  return (z);
}

/*****************************************************************************
*
*  Funktion       rechteckigen Block abspeichern (save_rechteck)
*  --------
*
*  Ergebnis     :
*                   Typ          : bzeil_typ*
*                   Wertebereich : Pointer auf bzeil_typ
*                   Bedeutung    : Anfang der verketteten Liste der
*                                  Blockzeilen
*
*  Beschreibung : Der markierte rechteckige Block wird in einer verketteten
*                 Liste, deren Anfang zurueckgegeben wird, abgespeichert.
*                 Das laenge-Element von akt_winp->block wird auf die Zeilen-
*                 anzahl gesetzt.
*
*****************************************************************************/

bzeil_typ *save_rechteck()
{
  /* *** interne Daten und Initialisietung *** */
  /* z zeigt auf aktuelle Blockzeile */
  bzeil_typ *z = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ)),
	    *anf;         /* Zeiger auf leeres Dummy-Element   */
  int       i,            /* Anzahl noch zu kopierender Zeilen */
	    old_tl = akt_winp->textline; /* Alte Zeilennummer  */
  zeil_typ *old_ap = akt_winp->alinep;   /* Alter Zeilenzeiger */

  i = akt_winp->block.laenge = akt_winp->block.e_line - akt_winp->block.s_line;
  (anf=z)->next = NULL;       /* Erstes Element ist leeres Element, wird wieder geloescht */

  /* Aus jeder zum Block gehoerigen Zeile wird der entsprechende Teil */
  /* ausgeschnitten und in die Blockstruktur kopiert.                 */
  for (gotox(akt_winp->block.s_line); i>=0; i--,down())
    z = save_part(z,akt_winp->block.s_col,akt_winp->block.e_col);
  z = anf->next;
  free (anf);   /* zuviel alloziertes Element wieder freigeben */
  akt_winp->alinep = old_ap;   /* Cursorposition restaurieren  */
  akt_winp->textline = old_tl;
  return (z);
}

/*****************************************************************************
*
*  Funktion       normalen Block abspeichern (save_normal)
*  --------
*
*  Ergebnis     :
*                   Typ          : bzeil_typ*
*                   Wertebereich : Pointer auf bzeil_typ
*                   Bedeutung    : Anfang der verketteten Liste der
*                                  Blockzeilen
*
*  Beschreibung : Der markierte normale Block wird in einer verketteten
*                 Liste, deren Anfang zurueckgegeben wird, abgespeichert.
*                 Das laenge-Element von akt_winp->block wird auf die Zeilen-
*                 anzahl gesetzt.
*
*****************************************************************************/

bzeil_typ *save_normal()
{
  /* *** interne Daten und Initialisierung *** */
  /* z zeigt auf die aktuelle Zeile in der Blocktextliste */
  register bzeil_typ *z = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ)),
		     *anf = z;   /* Erste Zeile in Blocktextliste */
  char               *ez_anf;    /* Erstes Zeichen der Blocks     */
  int                old_tl = akt_winp->textline; /* Alte Zeilennummer  */
  zeil_typ           *old_ap = akt_winp->alinep;  /* Alter Zeilenzeiger */
  block_typ          *bl = &akt_winp->block;      /* aktueller Block    */

  bl->laenge = bl->e_line-bl->s_line; /* Anzahl der Blockzeilen berechnen */
  z->next = NULL; /* Erstes Element ist leeres Element, wird wieder geloescht */
  gotox(bl->s_line);  /* Zum Blockanfang gehen */
  if(bl->e_line == bl->s_line)  /* Block innerhalb einer Zeile */
    /* Dann neue Zeile anlegen und entsprechenden Teil rauskopieren */
    z = save_part(z,bl->s_col,bl->e_col);
  else
  {                 /* Wenn der Block ueber mehrere Zeilen geht, */
    if(bl->s_col)   /* und der Block nicht in Spalte 0 anfaengt, */
    {               /* dann eine neue Zeile anlegen.             */
      z->next = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ));
      z = z->next;
      z->next = NULL;
      if ((ez_anf = fastzeichen(bl->s_col)) != &space)
	z->text = save_text(ez_anf); /* Falls Blockanfang nicht hinter */
      else /* Zeilenende, dann Blockteil der Zeile abspeichern, sonst  */
	z->text = NULL;                   /* Zeile als leer markieren. */
      down();  /* Zur naechsten blockzeile gehen */
    }
    while(akt_winp->textline < bl->e_line)  /* Alle komplett zum Block */
    {                      /* gehoerigen Zeilen an die Liste anhaengen */
      z->next = (bzeil_typ*) reserve_mem(sizeof(bzeil_typ));
      z = z->next;
      z->next = NULL;
      z->text = save_text(akt_winp->alinep->text);
      down();
    }
    z = save_part(z,0,bl->e_col); /* Blockteil der letzten Blockzeile */
  }                               /* in Liste eintragen               */
  z = anf->next;
  free (anf);   /* zuviel alloziertes Element wieder freigeben */
  akt_winp->alinep = old_ap;   /* Cursorposition restaurieren */
  akt_winp->textline = old_tl;
  return (z);
}

/*****************************************************************************
*
*  Funktion       abgespeicherten norm. Block in Text einfuegen (ins_normal)
*  --------
*
*  Parameter    : bl       :
*                   Typ          : block_typ *
*                   Wertebereich : Pointer auf block_typ
*                   Bedeutung    : Einzufuegender Block
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : TRUE = Einfuegen war moeglich
*                                  FALSE = Einfuegen war nicht moeglich
*
*  Beschreibung : Der in der mit bl->bstart beginnenden Liste abgespeicherte
*                 Block wird an der aktuellen Position eingefuegt, wenn dazu
*                 genuegend Platz ist. Der eingefuegte Block wird
*                 anschliessend zum aktuellen.
*                 Die in den Text eingefuegten Textpointer der Blockzeilen
*                 werden in der Blockstruktur auf NULL gesetzt, damit sie
*                 nicht von block_free freigegeben werden.
*
*****************************************************************************/

int ins_normal(bl)
block_typ *bl;
{
  /* *** interne Daten und Initialisierung *** */
  bzeil_typ     *akt_zeile = bl->bstart;      /* Aktuelle Blockzeile    */
  register int  fll, /* aktuelle Zeilenlaenge                           */
		i,   /* Zaehler zum Auffuellen einer Zeile mit Blanks   */
		pos, /* Startspalte des Blocks in der ersten Zeile      */
		old_tl = akt_winp->textline,  /* Alte Zeilennummer      */
		old_sc = akt_winp->screencol; /* Alte Spaltennummer     */
  int           line_len,     /* Neue Laenge der ersten Blockzeile      */
		bline_len,    /* Laenge der Blockzeile                  */
		tmp_sline,    /* Zwischenspeicher fuer echten Block-    */
		tmp_scol,     /* start (Zeile / Spalte)                 */
		tmp_eline,    /* Zwischenspeicher fuer echtes Block-    */
		tmp_ecol;     /* ende (Zeile / Spalte)                  */
  char          *fzsc,        /* Zeiger auf erstes Blockzeichen         */
		*rest = NULL, /* Zeiger auf Rest der Zeile, in die      */
			      /* Block eingefuegt wird, nach Splitten   */
		buff[3*MAXLENGTH+1]; /* Puffer fuer Zusammenfuegen der  */
				     /* ersten und letzten Blockzeile   */

  if (!akt_zeile || bl->laenge<0)       /* Falls Block leer, fertig */
    return (TRUE);
  if (bl->laenge + akt_winp->maxline + 2 >= MAX_ANZ_LINES) /* Noch Platz ? */
    return (FALSE); /* eee evtl. werden 2 Zeilen verschenkt */
  check_buff(); /* Evtl. noch im Puffer enthaltene Daten in Text aufnehmen */
  akt_winp->changeflag = TRUE;  /* irgendetwas muss eingefuegt werden */

  if(akt_winp->maxline <= -1)   /* Falls Text leer, eine Zeile erzeugen */
    koppel_line(IGNORE_COORS);

  bline_len = fastll(akt_zeile->text);  /* Laenge der aktuellen Zeile und */
  fll = fastll(akt_winp->alinep->text); /* der ersten Blockzeile merken   */

  /* Die Laenge der neuen Zeile ergibt sich aus der Summe von Cursorspalte */
  /* und Laenge der ersten Blockzeile.                                     */
  line_len = akt_winp->screencol + bline_len;

  bl->s_line = tmp_sline = akt_winp->textline; /* Blockkoordinaten setzen  */
  bl->s_col  = tmp_scol  = akt_winp->screencol;
  if (akt_winp->alinep->text)           /* Text in aktueller Zeile? */
  {
    /* Position relativ zum Zeilenanfang ermitteln, ab der 1. Blockzeile     */
    /* eingefuegt werden soll. Steht man hinter dem Zeilenende, dann wird    */
    /* direkt hinter dem Zeilenende mit der Einfuegung begonnen, sonst er-   */
    /* gibt sich die Position aus der Differenz des Zeigers auf das aktuelle */
    /* Zeichen und dem Zeiger auf den Zeilenanfang.                          */

    if((fzsc = fastzeichen(akt_winp->screencol)) == &space)
      pos = strlen(akt_winp->alinep->text);
    else
      pos = fzsc - akt_winp->alinep->text;
    strncpy(buff,akt_winp->alinep->text,pos);   /* Teil der Zeile vor Block  */
						/* in Puffer kopieren        */
    if(fll > akt_winp->screencol)          /* Blockeinfuegen splittet Zeile, */
      rest = save_text(akt_winp->alinep->text+pos); /* Rest entsteht, falls  */
    else                            /* Cursor nicht hinter Zeilenende stand. */
      rest = NULL;
  }
  else           /* aktuelle Zeile leer: */
  {              /* pos wird zwar auch auf 0 gesetzt, wenn Cursor nicht am    */
    pos = 0;     /* Zeilenanfang steht, wird jedoch durch Einfuegen der Spaces*/
    rest = NULL; /* bis zur Cursorposition wieder korrekt angepasst. (s.u.)   */
    buff[0]='\0';/* Kein Rest bei leerer Zeile, Puffer leer markieren         */
  }
  if (line_len <= MAXLENGTH) /* wird Zeile durch Anfuegen der */
  {                          /* 1. Blockzeile zu lang ?       */
    for (i = fll;i<akt_winp->screencol;i++)
      buff[pos++] = ' '; /* evtl. bis Cursorposition mit Spaces auffuellen   */
    if (akt_zeile->text)                /* ueberhaupt Text in 1. Blockzeile? */
      strcpy(&buff[pos],akt_zeile->text); /* 1. Blockzeile an Teil der aktu- */
    else                                 /* ellen Zeile im Puffer anhaengen. */
      buff[pos]='\0'; /* kein Text in 1. Blockz., aktuelle Zeile ist zu Ende */
    akt_zeile = akt_zeile->next;        /* Zur 2. Blockzeile */
  }
  else               /* Zeile wuerde zu lang werden, daher 1. Blockzeile in  */
  {                  /* eigener Zeile einfuegen. Neuer Block faengt daher    */
    tmp_sline++;     /* eine Zeile spaeter an                                */
    tmp_scol = 0;
  }
  line_free(akt_winp->alinep->text);        /* Aktuelle Zeile durch neue   */
  akt_winp->alinep->text = save_text(buff); /* Zeile ersetzen.             */

  /* Alle restlichen Blockzeilen werden in die Textstruktur eingefuegt */
  while (akt_zeile)
  {
    koppel_line(IGNORE_COORS);                /* damit die uebernommene Zeile*/
    akt_winp->alinep->text = akt_zeile->text; /* nicht von block_free freige-*/
    akt_zeile->text = NULL;                   /* geben wird, text auf NULL.  */
    akt_zeile = akt_zeile->next;
  }
  bl->e_line = tmp_eline = akt_winp->textline; /* Blockendkoordinaten setzen  */
  bl->e_col = tmp_ecol = fastll(akt_winp->alinep->text);

  /* Wenn durch das Splitten der ersten Zeile ein Rest entstanden ist, dann */
  /* wird versucht, den Rest an die letzte Blockzeile anzuhaengen. Wuerde   */
  /* die Zeile dadurch zu lang, wird der Rest in eine eigene Zeile geschrie-*/
  /* ben.                                                                   */
  if(rest)
    if (bl->e_col + (fll = fastll(rest)) <= MAXLENGTH) /* nicht zu lang ?   */
      if (akt_winp->alinep->text)
      {
	strcpy(buff,akt_winp->alinep->text); /* beide Teile in buff zusam-  */
	strcat(buff,rest);                   /* menkopieren.                */
	free(rest);                          /* Rest freigeben              */
	free(akt_winp->alinep->text);        /* Neu entstandene Zeile in    */
	akt_winp->alinep->text = save_text(buff); /* Textstruktur eintragen */
      }
      else                                   /* War die aktuelle Zeile leer,*/
	akt_winp->alinep->text = rest;       /* dann einfach Rest eintragen */
    else                                     /* Zeile waere zu lang geworden*/
    {
      koppel_line(IGNORE_COORS);             /* Neue Zeile erzeugen         */
      akt_winp->alinep->text = rest;         /* Rest separat abspeichern    */
      bl->e_line++;    /* Fuer adapt_pos Blockende temporaer "manipulieren" */
      bl->e_col = 0;
    }

  /* Eingefgten Block zum aktuellen machen, damit adapt_pos mit den */
  /* richtigen Koordinaten rechnet.                                  */
  memcpy(&akt_winp->block,bl,sizeof(block_typ));
  adapt_pos(AP_INSERT);          /* Marker etc. anpassen */
  bl->s_line = tmp_sline;        /* Echte Blockstart- und Endkoordinaten */
  bl->s_col  = tmp_scol;         /* restaurieren */
  bl->e_line = tmp_eline;
  bl->e_col  = tmp_ecol;
  /* Jetzt angepaáte Werte in den aktuellen Block bernehmen */
  memcpy(&akt_winp->block,bl,sizeof(block_typ));
  gotox (old_tl);                /* Cursorposition restaurieren */
  akt_winp->screencol = old_sc;  /* Cursorspalte bleibt gleich */
  check_underl();   /* evtl. wegen Unterstr. screencol und textcol anpassen */
  return (TRUE);
}

/*****************************************************************************
*
*  Funktion    abgespeicherten recht. Block in Text einfuegen (ins_rechteck)
*  --------
*
*  Parameter    : bl       :
*                   Typ          : block_typ *
*                   Wertebereich : Pointer auf block_typ
*                   Bedeutung    : Einzufuegender Block
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : TRUE = Einfuegen war moeglich
*                                  FALSE = Einfuegen war nicht moeglich
*
*  Beschreibung : Der in der mit bl->bstart beginnenden Liste abgespeicherte
*                 Block wird an der aktuellen Position eingefuegt, wenn dazu
*                 genuegend Platz ist. Der eingefuegte Block wird
*                 anschliessend zum aktuellen.
*
*****************************************************************************/

int ins_rechteck(bl)
register block_typ *bl;
{
  /* *** interne Daten und Initialisierung *** */
  register bzeil_typ *akt_zeile = bl->bstart;      /* Aktueller Block    */
  register int       i,        /* Zaehler zum Auffuellen mit Blanks      */
		     diff;     /* Anzahl einzufuegender Zeilen           */
  int                line_len, /* Laenge einer Textzeile                 */
		     old_tl = akt_winp->textline,  /* alte Zeilennummer  */
		     old_sc = akt_winp->screencol; /* alte Spaltennummer */
  zeil_typ           *old_ap = akt_winp->alinep;   /* alter Zeilenzeiger */
  register char      *tp,      /* Index fuer Textpuffer (buff)           */
		     *fzsc;    /* Zeiger auf Zeichen, wo eingefuegt wird */
  char               buff[3*MAXLENGTH+1]; /* Puffer zum Zusammenfuegen   */
					  /* der Zeilenbruchstuecke      */

  if (!akt_zeile || bl->laenge<0)          /* Falls Block leer, fertig */
    return (TRUE);
  check_buff();      /* Evtl. Pufferinhalt in Textstruktur uebernehmen */
  if(akt_winp->maxline <= -1)  /* Falls Text leer, eine Zeile erzeugen */
    koppel_line(IGNORE_COORS);

  /* Es werden alle Zeilen, in die Blockteile eingefuegt werden, darauf */
  /* untersucht, ob durch die Einfuegung die maximale Zeilenlaenge      */
  /* ueberschritten wuerde. Stellt man bei der Zeilenlaengenberechnung  */
  /* fest, dass der Block hinter dem Zeilenende eingefuegt werden soll, */
  /* dann wird als Zeilenlaenge die Cursorposition verwendet.           */
  do
  {
    if((line_len = fastll(akt_winp->alinep->text)) < akt_winp->screencol)
      line_len = akt_winp->screencol;
    if (line_len + bl->e_col - bl->s_col > MAXLENGTH)
      return (FALSE);
  }while ((akt_zeile = akt_zeile->next) && down());

  /* Falls zu viele Zeilen eingefuegt werden muessten, FALSE zurueckgeben. */
  if ((diff=old_tl+bl->laenge-akt_winp->textline) + akt_winp->maxline
  >= MAX_ANZ_LINES)
    return (FALSE);
  akt_winp->changeflag = TRUE; /* Text als geaendert markieren */
  while (diff--)
    koppel_line(IGNORE_COORS); /* Falls Text zu kurz, fehlende Zeilen anhaengen */
  akt_winp->textline = old_tl;                /* Cursorposition restaurieren */
  akt_winp->alinep = old_ap;
  akt_winp->screencol = old_sc; /* auch screencol, da von koppel_line geloescht */

  bl->e_col += akt_winp->screencol - bl->s_col; /* Blockkoordinaten eintragen */
  bl->s_col  = akt_winp->screencol;
  bl->e_line = bl->s_line = akt_winp->textline;

  /* Jetzt Pufferzeilen in Textzeilen hineinkopieren */
  akt_zeile = bl->bstart;
  do
  {
    if ((fzsc = fastzeichen(akt_winp->screencol)) == &space)
    { /* Falls nach Zeilenende angehaengt werden muss */
      if(akt_winp->alinep->text)
      {
	strcpy (buff,akt_winp->alinep->text); /* Ganze Zeile in Puffer kopieren */
	tp = buff+strlen(buff);               /* tp hinter Ende setzen */
      }
      else
	tp = buff;       /* Falls Zeile leer, nichts kopieren */
      for (i=fastll(akt_winp->alinep->text);i<akt_winp->screencol;i++)
	*tp++ = ' ';    /* Puffer mit Spaces bis zum Blockanfang auffuellen */
      *tp = '\0';       /* Puffer mit '\0' abschliessen */
    }
    else /* Falls nicht ans Zeilenende anhaengen, ersten Teil der Zeile kopieren */
    {
      strncpy(buff,akt_winp->alinep->text,fzsc-akt_winp->alinep->text);
      buff[fzsc-akt_winp->alinep->text] = '\0';
      tp = buff+(fzsc-akt_winp->alinep->text);
    }
    if (akt_zeile->text)            /* Steht etwas in der Pufferzeile */
    {
      strcat(buff,akt_zeile->text); /* Ja, dann an Puffer anhaengen */
      tp += strlen(akt_zeile->text);
    }
    for (i=fastll(buff);i<bl->e_col;i++) /* Evtl. Puffer bis e_col mit Spaces */
      *tp++ = ' ';                    /* auffuellen, falls Blockzeile zu kurz */
    *tp = '\0';
    if (fzsc != &space) /* Falls in aktueller Zeile noch Rest, diesen anhaengen */
      strcat (buff,fzsc);
    line_free(akt_winp->alinep->text);        /* alte Zeile freigeben */
    akt_winp->alinep->text = save_text(buff); /* neue Zeile abspeichern */
    down();                                   /* naechste Zeile */
    bl->e_line++;
  }while (akt_zeile=akt_zeile->next); /* Bis letzte Zeile des Blocks eingefuegt */
  bl->e_line--;                       /* e_line ist immer letzte Blockzeile    */

  /* Eingefgten Block zum aktuellen machen, damit adapt_pos mit den */
  /* richtigen Koordinaten rechnet.                                  */
  memcpy(&akt_winp->block,bl,sizeof(block_typ));
  adapt_pos(AP_INSERT);               /* Marker etc. anpassen                  */
  akt_winp->textline = old_tl;        /* Cursorposition restaurieren           */
  akt_winp->alinep = old_ap; /* Anschliessend testen, ob aktuelles Zeichen     */
  check_underl();  /* unterstrichen ist und evtl. screencol u. textcol anpassen*/
  return (TRUE);
}

/*****************************************************************************
*
*  Funktion       aktuellen Block ein/ausruecken (indent_block)
*  --------
*
*  Parameter    :
*                   Typ          : int
*                   Wertebereich : -MAXLENGTH - MAXLENGTH
*                   Bedeutung    : Anzahl der Spalten, um die eingerueckt
*                                  werden soll
*
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : TRUE,FALSE
*                   Bedeutung    : TRUE = Einruecken war moeglich
*                                  FALSE = Einruecken war nicht moeglich
*
*  Beschreibung : Der aktuelle Block wird um weite Spalten nach rechts ein-
*                 gerueckt. Ist weite negativ wird der Block daher nach links
*                 verschoben.  Wuerde durch das Einruecken eine Zeile zu lang,
*                 wird nicht eingerueckt.
*
*****************************************************************************/

int indent_block(weite)
register int weite;
{
  /* *** interne Daten und Initialisierung *** */
  int          i,      /* Schleifenzaehler zur Erzeugung des Leerstring */
	       /* ib gibt die Position des Cursors relativ zum Block an */
	       ib=in_block(akt_winp->textline,akt_winp->screencol);
  register int ie_line = akt_winp->block.e_line,  /* Letzte Blockzeile  */
	       old_sc = akt_winp->screencol; /* Alte Cursorspalte       */
  register int old_tl = akt_winp->textline;  /* Alte Cursorzeile        */
  zeil_typ     *old_ap = akt_winp->alinep;   /* Alter Zeilenzeiger      */
  char         leer[MAXLENGTH+1],            /* Puffer fuer Leerstring  */
	       buff[3*MAXLENGTH+1],          /* Puffer zum Zusammenfue- */
					     /* von Leerstring und Zeile*/
	       *hilf; /* Zeiger in Textzeile bei Einruecken nach links  */

  if(!akt_winp->block.e_col)
    ie_line--;             /* falls Blockende am Zeilenanfang, letzte Zeile */
  gotox(akt_winp->block.s_line); /* nicht mit einruecken! */
  if (weite > 0)
  {    /* Wenn Blockzeilen nach rechts verschoben werden, */
    do /* testen, ob Zeile durch Einruecken nach rechts zu lang wuerde */
    {
      if (fastll(akt_winp->alinep->text) + weite > MAXLENGTH)
      {
	akt_winp->alinep = old_ap;    /* Wenn ja, Cursor restaurieren, */
	akt_winp->textline = old_tl;  /* Fehlermeldung ausgeben und    */
	pe_or(PROMPT_INDNTLONG);
	return (FALSE);               /* Funktion verlassen            */
      }
    } while (akt_winp->textline < ie_line && down());

    for (i=weite-1;i>=0;i--)
      leer[i] = ' ';    /* String mit Spaces erzeugen, der vor die Zeile */
    leer[weite] = '\0'; /* kopiert wird.                                 */
    gotox(akt_winp->block.s_line);
  }

  do
  {
    if (akt_winp->alinep->text)
      if (weite > 0) /* bei Einruecken nach rechts String mit Spaces vor */
      {              /* aktuelle Zeile kopieren                          */
	strcpy(buff,leer);
	strcat(buff,akt_winp->alinep->text);
	free(akt_winp->alinep->text);
	akt_winp->alinep->text = save_text(buff);
      }
      else
      { /* bei Einruecken nach links die ersten <weite> Zeichen verwerfen */
	if ((hilf=fastzeichen(-1 * weite)) == &space)
	  hilf = NULL;
	else
	  hilf = save_text(hilf);
	free(akt_winp->alinep->text);
	akt_winp->alinep->text = hilf;
      }
  } while (akt_winp->textline < ie_line && down());

  akt_winp->alinep = old_ap;    /* Cursorposition restaurieren */
  akt_winp->textline = old_tl;
  akt_winp->screencol = old_sc;

  if (ib & B_LINE) /* Falls Cursor in Blockzeile steht, X-Position anpassen */
  {
    if (weite > 0 && akt_winp->screencol < MAXLENGTH-weite)
      akt_winp->screencol += weite;
    else
      if (weite < 0 && akt_winp->screencol >= -1 * weite)
	akt_winp->screencol += weite;
  }

  if((akt_winp->block.s_col += weite) < 0)  /* Blockkoordinaten X anpassen */
    akt_winp->block.s_col = 0;
  if (akt_winp->block.e_col) /* Ende nicht „ndern, wenn am Zeilenanfang */
    if((akt_winp->block.e_col += weite) < 0)
      akt_winp->block.e_col = 0;
  if(akt_winp->screencol < akt_winp->ws_col) /* Evtl. Fensterinhalt anpassen */
    akt_winp->ws_col = akt_winp->screencol;
  else
    if(akt_winp->screencol >= akt_winp->ws_col + akt_winp->dx)
      akt_winp->ws_col = akt_winp->screencol - akt_winp->dx + 1;
  return (TRUE);
}
