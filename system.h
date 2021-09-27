#ifndef	___SYSTEM_H___
#define	___SYSTEM_H___

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <piece.h>
#include <muslib.h>


/*** スプライト *******/
typedef struct
{
	PIECE_BMP*	bmp;			// ビットマップデータ
	int			sx;				// 転送元座標
	int			sy;
	int			w;				// 大きさ
	int			h;
	int			param;			// 描画パラメータ
	int			ox;				// 原点
	int			oy;
} Sprite;


extern BYTE		vram[SCREEN_W*SCREEN_H];		// 画面バッファ

extern int		common_counter;					// 汎用カウンタ
extern int		master_volume;					// 全体の音量


#define	rnd(n)	(rand() % (n))					// 乱数取得


void	fade_in(int);							// 画面フェードイン
void	fade_out(int);							// 画面フェードアウト

void	set_bmp(PIECE_BMP*, const unsigned char*);						// BMP設定
void	set_sprite(Sprite*, PIECE_BMP*, int, int, int, int, int);		// スプライト設定
void	draw_sprite(Sprite*, int, int);									// スプライト描画

void	play_bgm(BYTE const*);											// BGM再生
void	stop_bgm(int);													// BGM停止
void	set_wave(PCEWAVEINFO*, const unsigned char*);					// 効果音設定
void	play_se(PCEWAVEINFO*);											// 効果音再生

void	init_system(int);						// システム初期化
void	exit_system(void);						// システム終了
void	update_system(int);						// システム稼働

#endif
