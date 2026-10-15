/* msvc4.h : Visual C++ 4.x compatibility shims.
 *
 * Include AFTER system headers, BEFORE kilib headers, i.e.
 *   stdafx.h / uninst/StdAfx.h : after <shlobj.h>
 *   NoahXt/NoahXt.cpp          : after <lmaccess.h>
 * Inert on Visual C++ 5.0 and later.
 *
 * - MSVC 4.x has no bool/true/false/explicit keywords.
 *   (typedef+macros; must come after system headers because this
 *   SDK's own oaidl.h uses "bool" as a parameter name.)
 * - ICON_SMALL/ICON_BIG and the LVITEM/LVCOLUMN/NMLISTVIEW aliases
 *   are missing from this SDK's winuser.h/commctrl.h; map them to the
 *   names this SDK does have. Layouts are unchanged since Win95.
 */
#if _MSC_VER < 1100
#ifndef MSVC4_SHIMMED
#define MSVC4_SHIMMED

typedef int bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define explicit

#ifndef ICON_SMALL
#define ICON_SMALL 0
#endif
#ifndef ICON_BIG
#define ICON_BIG 1
#endif
#ifndef LVITEM
#define LVITEM LV_ITEM
#endif
#ifndef LVCOLUMN
#define LVCOLUMN LV_COLUMN
#endif
#ifndef NMLISTVIEW
#define NMLISTVIEW NM_LISTVIEW
#endif

#endif /* MSVC4_SHIMMED */
#endif /* _MSC_VER < 1100 */
