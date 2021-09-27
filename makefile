#
#	���˃p�Y���u�������� mini�v
#


.SUFFIXES:  .o .s .c


# �����R�}���h�E�I�v�V�����̃}�N��
CC = pcc33
CFLAGS = -c -g -gp=0x0 -near -O2 -Wall
AS = pcc33
ASFLAGS = -c -g -gp=0x0 -near
LD = pcc33
LDFLAGS = -g -ls -lm

# �����K��
.c.o:
	$(CC) $(CFLAGS) $<
.s.o:
	$(AS) $(ASFLAGS) $<

# �\���t�@�C���E�����t�@�C���̃}�N��
PRGNAME  = ishikari
FILENAME = ishikari
CAPTION  = "�������� mini"
OBJS     = main.o system.o menu.o game.o stage_data.o title.o pattern.o sound.o wavetbl.o

$(PRGNAME).srf : $(OBJS)
	$(LD) $(LDFLAGS) -e$(PRGNAME).srf $(OBJS)

# �t���b�V���������݃C���[�W����
pex : $(PRGNAME).srf
	ppack -e $(PRGNAME).srf -o$(FILENAME).pex -n$(CAPTION) -isprite\icon.pid

# �ˑ��֌W
main.o			: main.c system.h ishikari.h sound.h
system.o		: system.c system.h
menu.o			: menu.c system.h menu.h ishikari.h
game.o			: game.c system.h ishikari.h pattern.h menu.h sound.h str_def.h
stage_data.o	: stage_data.c system.h ishikari.h
title.o			: title.c system.h ishikari.h pattern.h menu.h sound.h str_def.h
pattern.o		: pattern.c pattern.h sprite\pat_back00.c sprite\pat_back01.c sprite\pat_back02.c sprite\pat_back03.c sprite\pat_back04.c sprite\pat_parts.c sprite\pat_message.c sprite\pat_title.c sprite\pat_stage.c
sound.o			: sound.c mml\c350.c mml\c379.c wave\wave_beep.c wave\wave_rot.c wave\wave_start.c wave\wave_crush.c
wavetbl.o		: wavetbl.c

# �N���[���A�b�v
clean:
	del $(PRGNAME).srf $(FILENAME).pex $(PRGNAME).sym $(PRGNAME).map *.o

