/****************************************************************/
/*                                                              */
/*      MODUL:  text_1.c                                        */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*              - copy_to_buff (zeile in puffer kopieren)       */
/*              - copy_from_buf (puffer zurueckschreiben)       */
/*              - check_buff (evtl. puffer zurueckschreiben)    */
/*              - fill_buff (evtl. Puffer besetzen)             */
/*              - is_last (prfe, ob Cursor an/hinter Z.-ende)  */
/*              - ul_char (ist aktuelles Zeichen unterstr. ?)   */
/*              - akt_zeichen (aktuelles zeichen ermitteln)     */
/*              - right (ein zeichen nach rechts)               */
/*              - left (ein zeichen nach links)                 */
/*              - wortende (testen, ob zeichen worttrenner)     */
/*              - word_left (ein wort links)                    */
/*              - word_right (ein wort rechts)                  */
/*              - bol (an zeilenanfang)                         */
/*              - eol (an zeilenende)                           */
/*              - check_underl (teste auf _^H)                  */
/*              - delete (aktuelles zeichen loeschen)           */
/*              - mdelete (mehrere Zeichen loeschen)            */
/*              - backspace (zeichen links loeschen)            */
/*              - delete_word (wort rechts loeschen)            */
/*              - delete_eol (Rest der Zeile loeschen)          */
/*              - join (Zeilen verknuepfen)                     */
/*              - up (eine zeile hoch)                          */
/*              - down (eine zeile runter)                      */
/****************************************************************/

#include "defs.h"

extern char backupflag;
extern marker_typ marker[];

/* *** globale Daten und Initialisierung *** */
char linebuff [3*MAXLENGTH+1]; /* Zeilenpuffer                  */
char        bufflag = FALSE;   /* Flag, ob linebuff belegt ist           */
char        *sd_line=NULL,     /* Zwischenspeicher fuer geloeschte Zeile */
	    space=' ';         /* Globales Leerzeichen                   */
char        *fastzeichen();

/****************************************************************
*
* FUNKTION:     copy_to_buff()  (zeile in puffer kopieren)
*
* BESCHREIBUNG: - die aktuelle zeile wird nach linebuff kopiert;
*               ist der textpointer NULL, so wird ein leerer
*               puffer angelegt
*               - dann wird der puffer bis zur maximallanege mit
*               spaces aufgefuellt
*               - das aktuelle zeichen wird gemaess
*               akt_winp->screencol gesetzt
*               - bufflag wird auf TRUE gesetzt
*****************************************************************/

void copy_to_buff()
{
  /* *** interne Daten *** */
  register int i; /* Zaehler fuer zu kopierende Zeichen */

  if (akt_winp->maxline == -1)  /* Wenn Datei leer, dann neue   */
    new_line();                 /* Zeile einfuegen.             */
  bufflag = TRUE;               /* Puffer als belegt markieren  */
  if(akt_winp->alinep->text)    /* wenn Zeile nicht leer, dann     */
    strcpy(linebuff,akt_winp->alinep->text); /* in Puffer kopieren */
  else
    linebuff[0] = '\0';         /* Sonst Pufferinhalt loeschen */

  /* Rest des Puffers mit Blanks auffuellen */
  for (i=strlen (linebuff);i<3*MAXLENGTH;linebuff[i++] = ' ');
  linebuff [3*MAXLENGTH] = '\0';
  i = akt_winp->screencol;      /* bildschirmpos. speichern */

  /* Jetzt textcol setzen. Dazu screencol mal right aufrufen */
  for (akt_winp->screencol = akt_winp->textcol = 0;i>0;i--)
    right();
}


/****************************************************************
*
* FUNKTION:     copy_from_buf() (zeile zurueckschreiben)
*
* BESCHREIBUNG: - der inhalt von linebuff wird in die aktuelle
*               zeile zurueckkopiert, dabei werden evtl. spaces
*               am ende geloescht
*               - ist die zeile leer, so wird der textpointer
*               auf NULL gesetzt
*****************************************************************/

void copy_from_buf()
{
  /* *** interne Daten *** */
  register int i; /* Zaehler fuer Zeichen im Puffer */

  bufflag = FALSE; /* Puffer als leer markieren */
  /* abschliessende Blanks skippen */
  for (i=3*MAXLENGTH-1;i>=0 && linebuff [i] == ' ';i--);
  /* Falls letztes Zeichen der Zeile ein unterstrichenes Space ist, so
     wuerde der Puffer normal ein Zeichen zu frueh abgeschnitten. Diesen
     Fall muss man also hier testen. */
  if (i>0 && linebuff[i-1]=='_' && linebuff[i]=='')
    linebuff[i+2] = '\0';
  else
    linebuff[i+1] = '\0';
  line_free(akt_winp->alinep->text);            /* aktuelle Zeile durch  */
  akt_winp->alinep->text = save_text(linebuff); /* Pufferinhalt ersetzen */
}


/****************************************************************
*
* FUNKTION:     check_buff()    (puffer evtl. zurueckschreiben)
*
* BESCHREIBUNG: - falls bufflag TRUE ist, wird copy_from_buf()
*               aufgerufen
*               - bufflag wird auf FALSE gesetzt
*****************************************************************/

void check_buff()
{
  if (bufflag)
    copy_from_buf(); /* Pufferinhalt in aktuelle Zeile uebernehmen */
}

/****************************************************************
*
* FUNKTION:     fill_buff() (Puffer evtl. mit aktueller Zeile laden)
*
* BESCHREIBUNG: - falls bufflag FALSE ist, wird copy_to_buff()
*               aufgerufen
*****************************************************************/

void fill_buff()
{
  if (!bufflag)
    copy_to_buff(); /* aktuelle Zeile in Puffer kopieren */
}

/****************************************************************
*
* FUNKTION:     is_last()         (an/hinter letzter Position in Zeile?)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - Von der Cursorposition aus wird getestet, ob
*                 nach rechts hin nur Blanks stehen, falls die aktuelle
*                 Zeile schon im Puffer ist. Ansonsten wird mit
*                 fastzeichen() auf &space getestet.
*               - stand man am oder hinterm Zeilenende, so wird 
*               TRUE, sonst FALSE zurueckgeliefert.
*****************************************************************/

int is_last()
{
  /* *** lokale Daten *** */
  char result = TRUE; /* Ergebnis */
  int  i;             /* Z„hlvariable zum Durchlaufen des Zeilenpuffers */

  if (bufflag) /* Zeile im Puffer, also Puffer auf Spaces testen */
  {
    for (i=akt_winp->textcol; result && i<3*MAXLENGTH; i++)
      result = (linebuff [i] == ' '); 
    return result; 
  }
  else /* Zeile steht nicht im Zeilenpuffer, dann liefert fastzeichen()
	  &space, wenn Zeile leer / Cursor hinter Zeilenende steht */
    return fastzeichen(akt_winp->screencol) == &space;
}

/****************************************************************
*
* FUNKTION:     ist aktuelles zeichen unterstrichen? (ul_char)
*
* BESCHREIBUNG: - Es wird fill_buff aufgerufen.
*               - ist das aktuelle zeichen unterstrichen, wird
*               TRUE, sonst FALSE zurueckgeliefert
*****************************************************************/

int ul_char()
{
  fill_buff(); /* Evtl. Zeile in Puffer kopieren */
  if(linebuff[akt_winp->textcol] == '_'
  && linebuff[akt_winp->textcol+1] == '' && linebuff[akt_winp->textcol+2])
    return(TRUE);
  return(FALSE);
}

/****************************************************************
*
* FUNKTION:     akt_zeichen()   (aktuelles zeichen holen)
*
* BESCHREIBUNG: - Es wird fill_buff aufgerufen
*               - das aktuelle zeichen wird zurueckgegeben
*               - auch unterstrichene zeichen werden
*               beruecksichtigt
*****************************************************************/

char akt_zeichen()
{
  fill_buff(); /* evtl. aktuelle Zeile in Puffer kopieren */
  if (ul_char()) /* Zeichen unterstrichen ? Dann 2 weiter rechts, da vor */
    return (linebuff[akt_winp->textcol+2]); /* Zeichen _  steht. */
  else
    return(linebuff[akt_winp->textcol]); /* sonst an aktueller Pos. */
}


/****************************************************************
*
* FUNKTION:     right()         (ein zeichen rechts)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - Es wird fill_buff aufgerufen
*               - screencol wird um 1 inkrementiert
*               - textcol wird auf den index des naechsten zeichens
*               gesetzt (auch unterstrichene werden beruecksichtigt)
*               - konnte nicht nach rechts gegangen werden, wird
*               FALSE, sonst TRUE zurueckgeliefert.
*****************************************************************/

int right()
{
  fill_buff(); /* evtl. aktuelle Zeile in Puffer kopieren */
  if (akt_winp->screencol < MAXLENGTH) /* geht's noch nach rechts ? */
  {
    akt_winp->screencol++;
    if (ul_char())            /* ist das aktuelle Zeichen unterstrichen, */
      akt_winp->textcol += 3; /* muss intern Spalte um 3 erhoeht werden, */
    else                      /* da '_' und '' geskippt werden muessen  */
      akt_winp->textcol++;
    return(TRUE);
  }
  return(FALSE);
}


/****************************************************************
*
* FUNKTION:     left()          (ein zeichen links)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - Es wird fill_buff aufgerufen
*               - screencol wird um 1 dekrementiert
*               - textcol wird auf den index des vorigen zeichens
*               gesetzt (auch unterstrichene werden beruecksichtigt)
*               - konnte die aktuelle position nicht um einen schritt
*               nach links bewegt werden, wird FALSE, sonst TRUE
*               zurueckgeliefert.
*****************************************************************/

int left()
{
  fill_buff(); /* evtl. aktuelle Zeile in Puffer kopieren */
  if (akt_winp->textcol > 0) /* geht's noch nach links ? */
  {
    akt_winp->screencol--;
    if (akt_winp->textcol >= 3) /* kann das anzuspringende Zeichen unter- */
				/* strichen sein ? */
      if (linebuff [akt_winp->textcol-3] == '_' /* ist es denn auch */
      && linebuff [akt_winp->textcol-2] == '') /* unterstrichen ?  */
	akt_winp->textcol -= 3; /* ja, dann intern um 3 nach links  */
      else
	akt_winp->textcol--;    /* sonst nur um 1 nach links */
    else
      akt_winp->textcol--;
    return(TRUE);
  }
  return(FALSE);
}


/****************************************************************
*
* FUNKTION:     wortende()  (ist zeichen worttrenner?)
*
* PARAMETER:    - char c : zu ueberpruefendes zeichen
* ERGEBNIS:     - TRUE oder FALSE
* BESCHREIBUNG: - es wird ueberprueft, ob c ein worttrennzeichen
*               ist und entsprechend TRUE oder FALSE zurueckge-
*               geben
*****************************************************************/

int wortende (c)
char c;
{
  return ((int) strchr (" \t.,#:;|@$&()[]{}!?\"'`/\\<>-+*^=",c));
}

/****************************************************************
*
* FUNKTION:     word_left()     (ein wort links)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - leading spaces und tabs werden uebersprungen
*               - anschliessend werden screencol und textcol auf
*               den beginn des naechsten wortes gesetzt
*****************************************************************/

int word_left()
{
  /* *** interne Daten und Initialisierung *** */
  register int old_sc = akt_winp->screencol, /* Zwischenspeicher Spalte */
	       ging=FALSE;           /* Rueckgabewert der Funktion left */

  while (left() && ((akt_zeichen() == ' ') || (akt_zeichen() == '\t')));
  while (!wortende(akt_zeichen()) && (ging=left()));
  if (ging)  /* Wenn man vor den Wortanfang gehen konnte, dann wieder */
    right(); /* eins nach rechts auf den Wortanfang. */
  if(old_sc == akt_winp->screencol) /* Cursorspalte restaurieren */
    return(FALSE);
  return(TRUE);
}


/****************************************************************
*
* FUNKTION:     word_right()    (ein wort rechts)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - es wird bis zum wortende gesprungen
*               - folgende spaces oder tabs werden ebenfalls
*               uebersprungen
*****************************************************************/

int word_right()
{
  /* *** interne Daten *** */
  register int  old_sc, /* Zwischenspeicher Cursorspalte */
		old_tc; /* Zwischenspeicher Cursorzeile  */
  register char az;     /* aktuelles Zeichen             */

  fill_buff(); /* evtl. aktuelle Zeile in Puffer kopieren */
  old_sc = akt_winp->screencol; /* Cursorposition merken */
  old_tc = akt_winp->textcol;
  if(wortende(akt_zeichen())) /* Steht man auf Worttrenner, */
    right();                  /* dann eins nach rechts      */
  else                        /* sonst Wort skippen         */
    while ((az=akt_zeichen()) && !wortende (az) && right());
  /* Jetzt die dem Wort folgenden Blanks und Tabs skippen */
  while ((((az=akt_zeichen()) == ' ') || (az == '\t')) && right());

  if(akt_winp->screencol == MAXLENGTH) /* steht man am rechten */
  { /* Rand, dann wieder zur alten Position zurueck */
    akt_winp->screencol = old_sc;
    akt_winp->textcol = old_tc;
    return(FALSE);
  }
  return(TRUE);
}


/****************************************************************
*
* FUNKTION:     bol()   (an Zeilenanfang)
*
* BESCHREIBUNG: - es wird zum Zeilenanfang gesprungen
*****************************************************************/

void bol()
{
  akt_winp->screencol = akt_winp->textcol = 0;
}


/****************************************************************
*
* FUNKTION:     eol()   (an Zeilenende)
*
* BESCHREIBUNG: - Es wird fill_buff aufgerufen
*               - es wird zum Zeilenende gesprungen
*               - spaces am ende werden ignoriert
*****************************************************************/

void eol()
{
  /* *** interne Daten *** */
  register int i; /* Index fuer linebuff */

  fill_buff(); /* evtl. aktuelle Zeile in Puffer kopieren */
  for (i=3*MAXLENGTH-1;linebuff [i] == ' ';i--);
  i++; /* Hinter letztes Zeichen gehen */
  bol();  /* vom Anfang der Zeile so oft nach rechts, bis Cursor in */
  while (akt_winp->textcol < i) /* richtiger interner Spalte steht. */
    right();              /* Dadurch hat textcol den richtigen Wert */
}


/****************************************************************
*
* FUNKTION:     check_underl()  (teste auf _^H kombination)
* ERGEBNIS:     TRUE, FALSE
* BESCHREIBUNG: - falls das aktuelle Zeichen ein Backspace und
*               das vorhergehende ein _ ist, oder das aktuelle
*               Zeichen das zu unterstreichende und die Zeichen
*               davor _ und  sind, werden screencol und
*               textcol so dekrementiert, dass sie auf
*               das _ zeigen
*               - Falls ein solcher Fall auftrat, wird mit
*               der Funktion lineout() die aktuelle Zeile auf dem
*               Bildschirm restauriert und TRUE zurueckgegeben.
*               Ansonsten wird FALSE zurueckgegeben.
*
*****************************************************************/

int check_underl()
{
  fill_buff(); /* evtl. aktuelle Zeile in Puffer kopieren */
  if(akt_winp->textcol && linebuff[akt_winp->textcol+1] /* noch 2 Zeichen da ? */
  && !strncmp(&linebuff[akt_winp->textcol-1],"_",2))       /* ^H eingefuegt? */
  {
    if (in_block(akt_winp->textline,akt_winp->screencol) & B_FIRST_CHAR)
      insdel_blockadapt(-1);  /* Blockgrenzen anpassen (eee stimmt nicht ganz) */
    else
      insdel_blockadapt(-2);
    akt_winp->textcol--;   /* Cursorposition korrigieren, da Cursor immer */
    akt_winp->screencol--; /* auf dem Unterstrich stehen muss             */
    return (TRUE);
  }
  else
    if (!strncmp(&linebuff[akt_winp->textcol],"_",2) /* _ eingefuegt ? */
    && linebuff [akt_winp->textcol+2]) /* und ein Zeichen dahinter ? */
    {
      insdel_blockadapt(-2); /* Blockgrenzen anpassen */
      return (TRUE);
    }
  return (FALSE);
}


/****************************************************************
*
* FUNKTION:     delete()        (loesche aktuelles zeichen)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - falls das aktuelle Zeichen nicht das Endzeichen
*               ist, wird es geloescht (unterstrichene zeichen
*               werden beruecksichtigt)
*               - Die Blockgrenzen werden angepaát
*               - der restliche text wird rangezogen
*               - danach wird check_underl() aufgerufen
*****************************************************************/

int delete()
{
  /* *** interne Daten *** */
  register char *in,     /* Zieladresse beim Verschieben  */
		*out;    /* Startadresse beim Verschieben */
  register int  anz_del; /* Anzahl zu loeschender Zeichen */

  if (akt_zeichen()) /* Letztes Zeichen (Abschlussnull) ? */
  { /* akt_zeichen kopiert evtl. auch die Zeile in den Puffer */
    insdel_blockadapt(-1); /* Blockgrenzen anpassen */
    in = (out = &linebuff[akt_winp->textcol]) + (anz_del = 1 + 2*ul_char());
    fwdcpy(out,in); /* Alles rechts vom Cursor eins ranziehen */
    check_underl(); /* Ist dadurch ein neues unterstrichenes Zeichen entstd., */
		    /* wird alles korrekt angepasst */
    /* Ende des Zeilenpuffers mit Blanks besetzen */
    strncpy(&linebuff[3*MAXLENGTH-anz_del],"   ",anz_del);
    akt_winp->changeflag = TRUE; /* Text als geaendert markieren */
    return(TRUE);
  }
  return(FALSE);
}

/*****************************************************************************
*
*  Funktion       mehrere Zeichen loeschen (mdelete)
*  --------
*
*  Parameter    : n         :
*                   Typ          : int
*                   Wertebereich : 0 - MAXLENGTH
*                   Bedeutung    : Anzahl zu loeschender Zeilen
*
*  Beschreibung : Es wird versucht, eine bestimmte Anzahl Zeichen von der
*                 aktuellen Position ab zu loeschen. Ist dies nicht moeglich,
*                 so werden so viele als moeglich geloescht.
*
*****************************************************************************/

void mdelete(n)
int n;
{
  /* *** interne Daten und Initialisierung *** */
  register char *p;        /* Zeiger in linebuff zum Auffuellen mit Blanks */
  register int  i,         /* Anzahl loeschbarer Zeichen                   */
		old_sc = akt_winp->screencol, /* alte Cursorspalte         */
		old_tc;    /* alte Cursorspalte intern                     */

  fill_buff(); /* evtl. aktuelle Zeile in Puffer kopieren */
  old_tc = akt_winp->textcol; /* Cursorspalte merken */
  insdel_blockadapt(-n); /* Blockgrenzen anpassen */
  for(i=0;i<n;i++) /* Berechnen, wieviele Zeichen geloescht */
    if(!right())   /* werden koennen */
      break;
  /* kann maximal i Zeichen loeschen */
  if(i)
  {
    akt_winp->changeflag = TRUE; /* Text als geaendert markieren */
    /* Jetzt Zeilenrest nach links verschieben */
    fwdcpy(&linebuff[old_tc],&linebuff[akt_winp->textcol]);
    /* Ende des Puffers wieder mit Blanks besetzen */
    p = linebuff + old_tc + strlen(linebuff + old_tc);
    for(i = akt_winp->textcol - old_tc;i>0;i--)
      *p++ = ' ';
    *p='\0';
  }
  akt_winp->screencol = old_sc; /* Cursorspalte restaurieren */
  akt_winp->textcol = old_tc;
}

/****************************************************************
*
* FUNKTION:     backspace()     (loesche linkes zeichen)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - falls der cursor sich nicht am linken rand be-
*               findet, wird das links vom aktuellen zeichen be-
*               findliche zeichen geloescht (mittels delete)
*               - die aktuelle position ist nach backspace die des
*               geloeschten zeichens
*****************************************************************/

int backspace()
{
  if(left() && delete())
    return(TRUE);
  return(FALSE);
}

/****************************************************************
*
* FUNKTION:     delete_word()   (wort loeschen)
* ERGEBNIS:     TRUE,FALSE
* BESCHREIBUNG: - das aktuelle zeichen wird deletet
*               - solange, wie das aktuelle zeichen kein wort-
*               trenner ist, wird es geloescht
*               - folgende spaces und tabs werden ebenfalls ge-
*               loescht
*               - Durch die Verwendung von delete() werden die
*               Blockgrenzen korrekt angepaát
*****************************************************************/

int delete_word()
{
  /* *** interne Daten *** */
  register int  max_del; /* maximal zu loeschende Tabs und Blanks */
  int           old_sc,  /* Alte Cursorposition Spalte            */
		old_tc;  /* Alte Cursorposition Spalte intern     */
  register char az;      /* Aktuelles Zeichen                     */

  /* Steht man auf einem Worttrenner, dann nur diesen loeschen */
  if(wortende(az = akt_zeichen()) && az != ' ')
    if (delete())
      return(TRUE);
    else   /* klappte das Loeschen nicht, False zurueckgeben */
      return (FALSE);

  old_sc = akt_winp->screencol; /* Cursorspalte merken */
  old_tc = akt_winp->textcol;

  /* Alle Zeichen bis zum naechsten Worttrenner skippen */
  while ((az=akt_zeichen()) && !wortende(az))
    right();

  swap_int(&old_sc,&akt_winp->screencol); /* Zur Anfangsposition zurueck */
  akt_winp->textcol = old_tc;
  mdelete(old_sc - akt_winp->screencol);  /* geskippte Zeichen loeschen */
  max_del = MAXLENGTH - akt_winp->screencol;
  old_sc = akt_winp->screencol;  /* Cursorspalte merken */
  old_tc = akt_winp->textcol;

  /* Die dem Wort folgenden Blanks und Tabs werden auch geloescht */
  while (max_del-- && ((az=akt_zeichen()) == ' ' || az == '\t'))
    right();
  swap_int(&old_sc,&akt_winp->screencol);
  akt_winp->textcol = old_tc;
  mdelete(old_sc - akt_winp->screencol);
  return(TRUE);
}

/****************************************************************
*
* FUNKTION:     delete_eol()    (Rest der Zeile loeschen)
* ERGEBNIS:     TRUE, FALSE
* BESCHREIBUNG: - Abhaengig von Bufflag wird entweder der Puffer-
*               rest mit Blanks ueberschrieben oder an der ak-
*               tuellen Position eine '\0' geschrieben. In diesem
*               Fall wird fuer die neu entstandene Zeile neuer
*               Speicher alloziert.
*               - Falls sich die Zeile veraendert hat, wird TRUE,
*               sonst FALSE zurueckgegeben.
*****************************************************************/

int delete_eol()
{
  /* *** interne Daten *** */
  register char *hilf,
		*alt_poi;
  register int  i;

  /* Steht Zeile schon im Puffer, dann im Puffer den Rest blanken */
  if (bufflag)
  {
    for (i=akt_winp->textcol;i<3*MAXLENGTH;i++)
      linebuff[i]=' ';
    return (akt_winp->changeflag = TRUE);
  }
  else  /* Steht Zeile nicht im Puffer, dann Zeile selber modifizieren */
  {
    if (akt_winp->alinep->text)
      if ((hilf=fastzeichen(akt_winp->screencol)) != &space)
      {
	if (hilf==akt_winp->alinep->text) /* ab erstem Zeichen, dann */
	{                                 /* Zeilenihalt komplett loeschen */
	  free(akt_winp->alinep->text);
	  akt_winp->alinep->text = NULL;
	}
	else                              /* Sonst Zeile an Cursorposition */
	{                                 /* abschneiden und neu sichern */
	  *hilf='\0';
	  alt_poi = akt_winp->alinep->text;
	  akt_winp->alinep->text = save_text(alt_poi);
	  free (alt_poi);
	}
	return (akt_winp->changeflag = TRUE);
      }
  }
  return (FALSE);
}

/*****************************************************************************
*
*  Funktion       Zeilen verknuepfen (join)
*  --------
*
*  Parameter    : modus     :
*                   Typ          : int
*                   Wertebereich : IGNORE_COORS, ADAPT_COORS
*                   Bedeutung    : IGNORE_COORS: join wird von Blockfunktion
*                                  aufgerufen (wichtig fuer Aufruf del_line)
*                                  ADAPT_COORS: join wird nicht von einer
*                                  Blockfunktion aufgerufen
*  Ergebnis     :
*                   Typ          : int
*                   Wertebereich : J_OK, J_TOOLONG, J_LASTLINE
*                   Bedeutung    : J_OK      : Hat geklappt
*                                  J_TOOLONG : Ging nicht, neue Zeile zu lang
*                                  J_LASTLINE: Ging nicht, keine Zeile mehr
*
*  Beschreibung : Die nachfolgende Zeile wird an die aktuelle Zeile angehaengt.
*                 Wird die Zeile dadurch zu lang oder existiert keine nach-
*                 folgende Zeile, so wird der entsprechende Fehlerwert zu-
*                 rueckgegeben.
*
*****************************************************************************/

int join(modus)
int modus;
{
  /* *** interne Daten *** */
  register int  l1,  /* Laenge der aktuellen Zeile       */
		ib;  /* Cursorposition relativ zum Block */
  register char *z1, /* Zeiger auf aktuelle Zeile        */
		*z2; /* Zeiger auf nachfolgende Zeile    */

  check_buff();  /* evtl. Pufferinhalt in Text uebernehmen */
  if (!(z1 = akt_winp->alinep->text)
  && akt_winp->alinep->next != akt_winp->dummyp)
    del_line(modus);          /* akt. zeile ist leer und nicht letzte z. */
  else
  {
    if (down())         /* akt. zeile nicht letzte zeile? */
    {
      if (!(z2 = akt_winp->alinep->text)) /* naechste z. leer? */
      {
	if(del_line(modus) != LAST_LINE_DEL) /* wenn letzte Zeile geloescht */
	  up();                              /* machte del_line selbst up() */
      }
      else /* Wenn nachfolgende Zeile nicht leer war */
      {
	/* Blockspalten muessen evtl. bei normalem Block angepasst werden */
	if(modus == ADAPT_COORS && block_defined()
	&& akt_winp->block.typ == BT_NORMAL)
	{
	  ib = in_block(akt_winp->textline,akt_winp->screencol);
	  if(ib & B_FIRST_LINE) /* Enthaelt anzuhaengende Zeile Blockanfang ?  */
	  {                                      /* Ja, dann Laenge der ersten */
	    akt_winp->block.s_col += fastll(z1); /* Zeile aufaddieren          */
	    akt_winp->block.s_line--;            /* Block faengt eine Zeile    */
	  }                                      /* weiter oben an.            */
	  if(ib & B_LAST_LINE) /* Blockende in anzuhaengender Zeile ? */
	  {
	    akt_winp->block.e_col += fastll(z1); /* s.o. */
	    akt_winp->block.e_line--;
	  }
	}

	up(); /* Wieder in urspruengliche Zeile zurueck */
	if (fastll(z2) + fastll(z1) < MAXLENGTH) /* Neue Zeile zu lang ? */
	{ /* (Man verschenkt hier ein Zeichen (< statt <=), damit */
	  /* do_join() keine Probleme beim nachtr„glichen Einfgen */
	  /* eines Leerzeichens bekommt). */
	  /* Nein, dann Platz fuer neue Zeile schaffen */
	  /* und neue Zeile in Text eintragen          */
	  akt_winp->alinep->text = reserve_mem((l1=strlen(z1))+strlen(z2)+1);
	  strcpy(akt_winp->alinep->text,z1); /* Aktuelle Zeile und nach- */
	  strcpy(akt_winp->alinep->text + l1,z2); /* folgende zusammenkop. */
	  free(z1);
	  down();
	  if(del_line(modus) != LAST_LINE_DEL)       /* s.o. */
	    up();
	  return (J_OK);
	}
	else /* Wenn Zeile zu lang geworden waere */
	  return (J_TOOLONG);
      }
    }
    else        /* Konnte keine Zeile nach unten gehen */
      return (J_LASTLINE);
  }
  return (J_OK);
}

/****************************************************************
*
* FUNKTION:     up()    (eine zeile hoch)
*
* BESCHREIBUNG: - die vorherige zeile wird zur aktuellen
*****************************************************************/

int up()
{
  return(lines_up(1));
}


/****************************************************************
*
* FUNKTION:     down()  (eine zeile runter)
*
* BESCHREIBUNG: - die nachfolgende zeile wird zur aktuellen
*****************************************************************/

int down()
{
  return(lines_down(1));
}
