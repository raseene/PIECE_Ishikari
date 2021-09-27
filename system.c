/****************************

		基本システム

 ****************************/

#include "system.h"


BYTE	vram[SCREEN_W*SCREEN_H];			// 画面バッファ

static int	fade_flag = 0;					// 画面フェード状態
static int	fade_cnt = 0;					// 画面の明るさ
static int	sys_bright;						// 明るさ設定値

static int	snd_fade_flag = 0;				// サウンドフェード状態
static int	snd_fade_cnt  = 0;				// サウンドフェードカウンタ

int		common_counter;						// 汎用カウンタ
int		master_volume;						// 全体の音量


/********************
    画面明るさ設定
 ********************/
static
void	set_bright(void)
{
	fade_cnt += fade_flag;
	if ( fade_cnt <= 0 ) {
		fade_cnt  = 0;
		fade_flag = 0;
	}
	else if ( fade_cnt >= 1000 ) {
		fade_cnt  = 1000;
		fade_flag = 0;
	}
	pceLCDSetBright(fade_cnt*sys_bright/1000);
}

/**********************************
    画面フェードイン
		引数	t = フェード時間
 **********************************/
void	fade_in(int t)
{
	fade_flag = (t == 0) ? 1000 : (1000 + t - 1)/t;
	set_bright();
}

/**********************************
    画面フェードアウト
		引数	t = フェード時間
 **********************************/
void	fade_out(int t)
{
	fade_flag = (t == 0) ? -1000 : -(1000 + t - 1)/t;
	set_bright();
}


/**************************************
    BMP設定
		引数	bmp = BMPバッファ
				pat = パターンデータ
 **************************************/
void	set_bmp(PIECE_BMP* bmp, const BYTE* pat)
{
	bmp->header = *(PBMP_FILEHEADER*)pat;							// ヘッダ
	bmp->buf    = (BYTE*)(pat + sizeof(PBMP_FILEHEADER));			// パターン
	bmp->mask   = bmp->buf + bmp->header.w*bmp->header.h/4;			// マスク
}

/***************************************
    スプライト設定
		引数	   spr = スプライト
				   bmp = ビットマップ
				sx, sy = 転送元座標
				  w, h = 大きさ
				 param = パラメータ
 ***************************************/
void	set_sprite(Sprite* spr, PIECE_BMP* bmp, int sx, int sy, int w, int h, int param)
{
	spr->bmp	= bmp;
	spr->sx		= sx;
	spr->sy		= sy;
	spr->w		= w;
	spr->h		= h;
	spr->param	= param;
	spr->ox		= w/2;
	spr->oy		= h/2;
}

/**********************************
    スプライト描画
		引数	spr = スプライト
				x, y = 描画座標
 **********************************/
void	draw_sprite(Sprite* spr, int x, int y)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x - spr->ox, y - spr->oy, spr->sx, spr->sy, spr->w, spr->h, spr->param);
	pceLCDDrawObject(obj);											// 描画
}


/*****************
    BGM音量設定
 *****************/
static
void	set_volume(void)
{
	snd_fade_cnt += snd_fade_flag;
	if ( snd_fade_cnt >= 1000 ) {
		StopMusic();								// BGM停止
		snd_fade_cnt  = 1000;
		snd_fade_flag = 0;
	}
	else {
		pceWaveSetChAtt(0, snd_fade_cnt*127/1000);
	}
}

BYTE const*		current_bgm = NULL;					// 再生中のBGM

/*********************************
    BGM再生
		引数	bgm = BGMデータ
 *********************************/
void	play_bgm(BYTE const* bgm)
{
	if ( bgm == current_bgm ) {						// すでに再生中
		return;
	}
	if ( current_bgm || (snd_fade_cnt < 1000) ) {
		StopMusic();
	}
	current_bgm  = bgm;
	snd_fade_cnt = 0;
	PlayMusic((unsigned char*)bgm);
	pceWaveSetChAtt(0, 0);
}

/******************************
    BGM停止
		引数	t = 停止時間
 ******************************/
void	stop_bgm(int t)
{
	current_bgm = NULL;
	if ( snd_fade_cnt < 1000 ) {
		snd_fade_flag = (t == 0) ? 1000 : (1000 + t - 1)/t;
		set_volume();
	}
}

/*************************************
    効果音設定
		引数	info = 効果音情報
				wave = 効果音データ
 *************************************/
void	set_wave(PCEWAVEINFO* info, const unsigned char* wave)
{
	memcpy(info, wave + 8, sizeof(PCEWAVEINFO));
	info->pData = wave + 8 + sizeof(PCEWAVEINFO);
}

/***********************************
    効果音再生
		引数	info = 効果音情報
 ***********************************/
void	play_se(PCEWAVEINFO* info)
{
	static int	n = 0;

	pceWaveDataOut(1 + n, info);
	n = ++n % 3;
}


/***************************************
    初期化
		引数	period = フレーム間隔
 ***************************************/
void	init_system(int period)
{
	PCETIME	tm;

	pceLCDDispStop();												// 画面表示停止
	pceLCDSetBuffer(vram);											// 画面バッファ設定
	sys_bright = pceLCDSetBright(0);								// 明るさ設定値
	pceLCDDispStart();												// 画面表示開始

	InitMusic();													// 音楽ライブラリ初期化
	master_volume = pceWaveSetMasterAtt(INVALIDVAL);				// 全体の音量

	pceTimeGet(&tm);
	srand((int)tm.s100 + (int)tm.ss*100 + (int)tm.mi*100*60);		// 乱数初期化

	pceAppSetProcPeriod(period);									// 同期設定
}

/**********
    終了
 **********/
void	exit_system(void)
{
	StopMusic();
	pceWaveStop(0);
	pceWaveSetMasterAtt(master_volume);								// 音量を戻す
	pceLCDSetBright(sys_bright);									// 明るさを戻す
}

/************************************
    稼働
		引数	cnt = 稼働カウンタ
 ************************************/
void	update_system(int cnt)
{
	pceLCDTrans();													// 実画面転送
	if ( fade_flag ) {												// 画面フェードアウト
		set_bright();
	}
	if ( snd_fade_flag ) {											// サウンドフェードアウト
		set_volume();
	}
	common_counter = cnt;											// 汎用カウンタ
	rand();
}
