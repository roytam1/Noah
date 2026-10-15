# AGENTS.md — Noah (nsrc3199)

Win32 drag-and-drop archiver front-end (圧縮・解凍 GUI). Noah implements **no compression
algorithms of its own** except MS-Compress (LZSS) and CompactPro decode. Everything else is
driven through external archiver DLLs / console tools / `.b2e` extension scripts.

Repo is a single-commit import of `nsrc3199.zip` with no remote and no CI.

---

## 1. CRITICAL: file encoding is Shift-JIS (CP932)

All `.cpp`, `.h`, `.rc`, `.dsp`, `.txt` files are **CP932**. Comments and string literals are
Japanese. Only `*.sln`, `*.vcxproj`, `*.filters`, `manifest.xml` are ASCII/UTF-8.

**Never save an existing source file as UTF-8.** Editing tools that silently transcode will
corrupt the source and the build will break.

Read a file as CP932 and edit while preserving the encoding:

```powershell
$p = "NoahAM.cpp"
$t = [System.IO.File]::ReadAllText($p, [System.Text.Encoding]::GetEncoding(932))
# ... modify $t ...
[System.IO.File]::WriteAllText($p, $t, [System.Text.Encoding]::GetEncoding(932))
```

Notes:
- `GetEncoding(932)` = `Default` on a Japanese-locale Windows, but be explicit.
- Avoid introducing new multi-byte characters; existing Japanese comments are fine and should
  stay untouched when editing nearby lines.
- `Noah.vcxproj` declares `<CharacterSet>MultiByte</CharacterSet>`, so CP932 is also what the
  compiler is told to assume.

---

## 2. Build

Toolchain targets **MSVC 6.0** (primary, per `ReadMe.txt`) and **MSVC 2010 Express** (ported).
MSVC 4.x (tested: cl 10.10 / Visual C++ 4.1) also builds via `msvc4.mak` — see below.

| Project | Output | VS6 project | VS2010 project |
|---|---|---|---|
| `Noah.exe` | root / `Release/` | `Noah.dsp` | `Noah.vcxproj` |
| `NoahXt.dll` (shell extension + config store) | `NoahXt/` | `NoahXt/NoahXt.dsp` | **none** |
| `uninst.exe` (installer/uninstaller) | `uninst/` | `uninst/kinst.dsp` | **none** |

- `Noah.dsw` (VC6 workspace) contains all three projects.
- `Noah.sln` contains **only** `Noah.vcxproj`. NoahXt and kinst are VC6-only; building them
  requires either VC6 or hand-written vcxproj files.
- Build via `devenv Noah.dsw`, or generate makefiles with `nmake -f Noah.mak`.
- On this machine only VC98 (`C:\Program Files (x86)\Microsoft Visual Studio\VC98`) is
  installed — there is no `msbuild`, no VS2010+, and no `vcvars32.bat`. The `vcxproj` path is
  therefore **not buildable locally**; verify changes with the VC6 `Noah.mak` path or by
  inspection.
- There are **no tests, no linters, no CI**. Verification is: it compiles, and manual smoke
  test (drag files onto the exe; run with no args to get the config dialog).

### MSVC 4.x (`msvc4.mak`)

- Prerequisites: `C:\MSDEV4\BIN\VCVARS32.BAT x86`, then `NMAKE /f msvc4.mak`
  (`NMAKE /f msvc4.mak clean` to wipe). Binaries land in `bin-msvc4/`, objects in
  `tmp\msvc4/`. Single Release (`NDEBUG`) config; no PCH, no IDE project (`.dsp` is
  VC6 format — MSDEV4 cannot read it).
- `msvc4.h` supplies what MSVC 4.x lacks (`bool`/`true`/`false`, `explicit`, `ICON_*`,
  `LVITEM`/`LVCOLUMN`/`NMLISTVIEW` aliases). It must stay included **after** system
  headers and **before** kilib headers (this SDK's own `oaidl.h` uses `bool` as a
  parameter name, so it cannot go first or on the command line). Inert on VC6+.
- The makefile builds with `STRICT` defined (fixes `DLGPROC`/`HOOKPROC` typing); do not
  rely on non-`STRICT` callback conversions.
- Code constraints for MSVC4 (not required by VC6): no member-function definitions or
  bases in local classes, no `if (T x = …)` condition-declarations, no top-level
  `const` on overriding parameters, and no `PBS_SMOOTH` symbol in `.rc` (guarded with
  `#ifndef` in `Noah.rc`).

### Linker / startup quirks — do not "fix" these

- Entry point is **`kilib_startUp`** (`Noah.vcxproj` `<EntryPointSymbol>`, `.dsp`
  `/entry:"kilib_startUp"`). There is **no `WinMain`**.
- `kilib/kl_app.cpp` provides global `operator new`/`operator delete` (backed by `GlobalAlloc`)
  plus a dummy `main()` — the comment notes libc fails to link without it.
- `Noah.cpp` forces `extern "C" { int _afxForceEXCLUDE; }` and stubs `_imp__IsDebuggerPresent()`
  to keep the CRT from dragging in unwanted startup behaviour.
- Release config sets `RuntimeTypeInfo=false` and `BufferSecurityCheck=false`. Do not add RTTI
  (`dynamic_cast`, `typeid`) or `/GS`-sensitive code.
- `Noah.vcxproj` sets no `OutDir`, so binaries land in the repo root. Objects go to
  `obj\<Config>\` (vcxproj) or `tmp/rel`, `tmp/dbg` (dsp).
- `kilib/*.cpp` is compiled directly into all three projects — it is a source-included
  library, not a static lib. Editing `kilib/` affects Noah.exe, NoahXt.dll and uninst.exe.

### Precompiled headers

Every `.cpp` starts with `#include "stdafx.h"`. `stdafx.cpp` is the only PCH creator for the
Noah project. `uninst/StdAfx.h` (capital S) is a separate, smaller PCH; `NoahXt/NoahXt.cpp`
uses no PCH at all and defines `WINVER`/`_WIN32_IE` inline.

---

## 3. Architecture

```
kilib/          K.I.LIB — generic Win32 framework (ki* classes)
NoahApp.h       CNoahApp        : kiApp        — entry logic, cmdline, temp dirs
NoahCM.*        CNoahConfigManager — Noah.ini I/O + NoahXt.dll linkage
NoahAM.*        CNoahArchiverManager — routes work to CArchiver instances
Archiver.*      CArchiver (abstract) + CArcModule (DLL/exe wrapper)
Arc*.{h,cpp}    concrete CArchiver subclasses / DLL-backed archivers
SubDlg.*        CArcViewDlg, CArcPwdDlg, CArcProgressDlg, CArcPathCheckDlg
NoahXt/         COM shell extension (context menu) + registry config store
uninst/         installer / uninstaller GUI
```

Dependency flow: `CNoahApp` → `CNoahArchiverManager` → `CArchiver*` → external DLL/tool.
`CNoahConfigManager` supplies settings to both. Accessors are macros in `NoahApp.h`:

```cpp
myapp()   // CNoahApp*
myarc()   // CNoahArchiverManager*
mycnf()   // CNoahConfigManager*
```

### K.I.LIB classes (`kilib/`)

`kiApp`, `kiWindow`, `kiDialog`, `kiPropSheet`, `kiPropSheetPage`, `kiListView`,
`kiStr`, `kiPath`, `kiCmdParser`, `kiFile`, `kiFindFile`, `kiRegKey`, `kiIniFile`,
`kiDataObject`/`kiDropSource` (OLE DnD), `kiSUtil` (shell helpers),
`kiArcDLLRaw` (archive-DLL ABI layer 2), `kiVar`/`kiRythpVM` (the `.b2e` script VM).

`kiStr` doubles as a byte array (`ki_strlen`, `ki_memzero`, `ki_strcmpi`); `kiStr::next()`
walks DBCS lead bytes via a table built in `kiStr::init()`. Do not treat these as
`std::string`-like UTF-8.

---

## 4. The `CArchiver` contract (`Archiver.h`)

Capability bits returned by `v_load()`:

| bit | meaning |
|---|---|
| `aCheck`    | can identify an archive by content (`v_check`) |
| `aMelt`     | can extract whole archives (`v_melt`) |
| `aList`     | can list contents (`v_list`) |
| `aMeltEach` | can extract selected members |
| `aCompress` | can create archives (`v_compress`) |
| `aArchive`  | supports multi-file archives in one job |
| `aSfx`      | can convert to SFX |

`v_load()` is **lazy** — invoked on first use through the inline public wrappers, results
cached in `m_Able`. Register methods: `set_cmp_ext()`, `add_cmp_mhd(name, isDefault)`.

Extension lists are dot-separated, lowercase, **trailing dot required**:
`CArchiver("lzh.lha.lzs.")`. Lowercasing happens in the constructor.

`cancompressby()` returns: index ≥ 0 = exact method match, `-2` = archive type matched only,
`-1` = no. `CNoahArchiverManager::map_compressor()` prefers exact matches, falls back to the
first type-only match using `cmp_mhd_default()`.

### Return-code convention (important)

- `< 0x8000` — treat as success (commonly `0`).
- `0x8020` — benign "nothing to report" (e.g. SFX produced but no new archive file). Callers
  deliberately skip the error dialog for this value. See `NoahAM.cpp` `do_melting` /
  `do_compressing`.
- `>= 0x8000` — error; the value is shown to the user as `Error No: [%x]`.
- `0xffff` — the operation is not supported by this archiver.

### Bundled archivers

DLL/console driven, declared in `ArcDLL.h` — each is a thin subclass of `CArcDLL` (or
`CArcDLL_DotSlash`, which prefixes file names with `./`):

| Class | Backend | Extensions |
|---|---|---|
| `CArcLzh`    | `Unlha32.dll` | lzh lha lzs |
| `CArc7z` / `CArc7zZip` | `7-zip32.dll` | 7z zip jar |
| `CArcUnZip`  | `UnZip32.dll` | zip jar |
| `CArcZip`    | `Zip32j.dll`  | (compress only) |
| `CArcTar`    | `Tar32.dll`   | tar tgz tbz gz bz2 z taz rpm deb ar xz lzma |
| `CArcCab`    | `Cab32.dll`   | cab |
| `CArcRar`    | `Unrar32.dll` | rar |
| `CArcArj`    | `Unarj32j.dll`| arj |
| `CArcBga`    | `Bga32.dll`   | gza bza |
| `CArcYz1`    | `Yz1.dll`     | yz1 |
| `CArcGca`    | `UnGCA32.dll` | gca |
| `CArcAce`    | `UnAceV2.dll` (via `unacedef.h`) | ace |
| `CArcXacRett`| `XacRett.dll` (melt only) | see below |
| `CArcMsc`    | built-in LZSS (`MSCompress`) | `??_` |
| `CArcCpt`    | built-in CompactPro/LZH-RLE decoder | cpt bin |
| `CArcB2e`    | `*.b2e` Rythp scripts + external exe | script-defined |

These DLLs are **not** in the repo; they are resolved from `PATH` at runtime. `CArcModule`
falls back to treating an unresolvable name as a shell command or `.exe`/`.com`.

To point Noah at an out-of-tree DLL folder without touching `PATH`, drop a `caldix.ini`
next to `Noah.exe` containing `[conf]` / `dll=<folder>` — `CNoahConfigManager::init()`
appends it to `PATH`.

### `CArcXacRett` (ArcXacRett.cpp) — the odd one out

`XacRett.dll` is a single DLL covering ~70 formats. Three things differ from `CArcDLL`:

- **Melt-only.** `XacRettQueryFunctionList` returns `FALSE` for `ISARC_ADD`; there is no
  compressor. `v_load()` therefore returns `aCheck|aMelt|aList|aMeltEach` and never
  `set_cmp_ext`/`add_cmp_mhd`, so it never appears in the compression method list.
- **It does not claim formats Noah already has a dedicated engine for** (lzh, zip, 7z, rar,
  cab, tar/gz/bz2/xz/lzma, arj, yz1, gca, ace, cpt, bin, gza, bza). Those stay with their
  own classes. Everything else is dispatched by content via `v_check`. It is registered
  **last** in `CNoahArchiverManager::init()` so the dedicated archivers also win the
  content-check fallback in `map_melters`.
- **Its command line has no verb.** `CSplitCmdLine::Split` unconditionally discards the
  first token, so `v_melt` emits a throwaway `Noah` token to keep it out of the way. The
  destination token **must** end in `\` or `IsBaseDir` rejects it and the path is silently
  treated as a wildcard (extraction then writes nothing) — hence `dir.beBackSlash(true)`.
  Selected members are passed as trailing wildcard arguments. `-n1` suppresses XacRett's
  own progress window, which would otherwise block on its own message loop.

  Verified command shape: `Noah -n1 "<archive>" "<dest>\" ["<member>"]`

### Adding a new archiver

1. Subclass `CArchiver` (self-contained) or `CArcDLL` (external backend) in `Arc<Name>.{h,cpp}`.
2. Register it in `CNoahArchiverManager::init()` (`NoahAM.cpp`) with a **kill character** —
   the char the user adds to `Noah.ini` `[Melt] Kill=` to disable it. The array is
   `static int dead[128]`, indexed by `0x7f & c`.
3. Add it to `ArcDLL.h` / relevant header, and add the files to **both** `Noah.vcxproj`
   (`ClCompile`/`ClInclude`) and `Noah.dsp` (`KINOBJS`/`SOURCE=`). They are not auto-globbed.
4. If it appears in the config UI: add an `IDC_*` in `Resource.h`, the checkbox in `Noah.rc`
   (`IDD_WINCFG`), a bitmap/icon in `NoahXt/icon/` + `NoahXt.rc` if it needs an overlay icon,
   and wire it in `CNoahConfigDialog::CWinXPage`.

`Noah.rc` carries **two copies of most dialogs**: a Japanese block (`LANG_JAPANESE`,
`AFX_TARG_JPN`) and an English block (`LANG_ENGLISH`, `AFX_TARG_ENU`). Windows loads
whichever matches the thread locale, so a control added to only one copy silently
vanishes on systems using the other language. When editing any dialog template
(`IDD_ARCVIEW` et al), update **both** copies. Known drift: the English `IDD_ARCVIEW`
once lacked `IDC_STATUSBAR` entirely.

---

## 5. Runtime behaviour

### Modes (`Noah.ini` `[Mode] Mode`)

`0` compress-only · `1` compress-preferred · `2` melt-preferred · `3` melt-only.
Resolution order in `CNoahApp::do_files()`: explicit `/A` or `/X` wins; otherwise the configured
mode; mode 0 (or mode 1 with ≥2 files) forces compress; melt assignment failure falls back to
compress unless mode is 3, which is a hard error.

### Command line (`Noah.cpp`)

Files are positional; options are case-insensitive and parsed in `do_files()`:

| Opt | Effect |
|---|---|
| `/A` `/a` | force compress |
| `/X` `/x` | force melt |
| `/D <dir>` | destination directory |
| `/T <type>` | archive type (e.g. `zip`) |
| `/M <method>` | compression method |
| `/W` | per-file archives (`each`) |
| `/C[ax]` | SFX / compression flags (`C`,`Cx`,`Ca`) |

Modifier keys act as the same switches: **Shift** = open config dialog, **Ctrl** = listing /
per-file mode, **Alt**/`Win` = `-w`. `Control`/`Alt` state is polled with `GetAsyncKeyState`
at startup, and `kl_app.cpp` calls `GetAsyncKeyState(VK_SHIFT)` once to clear stale state.

Launching with no arguments shows the settings dialog.

### Configuration (`NoahCM.cpp` → `Noah.ini`)

`[Mode]` `Mode` `MiniBoot` `OneExt` `NoExt` `MultiBootLimit`
`[Melt]` `MDir` `MkDir` `NoNum` `Kill`
`[Compress]` `CDir` `CExt` `CMhd`
`[OpenDir]` `MODir` `CODir` `OpenBy`
Shell-extension and file-association state is **not** in the ini — it is stored by
`NoahXt.dll` through its exported API (`Init`, `LoadSE`, `SaveSE`, `LoadAS`, `SaveAS`,
`LoadASEx`, `SaveASEx`), resolved lazily via `GetProcAddress`. `NoahXt.dll` is optional; the
app degrades gracefully without it.

### Two global-state rules that bite

1. **The current directory is global mutable state.** Nearly every `v_melt`/`v_compress` does
   `SetCurrentDirectory()`. `CNoahApp::do_files()` installs a local RAII guard that restores
   the exe directory on exit ("ディレクトリロックしないように"). `break_ddir()` also
   deliberately leaves the CWD inside the directory it is fixing. Don't "tidy" this.
2. **`Noah.ini` is loaded lazily by section.** `CNoahConfigManager::load(loading_flag)` uses
   bit flags (`Mode`/`Melt`/`Compress`/`Shell`/`OpenDir`/`All`) and tracks `m_Loaded`. Call
   `load()` before reading a new group of accessors.

### Process-count limiting

`ProcessNumLimitZone` (Noah.cpp) wraps the actual compress/extract work with a named semaphore
`"LimitterForNoahAtKmonosNet"`, sized by `MultiBootLimit`. Temp archives for SFX creation go
under a per-process unique dir from `CNoahApp::get_tempdir()`.

---

## 6. Coding conventions

- **Tabs** for indentation, Windows/Allman braces, brace-initializer lists, no space before
  the open paren in calls, `const` on locals that aren't modified.
- Header guards: `AFX_<NAME>_<GUID>_INCLUDED_` style (inherited from VC6 class wizard). Keep the
  existing guards; don't "modernise" them.
- Comments and in-code strings are **Japanese**. Match the surrounding language when editing.
- Naming: `ki` prefix for framework classes, `CArc*` for archivers, `m_` members, `v_` virtual
  overrides, `myapp()`/`myarc()`/`mycnf()` accessors.
- No MFC, no STL, no exceptions, no RTTI, no `std::` anything. Use `kiArray`, `kiStr`,
  `kiFile`, Win32 APIs.
- `#include "stdafx.h"` must be the first include in every `Noah*.cpp` / `Arc*.cpp` /
  `kilib/*.cpp` (PCH is `Use`d, not `NotUsing`).
- Prefer `inline` definitions in headers for hot small methods (as `Archiver.h` and
  `kl_wnd.h` do).
- No new comments unless they earn their place; existing Japanese comments explain non-obvious
  historical decisions — keep them.

## 7. Security-sensitive paths

Archive extraction is a classic zip-slip surface and is defended in two places:

- `containsDangerPath()` in `Archiver.h:216` — rejects absolute paths, drive-qualified paths,
  and any all-dots path segment (`..`, `...`).
- `CArcPathCheckDlg` (`SubDlg.h:152`) — prompts the user for suspicious entries.

If you add a new archiver backend, wire in both. Do not remove either check.

Other things worth not breaking: `uninst/kinst.cpp` writes registry uninstall data and deletes
installed files, and `NoahXt.cpp` installs a COM shell extension — both are privileged,
filesystem-destroying paths. Treat changes there as high risk.