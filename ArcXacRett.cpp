#include "stdafx.h"
#include "ArcXacRett.h"
#include "NoahApp.h"

// XacRett.h の ARCHIVETYPE_*** を写したもの。
// XacRett.h は EXTRACTINGINFO を二重定義するので include できない。
enum {
	XATYPE_ARC =  2, XATYPE_ARG =  3, XATYPE_ASD =  6, XATYPE_ATOB =  7,
	XATYPE_BEL =  8, XATYPE_BOO = 10, XATYPE_CPIO= 15, XATYPE_DZ   = 17,
	XATYPE_FISH= 18, XATYPE_FRZ = 19, XATYPE_HQX = 22, XATYPE_IMP  = 23,
	XATYPE_ISH = 24, XATYPE_IS  = 25, XATYPE_JAK = 26, XATYPE_JAM  = 27,
	XATYPE_MACBIN=29,XATYPE_MIME= 30, XATYPE_PACK=32, XATYPE_PIT  = 33,
	XATYPE_QPAK= 34, XATYPE_REZ = 36, XATYPE_SHAR= 37, XATYPE_SPL  = 38,
	XATYPE_UUE = 40, XATYPE_YNC = 41, XATYPE_ZAC = 42, XATYPE_ZOO  = 44,
	XATYPE_TPK = 49, XATYPE_XXD = 53, XATYPE_BLZ = 54,
	XATYPE_ALZ = 55, XATYPE_DMG = 56, XATYPE_ISO = 57, XATYPE_LZX  = 58,
	XATYPE_JC1 = 59, XATYPE_APK = 60, XATYPE_SQX = 61, XATYPE_ASK  = 62,
	XATYPE_LBR = 63, XATYPE_AIN = 64, XATYPE_TLZM= 67, XATYPE_TXZ  = 68,
	XATYPE_ZSTD= 69, XATYPE_TZST= 70
};

//----------------------------------------------------------------------//

int CArcXacRett::v_load()
{
	// 展開専用。圧縮系は XacRettQueryFunctionList が ISARC_ADD に偽を返すので無い
	if( !dll.exist() )
		return 0;
	return aCheck|aMelt|aList|aMeltEach;
}

//----------------------------------------------------------------------//

bool CArcXacRett::check_danger_path( const arcname& aname )
{
	// DLL は 1ファイルずつ書いてくるので、展開前に書庫内の名前を全部見ておく
	aflArray lst;
	if( !dll.lst_dll( aname, lst, "*" ) )
		return true;

	CArcPathCheckDlg chk;
	for( unsigned int i=0; i!=lst.len(); i++ )
		if( containsDangerPath( lst[i].inf.szFileName ) )
			if( !chk.is_ok_to_extract( lst[i].inf.szFileName, app()->mainwnd() ) )
				return false;
	return true;
}

//----------------------------------------------------------------------//

int CArcXacRett::v_melt( const arcname& aname, const kiPath& ddir, const aflArray* files )
{
	if( !check_danger_path( aname ) )
		return 0x8020; // ユーザにより中止

	// 出力先をカレントに
	::SetCurrentDirectory( ddir );

	// XacRett のコマンドラインは
	//   <Dummy> [-n1] "<書庫名>" "<展開先\>" [ファイル名…]
	// という形式で、動詞が無いぶん先頭トークンが必ず読み飛ばされる( CSplitCmdLine::Split )。
	// 展開先は \ で終わって無いと出力先と認識されない( IsBaseDir )。
	kiPath dir( ddir );
	dir.beShortPath(), dir.beBackSlash( true );

	kiStr cmd = "Noah -n1 \"";
	cmd += aname.basedir, cmd += aname.lname, cmd += "\" \"", cmd += dir, cmd += "\"";

	// ( もしあれば )ファイルリスト
	if( files )
		for( unsigned int i=0; i!=files->len(); i++ )
			if( (*files)[i].selected )
				cmd += " \"", cmd += (*files)[i].inf.szFileName, cmd += '"';

	// -n1 は「進捗ダイアログを出さない」。Noah 側が出すので重ねない
	return dll.cmd( cmd );
}

//----------------------------------------------------------------------//

kiStr CArcXacRett::v_name( const char* an ) const
{
	// XacRett は 1 つの DLL で 70 種類の形式を判別するので、判定結果で名前を出す
	switch( dll.arctype(an) )
	{
	case XATYPE_ALZ: return "ALZ";
	case XATYPE_DMG: return "DMG";
	case XATYPE_ISO: return "ISO";
	case XATYPE_LZX: return "LZX";
	case XATYPE_JC1: return "JCALG1";
	case XATYPE_APK: return "aPLib";
	case XATYPE_SQX: return "SQX";
	case XATYPE_ASK: return "ASK";
	case XATYPE_LBR: return "LBR";
	case XATYPE_AIN: return "AIN";
	case XATYPE_ZSTD:return "ZSTD";
	case XATYPE_TZST:return "TAR+ZSTD";
	case XATYPE_TXZ: return "TAR+XZ";
	case XATYPE_TLZM:return "TAR+LZMA";
	case XATYPE_TPK: return "TAR+PACK";
	case XATYPE_ARC: return "ARC";
	case XATYPE_ARG: return "ARG";
	case XATYPE_ASD: return "ASD";
	case XATYPE_CPIO:return "CPIO";
	case XATYPE_DZ:  return "DZIP";
	case XATYPE_FISH:return "FISH";
	case XATYPE_FRZ: return "FREEZE";
	case XATYPE_HQX: return "BINHEX";
	case XATYPE_IMP: return "IMP";
	case XATYPE_ISH: return "ISH";
	case XATYPE_IS:  return "InstallShield";
	case XATYPE_JAK: return "JAK";
	case XATYPE_JAM: return "JAM";
	case XATYPE_MACBIN:return "MacBinary";
	case XATYPE_MIME:return "BASE64";
	case XATYPE_PACK:return "PACK";
	case XATYPE_PIT: return "PIT";
	case XATYPE_QPAK:return "QuakePak";
	case XATYPE_REZ: return "REZ";
	case XATYPE_SHAR:return "SHAR";
	case XATYPE_SPL: return "SPLINT";
	case XATYPE_UUE: return "UUDecode";
	case XATYPE_XXD: return "XXDecode";
	case XATYPE_YNC: return "yDecode";
	case XATYPE_ZAC: return "ZAC";
	case XATYPE_ZOO: return "ZOO";
	case XATYPE_BEL: return "BELON";
	case XATYPE_ATOB:return "AsciiToBinary";
	case XATYPE_BOO: return "BOO";
	case XATYPE_BLZ: return "BRIEFLZ";
	}
	return "XacRett";
}