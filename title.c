/*****************************

		タイトル

 *****************************/

#include "system.h"
#include "ishikari.h"
#include "pattern.h"
#include "menu.h"
#include "sound.h"
#include "str_def.h"


/*** テクスチャ *******/
enum
{
	BMP_BACK,
	BMP_LOGO,
	BMP_STAGE,
	BMP_PARTS,

	BMP_MAX,
};

static PIECE_BMP	bmp_texture[BMP_MAX];			// テクスチャBMP


/*** スプライト *******/
enum
{
	SPR_BACK	= 0,					// 背景
	SPR_LOGO,							// タイトルロゴ

	SPR_ARROW,							// 矢印
	SPR_STAGE	= SPR_ARROW + 2,		// ステージ情報

	SPR_MAX		= SPR_STAGE + STAGE_MAX,
};

static Sprite	sprite[SPR_MAX];					// スプライト


/*** 実行段階 *******/
enum
{
	PHASE_TITLE,				// タイトル
	PHASE_SELECT,				// ステージ選択
};

static int		phase;			// 実行段階
static int		cnt;			// 汎用カウンタ
static int		screen_y;		// 画面描画原点
static int		stage_cnt;		// ステージ情報スクロールカウンタ
static int		exit_cnt;		// 終了カウンタ
static Menu		menu;			// メニュー管理


/********************************************
    初期化
		引数	mode =  TRUE：タイトル
				       FALSE：ステージ選択
 ********************************************/
void	init_title(BOOL mode)
{
	set_bmp(&bmp_texture[BMP_BACK],		PAT_TITLE);				// 背景
	set_bmp(&bmp_texture[BMP_LOGO],		PAT_LOGO);				// タイトルロゴ
	set_bmp(&bmp_texture[BMP_STAGE],	PAT_STAGE);				// ステージ情報
	set_bmp(&bmp_texture[BMP_PARTS],	PAT_PARTS);				// パーツ

	{
		static const
		BYTE	spr_data[SPR_STAGE][5] =
		{
			{BMP_BACK,	   0,  0,128, 88},						// 背景
			{BMP_LOGO,	   0,  0,112, 48},						// タイトルロゴ

			{BMP_PARTS,	 112, 32, 16, 16},						// 矢印
			{BMP_PARTS,	 112, 48, 16, 16},
		};
		BYTE const*		p = &spr_data[0][0];
		int		i;

		for (i = 0; i < SPR_STAGE; i++) {						// スプライト設定
			set_sprite(&sprite[i], &bmp_texture[p[0]], (int)p[1], (int)p[2], (int)p[3], (int)p[4], DRW_NOMAL);
			p += 5;
		}
		sprite[SPR_BACK].param = DRW_CLR(COLOR_BLACK, COLOR_GLAY_B);

		for (i = 0; i < STAGE_MAX; i++) {						// ステージ情報スプライト
			set_sprite(&sprite[SPR_STAGE + i], &bmp_texture[BMP_STAGE], (i % 5)*64, (i/5)*44, 64, 44, (i < limit_stage) ? DRW_CLR(COLOR_BLACK, COLOR_WHITE) : DRW_CLR(COLOR_BLACK, COLOR_GLAY_B));
		}

	}

	draw_sprite(&sprite[SPR_BACK], SCREEN_W/2, SCREEN_H/2);		// 背景描画
	memcpy(vram_buf, vram, SCREEN_W*SCREEN_H);					// 画面退避


	{
		static const char*	item[] =							// メニュー項目
		{
			STR_START_GAME,					// "  ゲーム開始  "
			STR_EXIT,						// "     終了     "
		};

		init_menu(&menu, 2, &item[0]);
	}

	if ( mode ) {												// タイトル
		phase		= PHASE_TITLE;
		screen_y	= 0;
	}
	else {														// ステージ選択
		phase		= PHASE_SELECT;
		screen_y	= -88;
	}
	cnt			= 0;
	stage_cnt	= 0;
	exit_cnt	= 0;

	play_bgm(seq_title);										// BGM再生
	fade_in(12);												// フェードイン
}


/****************************
    タイトル稼働
		戻り値	遷移シーン
 ****************************/
static
int		update(void)
{
	switch ( phase ) {
	  case PHASE_TITLE :				// タイトル
		if ( cnt > 0 ) {
			cnt--;
			screen_y = -cnt*cnt*88/(8*8);
		}
		else {
			switch ( update_menu(&menu) ) {
			  case 0 :								// ゲーム開始
				phase = PHASE_SELECT;
				cnt = 8;
				stage_cnt = 0;
				play_se(&se[SE_ROT]);
				break;

			  case 1 :								// 終了
				pceAppReqExit(0);
				break;
			}
		}
		break;

	  case PHASE_SELECT :				// ステージ選択
		if ( cnt > 0 ) {
			cnt--;
			screen_y = -88 + cnt*cnt*88/(8*8);
		}
		else {
			if ( stage_cnt > 0 ) {
				stage_cnt--;
			}
			else if ( stage_cnt < 0 ) {
				stage_cnt++;
			}
			if ( exit_cnt > 0 ) {
				if ( --exit_cnt == 0 ) {				// ゲームへ
					return	SCENE_GAME;
				}
			}
			else if ( pcePadGet() & TRG_A ) {			// ステージ決定
				play_se(&se[SE_ROT]);
				stop_bgm(10);
				exit_cnt = 12;
				fade_out(12);
			}
			else if ( pcePadGet() & TRG_RI ) {			// →
				if ( data.stage_num < limit_stage - 1 ) {
					data.stage_num++;
					stage_cnt = 4 - 1;
					play_se(&se[SE_BEEP]);
				}
			}
			else if ( pcePadGet() & TRG_LF ) {			// ←
				if ( data.stage_num > 0 ) {
					data.stage_num--;
					stage_cnt = -4 + 1;
					play_se(&se[SE_BEEP]);
				}
			}
			else if ( pcePadGet() & TRG_B ) {			// キャンセル
				phase = PHASE_TITLE;
				cnt = 8;
				play_se(&se[SE_START]);
			}
		}
		break;
	}

	return	-1;
}



/**************
    画面描画
 **************/
static
void	draw_screen(void)
{
#if 0
	draw_sprite(&sprite[SPR_BACK], SCREEN_W/2, SCREEN_H/2);					// 背景
#else
	memcpy(vram, vram_buf, SCREEN_W*SCREEN_H);								// 背景
#endif

	draw_sprite(&sprite[SPR_LOGO], 66, screen_y + 30);						// タイトルロゴ
	draw_menu(&menu, SCREEN_W/2, screen_y + 70);							// メニュー

	{																		// ステージ情報
		int		i, x = SCREEN_W/2;

		if ( stage_cnt > 0 ) {
			x += stage_cnt*stage_cnt*72/(4*4);
		}
		else if ( stage_cnt < 0 ) {
			x -= stage_cnt*stage_cnt*72/(4*4);
		}
		for (i = -2; i <= 2; i++) {
			if ( (data.stage_num + i >= 0) && (data.stage_num + i < STAGE_MAX) ) {
				pceLCDPaint(3, x + i*72 - 32, screen_y + 88 + 48 - 22, 68, 48);
				pceLCDPaint((data.stage_num + i < limit_stage) ? 1 : 2, x + i*72 - 34, screen_y + 88 + 48 - 24, 68, 48);
				draw_sprite(&sprite[SPR_STAGE + data.stage_num + i], x + i*72, screen_y + 88 + 48);
			}
		}
	}
	{																		// ステージ番号
		char	buf[0x10];

		sprintf(buf, STR_STAGE" %d", data.stage_num + 1);
		pceFontSetType(1);
		pceFontSetBkColor(FC_SPRITE);
		pceFontSetTxColor(COLOR_BLACK);
		pceFontSetPos(33, screen_y + 88 + 7);
		pceFontPutStr(buf);
		pceFontSetTxColor(COLOR_WHITE);
		pceFontSetPos(32, screen_y + 88 + 6);
		pceFontPutStr(buf);
	}
	if ( data.stage_num > 0 ) {												// ←
		int		d = (stage_cnt < 0) ? 1 : 0;
		draw_sprite(&sprite[SPR_ARROW + 0],  24 + d, screen_y + 88 + 14 + d);
	}
	if ( data.stage_num < limit_stage - 1 ) {								// →
		int		d = (stage_cnt > 0) ? 1 : 0;
		draw_sprite(&sprite[SPR_ARROW + 1], 104 + d, screen_y + 88 + 14 + d);
	}
	if ( data.clear_flag[data.stage_num/32] & (1 << (data.stage_num % 32)) && (stage_cnt == 0) ) {		// クリア済み
		const char*		str = STR_CLEARED;
		int		x = (SCREEN_W - strlen(str)*5)/2;

		pceFontSetType(0);
		pceFontSetBkColor(FC_SPRITE);
		pceFontSetTxColor(COLOR_BLACK);
		pceFontSetPos(x + 1, screen_y + 88 + 75);
		pceFontPutStr(str);
		pceFontSetPos(x, screen_y + 88 + 76);
		pceFontPutStr(str);
		pceFontSetTxColor(COLOR_WHITE);
		pceFontSetPos(x, screen_y + 88 + 75);
		pceFontPutStr(str);
	}
}


/****************************
    稼働
		戻り値	遷移シーン
 ****************************/
int		update_title(void)
{
	int		ret = update();										// ゲーム稼働

	if ( ret >= 0 ) {
		return	ret;
	}
	draw_screen();												// 画面描画
	return	-1;
}
