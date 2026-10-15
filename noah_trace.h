// noah_trace.h : opt-in runtime tracing, for diagnosing platform-specific
// startup/dialog failures on systems we cannot attach a debugger to
// (Win32s, real Win95, etc).
//
// Build with  /DNOAH_TRACE  to enable;  NMAKE /f msvc4.mak TRACE=1  does it
// for you. Every call site is wrapped in #ifdef NOAH_TRACE, so builds without
// it do not even need this file.
//
// Output is appended to NoahTrace.log in the directory holding the executable
// (Noah changes the current directory constantly, so a relative path is
// useless). The log is truncated on first write, so each run is self-contained.
//
// Deliberately uses only Win32 file I/O. Do NOT pull in <stdio.h> here: this
// VC4-era SDK defines getc/putc as macros, which collide with kiFile::getc()
// and kiFile::putc() in kilib/kl_file.h and break the build.
//
// The point of this file is that the whole window-creation chain becomes
// observable, including the API results the application itself throws away --
// notably PropertySheet()'s return value, which kiPropSheet::begin() discards,
// and the result of the WH_CBT hook that every kiWindow relies on to learn its
// own HWND.

#ifndef AFX_NOAHTRACE_H_GUID_INCLUDED_
#define AFX_NOAHTRACE_H_GUID_INCLUDED_

#ifdef NOAH_TRACE

#include <stdarg.h>

// Normally from <stdio.h>, which we must not include here (see header note).
extern "C" int __cdecl _vsnprintf( char* buf, unsigned int count, const char* fmt, va_list args );

inline void noah_trace( const char* fmt, ... )
{
	static HANDLE hFile = INVALID_HANDLE_VALUE;

	if( hFile == INVALID_HANDLE_VALUE )
	{
		char path[MAX_PATH];
		DWORD n = ::GetModuleFileName( NULL, path, sizeof(path) );
		if( n && n < sizeof(path) )
		{
			path[n] = 0;
			for( int i=(int)n-1; i>=0; i-- )
				if( path[i]=='\\' || path[i]=='/' ) { path[i+1] = 0; break; }
		}
		else
		{
			path[0] = '.'; path[1] = 0;
		}

		// No CRT string functions here -- keep it macro-safe (see header note).
		static const char tail[] = "\\NoahTrace.log";
		int i = 0;
		while( path[i] ) i++;
		int j = 0;
		while( tail[j] ) { path[i++] = tail[j]; ++j; }
		path[i] = 0;

		hFile = ::CreateFile( path, GENERIC_WRITE, FILE_SHARE_READ,
		                      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile == INVALID_HANDLE_VALUE )
		{
			hFile = ::CreateFile( "NoahTrace.log", GENERIC_WRITE, FILE_SHARE_READ,
			                      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			if( hFile == INVALID_HANDLE_VALUE )
				return;
		}
	}

	char line[1024];
	va_list args;
	va_start( args, fmt );
	int n = _vsnprintf( line, sizeof(line)-2, fmt, args );
	va_end( args );
	if( n < 0 )
		return;
	if( n > (int)sizeof(line)-2 )
		n = (int)sizeof(line)-2;

	line[n]    = '\r';
	line[n+1]  = '\n';

	DWORD written = 0;
	::WriteFile( hFile, line, n+2, &written, NULL );
}

#define NOAHTRACE  noah_trace

#else

#define NOAHTRACE(args) ((void)0)

#endif // NOAH_TRACE

#endif