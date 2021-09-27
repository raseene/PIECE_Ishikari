#include <musdef.h>

#include "snd\\i_DEFSND.h"
//#include "snd\\i_HANDCLAP.h"
//#include "snd\\i_CYMBD.h"
#include "snd\\i_BD909.h"
#include "snd\\i_HC909.h"
#include "snd\\i_HO909.h"
#include "snd\\i_SD909.h"
//#include "snd\\i_SDGATE.h"
//#include "snd\\i_TOMH1.h"
//#include "snd\\i_TOML1.h"
//#include "snd\\i_TOMM1.h"

INST *inst[] = {
	(INST*)&i_square0,		// 0
	(INST*)&i_saw0,			// 1
	(INST*)&i_triangle0,	// 2
	(INST*)&i_square,		// 3
	(INST*)&i_saw,			// 4
	(INST*)&i_triangle,		// 5
	(INST*)&i_BD909,		// 6  bdr
	(INST*)0,				// 7  sdr
	(INST*)&i_SD909,		// 8  rim
	(INST*)&i_HO909,		// 9  ohh
	(INST*)&i_HC909,		// 10 chh
	(INST*)0,				// 11 ccy
	(INST*)0,				// 12 rcy
	(INST*)0,				// 13 htm
	(INST*)0,				// 14 mtm
	(INST*)0,				// 15 ltm
	(INST*)0,				// 16 hcp
};



