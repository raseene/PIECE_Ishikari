/**************************

		���C��

 **************************/

#include "system.h"
#include "ishikari.h"
#include "sound.h"


PCEWAVEINFO		se[SE_MAX];					// ���ʉ�

SaveData	data;							// ���ʃf�[�^
int			limit_stage;					// �v���C�\�X�e�[�W��

#define	SAVE_FILE	"ishikari.sav"			// �Z�[�u�t�@�C����


BYTE	vram_buf[SCREEN_W*SCREEN_H];		// ��ʑޔ��o�b�t�@


extern void		init_game(void);			// �Q�[��������
extern int		update_game(void);			// �Q�[���ғ�

extern void		init_title(BOOL);			// �^�C�g��������
extern int		update_title(void);			// �^�C�g���ғ�


/******************************
    �v���C�\�X�e�[�W���ݒ�
 ******************************/
static
void	check_stage(void)
{
	int		t = 0;

	for (limit_stage = 0; limit_stage < STAGE_MAX; limit_stage++) {
		if ( data.clear_flag[limit_stage/32] & (1 << (limit_stage % 32)) ) {		// �N���A�ς�
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
    �X�e�[�W�N���A
 ********************/
void	clear_stage(void)
{
	if ( data.clear_flag[data.stage_num/32] & (1 << (data.stage_num % 32)) ) {
		return;								// �N���A�ς�
	}
	data.clear_flag[data.stage_num/32] |= (1 << (data.stage_num % 32));

	check_stage();							// �v���C�\�X�e�[�W���ݒ�
}


/************
    ������
 ************/
void	pceAppInit(void)
{
	init_system(40);						// �V�X�e��������


	data.stage_num = 0;						// �X�e�[�W�ԍ�
	{										// �N���A�t���O
		int		i;

		for (i = 0; i < sizeof(data.clear_flag)/sizeof(data.clear_flag[0]); i++) {
			data.clear_flag[i] = 0;
		}
	}
	{
		FILEACC	fp;

		if ( pceFileOpen(&fp, SAVE_FILE, FOMD_RD) == 0 ) {					// �Z�[�u�f�[�^�ǂݍ���
			pceFileReadSct(&fp, &data, 0, sizeof(SaveData));
			pceFileClose(&fp);
		}
	}
	check_stage();							// �v���C�\�X�e�[�W���ݒ�

	{										// ���ʉ��ݒ�
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
    �I��
 **********/
void	pceAppExit(void)
{
	StopMusic();
	pceWaveStop(0);

	{
		FILEACC	fp;

		if ( (pceFileOpen(&fp, SAVE_FILE, FOMD_WR) == 0)					// �Z�[�u�f�[�^��������
								|| ((pceFileCreate(SAVE_FILE, sizeof(SaveData)) == 0) && (pceFileOpen(&fp, SAVE_FILE, FOMD_WR) == 0)) ) {
			pceFileWriteSct(&fp, &data, 0, sizeof(SaveData));
			pceFileClose(&fp);
		}
	}

	exit_system();							// �V�X�e���I��
}

/**********
    ���s
 **********/
void	pceAppProc(int _cnt)
{
	static int	scene, next = SCENE_TITLE;
	static BOOL	sound_flag = TRUE;

	update_system(_cnt);					// �V�X�e���ғ�

	if ( pcePadGet() & TRG_SELECT ) {		// �T�E���h ON/OFF
		pceWaveSetMasterAtt((sound_flag = !sound_flag) ? master_volume : 127);
	}


	if ( next >= 0 ) {						// �V�[��������
		scene = next;
		switch ( next ) {
		  case SCENE_TITLE :		// �^�C�g��
			init_title(TRUE);
			break;

		  case SCENE_SELECT :		// �X�e�[�W�I��
			init_title(FALSE);
			break;

		  case SCENE_GAME :			// �Q�[��
			init_game();
			break;
		}
	}
	switch ( scene ) {						// �V�[���ғ�
	  case SCENE_TITLE :			// �^�C�g��
	  case SCENE_SELECT :			// �X�e�[�W�I��
		next = update_title();
		break;

	  case SCENE_GAME :				// �Q�[��
		next = update_game();
		break;
	}
}
