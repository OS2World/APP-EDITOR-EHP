CUR_OBJS=cur_main.o cur_wfcs.o cur_wget.o
OBJECTS=aus_1.o aus_2.o aus_3.o block_1.o block_2.o config.o main.o mouse.o regex.o se.o taste.o text_1.o text_2.o window.o $(CUR_OBJS)
OPTFLAG=-O2
OPTFLAG=
#DEBUGFLAG=-g
DEBUGFLAG=

.c.o:
	gcc $(OPTFLAG) -Zmtd $(DEBUGFLAG) -c -funsigned-char -DOS2 -DMOUSE -DCUR_DIRECT $<

all: ehp km

.PHONY: ehp
.PHONY: km

ehp: ehp.exe ehp.inf
km:  km.exe

ehp.inf: ehp.ipf
	ipfc /inf ehp

km.exe: km.o $(CUR_OBJS)
	gcc $(OPTFLAG) -Zmtd $(DEBUGFLAG) -o km.exe km.o $(CUR_OBJS) -los2

ehp.exe: $(OBJECTS)
	gcc $(OPTFLAG) -Zmtd $(DEBUGFLAG) -o ehp.exe $(OBJECTS) -los2

km.o          : curses.h
aus_1.o       : curses.h defs.h
aus_2.o       : curses.h defs.h
aus_3.o       : curses.h defs.h keys.h
block_1.o     : curses.h defs.h
block_2.o     : curses.h defs.h
config.o      : curses.h defs.h
main.o        : curses.h defs.h
mouse.o       : curses.h defs.h
regex.o       : regex.h G_config.h
se.o          : curses.h defs.h regex.h G_config.h
taste.o       : curses.h defs.h keys.h
text_1.o      : curses.h defs.h
text_2.o      : curses.h defs.h
window.o      : curses.h defs.h
cur_main.o    : curses.h
cur_wfcs.o    : curses.h
cur_wget.o    : curses.h keys.h
