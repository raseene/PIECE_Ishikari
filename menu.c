/*******************************

		メニュー管理

 *******************************/

#include "menu.h"
#include "ishikari.h"


/*******************************
    初期化
		引数	_num = 項目数
				_str = 項目名
 *******************************/
void	init_menu(Menu* menu, int _num, const char** _str)
{
	menu->num		= _num;							// 項目数
	menu->str		= _str;							// 項目名
	menu->cursor	= 0;
	menu->width		= strlen(_str[0])*5;			// 幅
	menu->height	= _num*(10 + 3) - 3;			// 高さ
	menu->active_flag	= 0;						// アクティブフラグ
}

/**********************************
    項目アクティブ設定
		引数	n = 項目番号
				f = アクティブか
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
    稼働
		戻り値	選択項目番号
				0xff：キャンセル
 **********************************/
int		update_menu(Menu* menu)
{
	if ( (pcePadGet() & TRG_UP) && (menu->cursor > 0) ) {						// ↑
		menu->cursor--;
		play_se(&se[SE_BEEP]);
	}
	else if ( (pcePadGet() & TRG_DN) && (menu->cursor <menu->num - 1) ) {		// ↓
		menu->cursor++;
		play_se(&se[SE_BEEP]);
	}
	if ( pcePadGet() & (TRG_B | TRG_START) ) {		// キャンセル
		return	0xff;
	}
	if ( pcePadGet() & TRG_A ) {					// 決定
		if ( !(menu->active_flag & (1 << menu->cursor)) ) {
			return	menu->cursor;
		}
	}
	return	-1;
}

/*********************************
    描画
		引数	x, y = 描画位置
 *********************************/
void	draw_menu(Menu* menu, int x, int y)
{
	int		i, t;
													// 枠
	pceLCDPaint(COLOR_GLAY_B, x - menu->width/2 - 6, y - menu->height/2 - 6, menu->width + 6*2, menu->height + 6*2);
	pceLCDPaint(COLOR_WHITE,  x - menu->width/2 - 4, y - menu->height/2 - 4, menu->width + 4*2, menu->height + 4*2);

	pceFontSetType(0);
	for (i = 0; i < menu->num; i++) {				// 項目
		t = (i == menu->cursor) ? COLOR_WHITE : COLOR_BLACK;
		pceFontSetTxColor(!(menu->active_flag & (1 << i)) ? t : ((i == menu->cursor) ? COLOR_GLAY_B : COLOR_GLAY_W));
		pceFontSetBkColor(3 - t);
		pceFontSetPos(x - menu->width/2, y - menu->height/2 + i*(10 + 3));
		pceFontPutStr(menu->str[i]);
	}
}
