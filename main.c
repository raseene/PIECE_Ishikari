/**************************

		メイン

 **************************/

#include "system.h"
#include "ishikari.h"
#include "sound.h"


PCEWAVEINFO		se[SE_MAX];					// 効果音

SaveData	data;							// 共通データ
int			limit_stage;					// プレイ可能ステージ数

#define	SAVE_FILE	"ishikari.sav"			// セーブファイル名


BYTE	vram_buf[SCREEN_W*SCREEN_H];		// 画面退避バッファ


extern void		init_game(void);			// ゲーム初期化
extern int		update_game(void);			// ゲーム稼働

extern void		init_title(BOOL);			// タイトル初期化
extern int		update_title(void);			// タイトル稼働


/******************************
    プレイ可能ステージ数設定
 ******************************/
static
void	check_stage(void)
{
	int		t = 0;

	for (limit_stage = 0; limit_stage < STAGE_MAX; limit_stage++) {
		if ( data.clear_flag[limit_stage/32] & (1 << (limit_stage % 32)) ) {		// クリア済み
			t++;
		}
		else if ( t*10 < limit_stage*9 ) {
			break;
		}
	}
	if ( limit_stage < 5 ) {
		limit_stage = 5;
	}
}

/********************
    ステージクリア
 ********************/
void	clear_stage(void)
{
	if ( data.clear_flag[data.stage_num/32] & (1 << (data.stage_num % 32)) ) {
		return;								// クリア済み
	}
	data.clear_flag[data.stage_num/32] |= (1 << (data.stage_num % 32));

	check_stage();							// プレイ可能ステージ数設定
}


/************
    初期化
 ************/
void	pceAppInit(void)
{
	init_system(40);						// システム初期化


	data.stage_num = 0;						// ステージ番号
	{										// クリアフラグ
		int		i;

		for (i = 0; i < sizeof(data.clear_flag)/sizeof(data.clear_flag[0]); i++) {
			data.clear_flag[i] = 0;
		}
	}
	{
		FILEACC	fp;

		if ( pceFileOpen(&fp, SAVE_FILE, FOMD_RD) == 0 ) {					// セーブデータ読み込み
			pceFileReadSct(&fp, &data, 0, sizeof(SaveData));
			pceFileClose(&fp);
		}
	}
	check_stage();							// プレイ可能ステージ数設定

	{										// 効果音設定
		static const
		unsigned char*	wave_data[SE_MAX] =
		{
			WAVE_BEEP, WAVE_ROT, WAVE_START, WAVE_CRUSH,
		};
		int		i;

		for (i = 0; i < SE_MAX; i++) {
			set_wave(&se[i], wave_data[i]);
		}
	}
}

/**********
    終了
 **********/
void	pceAppExit(void)
{
	StopMusic();
	pceWaveStop(0);

	{
		FILEACC	fp;

		if ( (pceFileOpen(&fp, SAVE_FILE, FOMD_WR) == 0)					// セーブデータ書き込み
								|| ((pceFileCreate(SAVE_FILE, sizeof(SaveData)) == 0) && (pceFileOpen(&fp, SAVE_FILE, FOMD_WR) == 0)) ) {
			pceFileWriteSct(&fp, &data, 0, sizeof(SaveData));
			pceFileClose(&fp);
		}
	}

	exit_system();							// システム終了
}

/**********
    実行
 **********/
void	pceAppProc(int _cnt)
{
	static int	scene, next = SCENE_TITLE;
	static BOOL	sound_flag = TRUE;

	update_system(_cnt);					// システム稼働

	if ( pcePadGet() & TRG_SELECT ) {		// サウンド ON/OFF
		pceWaveSetMasterAtt((sound_flag = !sound_flag) ? master_volume : 127);
	}


	if ( next >= 0 ) {						// シーン初期化
		scene = next;
		switch ( next ) {
		  case SCENE_TITLE :		// タイトル
			init_title(TRUE);
			break;

		  case SCENE_SELECT :		// ステージ選択
			init_title(FALSE);
			break;

		  case SCENE_GAME :			// ゲーム
			init_game();
			break;
		}
	}
	switch ( scene ) {						// シーン稼働
	  case SCENE_TITLE :			// タイトル
	  case SCENE_SELECT :			// ステージ選択
		next = update_title();
		break;

	  case SCENE_GAME :				// ゲーム
		next = update_game();
		break;
	}
}
