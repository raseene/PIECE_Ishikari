/*******************************

		���j���[�Ǘ�

 *******************************/

#include "menu.h"
#include "ishikari.h"


/*******************************
    ������
		����	_num = ���ڐ�
				_str = ���ږ�
 *******************************/
void	init_menu(Menu* menu, int _num, const char** _str)
{
	menu->num		= _num;							// ���ڐ�
	menu->str		= _str;							// ���ږ�
	menu->cursor	= 0;
	menu->width		= strlen(_str[0])*5;			// ��
	menu->height	= _num*(10 + 3) - 3;			// ����
	menu->active_flag	= 0;						// �A�N�e�B�u�t���O
}

/**********************************
    ���ڃA�N�e�B�u�ݒ�
		����	n = ���ڔԍ�
				f = �A�N�e�B�u��
 **********************************/
void	set_menu_active(Menu* menu, int n, BOOL f)
{
	n = 1 << n;
	if ( f ) {
		menu->active_flag &= ~n;
	}
	else {
		menu->active_flag |= n;
	}
}

/**********************************
    �ғ�
		�߂�l	�I�����ڔԍ�
				0xff�F�L�����Z��
 **********************************/
int		update_menu(Menu* menu)
{
	if ( (pcePadGet() & TRG_UP) && (menu->cursor > 0) ) {						// ��
		menu->cursor--;
		play_se(&se[SE_BEEP]);
	}
	else if ( (pcePadGet() & TRG_DN) && (menu->cursor <menu->num - 1) ) {		// ��
		menu->cursor++;
		play_se(&se[SE_BEEP]);
	}
	if ( pcePadGet() & (TRG_B | TRG_START) ) {		// �L�����Z��
		return	0xff;
	}
	if ( pcePadGet() & TRG_A ) {					// ����
		if ( !(menu->active_flag & (1 << menu->cursor)) ) {
			return	menu->cursor;
		}
	}
	return	-1;
}

/*********************************
    �`��
		����	x, y = �`��ʒu
 *********************************/
void	draw_menu(Menu* menu, int x, int y)
{
	int		i, t;
													// �g
	pceLCDPaint(COLOR_GLAY_B, x - menu->width/2 - 6, y - menu->height/2 - 6, menu->width + 6*2, menu->height + 6*2);
	pceLCDPaint(COLOR_WHITE,  x - menu->width/2 - 4, y - menu->height/2 - 4, menu->width + 4*2, menu->height + 4*2);

	pceFontSetType(0);
	for (i = 0; i < menu->num; i++) {				// ����
		t = (i == menu->cursor) ? COLOR_WHITE : COLOR_BLACK;
		pceFontSetTxColor(!(menu->active_flag & (1 << i)) ? t : ((i == menu->cursor) ? COLOR_GLAY_B : COLOR_GLAY_W));
		pceFontSetBkColor(3 - t);
		pceFontSetPos(x - menu->width/2, y - menu->height/2 + i*(10 + 3));
		pceFontPutStr(menu->str[i]);
	}
}
