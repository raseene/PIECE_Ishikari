
//#include "musdef.h"

static const signed char square[35] = {
	0,
	0x7f,0x7f,0x7f,0x7f, 0x7f,0x7f,0x7f,0x7f,
	0x7f,0x7f,0x7f,0x7f, 0x7f,0x7f,0x7f,0x7f,
	0,
	-0x7f,-0x7f,-0x7f,-0x7f, -0x7f,-0x7f,-0x7f,-0x7f,
	-0x7f,-0x7f,-0x7f,-0x7f, -0x7f,-0x7f,-0x7f,-0x7f,
	0
};

static const signed char saw[17] = {
	0,
	0x1f,0x2f,0x3f,0x4f,0x5f,0x6f,0x7f,
	0
	-0x7f,-0x6f,-0x5f,-0x4f,-0x3f,-0x2f,-0x1f, 
	0
};

static const signed char triangle[17] = {
	0,
	-0x1f,-0x3f,-0x5f, -0x7f, -0x5f,-0x3f,-0x1f, 
	0,
	0x1f,0x3f,0x5f, 0x7f, 0x5f,0x3f,0x1f,
	0
};

#define PITCH2  (12*256)
#define PITCH4  (PITCH2*2)
#define PITCH8  (PITCH2*3)
#define PITCH16 (PITCH2*4)
#define PITCH32 (PITCH2*5)
#define PITCH34 15629

#define PITCHFS  PICTH16K
//#define PITCHORG PICTH8K

const INST i_square0 =   {IT_FAST,0,0,0,PITCH16K,PITCH16K-PITCH4, WAVEADR(0),        0,  4<<14};
const INST i_saw0 =      {IT_FAST,1,0,0,PITCH16K,PITCH16K-PITCH4, WAVEADR(0),        0,  4<<14};
const INST i_triangle0 = {IT_FAST,2,0,0,PITCH16K,PITCH16K-PITCH4, WAVEADR(0),        0,  4<<14};
const INST i_square =    {IT_LOOP,0,0,0,PITCH16K,PITCH16K-PITCH34,WAVEADR(square),   0, 34<<14};
const INST i_saw =       {IT_LOOP,0,0,0,PITCH16K,PITCH16K-PITCH16,WAVEADR(saw),      0, 16<<14};
const INST i_triangle =  {IT_LOOP,0,0,0,PITCH16K,PITCH16K-PITCH16,WAVEADR(triangle), 0, 16<<14};

