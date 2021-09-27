#ifndef	___ISHIKARI_H___
#define	___ISHIKARI_H___

#include "system.h"


/**********
    ��`
 **********/

#define	STAGE_MAX	50			// �X�e�[�W��
#define	FIELD_W		11			// �Ղ̑傫��
#define	FIELD_H		 6
#define	STONE_W		16			// �΂̊Ԋu
#define	STONE_H		14


/************
    ���ʉ�
 ************/
enum
{
	SE_BEEP,
	SE_ROT,
	SE_START,
	SE_CRUSH,

	SE_MAX,
};

extern PCEWAVEINFO	se[SE_MAX];			// ���ʉ�


/******************
    �Z�[�u�f�[�^
 ******************/
typedef struct
{
	int		stage_num;								// �X�e�[�W�ԍ�
	DWORD	clear_flag[(STAGE_MAX + 31)/32];		// �N���A�t���O
} SaveData;

extern SaveData		data;				// ���ʃf�[�^
extern int			limit_stage;		// �v���C�\�X�e�[�W��

void	clear_stage(void);				// �X�e�[�W�N���A


extern BYTE		vram_buf[];				// ��ʑޔ��o�b�t�@


/*** �V�[�� *******/
enum
{
	SCENE_GAME,				// �Q�[��
	SCENE_TITLE,			// �^�C�g��
	SCENE_SELECT,			// �X�e�[�W�I��
};

#endif
