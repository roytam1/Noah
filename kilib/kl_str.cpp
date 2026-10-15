//--- K.I.LIB ---
// kl_str.cpp : string classes for K.I.LIB

#include "stdafx.h"
#include "kilib.h"


//------------------------ 2byte文字処理用 ----------------------//


char kiStr::st_lb[256];

void kiStr::init()
{
	st_lb[0] = 0;
	for( int c=1; c!=256; c++ )
		st_lb[c] = (::IsDBCSLeadByte(c) ? 2 : 1);
}


//-------------------------- コピー系色々 ------------------------//


kiStr::kiStr( int start_size )
{
	(m_pBuf = new char[ m_ALen = start_size ])[0] = '\0';
}

kiStr::kiStr( const char* s, int min_size )
{
	int slen = ki_strlen(s) + 1;
	m_ALen = ( slen < min_size ) ? min_size : slen;
	ki_memcpy( m_pBuf=new char[m_ALen], s, slen );
}

kiStr::kiStr( const kiStr& s )
{
	ki_memcpy( m_pBuf=new char[m_ALen=s.m_ALen], s.m_pBuf, m_ALen=s.m_ALen );
}

kiStr& kiStr::operator = ( const kiStr& s )
{
	if( this != &s )
		*this = (const char*)s;
	return *this;
}

kiStr& kiStr::operator = ( const char* s )
{
	int slen = ki_strlen( s ) + 1;
	int  len = this->len();

	if( m_ALen < slen || s <= m_pBuf+len || m_pBuf <= s+slen )
	{
		char* tmp = new char[ m_ALen = ( m_ALen>slen ? m_ALen : slen) ];
		ki_memcpy( tmp, s, slen );
		delete [] m_pBuf;
		m_pBuf = tmp;
	}
	else
		ki_memcpy( m_pBuf, s, slen );
	return *this;
}

kiStr& kiStr::operator += ( const char* s )
{
	int slen = ki_strlen( s ) + 1;
	int  len = this->len();

	if( m_ALen < len+slen+1
	 || ( s <= m_pBuf && m_pBuf <= s+len )
	 || ( m_pBuf <= s && s <= m_pBuf+slen ) )
	{
		char* tmp = new char[ m_ALen = ( m_ALen>slen+len+1 ? m_ALen : slen+len+1) ];
		ki_memcpy( tmp, m_pBuf, len );
		delete [] m_pBuf;
		m_pBuf = tmp;
	}

	ki_memcpy( m_pBuf+len, s, slen );
	return *this;
}

kiStr& kiStr::operator += ( char c )
{
	int  len = this->len();

	if( m_ALen < len+2 )
	{
		char* tmp = new char[ m_ALen=len+20 ];
		ki_memcpy( tmp, m_pBuf, len );
		delete [] m_pBuf;
		m_pBuf = tmp;
	}

	m_pBuf[len]=c, m_pBuf[len+1]='\0';
	return *this;
}

kiStr& kiStr::setInt( int n, bool cm )
{
	if( n==0 )
		m_pBuf[0] = '0', m_pBuf[1] = '\0';
	else
	{
		bool minus = (n<0);
		if( minus )
			n= -n;

		char tmp[30];
		tmp[29]='\0';
		int i;

		for( i=28; i>=0; i-- )
		{
			if( cm && (29-i)%4==0 )
				tmp[i--] = ',';
			tmp[i] = '0' + n%10;
			n /= 10;
			if( n==0 )
				break;
		}

		if( minus )
			tmp[--i] = '-';

		(*this) = tmp+i;
	}
	return (*this);
}

//-------------------------- 文字列処理全般 ------------------------//


kiStr::~kiStr()
{
	delete [] m_pBuf;
}

kiStr::operator const char*() const
{
	return m_pBuf;
}

bool kiStr::operator == ( const char* s ) const
{
	return 0==ki_strcmp( m_pBuf, s );
}

bool kiStr::isSame( const char* s ) const
{
	return 0==ki_strcmpi( m_pBuf, s );
}

int kiStr::len() const
{
	return ki_strlen( m_pBuf );
}


//-------------------------- ユーティリティー ------------------------//


kiStr& kiStr::removeTrailWS()
{
	char* m=m_pBuf-1;
	for( char *p=m_pBuf; *p!='\0'; p=next(p) )
		if( *p!=' ' && *p!='\t' && *p!='\n' )
			m = p;
	*next(m) = '\0';
	return *this;
}

kiStr& kiStr::loadRsrc( UINT id )
{
	::LoadString( GetModuleHandle(NULL), id, m_pBuf, m_ALen );
	return *this;
}

void kiPath::beSpecialPath( int nPATH )
{
	switch( nPATH )
	{
	case Win:	::GetWindowsDirectory( m_pBuf, m_ALen );	break;
	case Sys:	::GetSystemDirectory( m_pBuf, m_ALen );		break;
	case Tmp:	::GetTempPath( m_ALen, m_pBuf );			break;
	case Cur:	::GetCurrentDirectory( m_ALen, m_pBuf );	break;
	case Exe_name:
				::GetModuleFileName( NULL, m_pBuf, m_ALen );break;
	case Exe:
		{
			::GetModuleFileName( NULL, m_pBuf, m_ALen );

			char* m=NULL;
			for( char *p=m_pBuf; *p!='\0'; p=next(p) )
				if( *p=='\\' )
					m = p;
			if( m )
				*m='\0';
			break;
		}
	default:
		{
			*m_pBuf = '\0';

			LPITEMIDLIST il;
			NOAHTRACE( "[path] CSIDL %d ...", (int)nPATH );
			HRESULT hr = ::SHGetSpecialFolderLocation( NULL, nPATH, &il );
			NOAHTRACE( "[path]   SHGetSpecialFolderLocation -> 0x%08lX il=%p",
				(unsigned long)hr, (void*)il );
			if( NOERROR!=hr )
				return;
			::SHGetPathFromIDList( il, m_pBuf );
			NOAHTRACE( "[path]   SHGetPathFromIDList -> \"%s\"", m_pBuf );

			//-- シェルが 何も 返さない 環境 (Win32s) では CSIDL_DESKTOP などが
			//   空文字 になる。空文字 の ままだと 解凍先 などが 決まらないので、
			//   シェルを 介さない 候補 に フォールバックする。
			if( *m_pBuf=='\0' )
			{
				::GetCurrentDirectory( m_ALen, m_pBuf );
				NOAHTRACE( "[path]   shell gave nothing, falling back to cwd \"%s\"", m_pBuf );
			}
			app()->shellFree( il );
			NOAHTRACE( "[path]   shellFree done" );
		}
	}
}

void kiPath::beBackSlash( bool add )
{
	char* last = m_pBuf;
	for( char* p=m_pBuf; *p!='\0'; p=next(p) )
		last=p;
	if( *last=='\\' || *last=='/' )
	{
		if( !add )
			*last = '\0';
	}
	else if( add && last!=m_pBuf )
		*this += '\\';
}

bool kiPath::beDirOnly()
{
	char* lastslash = m_pBuf-1;
	for( char* p=m_pBuf; *p; p=next(p) )
		if( *p=='\\' || *p=='/' )
			lastslash = p;

	*(lastslash+1) = '\0';

	return (lastslash+1 != m_pBuf);
}

bool kiPath::isInSameDir(const char* q) const
{
	bool diff=false;
	for( const char *p=m_pBuf; *p && *q; p=next(p), q=next(q) )
		if( *p != *q )
			diff = true;
		else if( diff && (*p=='\\' || *p=='/' || *q=='\\' || *q=='/') )
			return false;

	const char* r = (*p ? p : q);
	if( *r )
		for( ; *r; r=next(r) )
			if( *r=='\\' || *r=='/' )
				return false;
	return true;
}

void kiPath::beShortPath()
{
	// 入力と 出力に 同じ バッファを 使うのは だめ。MSDN が 明言している。
	// Win32s では これが 失敗 して、8.3 化 されない 長いパスが そのまま
	// 残って 出力先に なり、抽出が 失敗する。戻り値も 捨てて いた。
	kiStr before( m_pBuf );
	char sp[MAX_PATH];
	DWORD r = ::GetShortPathName( m_pBuf, sp, sizeof(sp) );
	if( r )
		ki_strcpy( m_pBuf, sp );
	NOAHTRACE( "[short] \"%s\" -> \"%s\" (ret=%lu err=%lu)",
		(const char*)before, m_pBuf,
		(unsigned long)r, (unsigned long)::GetLastError() );
}
void kiPath::mkdir()
{
	for( char *p=m_pBuf; *p; p=kiStr::next(p) )
	{
		if( (*p!='\\' && *p!='/') || (p-m_pBuf<=4) )
			continue;
		*p = '\0';
		if( !kiSUtil::exist(m_pBuf) )
			if( !::CreateDirectory( m_pBuf, NULL ) )
				NOAHTRACE( "[mkdir] CreateDirectory(\"%s\") failed err=%lu",
					m_pBuf, (unsigned long)::GetLastError() );
			else
				::SHChangeNotify( SHCNE_MKDIR,SHCNF_PATH,(const void*)m_pBuf,NULL );
		*p = '\\';
	}
}

void kiPath::remove()
{
	//-- 保険。空文字やドライブルートを渡すとカレントドライブが消える。
	//   GetFileAttributes("") が失敗しない環境 (Win32s) では下の
	//   FindFirstFile("\*") がカレントドライブのルートに解決され、
	//   中身を全て削除してしまう。ここを素通りしてはいけない。
	if( *this=='\0' )
		return;

	if( !kiSUtil::exist(*this) )
		return;
	if( !kiSUtil::isdir(*this) )
	{
		::DeleteFile(*this);
		return;
	}

	// buf == filename with no last '\\'
	kiPath buf(*this);
	buf.beBackSlash(false);

	// "C:" や "/" のような ルート指定も 拒否する
	if( (buf[1]==':' && buf[2]=='\0')
	 || (buf[0]=='/' && buf[1]=='\0') )
		return;

	kiPath tmp(buf);
	WIN32_FIND_DATA fd;
	kiFindFile find;
	find.begin( tmp += "\\*" );
	while( find.next( &fd ) )
	{
		tmp = buf;
		tmp += '\\';
		tmp += fd.cFileName;
		tmp.remove();
	}
	find.close();

	::RemoveDirectory( buf );
}

void kiPath::getBody( kiStr& str ) const
{
	char *p=const_cast<char*>(name()),*x,c;
	for( x=(*p=='.'?p+1:p); *x; x=next(x) ) // 先頭の.は拡張子と見なさない
		if( *x=='.' )
			break;
	c=*x, *x='\0';
	str=p;
	*x=c;
}

void kiPath::getBody_all( kiStr& str ) const
{
// 最後の拡張子だけ削る版
	char *p=const_cast<char*>(name()),*x=NULL, *n, c;
	for( n=(*p=='.'?p+1:p); *n; n=next(n) ) // 先頭の.は拡張子と見なさない
		if( *n=='.' )
			x = n;
	if( !x )x = n;

	c  =*x;
	*x ='\0';
	str=p;
	*x =c;
}

const char* kiPath::ext( const char* str )
{
	const char *ans = NULL, *p = name(str);
	if( *p == '.' ) ++p; // 先頭の.は拡張子と見なさない
	for( ; *p; p=next(p) )
		if( *p=='.' )
			ans = p;
	return ans ? (ans+1) : p;
}

const char* kiPath::ext_all( const char* str )
{
	const char* p = name(str);
	if( *p == '.' ) ++p; // 先頭の.は拡張子と見なさない
	for( ; *p; p=next(p) )
		if( *p=='.' )
			return (p+1);
	return p;
}

const char* kiPath::name( const char* str )
{
	const char* ans = str - 1;
	for( const char* p=str; *p; p=next(p) )
		if( *p=='\\' || *p=='/' )
			ans = p;
	return (ans+1);
}

UINT kiPath::getDriveType() const
{
	char* p;
	for( p=m_pBuf; *p=='\\'; p=next(p) );
	for( p=m_pBuf; *p && *p!='\\'; p=next(p) );
	char c=*(++p);*p='\0';
	UINT ans=::GetDriveType( m_pBuf );
	*p=c; return ans;
}

bool kiPath::endwithyen( const char* str )
{
	for( const char *p=str,*last=str; *p; p=next(p) )
		last=p;
	return ( *last=='\\' || *last=='/' );
}

