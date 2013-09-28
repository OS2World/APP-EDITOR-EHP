/****************************************************************/
/*                                                              */
/*      MODUL:  mouse.c                                         */
/*                                                              */
/*      FUNKTIONEN:                                             */
/*       - Fenster ermitteln, in dem Position liegt (pos_to_win)*/
/*       - Zum Fenster wechseln, in dem Position liegt (sw_pos) */
/*       - Liegt Position innerhalb von akt_winp? (in_akt_win)  */
/*       - Mauscursor verstecken und wieder zeigen (hide_show)  */
/*       - Maus-Status lesen (get_mouse)                        */
/*       - Fenster mit Maus bewegen (mouse_movewin)             */
/*       - Fenstergrî·e mit Maus Ñndern (mouse_sizewin)         */
/*       - Bildschirm gemÑ· Mausposition scrollen (check_scroll)*/
/*       - Cursor setzen oder Block (set_cursor_or_mark_block)  */
/*       - Mausposition in SchaltfÑache wandeln (pos_to_button) */
/*       - Fenster mit Maus schlie·en (mouse_win_zu)            */
/*       - Mit Maus nach rechts scrollen (msr)                  */
/*       - Mit Maus nach links scrollen (msl)                   */
/*       - Mit Maus nach oben scrollen (msu)                    */
/*       - Mit Maus nach unten scrollen (msd)                   */
/*       - Maus-Routine (mouse_routine)                         */
/*       - Maus-Routine fÅr Funktion ja_nein (jn_mouse_routine) */
/*       - Mouse-Interrupt aktivieren/sperren (mouse_jn_init)   */
/*       - Mouse-Interrupt aktivieren/sperren (set_mouse_int)   */
/*       - Maus initialisieren (init_mouse)                     */
/*                                                              */
/****************************************************************/

#ifdef OS2

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include <os2.h>
#include <stdlib.h>

extern HMTX sem_handle; /* Handle fÅr Semaphor */

struct MPTRPOS          /* Kann die Position des Mauszeigers aufnehmen */
{
  short int line,
	    col;
};

#pragma pack(1)
typedef struct MEVENT   /* Nimmt das Mouse-Event aus der Event-Queue auf */
{
  short int button_status; /* Bit 2 links, Bit 4 rechts */
  int       time;
  short int y,
	    x;
} mouse_type;
#pragma pack(4)

short int mouse_handle; /* Globale Variable enthÑlt den Mouse-Handle */
TID       mouse_ThreadID, /* Hier steht die ID des Mouse-Threads */
	  mouse_jn_ThreadID=0; /* Mouse-Thread fÅr ja_nein-Abfrage */
char      mouse_jn_active = FALSE, /* Routine ja_nein aktiv ? */
	  mouse_active = FALSE;    /* Soll Mouse_Thread laufen ? */

#else

#pragma inline
#include <dos.h>
typedef struct mouse_struct
{
  int button_status,  /* Knopfstatus: Bit 0 links, Bit 1 rechts, Bit 2 Mitte */
      x,              /* X-Position (0..COLS-1) */
      y;              /* Y-Position (0..LINES-1) */
} mouse_type;


#endif

#include "defs.h"

extern (*funktion[])(),
       do_win_zu();
extern short int mc_index;
extern comm_typ *comm_feld;
extern char highblockflag;

int mouse_jn;         /* Variable fÅr Bedienung der Funktion ja_nein mit
			 der Maus. Kann die Werte NO_KLICK, KLICK_RIGHT und
			 KLICK_LEFT annehmen. */

/******************************************************************************

 Funktion     : Fenster ermitteln, in dem Position liegt (pos_to_win)
 --------------

 Parameter    : x           :
                  Typ          : int
                  Wertebereich : 0-COLS
                  Bedeutung    : X-Achse der Position

                y           :
                  Typ          : int
                  Wertebereich : 0-LINES
                  Bedeutung    : Y-Achse der Position

 Ergebnis     :
*                  Typ          : win_typ *
*                  Wertebereich : Pointer auf Fensterstruktur
*                  Bedeutung    : Zeigt auf das sichtbare Fenster, in dem die
*                                 angegebene Position liegt
*
* Beschreibung : Es werden in der Reihenfolge. in der sie beim Refresh
*                gezeichnet wÅrden, alle Fenster durchgegangen und der Zeiger
*                auf das Fenster gemerkt, falls die angegebene Position in
*                dem getesteten Fenster liegt. So erhÑlt man das Fenster,
*                in dem die Åbergebene Position liegt.
*
******************************************************************************/

win_typ *pos_to_win(x,y)
int x,y;
{
  win_typ *w,          /* Pointer zum Durchlaufen der Fensterliste */
	  *res = NULL; /* Pointe auf Fenster, in dem Position liegt */

  for(w=akt_winp->next->next; w != akt_winp->next; w=w->next)
    /* Alle Fenster durchlaufen */
    if(x >= w->x && x <= w->x + w->dx + 1
    && y >= w->y && y <= w->y + w->dy + 1)
      res = w;  /* Position liegt im Fenster, Fenster merken */
  return(res);
}

/******************************************************************************
*
* Funktion     : Zum Fenster wechseln, in dem Position liegt (sw_pos)
* --------------
*
* Parameter    : x           :
*                  Typ          : int
*                  Wertebereich : 0-COLS
*                  Bedeutung    : X-Achse der Position
*
*                y           :
*                  Typ          : int
*                  Wertebereich : 0-LINES
*                  Bedeutung    : Y-Achse der Position
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE: Fenster gefunden, FALSE: nicht gefunden
*
* Beschreibung : Mittels der Funktion pos_to_win wird das Fenster ermittelt,
*                in dem die Position liegt. Wird ein solches Fenster gefunden,
*                so wird es zum aktuellen gemacht und TRUE geliefert, ansonsten
*                wird FALSE geliefert.
*
******************************************************************************/

int sw_pos(x,y)
int x,y;
{
  win_typ *w, /* Zeiger auf zu aktivierendes Fenster */
	  *dummy;

  if(w = pos_to_win(x,y))  /* Liegt die Position in einem sichtbaren Fenster ? */
  {
    if(w != akt_winp)
    {
      dummy = akt_winp->next;   /* Dann gefundenes Fenster zum aktuellen */
      akt_winp->next = w;       /* machen, indem man es vor dummy in     */
      w->prev->next = w->next;  /* die Fensterliste einhaengt.           */
      w->next->prev = w->prev;
      w->prev = akt_winp;
      w->next = dummy;
      dummy->prev = w;
      akt_winp = w;
    }
    return(TRUE);
  }
  return(FALSE); /* Position lag nicht in einem Fenster */
}

/******************************************************************************
*
* Funktion     : Liegt Position innerhalb von akt_winp? (in_akt_win)
* --------------
*
* Parameter    : x           :
*                  Typ          : int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : X-Position, die zu testen ist
*
*                y           :
*                  Typ          : int
*                  Wertebereich : 0-MAXINT
*                  Bedeutung    : Y-Position, die zu testen ist
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE: Position liegt im aktuellen Fenster
*                                 FALSE: Position liegt nicht im aktuellen
*                                        Fenster
*
* Beschreibung : Die Åbergebenen Koordinaten werden mit den Koordinaten
*                des aktuellen Fensters verglichen. Liegen sie innerhalb,
*                also in der TextflÑche, so wird TRUE, sonst FALSE ge-
*                liefert.
*
******************************************************************************/

int in_akt_win(x,y)
int x,y;
{
  return(x > akt_winp->x && x <= akt_winp->x+akt_winp->dx
  && y > akt_winp->y && y <= akt_winp->y+akt_winp->dy);
}

/******************************************************************************
*
* Funktion     : Mauscursor verstecken und wieder zeigen (hide_show)
* --------------
*
* Parameter    : hs          :
*                  Typ          : int
*                  Wertebereich : MOUSE_SHOW, MOUSE_HIDE,
*                  Bedeutung    : MOUSE_HIDE : Mauszeiger verstecken
*                                 MOUSE_SHOW : Mauszeiger anzeigen
*
* Beschreibung : Der Mauscursor wird mit der Funktion 2 des Mausinterrupts
*                versteckt, falls MOUSE_HIDE gesetzt war, und anschlie·end
*                mit der Funktion 1 wieder dargestellt, falls MOUSE_SHOW
*                gesetzt war. War der Mauscursor also vorher eingeschal-
*                tet, so ist dann garantiert, da· er auch sichtbar ist.
*                Das ist wichtig, falls nach einem Refresh der Mauscursor
*                sichtbar sein soll.
*
******************************************************************************/

void hide_show(hs)
int hs;
{
#ifdef OS2
  static struct MOUREMOVESTRUCT
  {
     short int startline,
	       startcol,
	       endline,
	       endcol;
  } mrs;
  static char need_init = TRUE;

  if (hs & MOUSE_HIDE)
  {
    if (need_init)
    {
      mrs.startline = mrs.startcol = 0;
      mrs.endline   = LINES-1;
      mrs.endcol    = COLS-1;
      need_init     = FALSE;
    }
    MouRemovePtr (&mrs, mouse_handle);
  }
  if (hs & MOUSE_SHOW)
    MouDrawPtr (mouse_handle);
#else
  union  REGS  regs;

  if(hs & MOUSE_HIDE)
  {
    regs.x.ax = 2;
    int86(51,&regs,&regs);
  }
  if(hs & MOUSE_SHOW)
  {
    regs.x.ax = 1;
    int86(51,&regs,&regs);
  }
#endif
}

#ifndef OS2
/*******************************************************************************

* Funktion     : Maus-Status lesen (get_mouse)
* --------------
*
* Ergebnis     :
*                  Typ          : *mouse_type
*                  Wertebereich : Pointer auf Maus-Struktur
*                  Bedeutung    : Zeigt auf Struktur, die die aktuellen
*                                 Mausdaten enthÑlt.
*
* Beschreibung : Mittels der Funktion 3 des Mausinterrupts wird der Mausstatus
*                eingelesen.
*
******************************************************************************/

mouse_type *get_mouse()
{
  static mouse_type m; /* zur Ablage des Maus-Status */
  union  REGS  regs;

  regs.x.ax  = 3;
  int86(51,&regs,&regs);
  m.button_status = regs.x.bx;
  m.x             = regs.x.cx / 8;
  m.y             = regs.x.dx / 8;
  return(&m);
}
#endif

/******************************************************************************
*
* Funktion     : Fenster mit Maus bewegen (mouse_movewin)
* --------------
*
* Parameter    : m           :
*                  Typ          : mouse_type *
*                  Wertebereich : Pointer aus Mausstruktur
*                  Bedeutung    : Mauszustand mit Position und KnopfdrÅcken
*
* Beschreibung : Das aktuelle Fenster wird durch die Maus verschoben.
*                Die Mauskoordinaten werden solange geprÅft, bis der
*                linke Mausknopf nicht mehr gedrÅckt ist. Dann wird
*                das Fenster in seiner dortigen Position gezeichnet.
*
******************************************************************************/

void mouse_movewin(m)
mouse_type *m;
{
  int old_x = m->x, /* Ablage fÅr alte Mauskoordinaten */
      old_y = m->y,
      moved = FALSE;
#ifdef OS2
  short int wait = 1; /* Auf Mausereignisse warten */
#endif

  /* Wenn rechter Mausknopf gedrÅckt wurde, dann Fenster in den Hintergrund
     schieben. Bei linkem Mausknopf evtl. Fenster verschieben. */
  if(m->button_status & MOUSE_BUT2) /* rechter Knopf ? */
  {
    hide_show(MOUSE_HIDE); /* Mauszeiger verstecken */
    push_win_back(); /* Fenster zum hintersten machen */
    do_refresh();
    hide_show(MOUSE_SHOW); /* Mauszeiger wieder anzeigen */
  }
  else /* linker Mausknopf war gedrÅckt */
  {
    /* warten, bis entweder der Mausknopf losgelassen wird, oder */
    /* die Maus bewegt wird. */
#ifdef OS2
    MouReadEventQue (m, &wait, mouse_handle);
    while (m->button_status & (MOUSE_BUT1_MOVE | MOUSE_BUT1))
    {  /* Maus wurde mit gedrÅcktem linken Knopf bewegt. */
#else
    while((m=get_mouse())->button_status & MOUSE_BUT1)
    {
      if(m->x != old_x || m->y != old_y)  /* Maus wurde mit gedrÅcktem Knopf */
      {                                   /* bewegt. */
#endif
	hide_show(MOUSE_HIDE);
	if(!moved)
	{
	  moved = TRUE;
	  werase (akt_winp->winp);  /* Fensterinhalt auf Bildschirm loeschen */
	  wrefresh(akt_winp->winp); /* damit kein Fehler beim Verschieben passiert */
	  cpwins2stdscr(); /* Alle Fenster ausser akt. nach stdscr kopieren */
	}
	else
	  eckenhw();                 /* Ecken des Fensters demarkieren      */
	if(m->x > old_x)
	  win_right(m->x - old_x);
	else
	  win_left(old_x - m->x);
	if(m->y > old_y)
	  win_down(m->y - old_y);
	else
	  win_up(old_y - m->y);
	old_x = m->x; /* Mauskoordinaten merken */
	old_y = m->y;
	eckenhw();                   /* Ecken markieren */
	hide_show(MOUSE_SHOW);
#ifdef OS2
      MouReadEventQue (m, &wait, mouse_handle);
#else
      }
#endif
    }
    if(moved)    /* Falls Fenster verschoben wurde, Ecken wieder lîschen */
    {            /* und Fenster neu zeichnen */
      hide_show(MOUSE_HIDE);
      eckenhw();
      erase();
      refresh();
      mvwin (akt_winp->winp,akt_winp->y,akt_winp->x); /* Fenster verschieben */
      do_refresh();                   /* Alle Fenster neu zeichnen           */
      hide_show(MOUSE_SHOW);
    }
  }
}

/******************************************************************************
*
* Funktion     : Fenstergrî·e mit Maus Ñndern (mouse_sizewin)
* --------------
*
* Parameter    : m           :
*                  Typ          : mouse_type *
*                  Wertebereich : Pointer aus Mausstruktur
*                  Bedeutung    : Mauszustand mit Position und KnopfdrÅcken
*
* Beschreibung : Das aktuelle Fenster wird durch die Maus vergrî·ert
*                oder verkleinert.
*                Die Mauskoordinaten werden solange geprÅft, bis der
*                linke Mausknopf nicht mehr gedrÅckt ist. Dann wird
*                das Fenster in seiner dortigen Grî·e gezeichnet.
*
******************************************************************************/

void mouse_sizewin(m)
mouse_type *m;
{
  int old_x = m->x, /* Ablage fÅr alte Mauskoordinaten */
      old_y = m->y,
      moved = FALSE;
#ifdef OS2
  short int wait = 1; /* auf Mausereignisse warten */
#endif

  /* warten, bis entweder der Mausknopf losgelassen wird, oder */
  /* die Maus bewegt wird. */
#ifdef OS2
  MouReadEventQue (m, &wait, mouse_handle);
  while (m->button_status & (MOUSE_BUT1_MOVE | MOUSE_BUT2_MOVE
			   | MOUSE_BUT1 | MOUSE_BUT2))
  {
#else
  while((m=get_mouse())->button_status & (MOUSE_BUT1 | MOUSE_BUT2))
  {
    if(m->x != old_x || m->y != old_y)  /* Maus wurde mit gedrÅcktem Knopf */
    {                                   /* bewegt. */
#endif
      hide_show(MOUSE_HIDE);
      if(!moved)
      {
	moved = TRUE;
	werase (akt_winp->winp);  /* Fensterinhalt auf Bildschirm loeschen */
	wrefresh(akt_winp->winp); /* damit kein Fehler beim Verschieben passiert */
	cpwins2stdscr(); /* Alle Fenster ausser akt. nach stdscr kopieren */
      }
      else
	eckenhw();                 /* Ecken des Fensters demarkieren      */
      if(m->x > old_x)
	size_right(m->x - old_x);
      else
	size_left(old_x - m->x);
      if(m->y > old_y)
	size_down(m->y - old_y);
      else
	size_up(old_y - m->y);
      old_x = m->x; /* Mauskoordinaten merken */
      old_y = m->y;
      eckenhw();                   /* Ecken markieren */
      hide_show(MOUSE_SHOW);
#ifdef OS2
    MouReadEventQue (m, &wait, mouse_handle);
#else
    }
#endif
  }
  if(moved)    /* Falls Fenster verschoben wurde, Ecken wieder lîschen */
  {            /* und Fenster neu zeichnen */
    hide_show(MOUSE_HIDE);
    eckenhw();
    erase();                  /* gedrueckt wurde (RETURN) */
    refresh();                /* Fensterinhalt loeschen */
    delwin(akt_winp->winp);   /* Fenster mit Curses loeschen und neu anlegen */
    akt_winp->winp=newwin(akt_winp->dy+2,akt_winp->dx+2,akt_winp->y,akt_winp->x);
    init_win();               /* Fensterattribute setzen (raw etc.) */

    /* Falls Cursor jetzt ausserhalb des Fensters steht, Cursorposition anpassen */
    if (akt_winp->ws_line+akt_winp->dy <= akt_winp->textline)
      gotox(akt_winp->ws_line+akt_winp->dy-1);
    if (akt_winp->ws_col+akt_winp->dx <= akt_winp->screencol)
      akt_winp->screencol = akt_winp->ws_col+akt_winp->dx-1;
    do_refresh();  /* Alle Fenster neu zeichnen */
    hide_show(MOUSE_SHOW);
  }
}

/******************************************************************************
*
* Funktion     : Bildschirm gemÑ· Mausposition scrollen (check_scroll)
* --------------
*
* Parameter    : m           :
*                  Typ          : mouse_type *
*                  Wertebereich : Pointer auf Mausstruktur
*                  Bedeutung    : Mausstatus
*
* Beschreibung : Die Mausposition liegt au·erhalb des aktuellen Fensters.
*                Der Fensterinhalt soll in Richtung des Mauscursors an-
*                gepa·t werden. Steht also beispielsweise die Maus unter-
*                halb des aktuellen Fensters, so soll der Text im Fenster
*                nach oben geschoben werden (damit z.B. auch fensterÅber-
*                greifende Blîcke markiert werden kînnen.
*
******************************************************************************/

void check_scroll(m)
mouse_type *m;
{
  /* Zuerst die Verschiebung nach oben/unten testen */
  if(m->y < akt_winp->y+1
  && akt_winp->ws_line) /* Verschiebung des Inhalts nach unten */
  {
    akt_winp->ws_line--; /* dann Nummer der ersten sichtbaren Zeile erhoehen */
    text_down(0);        /* gesamten Fenstertext um 1 Zeile nach unten */
    up();                /* Cursor um 1 Zeile nach oben bewegen */
    setz_cursor(W_AKT);       /* Cursor an richtige Position setzen */
  }
  else
    if(m->y > akt_winp->y+akt_winp->dy /* Verschiebung der Inhalts nach */
    && akt_winp->ws_line < akt_winp->maxline) /* oben */
    {
      akt_winp->ws_line++; /* dann Nummer der ersten sichtbaren Zeile erhoehen */
      text_up(0);          /* gesamten Fenstertext um 1 Zeile nach oben */
      down();              /* Cursor um 1 Zeile nach unten bewegen */
      setz_cursor(W_AKT);       /* Cursor an richtige Position setzen */
    }

  /* Jetzt Verschiebung nach rechts/links testen */
  if(m->x < akt_winp->x+1
  && akt_winp->ws_col)  /* Verschiebung des Inhalts nach rechts */
  {
    text_right();
    left();
    setz_cursor(W_AKT);       /* Cursor an richtige Position setzen */
  }
  else
    if(m->x > akt_winp->x+akt_winp->dx
    && akt_winp->ws_col < MAXLENGTH)
    {
      text_left();
      right();
      setz_cursor(W_AKT);       /* Cursor an richtige Position setzen */
    }
}

/******************************************************************************
*
* Funktion     : Blockende setzen (mouse_blend)
* --------------
*
* Parameter    : m           :
*                  Typ          : mouse_type *
*                  Wertebereich : Pointer auf Mausstruktur
*                  Bedeutung    : Mauszustand
*
*              : old_x       :
*                  Typ          : int
*                  Wertebereich : 0-COLS
*                  Bedeutung    : Alte Mausposition Spalte
*
*              : old_y       :
*                  Typ          : int
*                  Wertebereich : 0-LINES
*                  Bedeutung    : Alte Mausposition Zeile
*
*              : bl_typ      :
*                  Typ          : char
*                  Wertebereich : BT_RECHTECK, BT_NORMAL
*                  Bedeutung    : Blocktyp
*
*              : need_show_win:
*                  Typ          : char
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : Wenn TRUE, mu· statt der Anzeige der Zeilen
*                                 zwischen alter und neuer Mausposition
*                                 ein show_win ausgefÅhrt werden.
*
* Beschreibung : Die Maus wurde mit gedrÅcktem Mausknopf von der Position
*                old_x/old_y nach der von m angegebenen Position verschoben.
*                An der neuen Position mu· also nun das Blockende gesetzt
*                werden und der so entstandene neue Block gehighlighted
*                werden, falls die Blockhervorhebung aktiviert ist.
*                Dazu werden die Zeilen zwischen dem alten Blockende (old_y)
*                und dem neuen Blockende (m->y) mittels lineout neu
*                angezeigt.
*
******************************************************************************/

void mouse_blend (mouse_type *m, int old_x, int old_y,
		  char bl_typ, char need_show_win)
{
  int y, /* Fensterzeile, in der aktuelle Zeile angezeigt wird */
      i, /* zÑhlt die Zeilen, um die sich Maus bewegt hat. */
      old_tl = akt_winp->textline, /* zur Restaurierung nîtig */
      woy,  wy,   /* Mauskoordinaten in Bezug auf das Window */
      first_l, last_l, /* bei rechteckigem Block: 1. und */
		       /* letzte anzuzeigende Zeile      */
      direction_dn; /* FALSE fÅr Mausbewegung nach oben, TRUE fÅr
		       Mausbewegung nach unten oder Stillstand */
  char block_got_unmarked;  /* Wurde Block bei dieser Aktion unmarkiert? */

  /* PrÅfen, ob Block durch diese Aktion einen negativen Zeilenumfang
     erhÑlt. Dann ist nÑmlich auch eine Nachbesserung vorzunehmen */
  if (akt_winp->block.e_line >= akt_winp->block.s_line
  &&  akt_winp->textline < akt_winp->block.s_line)
    block_got_unmarked = TRUE;
  else
    block_got_unmarked = FALSE;

  akt_winp->block.e_line = akt_winp->textline; /* Zeile und Spalte als Block- */
  akt_winp->block.e_col = akt_winp->screencol; /* ende eintragen              */
  akt_winp->block.typ = bl_typ;                /* Blocktyp eintragen          */
  if (need_show_win)
    show_win (W_AKT);
  else
  {
    /* Wenn Block zu highlighten ist und (der Block Zeilenumfang >= 0 hat
       oder gerade durch diesen Aufruf negativen Zeilenumfang erhÑlt),
       dann ist die Blockdarstellung nachzubessern. */
    if (highblockflag &&
    (akt_winp->block.s_line <= akt_winp->block.e_line || block_got_unmarked))
    {
      if (bl_typ == BT_NORMAL  /* bei normalem Block und bei rechteckigem   */
      ||  m->x == old_x) /* Block, falls sich die x-Position nicht geÑndert */
      { /* hat, mÅssen nur die Zeilen, die von der Maus Åberstrichen wurden,*/
	/* neu angezeigt werden. */
	direction_dn = m->y >= old_y;
	y = akt_winp->textline - akt_winp->ws_line;
	for (i=m->y; direction_dn ? i >= old_y : i <= old_y;
	     direction_dn ? i-- : i++)
	{
	  lineout (direction_dn ? y-- : y++);
	  direction_dn ? up() : down();
	}
      }
      else /* bei rechteckigem Block mÅssen alle sichtbaren Blockzeilen neu */
      { /* angezeigt werden, falls sich die x-Koordinate geÑndert hat.
	   Es wird immer mit der ersten sichtbaren Blockzeile begonnen und
	   von dort aus nach unten gegangen. */
	woy = akt_winp->ws_line-1-akt_winp->y+old_y; /* umrechnen in       */
	wy  = akt_winp->ws_line-1-akt_winp->y+m->y;  /* Form               */
	if (woy < wy) { first_l = woy; last_l  = wy;  }
	else          { first_l = wy;  last_l  = woy; }
	if (akt_winp->block.s_line < akt_winp->ws_line) /* Block beginnt */
	  first_l = akt_winp->ws_line;           /* oberhalb von Fenster */
	else /* beginnt Block oberhalb von Mausbewegungsbereich ? */
	  if (akt_winp->block.s_line < first_l)
	    first_l = akt_winp->block.s_line;
	if (akt_winp->block.e_line > last_l)
	  last_l = akt_winp->block.e_line;
	/* Jetzt liegt first_l und last_l im Fenster */
	gotox (first_l);
	y = akt_winp->textline - akt_winp->ws_line;
	for (i=first_l; i<=last_l; i++, down())
	  lineout (y++);
      }
      gotox (old_tl);
    }
    setz_cursor (W_AKT);
  }
}

/******************************************************************************
*
* Funktion     : Cursor setzen oder Block markieren (set_cursor_or_mark_block)
* --------------
*
* Parameter    : m           :
*                  Typ          : mouse_type *
*                  Wertebereich : Pointer auf Mausstruktur
*                  Bedeutung    : Mauszustand
*
* Beschreibung : Wird der Mausknopf ohne Mausbewegung losgelassen, dann
*                wird lediglich der Cursor auf die Mausposition gesetzt.
*                Wird die Maus jedoch mit gedrÅcktem Mausknopf bewegt,
*                dann wird damit ein Block markiert.
*
******************************************************************************/

void set_cursor_or_mark_block(m)
mouse_type *m;
{
#ifdef OS2
  short int wait = 1; /* auf Mausereignisse warten */
  mouse_type old_event; /* Damit bei leerem Event Position nicht weg ist */
#endif
  int old_x = m->x, /* Ablage fÅr alte Mauskoordinaten */
      old_y = m->y;
  int old_button = m->button_status, /* Zwischenspeicher fÅr Knîpfe */
      moved = FALSE;
  char event_valid = TRUE, /* Wird auf FALSE gesetzt, wenn in OS/2 leeres
			      Mouse-Event auftritt (bei wait=0 mîglich) */
       need_show_win = TRUE; /* wird TRUE, wenn in mouse_bl...end nicht
				nur die Zeilen zwischen alter und neuer
				Mausposition neu angezeigt werden mÅssen,
				sondern ein show_win notwendig geworden ist. */

  /* zunÑchst wird der Cursor an die Mausposition gesetzt */
  gotox(akt_winp->ws_line + m->y - 1 - akt_winp->y);
  akt_winp->screencol = akt_winp->ws_col + m->x - 1 - akt_winp->x;
  hide_show (MOUSE_HIDE);
  setz_cursor(W_AKT);
  hide_show (MOUSE_SHOW);
  /* warten, bis entweder der Mausknopf losgelassen wird, oder */
  /* die Maus bewegt wird. */
#ifdef OS2
  MouReadEventQue (m, &wait, mouse_handle);
  if (!(old_button & MOUSE_BUT3)) /* Wenn Mitte, dann sofort zur Block- */
  {                               /* behandlung springen                */
    /* solange, wie die Maus bewegt wird, oder kein Mausereignis stattgefunden
       hat (nur bei wait==0 mîglich) die Blockgrî·en anpassen. */
    while ((m->button_status & (MOUSE_BUT1_MOVE | MOUSE_BUT2_MOVE
			      | MOUSE_BUT1 | MOUSE_BUT2))
	  || (!wait && !m->button_status && !m->time && !m->x && !m->y))
    {
      if (!wait && !m->button_status && !m->time && !m->x && !m->y)
	event_valid = FALSE;  /* leeres Event gelesen bei wait=0 */
      else
      {
	old_event = *m;      /* gÅltige Events werden gemerkt */
	event_valid = TRUE;
      }
#else
    while((m=get_mouse())->button_status & (MOUSE_BUT1 | MOUSE_BUT2))
    {
#endif
      if(event_valid && in_akt_win(m->x,m->y)) /* Innerhalb des Fenster ? */
      {
#ifdef OS2
	wait = 1; /* Innerhalb auf Mausereignis warten */
#endif
	if(m->x != old_x || m->y != old_y)  /* Maus wurde mit gedrÅcktem Knopf */
	{                                   /* bewegt: */
	  hide_show(MOUSE_HIDE);
	  if(!moved) /* Falls erste Bewegung: */
	  {
	    moved = TRUE;
	    /* Jetzt Blockanfang auf die alte Cursorposition setzen */
	    do_blstart();
	    /* Dann Cursor auf neue Position setzen */
	    gotox(akt_winp->ws_line + m->y - 1 - akt_winp->y);
	    akt_winp->screencol = akt_winp->ws_col + m->x - 1 - akt_winp->x;
	  }
	  else
	  {
	    gotox(akt_winp->ws_line + m->y - 1 - akt_winp->y);
	    akt_winp->screencol = akt_winp->ws_col + m->x - 1 - akt_winp->x;
	  }
#ifdef OS2
	  if(m->button_status & MOUSE_BUT1_MOVE)
#else
	  if(m->button_status & MOUSE_BUT1) /* Bei linkem Knopf normaler Block */
#endif
	    mouse_blend(m, old_x, old_y, BT_NORMAL, need_show_win);
	  else                     /* Bei rechtem Knopf rechteckiger Block */
	    mouse_blend(m, old_x, old_y, BT_RECHTECK, need_show_win);
	  hide_show(MOUSE_SHOW);
	  old_x = m->x; /* neue Mauskoordinaten merken */
	  old_y = m->y;
	  need_show_win = FALSE; /* Block wurde richtig angezeigt, */
	} /* also mu· beim nÑchsten mal nur die Differenz gezeigt  */
      }   /* werden */
      else /* Maus steht au·erhalb des Fensters */
      { /* Jetzt mu· evtl. der Fensterinhalt gescrollt werden. */
	need_show_win = TRUE;  /* beim nÑchsten Blockendesetzen mu· */
	hide_show(MOUSE_HIDE); /* das Fenster neu gezeichnet werden */
	check_scroll(
#ifdef OS2
		     &old_event);
#else
		     m);
#endif
	hide_show(MOUSE_SHOW);   /* Mauscursor wieder sichtbar machen */
#ifdef OS2
	wait = 0; /* Solange Maus au·erhalb des Fensters steht, nicht auf */
		  /* Mausereignis warten, sondern stÑndig scrollen. */
#endif
	old_x = m->x; /* neue Mauskoordinaten merken */
	old_y = m->y;
      }
#ifdef OS2
      MouReadEventQue (m, &wait, mouse_handle);
    }
#endif
  }
  if(!moved)
    if((old_button | m->button_status) & MOUSE_BUT2) /* Wenn der Mauszeiger nicht mit */
    { /* gedrÅcktem Knopf bewegt wurde und der rechte Knopf gedrÅckt war: */
      hide_show(MOUSE_HIDE);
      if(block_defined()) /* Wenn im aktuellen Fenster ein Block markiert ist, */
      {                    /* und neben dem rechten noch der mittlere Knopf */
	if(old_button & MOUSE_BUT3) /* gedrÅckt war, dann Block lîschen */
	  do_blweg();
	else             /* Wenn nur der rechte Knopf gedrÅckt war, */
	{
	  do_blcut();    /* dann diesen Cutten */
	  do_blunmark(); /* und anschlie·end unmarkieren */
	}
      }
      else          /* Ist jedoch kein Block markiert, dann Pasten */
	do_blpaste();
      hide_show(MOUSE_SHOW);
    }
    else
      if(((old_button | m->button_status) & (MOUSE_BUT1 | MOUSE_BUT3))
      == (MOUSE_BUT1 | MOUSE_BUT3))
      { /* linker und mittlerer Knopf gedrÅckt ? */
	hide_show(MOUSE_HIDE);
	do_blmove();
	hide_show(MOUSE_SHOW);
      }
}

/******************************************************************************
*
* Funktion     : Mausposition in SchaltfÑache wandeln (pos_to_button)
* --------------
*
* Parameter    : m           :
*                  Typ          : mouse_type *
*                  Wertebereich : Pointer auf Mausstruktur
*                  Bedeutung    : Mausstatus
*
* Ergebnis     :
*                  Typ          : int
*                  Wertebereich : MOUSE_...
*                  Bedeutung    : Bezeichner der SchaltflÑche
*
* Beschreibung : Die Mausposition wird mit der Lage der SchaltflÑchen
*                verglichen und die entsprechende SchaltflÑche
*                zurÅckgeliefert.
*
******************************************************************************/

int pos_to_button(m)
mouse_type *m;
{
  if(in_akt_win(m->x,m->y))
    return(MOUSE_TEXT); /* Maus steht im Textbereich */
  if(m->x == akt_winp->x && m->y == akt_winp->y)
    return(MOUSE_CLOSE); /* linke obere Ecke: Schlie·knopf */
  if(m->x == akt_winp->x+akt_winp->dx+1 && m->y == akt_winp->y)
    return(MOUSE_TOG_SIZE);
  if(m->x == akt_winp->x+akt_winp->dx+1 
  && m->y == akt_winp->y+akt_winp->dy+1)
    return(MOUSE_SIZE);
  if(m->y == akt_winp->y)
    return(MOUSE_KOPF);
  if(m->x == akt_winp->x+akt_winp->dx+1)
    if(m->y == akt_winp->y+1)
      return(MOUSE_SCROLL_DOWN);
    else
      if(m->y == akt_winp->y+akt_winp->dy)
	return(MOUSE_SCROLL_UP);
  if(m->y == akt_winp->y+akt_winp->dy+1)
    if(m->x == akt_winp->x+1)
      return(MOUSE_SCROLL_RIGHT);
    else
      if(m->x == akt_winp->x+akt_winp->dx)
	return(MOUSE_SCROLL_LEFT);
  return(MOUSE_RAHMEN);
}

/******************************************************************************
*
* Funktion     : Fenster mit Maus schlie·en (mouse_win_zu)
* --------------
*
* Beschreibung : Offensichtlich kann wÑhrend der Maus-Routine keine andere
*                Maus-Routine installiert werden. Das hat zur Folge, da·
*                beim AusfÅhren der Mausfunktion "Fenster schlie·en" im
*                Falle der AusfÅhrung der Funktion ja_nein dort keine
*                neue Maus-Routine installiert werden kann, wenn die alte
*                Maus-Routine noch lÑuft.
*                Die Ñu·erst unschîne Lîsung, die nach Verbesserung schreit,
*                ist folgende:
*                In den Tastaturpuffer wird der Code fÅr das Kommando
*                "Fenster schlie·en" geschrieben.
*                Dann wird die Maus-Routine verlassen. WÑhrend der AusfÅhrung
*                des Kommandos ist die alte Maus-Routine sowieso gesperrt und
                die neue kann bedenkenlos installiert werden.
*
******************************************************************************/

void mouse_win_zu()
{
#ifdef OS2
  hide_show (MOUSE_HIDE);
  do_win_zu(FALSE);
  hide_show (MOUSE_SHOW);
#else
  int kom_nr=0, /* Nummer des Kommandos do_win_zu */
      i;        /* Zum Durchlaufen des Kommandostrings */
  short int *key_buff_in = (short int *) 0x41C,
	    *key_buff    = (short int *) 0x41E; /* Tastaturpuffer */

  /* Zuerst den Index des Kommandos "Fenster schlie·en" ermitteln */
  while(funktion[kom_nr] != do_win_zu && kom_nr++ <= mc_index);

  /* Jetzt Schreibadresse in Tastaturpuffer ermitteln: */
  key_buff += (*key_buff_in - 0x1E) / 2;
  for(i=0;i<comm_feld[kom_nr].blen;i++)
  {
    *key_buff++ = comm_feld[kom_nr].befehl[i];
    if(key_buff == (short int *) 0x43E) /* Am Ende wrap around */
      key_buff = (short int *) 0x41E;
  }
  *key_buff_in = FP_OFF(key_buff) - 0x400;
#endif
}

/******************************************************************************
*
* Funktion     : Mit Maus nach rechts scrollen (msr)
* --------------
*
* Beschreibung : Der Mauszeiger wird versteckt. Dann wird der Text im Fenster
*                nach rechts geschoben und nîtigenfalls der Cursor angepa·t.
*                Dann wird setz_cursor aufgerufem imd der Mauszeiger wieder
*                angezeigt.
*                Der Verschiebe-Vorgang wird solange wiederholt, bis kein
*                Mausknopf mehr gedrÅckt ist.
*
******************************************************************************/

void msr()
{
#ifdef OS2
  mouse_type m;
  short int wait = 0; /* auf Mausereignisse warten */
#endif

  hide_show(MOUSE_HIDE); /* Mauszeiger verstecken */
#ifdef OS2
  MouReadEventQue (&m, &wait, mouse_handle);
  while ((m.button_status & (MOUSE_BUT1 | MOUSE_BUT2 | MOUSE_BUT1_MOVE
			   | MOUSE_BUT2_MOVE))
       || (!m.button_status && !m.x && !m.y && !m.time)) /* kein Event */
#else
  while(get_mouse()->button_status & (MOUSE_BUT1 | MOUSE_BUT2))
#endif
  {
    text_right();          /* Text nach rechts scrollen */
    /* Falls Cursor am rechten Rand "rausgefallen", dann eins nach links */
    if(akt_winp->screencol >= akt_winp->ws_col + akt_winp->dx)
      left();
    setz_cursor(W_AKT);
#ifdef OS2
    MouReadEventQue (&m, &wait, mouse_handle);
#endif
  }
  hide_show(MOUSE_SHOW);
}

/******************************************************************************
*
* Funktion     : Mit Maus nach links scrollen (msl)
* --------------
*
* Beschreibung : Der Mauszeiger wird versteckt. Dann wird der Text im Fenster
*                nach links geschoben und nîtigenfalls der Cursor angepa·t.
*                Dann wird setz_cursor aufgerufem imd der Mauszeiger wieder
*                angezeigt.
*                Der Verschiebe-Vorgang wird solange wiederholt, bis kein
*                Mausknopf mehr gedrÅckt ist.
*
******************************************************************************/

void msl()
{
#ifdef OS2
  mouse_type m;
  short int wait = 0; /* auf Mausereignisse warten */
#endif

  hide_show(MOUSE_HIDE); /* Mauszeiger verstecken */
#ifdef OS2
  MouReadEventQue (&m, &wait, mouse_handle);
  while ((m.button_status & (MOUSE_BUT1 | MOUSE_BUT2 | MOUSE_BUT1_MOVE
			   | MOUSE_BUT2_MOVE))
       || (!m.button_status && !m.x && !m.y && !m.time)) /* kein Event */
#else
  while(get_mouse()->button_status & (MOUSE_BUT1 | MOUSE_BUT2))
#endif
  {
    text_left();          /* Text nach links scrollen */
    /* Falls Cursor am linken Rand "rausgefallen", dann eins nach rechts */
    if(akt_winp->screencol < akt_winp->ws_col)
      right();
    setz_cursor(W_AKT);
#ifdef OS2
    MouReadEventQue (&m, &wait, mouse_handle);
#endif
  }
  hide_show(MOUSE_SHOW);
}

/******************************************************************************
*
* Funktion     : Mit Maus nach oben scrollen (msu)
* --------------
*
* Beschreibung : Der Mauszeiger wird versteckt. Dann wird der Text im Fenster
*                nach oben geschoben und nîtigenfalls der Cursor angepa·t.
*                Dann wird setz_cursor aufgerufem imd der Mauszeiger wieder
*                angezeigt.
*                Der Verschiebe-Vorgang wird solange wiederholt, bis kein
*                Mausknopf mehr gedrÅckt ist.
*
******************************************************************************/

void msu()
{
#ifdef OS2
  mouse_type m;
  short int wait = 0; /* auf Mausereignisse warten */
#endif

  hide_show(MOUSE_HIDE);  /* Mauszeiger verstecken */
#ifdef OS2
  MouReadEventQue (&m, &wait, mouse_handle);
  while (((m.button_status & (MOUSE_BUT1 | MOUSE_BUT2 | MOUSE_BUT1_MOVE
			   | MOUSE_BUT2_MOVE))
       || (!m.button_status && !m.x && !m.y && !m.time)) /* kein Event */
#else
  while(get_mouse()->button_status & (MOUSE_BUT1 | MOUSE_BUT2)
#endif
  && akt_winp->ws_line < akt_winp->maxline)
  {
    akt_winp->ws_line++; /* dann Nummer der ersten sichtbaren Zeile erhoehen */
    text_up(0);          /* gesamten Fenstertext um 1 Zeile nach oben */
    if(akt_winp->textline < akt_winp->ws_line) /* Falls Cursor in oberster */
      down();            /* Zeile, dann Cursor um 1 Zeile nach unten bewegen */
    setz_cursor(W_AKT);       /* Cursor an richtige Position setzen */
#ifdef OS2
    MouReadEventQue (&m, &wait, mouse_handle);
#endif
  }
  hide_show(MOUSE_SHOW);
}

/******************************************************************************
*
* Funktion     : Mit Maus nach unten scrollen (msd)
* --------------
*
* Beschreibung : Der Mauszeiger wird versteckt. Dann wird der Text im Fenster
*                nach unten geschoben und nîtigenfalls der Cursor angepa·t.
*                Dann wird setz_cursor aufgerufem imd der Mauszeiger wieder
*                angezeigt.
*                Der Verschiebe-Vorgang wird solange wiederholt, bis kein
*                Mausknopf mehr gedrÅckt ist.
*
******************************************************************************/

void msd()
{
#ifdef OS2
  mouse_type m;
  short int wait = 0; /* auf Mausereignisse warten */
#endif

  hide_show(MOUSE_HIDE); /* Mauszeiger verstecken */
#ifdef OS2
  MouReadEventQue (&m, &wait, mouse_handle);
  while (((m.button_status & (MOUSE_BUT1 | MOUSE_BUT2 | MOUSE_BUT1_MOVE
			   | MOUSE_BUT2_MOVE))
       || (!m.button_status && !m.x && !m.y && !m.time)) /* kein Event */
#else
  while(get_mouse()->button_status & (MOUSE_BUT1 | MOUSE_BUT2)
#endif
  && akt_winp->ws_line)
  {
    akt_winp->ws_line--; /* Nummer der ersten sichtbaren Zeile dekrementieren */
    text_down(0);        /* Text ab Zeile 0 (ganzes Fenster) um 1 nach unten  */
    if(akt_winp->textline >= akt_winp->ws_line + akt_winp->dy)
      up(); /* Stand Cursor in letzter Schirmzeile, Cursor 1 Zeile hoch */
    setz_cursor(W_AKT);        /* Cursor an richtige Position */
#ifdef OS2
    MouReadEventQue (&m, &wait, mouse_handle);
#endif
  }
  hide_show(MOUSE_SHOW);
}

/******************************************************************************
*
* Funktion     : Fenstergrî·e mit Maus togglen (mouse_tog_size)
* --------------
*
* Beschreibung : Diese Routine wird aufgerufen, wenn der Benutzer mit der
*                Maus auf die obere rechte Fensterecke geklickt hat. Es
*                wird die Fenstergrî·e korrekt eingestellt. Im Falle von OS2
*                mu· man anschlie·end noch auf das Loslassen des Mausknopfs
*                warten, da dies sonst als weiteres Ereignis in der Queue
*                landet.
*
******************************************************************************/

void mouse_tog_size()
{
  mouse_type m;
  short int wait = 1; /* Auf Mausereignis warten */

  hide_show(MOUSE_HIDE);
  do_toggle_size();
  hide_show(MOUSE_SHOW);
#ifdef OS2            /* Im Fall von OS2 auf Loslassen des Knopfes warten */
  do
  {
    MouReadEventQue (&m, &wait, mouse_handle);
  } while (m.button_status & (MOUSE_BUT1 | MOUSE_BUT2 | MOUSE_BUT3
	 |  MOUSE_BUT1_MOVE | MOUSE_BUT2_MOVE | MOUSE_BUT3_MOVE));
#endif
}

/******************************************************************************
*
* Funktion     : Maus-Routine (mouse_routine)
* --------------
*
* Beschreibung : Diese Routine wird angesrpugen, wenn mit set_mouse_int eine
*                Event-Mask ungleich 0 eingetragen wurde und das entsprechende
*                Ereignis eingetreten ist.
*                Diese Funktion fÅhrt dann die entsprechenden Mausfunktionen
*                aus.
*
******************************************************************************/

void mouse_routine(
#ifdef OS2
		   mouse_type *m
#endif
		  )
{
#ifdef OS2  /* Wurde einer der Knîpfe gedrÅckt, oder hat man nur eine
	       Message "Losgelassen" empfangen? */
  if (m->button_status & (MOUSE_BUT1 | MOUSE_BUT2 | MOUSE_BUT3))
  {
#else
  mouse_type *m;    /* Mausstatus */

  asm push ds;
  asm push es;
  asm mov  ax,DGROUP;
  asm mov  es,ax;
  asm mov  ds,ax;

/*  outportb(32,(char) 32); Interrupt fÅr beendet erklÑren, (nicht auf 386)*/
	  /* damit z.B. im Schneider PC Maus wieder erkannt wird. */
  m = get_mouse();  /* Mauskoordinaten und Knopfstatus einlesen */
#endif
  check_buff();     /* Pufferinhalt in Text uebernehmen       */
  if(pos_to_win(m->x,m->y) != akt_winp) /* Klick auf aktuelles Fenster? */
  {                     /* Nein: */
    hide_show(MOUSE_HIDE); /* Maus solange verstecken */
    kopf(W_NOTAKT);     /* Altes Fenster als inaktiv markieren */
    rahmen(W_NOTAKT);
    wrefresh(akt_winp->winp);
    sw_pos(m->x,m->y);  /* Zum Fenster wechseln */
    show_win(W_AKT);    /* Fenster neu zeichnen */
    hide_show(MOUSE_SHOW); /* Maus wieder anzeigen */
  }
  switch(pos_to_button(m))
  {
    case MOUSE_TEXT        : set_cursor_or_mark_block(m); break;
    case MOUSE_KOPF        : mouse_movewin(m); break;
    case MOUSE_SIZE        : mouse_sizewin(m); break;
    case MOUSE_CLOSE       : mouse_win_zu(); break;
    case MOUSE_TOG_SIZE    : mouse_tog_size(); break;

    case MOUSE_SCROLL_UP   : msu(); break;
    case MOUSE_SCROLL_DOWN : msd(); break;
    case MOUSE_SCROLL_RIGHT: msr(); break;
    case MOUSE_SCROLL_LEFT : msl(); break;
  }

#ifdef OS2
  }
#else
  asm pop  es;
  asm pop  ds;
#endif
}

/******************************************************************************
*
* Funktion     : Maus-Routine fÅr Funktion ja_nein (jn_mouse_routine)
* --------------
*
* Beschreibung : Diese Routine wird angesrpugen, wenn mit mouse_jn_init die
*                Maus aktiviert wurde und das entsprechende Ereignis
*                eingetreten ist. (DOS-Fall)
*                Bei OS/2 wird diese Funktion als ein eigener Thread
*                gestartet. FÅhrt ein Maus-Ereignis zum Setzen der Variablen
*                mouse_jn, so wird der Thread beendet. Ansonsten mu· der
*                Thread extern beendet werden (DosKillThread), falls
*                die JN-Antwort von anderswo erhalten wurde.
*
*                Diese Funktion setzt dann gemÑ· dem gedrÅckten Mausknopf
*                die globale Variable mouse_jn auf KLICK_LEFT bzw.
*                KLICK_RIGHT. 
*
******************************************************************************/

void jn_mouse_routine()
{
#ifdef OS2
  mouse_type m;
  short int wait = 0; /* Auf Mausereignis warten */

  while (mouse_active && mouse_jn == NO_KLICK)
  {
    MouReadEventQue (&m, &wait, mouse_handle);
    if (!m.time && !m.button_status) /* Kein Ereignis? Dann 10 ms warten */
      DosSleep (10);
    else
      if (m.button_status & (MOUSE_BUT1 | MOUSE_BUT1_MOVE))
      {
/*      DosSleep(200);    eee ohne diese Zeile terminiert in einigen FÑllen
			  der mouse_thread nicht: Wenn beispielsweise der
			  Editor durch "Beenden ohne Sichern" verlassen
			  wird, die Sicherheitsabfrage jedoch mit der Maus
			  beantwortet wird, so hÑngt der mouse_thread
			  ohne diese Zeile... */
	mouse_jn = KLICK_LEFT;
      }
      else if (m.button_status & (MOUSE_BUT2 | MOUSE_BUT2_MOVE))
	mouse_jn = KLICK_RIGHT;
  }
#else
  int event; /* FÅr Ablage des eingetretenen Ereignisses */

  asm push ds;
  asm push es;
  asm mov  bx,DGROUP;
  asm mov  es,bx;
  asm mov  ds,bx;
  event = _AX;
  mouse_jn = event & 8 ? KLICK_RIGHT : KLICK_LEFT;
  asm pop  es;
  asm pop  ds;
#endif
}

/******************************************************************************
*
* Funktion     : Mouse-Interrupt aktivieren/sperren (mouse_jn_init)
* --------------
*
* Parameter    : on_off    :
*                  Typ          : int
*                  Wertebereich : TRUE, FALSE
*                  Bedeutung    : TRUE : jn_mouse_routine aktivieren
*                                 FALSE: jn_mouse_routine deaktivieren
*
* Beschreibung : Mittels des Mausinterrupts 51, Funktion 12 wird die
*                Funktion jn_mouse_routine als anzuspringende Funktion
*                eingetragen. Die Event-Mask wird auf rechten Knopf +
*                linken Knopf gesetzt, falls aktiviert wird, auf 0 sonst.
*
******************************************************************************/

void mouse_jn_init(on_off)
int on_off;
{
#ifdef OS2
  mouse_jn_active = on_off; /* mouse_jn soll auch in der "normalen"
			       Mouse-Kontrollschleife korrekt gesetzt
			       werden. */
  if (on_off)
  {
    mouse_jn = NO_KLICK;
    /* eigener Thread fÅr Maus-JN */
    mouse_jn_ThreadID = _beginthread (jn_mouse_routine, 0, 20000, 0);
  }
  /* else-part nicht nîtig, da jn_mouse_routine sich selbst beendet, wenn
     mouse_jn_active=FALSE festgestellt wird. */
#else
  union  REGS  regs;
  struct SREGS sregs;

  /* Mauszeiger lîschen, falls Maus demaskiert wird, sonst */
  /* Mauszeiger anzeigen */
  if(on_off) /* Bei Initialisierung: */
    mouse_jn = NO_KLICK; /* Variable initialisieren */
  hide_show(on_off ? MOUSE_SHOW : MOUSE_HIDE);
  regs.x.ax  = 12;
  regs.x.cx  = on_off ? MOUSE_MASK : 0;
  regs.x.dx  = FP_OFF(jn_mouse_routine);
  sregs.es   = FP_SEG(jn_mouse_routine);
  int86x(51,&regs,&regs,&sregs);
#endif
}

/******************************************************************************
*
* Funktion     : Mouse-Interrupt aktivieren/sperren (set_mouse_int)
* --------------
*
* Parameter    : event_mask :
*                  Typ          : int
*                  Wertebereich : 0-31
*                  Bedeutung    : Bit 0 : Mausbewegung melden
*                                 Bit 1 : Druck linker Mausknopf melden
*                                 Bit 2 : Lîsen linker Mausknopf melden
*                                 Bit 3 : Druck rechter Mausknopf melden
*                                 Bit 4 : Lîsen linker Mausknopf melden
*
* Beschreibung : Mittels des Mausinterrupts 51, Funktion 12 wird die
*                Funktion mouse_routine als anzuspringende Funktion
*                eingetragen. Die Event-Mask wird gesetzt.
*
******************************************************************************/

#ifndef OS2
void set_mouse_int(event_mask)
int event_mask;
{
  union  REGS  regs;
  struct SREGS sregs;

  /* Mauszeiger lîschen, falls Maus demaskiert wird, sonst */
  /* Mauszeiger anzeigen */
  hide_show(event_mask ? MOUSE_SHOW : MOUSE_HIDE);
  regs.x.ax  = 12;
  regs.x.cx  = event_mask; /* Linken Mausknopf melden */
  regs.x.dx  = FP_OFF(mouse_routine);
  sregs.es   = FP_SEG(mouse_routine);
  int86x(51,&regs,&regs,&sregs);
}
#endif

/******************************************************************************
*
* Funktion     : Maus initialisieren und abfragen (mouse_thread) (Nur OS/2!!!)
* --------------
*
* Beschreibung : Es werden die Attribute des Maus-Cursors festgelegt, der
*                Maus-Cursor wird eingeschaltet, als Events sollen Druck des
*                linken oder rechten Mausknopfs erkannt werden.
*                Der ZÑhler sleep_time gibt die Anzahl der Mikrosekunden an,
*                die zwischen zwei Abfragen gewartet werden soll. Dieser
*                ZÑhler wird nach einem gefundenen Mausereignis auf 0 gesetzt,
*                da weitere Mausereignisse erwartet werden. Schnell nach-
*                folgende Ereignisse werden somit nahezu verzîgerungsfrei be-
*                arbeitet, wÑhrend nach lÑngerer Zeit eine maximale Zeit
*                von 1/10 s bis zur Bearbeitung des Ereignisses vergehen kann.
*                Durch diese Technik wird trotz des Pollings die Systemaus-
*                lastung gering gehalten.
*
******************************************************************************/

#ifdef OS2
void mouse_thread ()
{
  short int  wait = 0,            /* nicht auf Ereignisse warten */
	     mask = MOUSE_MASK;
  int        sleep_time = 0;      /* sleep in us zwischen zwei Aufrufen */
  mouse_type event;

  MouOpen (NULL, &mouse_handle);  /* Maus "îffnen" */
  hide_show (MOUSE_SHOW);
  MouSetEventMask (&mask, mouse_handle);
  while (mouse_active)
  {
    DosSleep (sleep_time/1000); /* Zwischen Ereignissen warten */
    if (sleep_time < 100000)    /* Auf maximal 100 ms erhîhen */
      sleep_time++;
    MouReadEventQue (&event, &wait, mouse_handle);
    if (event.time || event.button_status) /* Ereignis ? */
    {
      sleep_time = 0;      /* Wartezeit wieder von 0 beginnen lassen */
      if (mouse_jn_active)
      { /* Nur das Flag mouse_jn korrekt besetzen, dazu mu· nicht der
	   Semaphor angefordert werden. */
	if (event.button_status & (MOUSE_BUT1 | MOUSE_BUT1_MOVE))
	  mouse_jn = KLICK_LEFT;
	if (event.button_status & (MOUSE_BUT2 | MOUSE_BUT2_MOVE))
	  mouse_jn = KLICK_RIGHT;
      }
      else /* mouse_jn nicht aktiv, also "richtiges" Kommando ausfÅhren */
      {
	DosRequestMutexSem (sem_handle, -1); /* ohne Timeout auf Semaphor warten */
	mouse_routine (&event);
	DosReleaseMutexSem (sem_handle);     /* Semaphor wieder freigeben */
      }
    }
  }
  if (mouse_jn_ThreadID)
    DosWaitThread (&mouse_jn_ThreadID, DCWW_WAIT);
  MouClose (mouse_handle);
}
#endif

/******************************************************************************
*
* Funktion     : Maus initialisieren (init_mouse)
* --------------
*
* Beschreibung : Es werden die Attribute des Maus-Cursors festgelegt, der
*                Maus-Cursor wird eingeschaltet, die Maus-Routine eingebunden
*                und so eingestellt, da· sie durch Druck auf den linken
*                Mausknopf aktiviert wird.
*
******************************************************************************/

void init_mouse()
{
#ifdef OS2
  /* Starte eigenen Mouse-Thread */
  mouse_active = TRUE;
  mouse_ThreadID = _beginthread (mouse_thread, 0, 20000, 0); 
#else
  /* Ruft die Maus-Funktion 12 auf, wodurch der Vektor der User-Routine */
  /* auf die Funktion mouse_routine gesetzt werden soll. */
  union  REGS  regs;
  struct SREGS sregs;

  regs.x.ax  = 10;  /* Set Text Cursor */
  regs.x.bx  = 0;   /* Software-Cursor */
  regs.x.cx  = 0x77ff;  /* Screen Mask */
  regs.x.dx  = 0x7700;  /* Cursor Mask */
  int86(51,&regs,&regs);
  regs.x.ax  = 7;   /* Set Minimum and Maximum X-Cursor Position */
  regs.x.cx  = 0;
  regs.x.dx  = 8*(COLS-1);
  int86(51,&regs,&regs);
  regs.x.ax  = 8;   /* Set Minimum and Maximum Y-Cursor Position */
  regs.x.cx  = 0;
  regs.x.dx  = 8*(LINES-1);
  int86(51,&regs,&regs);
#endif
}
