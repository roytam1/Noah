//--- K.I.LIB ---
// kl_app.h : application class for K.I.LIB

#include "stdafx.h"
#include "kilib.h"

//------------ 唯一のアプリケーションオブジェクトの管理 ------------//

kiApp* kiApp::st_pApp = NULL;

kiApp* app()
{
	return kiApp::st_pApp;
}

//-------------------- スタートアップコード ------------------------//

void kilib_startUp()
{
	// 英語ロケールテスト用
	//::SetThreadUILanguage(0x0409);

	//-- K.I.LIB 初期化
	NOAHTRACE( "[up] start" );
	kiStr::init();
	kiWindow::init();
	NOAHTRACE( "[up] kilib init done" );

	//-- キーボード状態クリア
	::GetAsyncKeyState( VK_SHIFT );

	//-- アプリケーションインスタンスを作成
	kilib_create_new_app();
	NOAHTRACE( "[up] app created = %p", (void*)app() );
	if( app() )
	{
		// コマンドライン分割
		kiCmdParser cmd( ::GetCommandLine(), true );
		NOAHTRACE( "[up] cmdline parsed, %d file(s)", cmd.param().len() );

		// 実行
		app()->run( cmd );
		NOAHTRACE( "[up] run() returned normally" );
	}

	//-- K.I.LIB 終了
	kiWindow::finish();

	delete app();
	NOAHTRACE( "[up] reached ExitProcess" );
	::ExitProcess( 0 );
}

//--------------- C-Runtime初期化コード削除周りの処理 ---------------//

#if 0

int APIENTRY WinMain( HINSTANCE, HINSTANCE, char*, int )
{
	kilib_startUp();
	return 0;
}

#else

void* operator new( size_t siz )
{
	return (void*)::GlobalAlloc( GMEM_FIXED, siz );
}

void operator delete( void* ptr )
{
	::GlobalFree( (HGLOBAL)ptr );
}

void main()
{
	// main がないと何故か libc.lib がリンクエラーになるのでダミー
}

#endif

//--------------------------------------------------------------//
