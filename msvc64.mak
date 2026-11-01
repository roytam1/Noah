# msvc64.mak -- NMAKE makefile for AMD64 (x64).
#
# Environment: the Win64 cross toolchain. On the reference machine the one
# that works is the "03sp1" set next to the default one:
#
#   set PATH=C:\AMD64sdk\Bin\Win64-03sp1;%PATH%
#   set INCLUDE=C:\AMD64sdk\Include;C:\AMD64sdk\Include\crt
#   set LIB=C:\AMD64sdk\Lib\Win64-03sp1;C:\AMD64sdk\Lib\Win64
#   NMAKE /f msvc64.mak
#
# VCVARS_AMD64.BAT alone is not enough: it selects Bin\Win64, whose cl.exe
# cannot load c1.dll (msvcp70.dll is not present on a normal machine), and its
# Lib\Win64 CRT is an older drop that has no operator new for C++ and keeps
# _amsg_exit inside wwincrt0.obj, so even /entry:kilib_startUp still drags in
# wWinMain and fails to link. The 03sp1 folders beside them have the matching
# compiler and CRT. Any other MSVC that can target x64 should work as well.
#
# IMPORTANT: run this from a LOCAL drive, not a UNC path. CMD.EXE cannot use a
#   UNC share as its current directory, so every recipe command silently runs
#   in the wrong directory (and with the -del / cl / rc prefixes the failure is
#   invisible). If the repo lives on \\server\share, enter it with
#   pushd \\server\share\noah  (which maps a temporary drive) first.
#
# Output: bin-win64\Noah.exe, bin-win64\NoahXt.dll, bin-win64\uninst.exe
# Objects: tmp\win64\ (mirrors the source layout, so that stdafx.obj and
#   uninst\StdAfx.obj do not collide on case-insensitive filesystems).
# Single Release (NDEBUG) configuration.
#
# The _WIN64 specific source changes (GetWindowLongPtr, INT_PTR dialog procs,
# UINT_PTR shell interfaces, amd64 manifest) are behind #ifdef _WIN64, so the
# Win32 build from msvc4.mak / Noah.dsp is unaffected.
#
# NMAKE /f msvc64.mak TRACE=1 builds the instrumented Noah.exe instead.

CC      = cl
RC      = rc
LINK    = link
CFLAGS  = /nologo /W3 /O2 /DWIN32 /DNDEBUG /D_WINDOWS /D_MBCS /DSTRICT /MT /GS- /GR- /I.
RCFLAGS = /l 0x411 /d NDEBUG /d WIN64
LFLAGS  = /nologo /entry:kilib_startUp /subsystem:windows
# This SDK's CRT is itself built with /GS and keeps __security_cookie in
# bufferoverflowu.lib, so it is needed even though our objects use /GS-.
# Toolchains whose CRT carries the cookie can pass SECLIB= on the command line.
SECLIB  = bufferoverflowu.lib
LIBS    = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
          advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
          odbc32.lib odbccp32.lib comctl32.lib lz32.lib version.lib $(SECLIB)
OBJDIR  = tmp\win64
OUTDIR  = bin-win64

!IFDEF TRACE
CFLAGS  = $(CFLAGS) /DNOAH_TRACE
OBJDIR  = $(OBJDIR)-trace
OUTDIR  = bin-win64-trace
!ENDIF

NOAH_OBJS = \
	$(OBJDIR)\stdafx.obj \
	$(OBJDIR)\Noah.obj \
	$(OBJDIR)\NoahAM.obj \
	$(OBJDIR)\NoahCM.obj \
	$(OBJDIR)\SubDlg.obj \
	$(OBJDIR)\Archiver.obj \
	$(OBJDIR)\ArcDLL.obj \
	$(OBJDIR)\ArcACE.obj \
	$(OBJDIR)\ArcB2e.obj \
	$(OBJDIR)\ArcCpt.obj \
	$(OBJDIR)\ArcMsc.obj \
	$(OBJDIR)\ArcXacRett.obj \
	$(OBJDIR)\kilib\kl_app.obj \
	$(OBJDIR)\kilib\kl_carc.obj \
	$(OBJDIR)\kilib\kl_cmd.obj \
	$(OBJDIR)\kilib\kl_dnd.obj \
	$(OBJDIR)\kilib\kl_file.obj \
	$(OBJDIR)\kilib\kl_find.obj \
	$(OBJDIR)\kilib\kl_reg.obj \
	$(OBJDIR)\kilib\kl_rythp.obj \
	$(OBJDIR)\kilib\kl_str.obj \
	$(OBJDIR)\kilib\kl_wcmn.obj \
	$(OBJDIR)\kilib\kl_wnd.obj

UI_OBJS = \
	$(OBJDIR)\uninst\kinst.obj \
	$(OBJDIR)\uninst\StdAfx.obj \
	$(OBJDIR)\kilib\kl_app.obj \
	$(OBJDIR)\kilib\kl_carc.obj \
	$(OBJDIR)\kilib\kl_cmd.obj \
	$(OBJDIR)\kilib\kl_dnd.obj \
	$(OBJDIR)\kilib\kl_file.obj \
	$(OBJDIR)\kilib\kl_find.obj \
	$(OBJDIR)\kilib\kl_reg.obj \
	$(OBJDIR)\kilib\kl_rythp.obj \
	$(OBJDIR)\kilib\kl_str.obj \
	$(OBJDIR)\kilib\kl_wcmn.obj \
	$(OBJDIR)\kilib\kl_wnd.obj

XT_OBJS = \
	$(OBJDIR)\NoahXt\NoahXt.obj

HEADERS = \
	stdafx.h \
	NoahApp.h \
	NoahAM.h \
	NoahCM.h \
	Archiver.h \
	ArcDLL.h \
	ArcACE.h \
	ArcB2e.h \
	ArcCpt.h \
	ArcMsc.h \
	ArcXacRett.h \
	SubDlg.h \
	Resource.h \
	msvc4.h \
	kilib\kilib.h \
	kilib\kilibext.h \
	kilib\kl_app.h \
	kilib\kl_carc.h \
	kilib\kl_cmd.h \
	kilib\kl_dnd.h \
	kilib\kl_file.h \
	kilib\kl_find.h \
	kilib\kl_misc.h \
	kilib\kl_reg.h \
	kilib\kl_rythp.h \
	kilib\kl_str.h \
	kilib\kl_wcmn.h \
	kilib\kl_wnd.h \
	uninst\StdAfx.h \
	uninst\resource.h \
	NoahXt\resource.h

all: dirs $(OUTDIR)\Noah.exe $(OUTDIR)\NoahXt.dll $(OUTDIR)\uninst.exe

dirs:
	-mkdir $(OBJDIR)
	-mkdir $(OBJDIR)\kilib
	-mkdir $(OBJDIR)\uninst
	-mkdir $(OBJDIR)\NoahXt
	-mkdir $(OUTDIR)

clean:
	-del $(NOAH_OBJS)
	-del $(UI_OBJS)
	-del $(XT_OBJS)
	-del $(OBJDIR)\Noah.res
	-del $(OBJDIR)\NoahXt.res
	-del $(OBJDIR)\kinst.res
	-del $(OUTDIR)\Noah.exe
	-del $(OUTDIR)\NoahXt.dll
	-del $(OUTDIR)\NoahXt.lib
	-del $(OUTDIR)\NoahXt.exp
	-del $(OUTDIR)\uninst.exe

$(OUTDIR)\Noah.exe: $(NOAH_OBJS) $(OBJDIR)\Noah.res
	$(LINK) $(LFLAGS) /out:$@ $(NOAH_OBJS) $(OBJDIR)\Noah.res $(LIBS)

$(OUTDIR)\NoahXt.dll: $(XT_OBJS) $(OBJDIR)\NoahXt.res
	$(LINK) /nologo /dll /def:NoahXt\NoahXt.def /out:$@ $(XT_OBJS) $(OBJDIR)\NoahXt.res $(LIBS) libcpmt.lib

$(OUTDIR)\uninst.exe: $(UI_OBJS) $(OBJDIR)\kinst.res
	$(LINK) $(LFLAGS) /out:$@ $(UI_OBJS) $(OBJDIR)\kinst.res $(LIBS) libcpmt.lib

$(OBJDIR)\Noah.res: Noah.rc resource.h Noah.ico b2e.ico manifest.xml manifest64.xml
	$(RC) $(RCFLAGS) /fo $@ Noah.rc

$(OBJDIR)\NoahXt.res: NoahXt\NoahXt.rc NoahXt\resource.h
	cd NoahXt
	$(RC) $(RCFLAGS) /fo ..\$(OBJDIR)\NoahXt.res NoahXt.rc
	cd ..

$(OBJDIR)\kinst.res: uninst\kinst.rc uninst\resource.h
	cd uninst
	$(RC) $(RCFLAGS) /fo ..\$(OBJDIR)\kinst.res kinst.rc
	cd ..

$(NOAH_OBJS) $(UI_OBJS) $(XT_OBJS): $(HEADERS)

{}.cpp{$(OBJDIR)}.obj:
	$(CC) $(CFLAGS) /Fo$@ /c $<

{kilib}.cpp{$(OBJDIR)\kilib}.obj:
	$(CC) $(CFLAGS) /Fo$@ /c $<

{uninst}.cpp{$(OBJDIR)\uninst}.obj:
	$(CC) $(CFLAGS) /Fo$@ /c $<

{NoahXt}.cpp{$(OBJDIR)\NoahXt}.obj:
	$(CC) $(CFLAGS) /D_USRDLL /DNOAHXT_EXPORTS /Fo$@ /c $<
