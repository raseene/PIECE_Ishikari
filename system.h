#ifndef	___SYSTEM_H___
#define	___SYSTEM_H___

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <piece.h>
#include <muslib.h>


/*** �X�v���C�g *******/
typedef struct
{
	PIECE_BMP*	bmp;			// �r�b�g�}�b�v�f�[�^
	int			sx;				// �]�������W
	int			sy;
	int			w;				// �傫��
	int			h;
	int			param;			// �`��p�����[�^
	int			ox;				// ���_
	int			oy;
} Sprite;


extern BYTE		vram[SCREEN_W*SCREEN_H];		// ��ʃo�b�t�@

extern int		common_counter;					// �ėp�J�E���^
extern int		master_volume;					// �S�̂̉���


#define	rnd(n)	(rand() % (n))					// �����擾


void	fade_in(int);							// ��ʃt�F�[�h�C��
void	fade_out(int);							// ��ʃt�F�[�h�A�E�g

void	set_bmp(PIECE_BMP*, const unsigned char*);						// BMP�ݒ�
void	set_sprite(Sprite*, PIECE_BMP*, int, int, int, int, int);		// �X�v���C�g�ݒ�
void	draw_sprite(Sprite*, int, int);									// �X�v���C�g�`��

void	play_bgm(BYTE const*);											// BGM�Đ�
void	stop_bgm(int);													// BGM��~
void	set_wave(PCEWAVEINFO*, const unsigned char*);					// ���ʉ��ݒ�
void	play_se(PCEWAVEINFO*);											// ���ʉ��Đ�

void	init_system(int);						// �V�X�e��������
void	exit_system(void);						// �V�X�e���I��
void	update_system(int);						// �V�X�e���ғ�

#endif
