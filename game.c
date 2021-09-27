/*****************************

		ゲームメイン

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
	BMP_PARTS,
	BMP_MESSAGE,

	BMP_MAX,
};

static PIECE_BMP	bmp_texture[BMP_MAX];			// テクスチャBMP


/*** スプライト *******/
enum
{
	SPR_BACK	= 0,					// 背景

	SPR_STONE,							// 石
	SPR_CURSOR	= SPR_STONE + 6,		// カーソル
	SPR_BEAM,							// 光線発射位置
	SPR_BLUR,							// 光線の軌跡

	SPR_NUMBER	= SPR_BLUR + 3*8,		// 数字
	SPR_STAGE	= SPR_NUMBER + 10,		// "STAGE"

	SPR_CLEAR,							// "くりあ"
	SPR_FAILED,							// "しっぱい"

	SPR_MAX,
};

static Sprite	sprite[SPR_MAX];					// スプライト



/*** 石 *******/
typedef struct
{
	int		dir;			// 向き
	int		cnt;			// 消去カウンタ
} Stone;

/*** 光線 *******/
typedef struct
{
	int		x;				// 位置
	int		y;
	int		dir;			// 向き
	int		cnt;			// 移動カウンタ
} Beam;

/*** 光線の軌跡 *******/
typedef struct
{
	int			x;			// 表示座標
	int			y;
	int			cnt;		// 表示カウンタ
	Sprite*		spr;		// スプライト
} Blur;

static Stone	stone[FIELD_H][FIELD_W];			// 盤
static Beam		beam;								// 光線
static Beam		beam_default;						// 光線初期値
static Blur		blur[8];							// 光線の軌跡
static int		field_x;							// 盤の位置
static int		field_y;
static int		cursor_x;							// カーソル位置
static int		cursor_y;
static int		stone_max;							// 石最大数
static int		stone_rest;							// 残りの石の数

#define	FIELD_X(x, y)		field_x + (x)*STONE_W - (y)*(STONE_W/2)
#define	FIELD_Y(y)			field_y + (y)*STONE_H
#define	FIELD_POS(x, y)		FIELD_X(x, y), FIELD_Y(y)


/*** 実行段階 *******/
enum
{
	PHASE_EDIT,					// 石の向き設定
	PHASE_MOVE,					// 光線移動
	PHASE_CLEAR,				// クリア
	PHASE_FAILED,				// 失敗
};

static int		phase;			// 実行段階
static int		cnt;			// 汎用カウンタ
static int		exit_cnt;		// 終了カウンタ
static int		exit_flag;		// 終了後のシーン
static Menu		menu;			// メニュー管理


/**************
    盤初期化
 **************/
static
void	init_field(void)
{
extern const BYTE	stage_data[][FIELD_H][FIELD_W - FIELD_H/2];				// ステージデータ

	BYTE const*		p = &stage_data[data.stage_num][0][0];
	int		i, j, sx = SCREEN_W, sy = SCREEN_H, ex = 0, ey = 0;

	stone_max = 0;
	for (i = 0; i < FIELD_H; i++) {
		for (j = 0; j < FIELD_W; j++) {
			stone[i][j].dir = -1;
			stone[i][j].cnt = 0;
			if ( (j >= i/2) && (j < i/2 + FIELD_W - FIELD_H/2) ) {
				if ( *p != 0xff ) {
					int		x = j*STONE_W - i*(STONE_W/2),
							y = i*STONE_H;

					if ( x < sx ) {
						sx = x;
					}
					if ( x > ex ) {
						ex = x;
					}
					if ( y < sy ) {
						sy = y;
					}
					if ( y > ey ) {
						ey = y;
					}

					if ( *p < 4 ) {								// 石
						stone[i][j].dir = (int)*p;
						stone_max++;
					}
					else if ( *p == 5 ) {						// 光線発射位置
						beam_default.x = j;
						beam_default.y = i;
						beam_default.dir = 0;
						cursor_x = j;							// カーソル位置
						cursor_y = i;
					}
				}
				p++;
			}
		}
	}

	field_x = 63 - (sx + ex)/2;									// 盤の位置
	field_y = 44 - (sy + ey)/2 + (71 - ey)/4;
}

/****************
    盤再初期化
 ****************/
static
void	reset_field(void)
{
	int		i, j;

	for (i = 0; i < FIELD_H; i++) {
		for (j = 0; j < FIELD_W; j++) {
			if ( stone[i][j].dir >= 0 ) {						// 石
				stone[i][j].cnt = -1;
			}
		}
	}
	stone_rest = stone_max;										// 残りの石の数
	beam = beam_default;										// 光線
	for (i = 0; i < sizeof(blur)/sizeof(blur[0]); i++) {		// 光線の軌跡
		blur[i].cnt = 0;
	}

	play_bgm(seq_game);											// BGM再生
	phase = PHASE_EDIT;											// 実行段階
}


/************
    初期化
 ************/
void	init_game(void)
{
	static const
	unsigned char*	bmp_back[] =
	{
		PAT_BACK00, PAT_BACK01, PAT_BACK02, PAT_BACK03, PAT_BACK04,
	};

	set_bmp(&bmp_texture[BMP_BACK],		bmp_back[data.stage_num/10]);		// 背景
	set_bmp(&bmp_texture[BMP_PARTS],	PAT_PARTS);							// パーツ
	set_bmp(&bmp_texture[BMP_MESSAGE],	PAT_MESSAGE);						// メッセージ

	{
		static const
		BYTE	spr_data[SPR_MAX][5] =
		{
			{BMP_BACK,	 0,  0,128, 88},						// 背景

			{BMP_PARTS,	 0,  0, 13, 13},						// 石
			{BMP_PARTS,	16,  0, 15, 17},
			{BMP_PARTS,	32,  0, 15, 17},
			{BMP_PARTS,	48,  0, 15, 17},
			{BMP_PARTS,	104, 0, 14, 14},
			{BMP_PARTS,	104,16, 18, 16},

			{BMP_PARTS,	64,  0, 20, 18},						// カーソル
			{BMP_PARTS,	88,  0, 12, 12},						// 光線発射位置

			{BMP_PARTS,	80, 20, 18,  2},						// 光線の軌跡
			{BMP_PARTS,	80, 24,  6,  2},
			{BMP_PARTS,	80, 28, 10,  2},
			{BMP_PARTS,	80, 32, 14,  2},
			{BMP_PARTS,	80, 36, 18,  2},
			{BMP_PARTS,	84, 40, 14,  2},
			{BMP_PARTS,	88, 44, 10,  2},
			{BMP_PARTS,	92, 48,  6,  2},

			{BMP_PARTS,	 0, 20, 10, 16},
			{BMP_PARTS,	16, 30,  4,  6},
			{BMP_PARTS,	24, 27,  6,  9},
			{BMP_PARTS,	32, 23,  8, 13},
			{BMP_PARTS,	40, 20, 10, 16},
			{BMP_PARTS,	56, 20,  8, 12},
			{BMP_PARTS,	64, 20,  6,  9},
			{BMP_PARTS,	72, 20,  4,  5},

			{BMP_PARTS,	 0, 38, 10, 16},
			{BMP_PARTS,	16, 38,  4,  6},
			{BMP_PARTS,	24, 38,  6,  9},
			{BMP_PARTS,	32, 38,  8, 13},
			{BMP_PARTS,	40, 38, 10, 16},
			{BMP_PARTS,	56, 42,  8, 12},
			{BMP_PARTS,	64, 45,  6,  9},
			{BMP_PARTS,	72, 49,  4,  5},

			{BMP_PARTS,	 0, 56,  6,  8},						// 数字
			{BMP_PARTS,	 8, 56,  6,  8},
			{BMP_PARTS,	16, 56,  6,  8},
			{BMP_PARTS,	24, 56,  6,  8},
			{BMP_PARTS,	32, 56,  6,  8},
			{BMP_PARTS,	40, 56,  6,  8},
			{BMP_PARTS,	48, 56,  6,  8},
			{BMP_PARTS,	56, 56,  6,  8},
			{BMP_PARTS,	64, 56,  6,  8},
			{BMP_PARTS,	72, 56,  6,  8},
			{BMP_PARTS,	80, 56, 29,  8},						// "STAGE"

			{BMP_MESSAGE,	0,  0, 52, 16},						// "くりあ"
			{BMP_MESSAGE,	0, 18, 68, 16},						// "しっぱい"
		};
		BYTE const*		p = &spr_data[0][0];
		int		i;

		for (i = 0; i < SPR_MAX; i++) {
			set_sprite(&sprite[i], &bmp_texture[p[0]], (int)p[1], (int)p[2], (int)p[3], (int)p[4], DRW_NOMAL);
			p += 5;
		}
		sprite[SPR_BACK].param = DRW_CLR(COLOR_BLACK, COLOR_GLAY_B);
	}
	{
		static const
		short	blur_pos[] =
		{
			1, 1,	1, 1,	1, 1,	1, 1,	1, 1,	-3, 1,	-7, 1,	-11, 1,
			9, 15,	3, 5,	5, 8,	7, 12,	9, 15,	9, 15,	9, 15,	9, 15,
			9, 1,	3, 1,	5, 1,	7, 1,	9, 1,	9, -3,	9, -6,	9, -10,
		};

		short const*	p = &blur_pos[0];
		int		i;

		for (i = SPR_BLUR; i < SPR_BLUR + 3*8; i++) {
			sprite[i].ox = (int)*p++;
			sprite[i].oy = (int)*p++;
		}
	}


	draw_sprite(&sprite[SPR_BACK], SCREEN_W/2, SCREEN_H/2);		// 背景描画
	memcpy(vram_buf, vram, SCREEN_W*SCREEN_H);					// 画面退避


	menu.num = -1;												// メニュー
	exit_cnt = 0;												// 終了カウンタ

	init_field();												// 盤初期化
	reset_field();

	fade_in(12);												// フェードイン
}



/**********************************
    石の向き設定操作
		戻り値	光線を発射するか
 **********************************/
static
BOOL	edit(void)
{
	if ( pcePadGet() & TRG_LF ) {				// ←
		if ( cursor_x > cursor_y/2 ) {
			cursor_x--;
		}
	}
	else if ( pcePadGet() & TRG_RI ) {			// →
		if ( cursor_x < FIELD_W - 1 - (FIELD_H - cursor_y + 1)/2 - field_x/STONE_W ) {
			cursor_x++;
		}
	}
	else if ( pcePadGet() & TRG_UP ) {			// ↑
		if ( cursor_y > 0 ) {
			cursor_y--;
			cursor_x -= cursor_y % 2;
			if ( cursor_x < cursor_y/2 ) {
				cursor_x++;
			}
		}
	}
	else if ( pcePadGet() & TRG_DN ) {			// ↓
		if ( cursor_y < FIELD_H - 1 - (field_y - 2)/STONE_H ) {
			cursor_x += cursor_y % 2;
			cursor_y++;
			if ( cursor_x > FIELD_W - (FIELD_H - cursor_y + 1)/2 - field_x/STONE_W ) {
				cursor_x--;
			}
		}
	}

	if ( pcePadGet() & TRG_A ) {				// Ａ
		Stone*	p = &stone[cursor_y][cursor_x];

		if ( (p->dir >= 1) && (p->dir <= 3) ) {					// 石回転
			p->dir = 1 + (p->dir % 3);
			play_se(&se[SE_ROT]);
		}
		else if ( p->dir < 0 ) {								// 光線発射
			return	TRUE;
		}
	}
	else if ( pcePadGet() & TRG_B ) {			// Ｂ
		return	TRUE;											// 光線発射
	}
	return	FALSE;
}


/********************
    光線の軌跡設定
 ********************/
static
void	set_blur(void)
{
	Blur*	p= &blur[0];
	int		i;

	for (i = 0; i < sizeof(blur)/sizeof(blur[0]) - 1; i++) {
		p[0] = p[1];
		p++;
	}

	p->x = FIELD_X(beam.x, beam.y);								// 表示座標
	p->y = FIELD_Y(beam.y);
	p->spr = &sprite[SPR_BLUR + beam.dir*8];					// スプライト
	p->cnt = ((p->x - p->spr->ox < SCREEN_W) && (p->x - p->spr->ox + p->spr->w > 0) && (p->y - p->spr->oy < SCREEN_H) && (p->y - p->spr->oy + p->spr->h > 0)) ? 30 : 0;

	beam.cnt = 4;												// 光線移動カウンタ
}

/***************************
    光線移動
		戻り値	0：続行
				1：クリア
				2：失敗
 ***************************/
static
int		move_beam(void)
{
	int		ret = 0;

	if ( --beam.cnt > 0 ) {
		return	0;
	}

	switch ( beam.dir )
	{
	  case 0 :
		beam.x++;
		break;
	  case 1 :
		beam.x--;
		beam.y--;
		break;
	  case 2 :
		beam.y++;
		break;
	}
	if ( (beam.x >= 0) && (beam.x < FIELD_W) && (beam.y >= 0) && (beam.y < FIELD_H) ) {
		Stone*	p;

		p = &stone[beam.y][beam.x];
		if ( p->cnt < 0 ) {
			play_se(&se[SE_CRUSH]);
			if ( p->dir > 0 ) {									// 反射
				beam.dir = (7 - (beam.dir + p->dir)) % 3;
			}
			p->cnt = 6;
			if ( --stone_rest == 0 ) {							// クリア
				ret = 1;
			}
			else {												// 失敗チェック
				int		x = beam.x, y = beam.y;

				ret = 2;
				while ( (x >= 0) && (x < FIELD_W) && (y >= 0) && (y < FIELD_H) ) {
					if ( stone[y][x].cnt < 0 ) {
						ret = 0;
						break;
					}
					switch ( beam.dir )
					{
					  case 0 :
						x++;
						break;
					  case 1 :
						x--;
						y--;
						break;
					  case 2 :
						y++;
						break;
					}
				}
			}
		}
	}
	set_blur();													// 光線の軌跡

	return	ret;
}

/************
    石消去
 ************/
static
void	update_stone(void)
{
	Stone*	p = &stone[0][0];
	int		i, j;

	for (i = 0; i < FIELD_H; i++) {
		for (j = 0; j < FIELD_W; j++) {
			if ( p->cnt > 0 ) {
				p->cnt--;
			}
			p++;
		}
	}
}

/********************
    光線の軌跡稼働
 ********************/
static
void	update_blur(void)
{
	Blur*	p = &blur[0];
	int		i;

	for (i = 0; i < sizeof(blur)/sizeof(blur[0]); i++) {
		if ( p->cnt > 0 ) {
			p->cnt--;
		}
		p++;
	}
}

/************************************
    メニューオープン
		引数	f = エディット中か
 ************************************/
static
void	open_menu(BOOL f)
{
	static const char*	item[] =								// メニュー項目
	{
		STR_RERTY,					// "  やりなおす  "
		STR_NEXT_STAGE,				// " 次のステージ "
		STR_SELECT_STAGE,			// " ステージ選択 "
		STR_EXIT_GAME,				// "  ゲーム終了  "
	};

	int		t = f ? 1 : 0;

	init_menu(&menu, 4 - t, &item[t]);
	set_menu_active(&menu, 1 - t, (data.stage_num + 1 < limit_stage));
	play_se(&se[SE_BEEP]);
}

/****************************
    ゲーム稼働
		戻り値	遷移シーン
 ****************************/
static
int		update(void)
{
	if ( exit_cnt > 0 ) {										// 終了中
		if ( --exit_cnt == 0 ) {
			switch ( exit_flag ) {
			  case SCENE_GAME :				// 次のステージ
				data.stage_num++;
				return	SCENE_GAME;

			  case SCENE_TITLE :			// ステージ選択
				return	SCENE_SELECT;
			}
		}
		return	-1;
	}
	if ( menu.num >= 0 ) {										// メニュー
		int		n = update_menu(&menu);

		if ( n >= 0 ) {
			if ( (phase == PHASE_EDIT) && (n < 0xff) ) {
				n++;
			}
			switch ( n ) {
			  case 0xff :					// キャンセル
				menu.num = -1;
				if ( (phase == PHASE_CLEAR) || (phase == PHASE_FAILED) ) {
					reset_field();
				}
				play_se(&se[SE_START]);
				break;

			  case 0 :						// やりなおす
				menu.num = -1;
				reset_field();
				play_se(&se[SE_BEEP]);
				break;

			  case 2 :						// ステージ選択
				stop_bgm(10);
			  case 1 :						// 次のステージ
				play_se(&se[SE_BEEP]);
				exit_flag = (n == 1) ? SCENE_GAME : SCENE_TITLE;
				exit_cnt = 12;
				fade_out(12);
				break;

			  case 3 :						// ゲーム終了
				pceAppReqExit(0);
				break;
			}
		}
		return	-1;
	}

	if ( cnt > 0 ) {
		cnt--;
	}
	switch ( phase ) {
	  case PHASE_EDIT :						// 石の向き設定
		if ( pcePadGet() & TRG_START ) {						// メニュー
			open_menu(TRUE);
		}
		else if ( edit() ) {									// 操作
			set_blur();
			play_se(&se[SE_START]);
			phase = PHASE_MOVE;
		}
		break;

	  case PHASE_MOVE :						// 光線移動
		if ( pcePadGet() & TRG_A ) {
			play_se(&se[SE_ROT]);
			reset_field();										// リセット
			break;
		}
		if ( pcePadGet() & PAD_B ) {							// 一時停止
			break;
		}
		if ( pcePadGet() & TRG_START ) {						// メニュー
			open_menu(FALSE);
			break;
		}

		update_blur();											// 光線の軌跡稼働
		update_stone();											// 石消去
		switch ( move_beam() ) {								// 光線移動
		  case 1 :												// クリア
			phase = PHASE_CLEAR;
			cnt = 45;
			clear_stage();
			break;

		  case 2 :												// 失敗
			phase = PHASE_FAILED;
			cnt = 45;
			break;
		}
		break;

	  case PHASE_CLEAR :					// クリア
		if ( cnt == 40 ) {
			stop_bgm(50);
		}
	  case PHASE_FAILED :					// 失敗
		if ( pcePadGet() & PAD_B ) {							// 一時停止
			break;
		}
		if ( pcePadGet() & TRG_A ) {
			if ( cnt > 0 ) {
				if ( phase == PHASE_CLEAR ) {
					stop_bgm(0);
					play_se(&se[SE_BEEP]);
					cnt = 0;
				}
				else {
					play_se(&se[SE_ROT]);
					reset_field();								// リセット
					break;
				}
			}
			else {												// メニュー
				open_menu(FALSE);
				cnt = -1;
				if ( phase == PHASE_CLEAR ) {
					menu.cursor = 1;
				}
				break;
			}
		}
		if ( pcePadGet() & TRG_START ) {
			open_menu(FALSE);
			cnt = -1;
			break;
		}

		update_blur();											// 光線の軌跡稼働
		update_stone();											// 石消去
		move_beam();											// 光線移動
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

	switch ( phase ) {
	  case PHASE_MOVE :							// 光線移動
	  case PHASE_CLEAR :						// クリア
	  case PHASE_FAILED :						// 失敗
		{																	// 光線の軌跡
			Blur*	p;
			int		i, t;

			p = &blur[0];
			for (i = 0; i < sizeof(blur)/sizeof(blur[0]); i++) {
				if ( p->cnt > 0 ) {
					if ( p->cnt > 30 - 3 ) {
						t = 1 + 30 - p->cnt;
					}
					else if ( p->cnt < 5 ) {
						t = 8 - p->cnt;
					}
					else {
						t = 0;
					}
					draw_sprite(p->spr + t, p->x, p->y);
				}
				p++;
			}
		}
		break;
	}

	{																		// 石
		Stone*	p = &stone[0][0];
		int		i, j;

		for (i = 0; i < FIELD_H; i++) {
			for (j = 0; j < FIELD_W; j++) {
				if ( p->dir >= 0 ) {
					if ( p->cnt < 0 ) {
						draw_sprite(&sprite[SPR_STONE + p->dir], FIELD_POS(j, i));
					}
					else if ( p->cnt % 2 ) {
						draw_sprite(&sprite[p->dir ? (SPR_STONE + 4) : (SPR_STONE + 5)], FIELD_POS(j, i));
					}
				}
				p++;
			}
		}
	}

	switch ( phase ) {
	  case PHASE_EDIT :							// 石の向き設定
		draw_sprite(&sprite[SPR_BEAM], FIELD_POS(beam.x, beam.y));			// 光線発射位置
		draw_sprite(&sprite[SPR_CURSOR], FIELD_POS(cursor_x, cursor_y));	// カーソル
		break;
	}

	draw_sprite(&sprite[SPR_STAGE], 15, 5);									// "STAGE"
	{																		// ステージ番号
		int		t = data.stage_num + 1;

		if ( t < 10 ) {
			draw_sprite(&sprite[SPR_NUMBER + t], 35, 5);
		}
		else {
			draw_sprite(&sprite[SPR_NUMBER + t/10], 34, 5);
			draw_sprite(&sprite[SPR_NUMBER + (t % 10)], 40, 5);
		}
	}

	switch ( phase ) {
	  case PHASE_CLEAR :						// クリア
		if ( cnt >= 0 ) {
			draw_sprite(&sprite[SPR_CLEAR], SCREEN_W/2, SCREEN_H/2 - cnt*cnt*cnt/120);						// "くりあ"
		}
		break;

	  case PHASE_FAILED :						// 失敗
		if ( cnt >= 0 ) {
			draw_sprite(&sprite[SPR_FAILED], SCREEN_W/2, SCREEN_H/2 + ((cnt - 4)*(cnt - 4) - 4*4)/3);		// "しっぱい"
		}
		break;
	}


	if ( menu.num >= 0 ) {										// メニュー
		draw_menu(&menu, SCREEN_W/2, SCREEN_H/2);
	}
}


/****************************
    稼働
		戻り値	遷移シーン
 ****************************/
int		update_game(void)
{
	int		ret = update();										// ゲーム稼働

	if ( ret >= 0 ) {
		return	ret;
	}
	draw_screen();												// 画面描画
	return	-1;
}
