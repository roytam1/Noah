//--- K.I.LIB ---
// kl_wcmn.h : windows-common-interface operatin

#ifndef AFX_KIWINCOMMON_H__0686721C_CAFB_4C2C_9FE5_0F482EA6A60B__INCLUDED_
#define AFX_KIWINCOMMON_H__0686721C_CAFB_4C2C_9FE5_0F482EA6A60B__INCLUDED_

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// シェルユーティリティークラス

class kiSUtil
{
public:
	// カレントディレクトリ退避
	static void switchCurDirToExeDir();

	// 「フォルダの選択」ダイアログ
	static bool getFolderDlg( char* buf, HWND par, const char* title, const char* def );
	static void getFolderDlgOfEditBox( HWND wnd, HWND par, const char* title );

	// 対応する拡張子用アイコンの、システムイメージリストのインデックスを返す。
	static int getSysIcon( const char* ext );

	// 最後のエラーを表示
	static void msgLastError( const char* msg = NULL );

	// 自分へのショートカット作成
	static void createShortCut( const kiPath& at, const char* name );

	//-- SHGetFileInfoA を 動的に 引く。古い シェル には 無い (あるいは 名前が 違う) ことが
	//   あり、 静的に import すると バイナリごと 起動できなくなる。無い 場合は NULL を 返す。
	static HIMAGELIST getFileInfo( const char* path, SHFILEINFO* fi, UINT flags );

	// ファイルが存在する？
	static bool exist( const char* fname );
	static bool isdir( const char* fname );

	// フォルダ選択ダイアログ。uninst/NoahXt は ルートの Resource.h を
	// 読まないので、kilib 側に置いて 値も ここに 固定する。
	// Resource.h にも 同じ 値 を 置いて ある (RC 用)。
	#define IDD_FOLDER    124
	#define IDC_FLIST     1061
	#define IDC_FPATH     1062
	#define IDC_FDRIVE    1063

	// カレントディレクトリを安全なところに移してLoadLibrary
	static HMODULE loadLibrary(LPCTSTR lpFileName)
	{
		char original_cur[MAX_PATH], sys[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH, original_cur);
		::GetSystemDirectory(sys, MAX_PATH);
		::SetCurrentDirectory(sys);

		// 任意 DLL は 無かった 場合も 正常?行なので、Windows 3.1 / Win32s の
		// 「ファイルが見つかりません」システムダイアログを 抑止する。
		UINT oldErrMode = ::SetErrorMode( SEM_NOOPENFILEERRORBOX );
		HMODULE han = ::LoadLibrary(lpFileName);
		::SetErrorMode( oldErrMode );

		::SetCurrentDirectory(original_cur);
		return han;
	}
};

#endif
