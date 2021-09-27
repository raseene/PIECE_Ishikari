#ifndef	___MENU_H___
#define	___MENU_H___

#include "system.h"


/**************
    ���j���[
 **************/
typedef struct
{
	int		num;					// ���ڐ�
	char const**	str;			// ���ږ�
	int		cursor;					// �I��ԍ�
	int		width;					// ��
	int		height;					// ����
	WORD	active_flag;			// �A�N�e�B�u�t���O
} Menu;


void	init_menu(Menu*, int, const char**);			// ������
void	set_menu_active(Menu*, int, BOOL);				// ���ڃA�N�e�B�u�ݒ�
int		update_menu(Menu*);								// �ғ�
void	draw_menu(Menu*, int, int);						// �`��

#endif
