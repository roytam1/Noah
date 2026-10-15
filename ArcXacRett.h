#ifndef AFX_ARCXACRETT_H__6E2D8C41_7A05_4F3B_B8D7_1C4E9A305F62__INCLUDED_
#define AFX_ARCXACRETT_H__6E2D8C41_7A05_4F3B_B8D7_1C4E9A305F62__INCLUDED_

#include "ArcDLL.h"
#include "SubDlg.h"

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// XacRett.dll … 展開専用。Unlha32.dll と同じ統合アーカイバDLLレイヤ2なので
// 出ているAPIは KiArcDLLRaw が引ける物だけで、コマンドライン書式も独特なので
// v_melt は自前で組み立てる。
//
// Noah に専用エンジンがある形式 ( lzh/zip/7z/rar/cab/tar/gz/bz2/xz/lzma/arj/
// yz1/gca/ace/cpt/bin/gza/bza ) は受け持たせない。受け持つのは Noah が他に
// 展開できない形式だけで、残りは全て v_check ( ヘッダ判定 ) に任せる。
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

class CArcXacRett : public CArcDLL
{
public:
	CArcXacRett() : CArcDLL(
		"XacRett.dll",
		"ain.alz.apk.arc.arg.asd.blz.cpio.dmg.dz.f.hdr.hqx.imp.ish.iso."
		"jak.jcalg1.jc.jc1.lbr.lzx.ntx.pak.pff.pit.pma.rez.sp.spl.sqx."
		"tlz.txz.tzst.wad.zac.zst.zoo.",
		NULL, NULL,		// 解凍コマンドは v_melt で自前に組み立てるので無い
		1 ) {}

private:
	int  v_load();
	int  v_melt( const arcname& aname, const kiPath& ddir, const aflArray* files );
	kiStr v_name( const char* an ) const;

	// 展開前に書庫内パスを検査し、怪しいものがあればユーザに確認する
	bool check_danger_path( const arcname& aname );
};

#endif