//--- K.I.LIB ---
// kl_wnd.h : window information manager

#ifndef AFX_KIWINDOW_H__26105B94_1E36_42FA_8916_C2F7FB9EF994__INCLUDED_
#define AFX_KIWINDOW_H__26105B94_1E36_42FA_8916_C2F7FB9EF994__INCLUDED_

//-- Win64: Get/SetWindowLong only carry a LONG, so a pointer kept in a window
//   (or in its class) is truncated. Use the *Ptr forms there; the 32bit build
//   keeps the original calls, so behaviour and the old toolchains are unchanged.
//   KI_DLGRET is what a DLGPROC must return: BOOL on Win32, INT_PTR on Win64.
#ifdef _WIN64
#define ki_get_userdata(h)       ((void*)::GetWindowLongPtr((h), GWLP_USERDATA))
#define ki_set_userdata(h,p)     ::SetWindowLongPtr((h), GWLP_USERDATA, (LONG_PTR)(p))
#define ki_get_wndproc(h)        ((WNDPROC)::GetWindowLongPtr((h), GWLP_WNDPROC))
#define ki_set_wndproc(h,p)      ((WNDPROC)::SetWindowLongPtr((h), GWLP_WNDPROC, (LONG_PTR)(p)))
#define ki_get_class_wndproc(h)  ((WNDPROC)::GetClassLongPtr((h), GCLP_WNDPROC))
#define ki_set_class_cursor(h,c) ((HCURSOR)::SetClassLongPtr((h), GCLP_HCURSOR, (LONG_PTR)(c)))
#define KI_DLGRET                INT_PTR
#else
#define ki_get_userdata(h)       ((void*)::GetWindowLong((h), GWL_USERDATA))
#define ki_set_userdata(h,p)     ::SetWindowLong((h), GWL_USERDATA, (LONG)(p))
#define ki_get_wndproc(h)        ((WNDPROC)::GetWindowLong((h), GWL_WNDPROC))
#define ki_set_wndproc(h,p)      ((WNDPROC)::SetWindowLong((h), GWL_WNDPROC, (LONG)(p)))
#define ki_get_class_wndproc(h)  ((WNDPROC)::GetClassLong((h), GCL_WNDPROC))
#define ki_set_class_cursor(h,c) ((HCURSOR)::SetClassLong((h), GCL_HCURSOR, (LONG)(c)))
#define KI_DLGRET                BOOL
#endif

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// kiWindow : Windowの簡単な管理

class kiWindow
{
friend void kilib_startUp();

private: //-- グローバルな初期化処理など ---------------------

	static void init();
	static void finish();

public: //-- 外向きインターフェイス --------------------------

	// 関連付けられているHWND
	HWND hwnd()
		{
			return m_hWnd;
		}

	// そのWindow用にアクセラレータをロード
	void loadAccel( UINT id );

	// ウインドウがまだ残っているかどうかチェック
	bool isAlive()
		{
			if( !m_hWnd )
				return false;
			if( ::IsWindow(m_hWnd) )
				return true;
			m_hWnd = NULL;
			return false;
		}

	// 親
	kiWindow* parent()
		{
			return kiwnd( ::GetParent( hwnd() ) );
		}

	// メッセージ送信
	int sendMsg( UINT msg, WPARAM wp=0, LPARAM lp=0 )
		{
			return ::SendMessage( hwnd(), msg, wp, lp );
		}

	// [static] キューにあるメッセージを全て処理
	static void msg();

	// [static] メッセージループをまわす。
	enum msglooptype {PEEK, GET};
	static void msgLoop( msglooptype type = GET );

	// [static] Windowを強制的に front へ
	static void setFront( HWND wnd );

	// [static] Windowを中央へ
	//-- Cursor swap: hourglass in the ctor, previous cursor back in the dtor.
	//   SetCursor alone is undone the moment the mouse moves, so where it is
	//   safe the window CLASS cursor is replaced too. It is NOT safe on a
	//   dialog: every dialog in the process shares the #32770 class, so that
	//   would hand the hourglass to modal dialogs raised by an archiver DLL.
	//   On a dialog this degrades to plain SetCursor.
	class WaitCursor
		{
	public:
		WaitCursor( HWND wnd );
		~WaitCursor();

	private:
		HWND    m_hWnd;
		HCURSOR m_hPrev;
		HCURSOR m_hWait;
		};

	static void setCenter( HWND wnd, HWND rel=NULL );

	// [static] HWND -> kiWindow ( もしあれば )
	static kiWindow* kiwnd( HWND wnd )
		{
			kiWindow* ptr = (kiWindow*)ki_get_userdata( wnd );
			if( !ptr ) return NULL;
			if( ::IsBadCodePtr((FARPROC)&ptr) ) return NULL;
			return ptr;
		}

protected: //-- 派生クラス向け -----------------------------

	// 派生クラスは、作成直前にコレを呼ぶこと。
	static void preCreate( kiWindow* wnd )
		{ st_pCurInit = wnd; }
	// 破棄直前にコレを呼ぶこと。
	void detachHwnd();
	// いったんGET/POSTメッセージループを停止
	static void loopbreak()
		{
			loopbreaker = true;
		}

private: //-- 内部処理 -------------------------------------

	// ウインドウハンドル設定
	static LRESULT CALLBACK CBTProc( int code, WPARAM wp, LPARAM lp );
	static HHOOK st_hHook;
	static kiWindow* st_pCurInit;
	void setHwnd( HWND wnd )
		{
			m_hWnd = wnd;
		}

	// ウインドウ情報保持用変数
	HWND m_hWnd;
	HACCEL m_hAccel;
	// ダイアログメッセージ
	virtual bool isDlgMsg( MSG* msg )
		{ return false; }
	// GETループ一時抜けだし
	static bool loopbreaker;

protected:
	kiWindow();
public:
	virtual ~kiWindow();
};

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// kiDialog : DialogをkiWindowとして管理する

class kiDialog : public kiWindow
{
public: //-- 外向きインターフェイス --------------------------

	// モーダルダイアログとして実行
	virtual void doModal( HWND parent=NULL );

	// モードレスダイアログとして作成
	virtual void createModeless( HWND parent=NULL );

	// 終了コード取得
	UINT getEndCode()
		{
			return m_EndCode;
		}

	// モーダルか否か
	bool isModal()
		{
			return m_bStateModal;
		}

	// ダイアログアイテム
	int sendMsgToItem( UINT id, UINT msg, WPARAM wp=0, LPARAM lp=0 )
		{
			return ::SendDlgItemMessage( hwnd(), id, msg, wp, lp );
		}
	HWND item( UINT id )
		{
			return ::GetDlgItem( hwnd(), id );
		}

protected: //-- 派生クラス向け -----------------------------

	// リソースIDで初期化
	kiDialog( UINT id );

	// リソースID取得
	UINT getRsrcID()
		{
			return m_Rsrc;
		}

	// 終了コードをセット
	void setEndCode( UINT endcode )
		{
			m_EndCode = endcode;
		}

	// モーダルか否かのフラグ"のみ"を切替
	void setState( bool modal )
		{
			m_bStateModal = modal;
		}

	// 終了コードをセットして、終了する( IDOK を渡しても onOK() は呼ばれないことに注意！ )
	virtual void end( UINT endcode );

	// コマンド・メッセージ発生時に呼ばれる

		// ＯＫ -> onOK     -> if true end(IDOK)
		virtual bool onOK() {return true;}
		// 取消 -> onCancel -> if true end(IDCANCEL)
		virtual bool onCancel() {return true;}
		// WM_INITDIALOG      -> onInit
		virtual BOOL onInit() {return FALSE;}
		// WM_????            -> proc
		virtual BOOL CALLBACK proc( UINT msg, WPARAM wp, LPARAM lp ) {return FALSE;}

private: //-- 内部処理 -------------------------------------

	UINT m_EndCode;
	UINT m_Rsrc;
	bool m_bStateModal;
	bool isDlgMsg( MSG* msg )
		{
			return (!!::IsDialogMessage( hwnd(), msg ));
		}
	static KI_DLGRET CALLBACK commonDlg( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
};

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// kiPropSheet : PropertySheetをkiWindowとして管理する。

#define IDAPPLY      (0x3021)
#define ID_KIPS_HELP (0x0009)

class kiPropSheetPage : public kiDialog
{
friend class kiPropSheet;

protected: //-- 派生クラス向け ----------------------------

	// ダイアログやアイコンのIDで初期化
	kiPropSheetPage( UINT dlgid )
		: kiDialog( dlgid ), m_hIcon( NULL ) {}
	void setIcon( HICON h )
		{ m_hIcon = h; }

	// ＯＫ/適用 -> page::onOK -> sheet::onOK -> (if ok 終了)
	// virtual bool onOK()
	// WM_INITDIALOG
	// virtual BOOL onInit()
	// その他
	// virtual BOOL CALLBACK proc( UINT msg, WPARAM wp, LPARAM lp )

private: //-- 内部処理 -------------------------------------

	void end( UINT endcode ) {}
	void setInfo( PROPSHEETPAGE* p );
	HICON m_hIcon;
};

class kiPropSheet : public kiDialog
{
friend class kiPropSheetPage;

public: //-- 外向きインターフェイス --------------------------

	// モーダルダイアログとして実行
	void doModal( HWND parent );

	// モードレスダイアログとして作成
	void createModeless( HWND parent );

protected: //-- 派生クラス向け ----------------------------

	// コンストラクタ辺りで↓これをいじるべし
	PROPSHEETHEADER m_Header;
	kiArray<kiPropSheetPage*> m_Pages;

	// 終了
	void end( UINT endcode );
	// 
	void sendOK2All()
	{
		for( unsigned int i=0;i!=m_Pages.len(); i++ )
			if( m_Pages[i]->isAlive() )
				m_Pages[i]->onOK();
	}

	// ＯＫ/適用 -> page::onOK -> sheet::onOK -> (if ok 終了)
	// virtual void onOK()
	// キャンセル -> sheet::onCancel -> 終了
	// virtual void onCancel()
	// PSCB_INITIALIZED
	// virtual BOOL onInit()
	// 適用
	virtual void onApply() {}
	// ヘルプ
	virtual void onHelp() {}
	// その他コマンド
	virtual void onCommand( UINT id ) {}
	// ファイルドロップ
	virtual void onDrop( HDROP hdrop ) {}

private: //-- 内部処理 ---------------------------------------

	void begin();
	bool m_bStateModal;

	static kiPropSheet* st_CurInitPS;
	WNDPROC m_DefProc;
	bool isDlgMsg( MSG* msg )
		{ return !!PropSheet_IsDialogMessage( hwnd(),msg ); }
	static int CALLBACK main_initProc( HWND dlg, UINT msg, LPARAM lp );
	static LRESULT CALLBACK main_cmmnProc( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
	static KI_DLGRET CALLBACK page_cmmnProc( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
	static UINT CALLBACK page_initProc( HWND dlg, UINT msg, LPPROPSHEETPAGE ppsp );

protected:
	kiPropSheet();
public:
	~kiPropSheet()
		{ for( unsigned int i=0; i!=m_Pages.len(); i++ ) delete m_Pages[i]; }
};

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// kiListView : ListViewコントロールの簡単なラッパ

class kiListView
{
public:
	kiListView( kiDialog* dlg, UINT id )
		{
			m_hWnd = ::GetDlgItem( dlg->hwnd(), id );
		}

	void insertColumn( int y, const char* title,
						int width=100, int fmt=LVCFMT_LEFT )
		{
			LVCOLUMN col;
			col.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
			col.pszText = const_cast<char*>(title);
			col.cx = width;
			col.fmt = fmt;
			::SendMessage( m_hWnd, LVM_INSERTCOLUMN, y, (LPARAM)&col );
		}

	void insertItem( int x, const char* str, LPARAM param=0, int iImage=-1 )
		{
			LVITEM item;
			item.mask = LVIF_TEXT | LVIF_PARAM | (iImage!=-1 ? LVIF_IMAGE : 0);
			item.pszText = const_cast<char*>(str);
			item.iItem = x;
			item.iSubItem = 0;
			item.iImage = iImage;
			item.lParam = param; 
			::SendMessage( m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&item );
		}

	void setSubItem( int x, int y, const char* str )
		{
			LVITEM item;
			item.mask = LVIF_TEXT;
			item.pszText = const_cast<char*>(str);
			item.iItem = x;
			item.iSubItem = y;
			::SendMessage( m_hWnd, LVM_SETITEM, 0, (LPARAM)&item );
		}

	void setImageList( HIMAGELIST Large, HIMAGELIST Small )
		{
			::SendMessage( m_hWnd, LVM_SETIMAGELIST, LVSIL_NORMAL, (LPARAM)Large );
			::SendMessage( m_hWnd, LVM_SETIMAGELIST, LVSIL_SMALL,  (LPARAM)Small );
		}

private:
	HWND m_hWnd;
};

#endif
