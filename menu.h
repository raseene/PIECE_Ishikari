#ifndef	___MENU_H___
#define	___MENU_H___

#include "system.h"


/**************
    メニュー
 **************/
typedef struct
{
	int		num;					// 項目数
	char const**	str;			// 項目名
	int		cursor;					// 選択番号
	int		width;					// 幅
	int		height;					// 高さ
	WORD	active_flag;			// アクティブフラグ
} Menu;


void	init_menu(Menu*, int, const char**);			// 初期化
void	set_menu_active(Menu*, int, BOOL);				// 項目アクティブ設定
int		update_menu(Menu*);								// 稼働
void	draw_menu(Menu*, int, int);						// 描画

#endif
