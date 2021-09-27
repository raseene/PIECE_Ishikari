/****************************

		��{�V�X�e��

 ****************************/

#include "system.h"


BYTE	vram[SCREEN_W*SCREEN_H];			// ��ʃo�b�t�@

static int	fade_flag = 0;					// ��ʃt�F�[�h���
static int	fade_cnt = 0;					// ��ʂ̖��邳
static int	sys_bright;						// ���邳�ݒ�l

static int	snd_fade_flag = 0;				// �T�E���h�t�F�[�h���
static int	snd_fade_cnt  = 0;				// �T�E���h�t�F�[�h�J�E���^

int		common_counter;						// �ėp�J�E���^
int		master_volume;						// �S�̂̉���


/********************
    ��ʖ��邳�ݒ�
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
    ��ʃt�F�[�h�C��
		����	t = �t�F�[�h����
 **********************************/
void	fade_in(int t)
{
	fade_flag = (t == 0) ? 1000 : (1000 + t - 1)/t;
	set_bright();
}

/**********************************
    ��ʃt�F�[�h�A�E�g
		����	t = �t�F�[�h����
 **********************************/
void	fade_out(int t)
{
	fade_flag = (t == 0) ? -1000 : -(1000 + t - 1)/t;
	set_bright();
}


/**************************************
    BMP�ݒ�
		����	bmp = BMP�o�b�t�@
				pat = �p�^�[���f�[�^
 **************************************/
void	set_bmp(PIECE_BMP* bmp, const BYTE* pat)
{
	bmp->header = *(PBMP_FILEHEADER*)pat;							// �w�b�_
	bmp->buf    = (BYTE*)(pat + sizeof(PBMP_FILEHEADER));			// �p�^�[��
	bmp->mask   = bmp->buf + bmp->header.w*bmp->header.h/4;			// �}�X�N
}

/***************************************
    �X�v���C�g�ݒ�
		����	   spr = �X�v���C�g
				   bmp = �r�b�g�}�b�v
				sx, sy = �]�������W
				  w, h = �傫��
				 param = �p�����[�^
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
    �X�v���C�g�`��
		����	spr = �X�v���C�g
				x, y = �`����W
 **********************************/
void	draw_sprite(Sprite* spr, int x, int y)
{
	DRAW_OBJECT		obj;

	pceLCDSetObject(&obj, spr->bmp, x - spr->ox, y - spr->oy, spr->sx, spr->sy, spr->w, spr->h, spr->param);
	pceLCDDrawObject(obj);											// �`��
}


/*****************
    BGM���ʐݒ�
 *****************/
static
void	set_volume(void)
{
	snd_fade_cnt += snd_fade_flag;
	if ( snd_fade_cnt >= 1000 ) {
		StopMusic();								// BGM��~
		snd_fade_cnt  = 1000;
		snd_fade_flag = 0;
	}
	else {
		pceWaveSetChAtt(0, snd_fade_cnt*127/1000);
	}
}

BYTE const*		current_bgm = NULL;					// �Đ�����BGM

/*********************************
    BGM�Đ�
		����	bgm = BGM�f�[�^
 *********************************/
void	play_bgm(BYTE const* bgm)
{
	if ( bgm == current_bgm ) {						// ���łɍĐ���
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
    BGM��~
		����	t = ��~����
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
    ���ʉ��ݒ�
		����	info = ���ʉ����
				wave = ���ʉ��f�[�^
 *************************************/
void	set_wave(PCEWAVEINFO* info, const unsigned char* wave)
{
	memcpy(info, wave + 8, sizeof(PCEWAVEINFO));
	info->pData = wave + 8 + sizeof(PCEWAVEINFO);
}

/***********************************
    ���ʉ��Đ�
		����	info = ���ʉ����
 ***********************************/
void	play_se(PCEWAVEINFO* info)
{
	static int	n = 0;

	pceWaveDataOut(1 + n, info);
	n = ++n % 3;
}


/***************************************
    ������
		����	period = �t���[���Ԋu
 ***************************************/
void	init_system(int period)
{
	PCETIME	tm;

	pceLCDDispStop();												// ��ʕ\����~
	pceLCDSetBuffer(vram);											// ��ʃo�b�t�@�ݒ�
	sys_bright = pceLCDSetBright(0);								// ���邳�ݒ�l
	pceLCDDispStart();												// ��ʕ\���J�n

	InitMusic();													// ���y���C�u����������
	master_volume = pceWaveSetMasterAtt(INVALIDVAL);				// �S�̂̉���

	pceTimeGet(&tm);
	srand((int)tm.s100 + (int)tm.ss*100 + (int)tm.mi*100*60);		// ����������

	pceAppSetProcPeriod(period);									// �����ݒ�
}

/**********
    �I��
 **********/
void	exit_system(void)
{
	StopMusic();
	pceWaveStop(0);
	pceWaveSetMasterAtt(master_volume);								// ���ʂ�߂�
	pceLCDSetBright(sys_bright);									// ���邳��߂�
}

/************************************
    �ғ�
		����	cnt = �ғ��J�E���^
 ************************************/
void	update_system(int cnt)
{
	pceLCDTrans();													// ����ʓ]��
	if ( fade_flag ) {												// ��ʃt�F�[�h�A�E�g
		set_bright();
	}
	if ( snd_fade_flag ) {											// �T�E���h�t�F�[�h�A�E�g
		set_volume();
	}
	common_counter = cnt;											// �ėp�J�E���^
	rand();
}
