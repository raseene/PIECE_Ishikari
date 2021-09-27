/*****************************

		�Q�[�����C��

 *****************************/

#include "system.h"
#include "ishikari.h"
#include "pattern.h"
#include "menu.h"
#include "sound.h"
#include "str_def.h"


/*** �e�N�X�`�� *******/
enum
{
	BMP_BACK,
	BMP_PARTS,
	BMP_MESSAGE,

	BMP_MAX,
};

static PIECE_BMP	bmp_texture[BMP_MAX];			// �e�N�X�`��BMP


/*** �X�v���C�g *******/
enum
{
	SPR_BACK	= 0,					// �w�i

	SPR_STONE,							// ��
	SPR_CURSOR	= SPR_STONE + 6,		// �J�[�\��
	SPR_BEAM,							// �������ˈʒu
	SPR_BLUR,							// �����̋O��

	SPR_NUMBER	= SPR_BLUR + 3*8,		// ����
	SPR_STAGE	= SPR_NUMBER + 10,		// "STAGE"

	SPR_CLEAR,							// "���肠"
	SPR_FAILED,							// "�����ς�"

	SPR_MAX,
};

static Sprite	sprite[SPR_MAX];					// �X�v���C�g



/*** �� *******/
typedef struct
{
	int		dir;			// ����
	int		cnt;			// �����J�E���^
} Stone;

/*** ���� *******/
typedef struct
{
	int		x;				// �ʒu
	int		y;
	int		dir;			// ����
	int		cnt;			// �ړ��J�E���^
} Beam;

/*** �����̋O�� *******/
typedef struct
{
	int			x;			// �\�����W
	int			y;
	int			cnt;		// �\���J�E���^
	Sprite*		spr;		// �X�v���C�g
} Blur;

static Stone	stone[FIELD_H][FIELD_W];			// ��
static Beam		beam;								// ����
static Beam		beam_default;						// ���������l
static Blur		blur[8];							// �����̋O��
static int		field_x;							// �Ղ̈ʒu
static int		field_y;
static int		cursor_x;							// �J�[�\���ʒu
static int		cursor_y;
static int		stone_max;							// �΍ő吔
static int		stone_rest;							// �c��̐΂̐�

#define	FIELD_X(x, y)		field_x + (x)*STONE_W - (y)*(STONE_W/2)
#define	FIELD_Y(y)			field_y + (y)*STONE_H
#define	FIELD_POS(x, y)		FIELD_X(x, y), FIELD_Y(y)


/*** ���s�i�K *******/
enum
{
	PHASE_EDIT,					// �΂̌����ݒ�
	PHASE_MOVE,					// �����ړ�
	PHASE_CLEAR,				// �N���A
	PHASE_FAILED,				// ���s
};

static int		phase;			// ���s�i�K
static int		cnt;			// �ėp�J�E���^
static int		exit_cnt;		// �I���J�E���^
static int		exit_flag;		// �I����̃V�[��
static Menu		menu;			// ���j���[�Ǘ�


/**************
    �Տ�����
 **************/
static
void	init_field(void)
{
extern const BYTE	stage_data[][FIELD_H][FIELD_W - FIELD_H/2];				// �X�e�[�W�f�[�^

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

					if ( *p < 4 ) {								// ��
						stone[i][j].dir = (int)*p;
						stone_max++;
					}
					else if ( *p == 5 ) {						// �������ˈʒu
						beam_default.x = j;
						beam_default.y = i;
						beam_default.dir = 0;
						cursor_x = j;							// �J�[�\���ʒu
						cursor_y = i;
					}
				}
				p++;
			}
		}
	}

	field_x = 63 - (sx + ex)/2;									// �Ղ̈ʒu
	field_y = 44 - (sy + ey)/2 + (71 - ey)/4;
}

/****************
    �Սď�����
 ****************/
static
void	reset_field(void)
{
	int		i, j;

	for (i = 0; i < FIELD_H; i++) {
		for (j = 0; j < FIELD_W; j++) {
			if ( stone[i][j].dir >= 0 ) {						// ��
				stone[i][j].cnt = -1;
			}
		}
	}
	stone_rest = stone_max;										// �c��̐΂̐�
	beam = beam_default;										// ����
	for (i = 0; i < sizeof(blur)/sizeof(blur[0]); i++) {		// �����̋O��
		blur[i].cnt = 0;
	}

	play_bgm(seq_game);											// BGM�Đ�
	phase = PHASE_EDIT;											// ���s�i�K
}


/************
    ������
 ************/
void	init_game(void)
{
	static const
	unsigned char*	bmp_back[] =
	{
		PAT_BACK00, PAT_BACK01, PAT_BACK02, PAT_BACK03, PAT_BACK04,
	};

	set_bmp(&bmp_texture[BMP_BACK],		bmp_back[data.stage_num/10]);		// �w�i
	set_bmp(&bmp_texture[BMP_PARTS],	PAT_PARTS);							// �p�[�c
	set_bmp(&bmp_texture[BMP_MESSAGE],	PAT_MESSAGE);						// ���b�Z�[�W

	{
		static const
		BYTE	spr_data[SPR_MAX][5] =
		{
			{BMP_BACK,	 0,  0,128, 88},						// �w�i

			{BMP_PARTS,	 0,  0, 13, 13},						// ��
			{BMP_PARTS,	16,  0, 15, 17},
			{BMP_PARTS,	32,  0, 15, 17},
			{BMP_PARTS,	48,  0, 15, 17},
			{BMP_PARTS,	104, 0, 14, 14},
			{BMP_PARTS,	104,16, 18, 16},

			{BMP_PARTS,	64,  0, 20, 18},						// �J�[�\��
			{BMP_PARTS,	88,  0, 12, 12},						// �������ˈʒu

			{BMP_PARTS,	80, 20, 18,  2},						// �����̋O��
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

			{BMP_PARTS,	 0, 56,  6,  8},						// ����
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

			{BMP_MESSAGE,	0,  0, 52, 16},						// "���肠"
			{BMP_MESSAGE,	0, 18, 68, 16},						// "�����ς�"
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


	draw_sprite(&sprite[SPR_BACK], SCREEN_W/2, SCREEN_H/2);		// �w�i�`��
	memcpy(vram_buf, vram, SCREEN_W*SCREEN_H);					// ��ʑޔ�


	menu.num = -1;												// ���j���[
	exit_cnt = 0;												// �I���J�E���^

	init_field();												// �Տ�����
	reset_field();

	fade_in(12);												// �t�F�[�h�C��
}



/**********************************
    �΂̌����ݒ葀��
		�߂�l	�����𔭎˂��邩
 **********************************/
static
BOOL	edit(void)
{
	if ( pcePadGet() & TRG_LF ) {				// ��
		if ( cursor_x > cursor_y/2 ) {
			cursor_x--;
		}
	}
	else if ( pcePadGet() & TRG_RI ) {			// ��
		if ( cursor_x < FIELD_W - 1 - (FIELD_H - cursor_y + 1)/2 - field_x/STONE_W ) {
			cursor_x++;
		}
	}
	else if ( pcePadGet() & TRG_UP ) {			// ��
		if ( cursor_y > 0 ) {
			cursor_y--;
			cursor_x -= cursor_y % 2;
			if ( cursor_x < cursor_y/2 ) {
				cursor_x++;
			}
		}
	}
	else if ( pcePadGet() & TRG_DN ) {			// ��
		if ( cursor_y < FIELD_H - 1 - (field_y - 2)/STONE_H ) {
			cursor_x += cursor_y % 2;
			cursor_y++;
			if ( cursor_x > FIELD_W - (FIELD_H - cursor_y + 1)/2 - field_x/STONE_W ) {
				cursor_x--;
			}
		}
	}

	if ( pcePadGet() & TRG_A ) {				// �`
		Stone*	p = &stone[cursor_y][cursor_x];

		if ( (p->dir >= 1) && (p->dir <= 3) ) {					// �Ή�]
			p->dir = 1 + (p->dir % 3);
			play_se(&se[SE_ROT]);
		}
		else if ( p->dir < 0 ) {								// ��������
			return	TRUE;
		}
	}
	else if ( pcePadGet() & TRG_B ) {			// �a
		return	TRUE;											// ��������
	}
	return	FALSE;
}


/********************
    �����̋O�Րݒ�
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

	p->x = FIELD_X(beam.x, beam.y);								// �\�����W
	p->y = FIELD_Y(beam.y);
	p->spr = &sprite[SPR_BLUR + beam.dir*8];					// �X�v���C�g
	p->cnt = ((p->x - p->spr->ox < SCREEN_W) && (p->x - p->spr->ox + p->spr->w > 0) && (p->y - p->spr->oy < SCREEN_H) && (p->y - p->spr->oy + p->spr->h > 0)) ? 30 : 0;

	beam.cnt = 4;												// �����ړ��J�E���^
}

/***************************
    �����ړ�
		�߂�l	0�F���s
				1�F�N���A
				2�F���s
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
			if ( p->dir > 0 ) {									// ����
				beam.dir = (7 - (beam.dir + p->dir)) % 3;
			}
			p->cnt = 6;
			if ( --stone_rest == 0 ) {							// �N���A
				ret = 1;
			}
			else {												// ���s�`�F�b�N
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
	set_blur();													// �����̋O��

	return	ret;
}

/************
    �Ώ���
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
    �����̋O�Չғ�
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
    ���j���[�I�[�v��
		����	f = �G�f�B�b�g����
 ************************************/
static
void	open_menu(BOOL f)
{
	static const char*	item[] =								// ���j���[����
	{
		STR_RERTY,					// "  ���Ȃ���  "
		STR_NEXT_STAGE,				// " ���̃X�e�[�W "
		STR_SELECT_STAGE,			// " �X�e�[�W�I�� "
		STR_EXIT_GAME,				// "  �Q�[���I��  "
	};

	int		t = f ? 1 : 0;

	init_menu(&menu, 4 - t, &item[t]);
	set_menu_active(&menu, 1 - t, (data.stage_num + 1 < limit_stage));
	play_se(&se[SE_BEEP]);
}

/****************************
    �Q�[���ғ�
		�߂�l	�J�ڃV�[��
 ****************************/
static
int		update(void)
{
	if ( exit_cnt > 0 ) {										// �I����
		if ( --exit_cnt == 0 ) {
			switch ( exit_flag ) {
			  case SCENE_GAME :				// ���̃X�e�[�W
				data.stage_num++;
				return	SCENE_GAME;

			  case SCENE_TITLE :			// �X�e�[�W�I��
				return	SCENE_SELECT;
			}
		}
		return	-1;
	}
	if ( menu.num >= 0 ) {										// ���j���[
		int		n = update_menu(&menu);

		if ( n >= 0 ) {
			if ( (phase == PHASE_EDIT) && (n < 0xff) ) {
				n++;
			}
			switch ( n ) {
			  case 0xff :					// �L�����Z��
				menu.num = -1;
				if ( (phase == PHASE_CLEAR) || (phase == PHASE_FAILED) ) {
					reset_field();
				}
				play_se(&se[SE_START]);
				break;

			  case 0 :						// ���Ȃ���
				menu.num = -1;
				reset_field();
				play_se(&se[SE_BEEP]);
				break;

			  case 2 :						// �X�e�[�W�I��
				stop_bgm(10);
			  case 1 :						// ���̃X�e�[�W
				play_se(&se[SE_BEEP]);
				exit_flag = (n == 1) ? SCENE_GAME : SCENE_TITLE;
				exit_cnt = 12;
				fade_out(12);
				break;

			  case 3 :						// �Q�[���I��
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
	  case PHASE_EDIT :						// �΂̌����ݒ�
		if ( pcePadGet() & TRG_START ) {						// ���j���[
			open_menu(TRUE);
		}
		else if ( edit() ) {									// ����
			set_blur();
			play_se(&se[SE_START]);
			phase = PHASE_MOVE;
		}
		break;

	  case PHASE_MOVE :						// �����ړ�
		if ( pcePadGet() & TRG_A ) {
			play_se(&se[SE_ROT]);
			reset_field();										// ���Z�b�g
			break;
		}
		if ( pcePadGet() & PAD_B ) {							// �ꎞ��~
			break;
		}
		if ( pcePadGet() & TRG_START ) {						// ���j���[
			open_menu(FALSE);
			break;
		}

		update_blur();											// �����̋O�Չғ�
		update_stone();											// �Ώ���
		switch ( move_beam() ) {								// �����ړ�
		  case 1 :												// �N���A
			phase = PHASE_CLEAR;
			cnt = 45;
			clear_stage();
			break;

		  case 2 :												// ���s
			phase = PHASE_FAILED;
			cnt = 45;
			break;
		}
		break;

	  case PHASE_CLEAR :					// �N���A
		if ( cnt == 40 ) {
			stop_bgm(50);
		}
	  case PHASE_FAILED :					// ���s
		if ( pcePadGet() & PAD_B ) {							// �ꎞ��~
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
					reset_field();								// ���Z�b�g
					break;
				}
			}
			else {												// ���j���[
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

		update_blur();											// �����̋O�Չғ�
		update_stone();											// �Ώ���
		move_beam();											// �����ړ�
		break;
	}
	return	-1;
}



/**************
    ��ʕ`��
 **************/
static
void	draw_screen(void)
{
#if 0
	draw_sprite(&sprite[SPR_BACK], SCREEN_W/2, SCREEN_H/2);					// �w�i
#else
	memcpy(vram, vram_buf, SCREEN_W*SCREEN_H);								// �w�i
#endif

	switch ( phase ) {
	  case PHASE_MOVE :							// �����ړ�
	  case PHASE_CLEAR :						// �N���A
	  case PHASE_FAILED :						// ���s
		{																	// �����̋O��
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

	{																		// ��
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
	  case PHASE_EDIT :							// �΂̌����ݒ�
		draw_sprite(&sprite[SPR_BEAM], FIELD_POS(beam.x, beam.y));			// �������ˈʒu
		draw_sprite(&sprite[SPR_CURSOR], FIELD_POS(cursor_x, cursor_y));	// �J�[�\��
		break;
	}

	draw_sprite(&sprite[SPR_STAGE], 15, 5);									// "STAGE"
	{																		// �X�e�[�W�ԍ�
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
	  case PHASE_CLEAR :						// �N���A
		if ( cnt >= 0 ) {
			draw_sprite(&sprite[SPR_CLEAR], SCREEN_W/2, SCREEN_H/2 - cnt*cnt*cnt/120);						// "���肠"
		}
		break;

	  case PHASE_FAILED :						// ���s
		if ( cnt >= 0 ) {
			draw_sprite(&sprite[SPR_FAILED], SCREEN_W/2, SCREEN_H/2 + ((cnt - 4)*(cnt - 4) - 4*4)/3);		// "�����ς�"
		}
		break;
	}


	if ( menu.num >= 0 ) {										// ���j���[
		draw_menu(&menu, SCREEN_W/2, SCREEN_H/2);
	}
}


/****************************
    �ғ�
		�߂�l	�J�ڃV�[��
 ****************************/
int		update_game(void)
{
	int		ret = update();										// �Q�[���ғ�

	if ( ret >= 0 ) {
		return	ret;
	}
	draw_screen();												// ��ʕ`��
	return	-1;
}
