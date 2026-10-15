# Noah.mak -- NMAKE makefile for MSVC 4.x (tested with cl 10.10 / Visual C++ 4.1).
#
# Usage:
#   C:\MSDEV4\BIN\VCVARS32.BAT x86
#   NMAKE /f msvc4.mak
#   NMAKE /f msvc4.mak clean
#
# Output: bin-msvc4\Noah.exe, bin-msvc4\NoahXt.dll, bin-msvc4\uninst.exe
# Objects: tmp\msvc4\ (mirrors the source layout, so that stdafx.obj and
#   uninst\StdAfx.obj do not collide on case-insensitive filesystems).
# Single Release (NDEBUG) configuration. Requires msvc4.h, which is inert
# on Visual C++ 5.0 and later. The VC6/VC2010 builds use Noah.dsp /
# Noah.vcxproj instead.

CC      = cl
RC      = rc
LINK    = link
CFLAGS  = /nologo /W3 /O2 /DWIN32 /DNDEBUG /D_WINDOWS /D_MBCS /MT /DSTRICT /I.
RCFLAGS = /l 0x411 /d NDEBUG
LFLAGS  = /nologo /entry:kilib_startUp /subsystem:windows
LIBS    = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib \
          advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib \
          odbc32.lib odbccp32.lib comctl32.lib lz32.lib version.lib
OBJDIR  = tmp\msvc4
OUTDIR  = bin-msvc4

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
	$(LINK) /nologo /dll /def:NoahXt\NoahXt.def /out:$@ $(XT_OBJS) $(OBJDIR)\NoahXt.res $(LIBS)

$(OUTDIR)\uninst.exe: $(UI_OBJS) $(OBJDIR)\kinst.res
	$(LINK) $(LFLAGS) /out:$@ $(UI_OBJS) $(OBJDIR)\kinst.res $(LIBS)

$(OBJDIR)\Noah.res: Noah.rc resource.h Noah.ico b2e.ico
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
