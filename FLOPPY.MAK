#
.autodepend
.swap

REL=41
RELDIR=\PD
TARGETS=ffcit.exe finfo.exe fgrp.exe ri.exe
#
#   ALL JOBS:
#
all: $(TARGETS)
#
CC    = bccx
.c.obj:
       bccx $(CFLAGS) -c {$*.c }
.asm.obj:
       @tasm /z /zi $*;
#
# Options: (compile time and Defines)
# CFLAGS= -v -d -w -G -N -O -Z -ml -DALPHA_TEST
#
# Most options in TURBOC.CFG file.
#

#CFLAGS = -DALPHA_TEST -DFLOPPY -ml -DREL="$(REL)"
#CFLAGS = -DFLOPPY -ml -DREL="$(REL)" -DALPHA_TEST
CFLAGS = -DFLOPPY -ml -DREL="$(REL)"

#
#       -DALPHA_TEST
#       -DBETA_TEST
#
# Options: (linker)
#LFLAGS= /stack:12288 /NOIGNORECASE /E /F /SEGMENTS:256
LFLAGS=/v /c /s
#
OBJS  = \
account.obj \
ctdl.obj \
doenter.obj \
dogoto.obj \
doread.obj \
dosysop.obj \
doaide.obj \
doupdown.obj \
dochat.obj \
format.obj \
grphall.obj \
ibmcom.obj \
idle.obj \
init.obj \
input.obj \
library.obj \
log.obj \
misc.obj \
modem.obj \
msg.obj \
msg2.obj \
output.obj \
port.obj \
rooma.obj \
timedate.obj \
version.obj \
window.obj \
aplic.obj \
command.obj \
config.obj \
cron.obj \
edit.obj \
files.obj \
info.obj \
infofile.obj \
log2.obj \
log3.obj \
machine.obj \
misc2.obj \
msg3.obj \
net.obj \
roomb.obj \
swap.obj \
sysop1.obj \
xmodem.obj \
video.obj \
zipfile.obj

TCLIBL = c:\tc\lib\EMU.LIB c:\tc\lib\MATHL.lib c:\tc\lib\CL.lib
TCOBJ  = c:\tc\lib\c0l.obj

#
#   RI.EXE
#

ri.exe: ri\ri.c ctdl.h
        @cd $&
        @bccx $&.c
        @-tdstrip $&.exe
        @-pklite $&.exe
        @-rm $&.old
        @copy $&.exe ..
        @cd ..

#
#   FINFO.EXE
#

finfo.exe: finfo\finfo.c finfo\finfo.h
        @cd $&
        @bccx $&.c
        @-tdstrip $&.exe
        @-pklite $&.exe
        @-rm $&.old
        @copy $&.exe ..
        @cd ..
#
#   FGRP.EXE
#

fgrp.exe: fgrp\fgrp.c fgrp\fgrp.h
        @cd $&
        @bccx $&.c
        @-tdstrip $&.exe
        @-pklite $&.exe
        @-rm $&.old
        @copy $&.exe ..
        @cd ..
#
#   ffcit.EXE
#

ffcit.exe: $(OBJS)
        tlinkx $(LFLAGS) @ffcit.rsp

#   beep the speaker (Norton Util)
beep:
        @be beep /f3000 /r5 /d9

# release macro
rel:
        put -l -n *.??v
        vcs -y -V"FFCit Release $(REL)" *.??v
        make -ffloppy clean zip
        ren ffcit.map ffc$(REL).map

#
# update revisions
#
put:
        @put -l -n *.??v
#
# cleanup macro
#
clean:
        @-rm *.obj *.bak *.tmp ffcit.map *.tds *.exe fcit.map
        @cd ri
        @-rm *.obj *.bak *.tmp *.map *.tds *.exe
        @cd ..
        @cd finfo
        @-rm *.obj *.bak *.tmp *.map *.tds *.exe
        @cd ..
        @cd fgrp
        @-rm *.obj *.bak *.tmp *.map *.tds *.exe
        @cd ..
        @-df

# lint check macro
lint:
        @-lnt fcit.lnt
#        @-lnt -format="Error %f %l: %m\n" fcit.lnt

# macro to remove debugger info
strip: ffcit.exe
        @-tdstrip ffcit.exe

zip: all
	@-tdstrip ffcit.exe
	@-pklite ffcit.exe
	@-del ffcit.old
	@-pkzip -u $(RELDIR)\ffcit$(REL) history.cit $(TARGETS)
	@-pkzip -z $(RELDIR)\ffcit$(REL) < citfile\fredcit.asc
	@-pkzip -vc $(RELDIR)\ffcit$(REL)

install: ffcit.exe
        @copy ffcit.exe d:\cit
        @copy history.cit d:\cit

#

#kernal files

ctdl.obj: ctdl.c

swap.obj: swap.asm
        @tasm /w2 /m2 /mx /JMASM51 /zi /z /l /D_Large swap

ibmcom.obj: ibmcom.c

modem.obj: modem.c

port.obj: port.c

window.obj: window.c

output.obj: output.c

command.obj: command.c

timedate.obj: timedate.c

input.obj: input.c

# overlayed files

account.obj: account.c

format.obj: format.c

grphall.obj: grphall.c

idle.obj: idle.c

init.obj: init.c

library.obj: library.c

log.obj: log.c

machine.obj: machine.c machine.h

misc.obj: misc.c

msg.obj: msg.c

msg2.obj: msg2.c

rooma.obj: rooma.c

version.obj: version.c floppy.mak

aplic.obj: aplic.c

config.obj: config.c

cron.obj: cron.c

edit.obj: edit.c

files.obj: files.c

infofile.obj: infofile.c

info.obj: info.c

log2.obj: log2.c

log3.obj: log3.c

misc2.obj: misc2.c

msg3.obj: msg3.c

net.obj: net.c

sysop1.obj: sysop1.c

roomb.obj: roomb.c

xmodem.obj: xmodem.c

zipfile.obj: zipfile.c


dosysop.obj: dosysop.c

doaide.obj: doaide.c

doupdown.obj: doupdown.c

dochat.obj: dochat.c

doenter.obj: doenter.c

dogoto.obj: dogoto.c

doread.obj: doread.c

video.obj: video.c


