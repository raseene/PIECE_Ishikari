#ifndef	___ISHIKARI_H___
#define	___ISHIKARI_H___

#include "system.h"


/**********
    定義
 **********/

#define	STAGE_MAX	50			// ステージ数
#define	FIELD_W		11			// 盤の大きさ
#define	FIELD_H		 6
#define	STONE_W		16			// 石の間隔
#define	STONE_H		14


/************
    効果音
 ************/
enum
{
	SE_BEEP,
	SE_ROT,
	SE_START,
	SE_CRUSH,

	SE_MAX,
};

extern PCEWAVEINFO	se[SE_MAX];			// 効果音


/******************
    セーブデータ
 ******************/
typedef struct
{
	int		stage_num;								// ステージ番号
	DWORD	clear_flag[(STAGE_MAX + 31)/32];		// クリアフラグ
} SaveData;

extern SaveData		data;				// 共通データ
extern int			limit_stage;		// プレイ可能ステージ数

void	clear_stage(void);				// ステージクリア


extern BYTE		vram_buf[];				// 画面退避バッファ


/*** シーン *******/
enum
{
	SCENE_GAME,				// ゲーム
	SCENE_TITLE,			// タイトル
	SCENE_SELECT,			// ステージ選択
};

#endif
