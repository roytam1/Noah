//--- K.I.LIB ---
// kl_wcmn.h : windows-common-interface operatin

#include "stdafx.h"
#include "kilib.h"

void kiSUtil::switchCurDirToExeDir()
{
	char exepath[MAX_PATH+50];
	GetModuleFileName( NULL, exepath, MAX_PATH );
	char* lastslash = 0;
	for( char* p=exepath; *p; p=CharNext(p) )
		if( *p=='\\' || *p=='/' )
			lastslash = p;
	if(lastslash)
		*lastslash = '\0';
	SetCurrentDirectory(exepath);
}

static int CALLBACK __ki__ofp( HWND w, UINT m, LPARAM l, LPARAM d )
{
	if( m==BFFM_INITIALIZED && d )
		::SendMessage( w, BFFM_SETSELECTION, TRUE, d );
	return 0;
}

//-- フォルダ選択ダイアログ -------------------------------------------------//
//   SHBrowseForFolder が 無い 環境 (Win32s) 用。 orthodox な ブラウザは
//   Win32s に 無いので、最小構成 の ダイアログを 自作する。
//   現在?? フォルダを 表示し、 サブフォルダを リストする。
//   ダブルクリックで 階層を 降り、 OK で 現在?? フォルダを 確定する。

struct FOLDERDLG
{
	kiStr cur;		// 移動中のフォルダ
	kiStr out;		// 確定されたフォルダ (空ならキャンセル)
};

// 使えるドライブを コンボに 入れる。現在地 映?? に 合わせる。
static void folderDlgFillDrives( HWND dlg, char curdrive )
{
	HWND cb = ::GetDlgItem( dlg, IDC_FDRIVE );
	::SendMessage( cb, CB_RESETCONTENT, 0, 0 );

	char old[MAX_PATH];
	::GetCurrentDirectory( MAX_PATH, old );

	for( char d='A'; d<='Z'; d++ )
	{
		char root[4];
		root[0] = d;
		root[1] = ':';
		root[2] = '\\';
		root[3] = '\0';

		// SetCurrentDirectory は Win32s に 確実に あるので 存在判定に 使う
		if( !::SetCurrentDirectory( root ) )
			continue;
		::SetCurrentDirectory( old );

		LRESULT i = ::SendMessage( cb, CB_ADDSTRING, 0, (LPARAM)root );
		if( i>=0 && (root[0]&~0x20)==(curdrive&~0x20) )
			::SendMessage( cb, CB_SETCURSEL, (WPARAM)i, 0 );
	}
	::SetCurrentDirectory( old );
}

static void folderDlgFill( HWND dlg, const char* cur )
{
	folderDlgFillDrives( dlg, (cur && cur[0]) ? cur[0] : 'C' );

	HWND lst = ::GetDlgItem( dlg, IDC_FLIST );
	::SendMessage( lst, LB_RESETCONTENT, 0, 0 );

	// 上へ戻るための項目
	::SendMessage( lst, LB_ADDSTRING, 0, (LPARAM)".." );

	kiPath wild( cur );
	wild += "\\*";

	WIN32_FIND_DATA fd;
	HANDLE h = ::FindFirstFile( wild, &fd );
	if( h!=INVALID_HANDLE_VALUE )
	{
		do
		{
			if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				continue;
			if( fd.cFileName[0]=='.'
			 && (fd.cFileName[1]=='\0'
			  || (fd.cFileName[1]=='.' && fd.cFileName[2]=='\0')) )
				continue;
			::SendMessage( lst, LB_ADDSTRING, 0, (LPARAM)fd.cFileName );
		} while( ::FindNextFile( h, &fd ) );
		::FindClose( h );
	}

	kiPath here( cur );
	here.beBackSlash( false );
	::SetDlgItemText( dlg, IDC_FPATH, here );
	::SendMessage( lst, LB_SETCURSEL, 0, 0 );
}

// 選択項目へ 一つ階層 降りる。".." なら 上へ 上がる
static void folderDlgEnter( HWND dlg )
{
	FOLDERDLG* fd = (FOLDERDLG*)::GetWindowLong( dlg, GWL_USERDATA );
	if( !fd )
		return;

	char sel[MAX_PATH];
	HWND lst = ::GetDlgItem( dlg, IDC_FLIST );
	LRESULT i = ::SendMessage( lst, LB_GETCURSEL, 0, 0 );
	if( i==LB_ERR )
		return;
	::SendMessage( lst, LB_GETTEXT, (WPARAM)i, (LPARAM)sel );

	kiPath next( fd->cur );
	next += sel;

	char old[MAX_PATH];
	::GetCurrentDirectory( MAX_PATH, old );
	if( !::SetCurrentDirectory( next ) )
	{
		::SetCurrentDirectory( old );
		return;
	}

	char now[MAX_PATH];
	::GetCurrentDirectory( MAX_PATH, now );
	::SetCurrentDirectory( old );			// プロセス全体は 動かさない
	kiPath nowp( now );
	nowp.beBackSlash( true );
	fd->cur = (const char*)nowp;

	folderDlgFill( dlg, fd->cur );
}

static BOOL CALLBACK folderDlgProc( HWND dlg, UINT msg, WPARAM wp, LPARAM lp )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			FOLDERDLG* fd = (FOLDERDLG*)lp;
			::SetWindowLong( dlg, GWL_USERDATA, (LONG)fd );

			// 初期位置。存在しない なら Windows ディレクトリ から やり直す
			char old[MAX_PATH];
			::GetCurrentDirectory( MAX_PATH, old );

			kiPath start( fd->cur );
			if( !::SetCurrentDirectory( start ) )
				// 記入が 無い とき は アーカイブの 場所 (カレント) から 始める
				::SetCurrentDirectory( old );


			char cwd[MAX_PATH];
			::GetCurrentDirectory( MAX_PATH, cwd );
			::SetCurrentDirectory( old );	// プロセス全体は 動かさない
			kiPath cur( cwd );
			cur.beBackSlash( true );

			fd->cur = (const char*)cur;

			folderDlgFill( dlg, fd->cur );
			NOAHTRACE( "[browse] folder dialog at \"%s\"", (const char*)fd->cur );
		}
		return TRUE;

	case WM_COMMAND:
		switch( LOWORD(wp) )
		{
		case IDC_FDRIVE:
			if( HIWORD(wp)==CBN_SELCHANGE )
			{
				HWND cb = ::GetDlgItem( dlg, IDC_FDRIVE );
				char root[MAX_PATH];
				LRESULT i = ::SendMessage( cb, CB_GETCURSEL, 0, 0 );
				if( i!=CB_ERR )
				{
					::SendMessage( cb, CB_GETLBTEXT, (WPARAM)i, (LPARAM)root );
					FOLDERDLG* fd = (FOLDERDLG*)::GetWindowLong( dlg, GWL_USERDATA );
					if( fd && ::SetCurrentDirectory( root ) )
					{
						char np[MAX_PATH];
						::GetCurrentDirectory( MAX_PATH, np );
						kiPath p( np );
						fd->cur = (const char*)p;
						folderDlgFill( dlg, fd->cur );
					}
				}
				return TRUE;
			}
			break;

		case IDC_FLIST:
			if( HIWORD(wp)==LBN_DBLCLK )
			{
				folderDlgEnter( dlg );
				return TRUE;
			}
			break;

		case IDOK:
			{
				FOLDERDLG* fd = (FOLDERDLG*)::GetWindowLong( dlg, GWL_USERDATA );
				if( fd )
					fd->out = fd->cur;
				::EndDialog( dlg, IDOK );
			}
			return TRUE;

		case IDCANCEL:
			::EndDialog( dlg, IDCANCEL );
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static bool kiSUtil_folderDlg( char* buf, HWND par, const char* title, const char* def )
{
	FOLDERDLG fd;
	fd.cur = (def && *def) ? def : "";
	fd.out = "";

	::DialogBoxParam( app()->inst(), MAKEINTRESOURCE(IDD_FOLDER),
		par, folderDlgProc, (LPARAM)&fd );

	if( fd.out.len()==0 )
	{
		NOAHTRACE( "[browse] folder dialog cancelled" );
		return false;
	}
	for( int i=0; i<MAX_PATH-1 && fd.out[i]; i++ )
		buf[i] = fd.out[i];
	buf[fd.out.len()<MAX_PATH ? fd.out.len() : MAX_PATH-1] = '\0';
	NOAHTRACE( "[browse] folder dialog -> \"%s\"", buf );
	return buf[0]!=('\0');
}

bool kiSUtil::getFolderDlg( char* buf, HWND par, const char* title, const char* def )
{
	// 情報セット
	BROWSEINFO bi;
	ki_memzero( &bi, sizeof(bi) );
	bi.hwndOwner      = par;
	bi.pszDisplayName = buf;
	bi.lpszTitle      = title;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN;
	bi.lpfn           = __ki__ofp;
	bi.lParam         = (long)def;

	// ダイアログ表示
	ITEMIDLIST* id = ::SHBrowseForFolder( &bi );
	NOAHTRACE( "[browse] SHBrowseForFolder -> %p", (void*)id );
	if( id )
	{
		::SHGetPathFromIDList( id, buf );
		app()->shellFree( id );
		NOAHTRACE( "[browse] picked \"%s\"", buf );
		return true;
	}

	//-- SHBrowseForFolder が 無かった 環境 (Win32s) は 自前の ダイアログ で 代替
	NOAHTRACE( "[browse] SHBrowseForFolder unavailable, using built-in dialog" );
	return kiSUtil_folderDlg( buf, par, title, def );
}

void kiSUtil::getFolderDlgOfEditBox( HWND wnd, HWND par, const char* title )
{
	char str[MAX_PATH];
	::SendMessage( wnd, WM_GETTEXT, MAX_PATH, (LPARAM)str );
	for( char* x=str,*l=str; *x; x=kiStr::next(x) )
		l=x;
	if( *l=='\\' || *l=='/' )
		*l='\0';
	if( getFolderDlg( str, par, title, str ) )
		::SendMessage( wnd, WM_SETTEXT, 0, (LPARAM)str );
}

int kiSUtil::getSysIcon( const char* ext )
{
	kiPath tmp( kiPath::Tmp );
	tmp += "noahicontest.";
	tmp += ext;

	SHFILEINFO fi;
	::SHGetFileInfo( tmp, 0, &fi, sizeof(fi), 
		SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX );
	return fi.iIcon;
}

void kiSUtil::msgLastError( const char* msg )
{
	char* pMsg;
	::FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,::GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&pMsg,0,NULL );
	if( msg )
		app()->msgBox( kiStr(msg) + "\r\n\r\n" + pMsg );
	else
		app()->msgBox( pMsg );
	::LocalFree( pMsg );
}

void kiSUtil::createShortCut( const kiPath& at, const char* name )
{
	::CoInitialize(NULL);

	IShellLink* psl;
	if( SUCCEEDED(::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl)) )
	{
		psl->SetPath( kiPath(kiPath::Exe_name) );
		psl->SetWorkingDirectory( kiPath(kiPath::Exe,false) );

		IPersistFile* ppf;
		if( SUCCEEDED(psl->QueryInterface(IID_IPersistFile,(void**)&ppf)) )
		{
			wchar_t wsz[MAX_PATH]; 
			kiPath lnkfile( at );
			lnkfile += name, lnkfile += ".lnk";
			::MultiByteToWideChar(CP_ACP,0,lnkfile,-1,wsz,MAX_PATH);
			ppf->Save(wsz,TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	::CoUninitialize();
}

bool kiSUtil::exist( const char* fname )
{
	return 0xffffffff != ::GetFileAttributes( fname );
}

bool kiSUtil::isdir( const char* fname )
{
	DWORD attr = ::GetFileAttributes( fname );
	return attr!=0xffffffff && (attr&FILE_ATTRIBUTE_DIRECTORY);
}
