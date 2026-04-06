#include "NpcAct.h"

#include <stddef.h>

#include "WindowsWrapper.h"

#include "Bullet.h"
#include "Caret.h"
#include "CommonDefines.h"
#include "Frame.h"
#include "Flash.h"
#include "Game.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Sound.h"
#include "TextScr.h"
#include "Triangle.h"

// Toroko (frenzied)
void ActNpc140(NPCHAR *npc)
{
	int i;

	RECT rcLeft[14] = {
		{0, 0, 32, 32},
		{32, 0, 64, 32},
		{64, 0, 96, 32},
		{96, 0, 128, 32},
		{128, 0, 160, 32},
		{160, 0, 192, 32},
		{192, 0, 224, 32},
		{224, 0, 256, 32},
		{0, 64, 32, 96},
		{32, 64, 64, 96},
		{64, 64, 96, 96},
		{96, 64, 128, 96},
		{128, 64, 160, 96},
		{0, 0, 0, 0},
	};

	RECT rcRight[14] = {
		{0, 32, 32, 64},
		{32, 32, 64, 64},
		{64, 32, 96, 64},
		{96, 32, 128, 64},
		{128, 32, 160, 64},
		{160, 32, 192, 64},
		{192, 32, 224, 64},
		{224, 32, 256, 64},
		{0, 96, 32, 128},
		{32, 96, 64, 128},
		{64, 96, 96, 128},
		{96, 96, 128, 128},
		{128, 96, 160, 128},
		{0, 0, 0, 0},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->ani_no = 9;
			npc->act_wait = 0;
			npc->bits &= ~NPC_INTERACTABLE;
			// Fallthrough
		case 1:
			if (++npc->act_wait > 50)
			{
				npc->act_no = 2;
				npc->act_wait = 0;
				npc->ani_no = 8;
			}

			break;

		case 2:
			if (++npc->ani_no > 10)
				npc->ani_no = 9;

			if (++npc->act_wait > 50)
			{
				npc->act_no = 3;
				npc->act_wait = 0;
				npc->ani_no = 0;
			}

			break;

		case 3:
			if (++npc->act_wait > 50)
			{
				npc->act_no = 10;
				npc->bits |= NPC_SHOOTABLE;
			}

			break;

		case 10:
			npc->act_no = 11;
			npc->ani_no = 0;
			npc->ani_wait = 0;
			npc->act_wait = Random(20, 130);
			npc->xm = 0;
			// Fallthrough
		case 11:
			if (npc->x > gMC.x)
				npc->direct = 0;
			else
				npc->direct = 2;

			if (++npc->ani_wait > 4)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 1)
				npc->ani_no = 0;

			// [Mod] Igor jump-trigger updated.
			// Vanilla checked for Machine Gun (4).
			// Modded ASM (sub_494700) checks for Missiles (Weapon ID 6) or 
			// if more than 3 Fireballs (Weapon ID 3) are on screen.
			if (CountWeaponShotOccurrences(6) || CountWeaponShotOccurrences(3) > 3)
				npc->act_no = 20;

			if (npc->act_wait != 0)
			{
				--npc->act_wait;
			}
			else
			{
				if (Random(0, 99) % 2)
					npc->act_no = 20;
				else
					npc->act_no = 50;
			}

			break;

		case 20:
			npc->act_no = 21;
			npc->ani_no = 2;
			npc->act_wait = 0;
			// Fallthrough
		case 21:
			if (++npc->act_wait > 10)
			{
				npc->act_no = 22;
				npc->act_wait = 0;
				npc->ani_no = 3;
				npc->ym = -0x5FF;

				if (npc->direct == 0)
					npc->xm = -0x200;
				else
					npc->xm = 0x200;
			}

			break;

		case 22:
			if (++npc->act_wait > 10)
			{
				npc->act_no = 23;
				npc->act_wait = 0;
				npc->ani_no = 6;
				SetNpChar(141, 0, 0, 0, 0, 0, npc, 0);
			}

			break;

		case 23:
			if (++npc->act_wait > 30)
			{
				npc->act_no = 24;
				npc->act_wait = 0;
				npc->ani_no = 7;
			}

			if (npc->x > gMC.x)
				npc->direct = 0;
			else
				npc->direct = 2;

			break;

		case 24:
			if (++npc->act_wait > 3)
			{
				npc->act_no = 25;
				npc->ani_no = 3;
			}

			break;

		case 25:
			if (npc->flag & 8)
			{
				npc->act_no = 26;
				npc->act_wait = 0;
				npc->ani_no = 2;
				PlaySoundObject(26, SOUND_MODE_PLAY);
				SetQuake(20);
			}

			break;

		case 26:
			npc->xm = (npc->xm * 8) / 9;

			if (++npc->act_wait > 20)
			{
				npc->act_no = 10;
				npc->ani_no = 0;
			}

			break;

		case 50:
			npc->act_no = 51;
			npc->act_wait = 0;
			npc->ani_no = 4;
			SetNpChar(141, 0, 0, 0, 0, 0, npc, 0);
			// Fallthrough
		case 51:
			if (++npc->act_wait > 30)
			{
				npc->act_no = 52;
				npc->act_wait = 0;
				npc->ani_no = 5;
			}

			if (npc->x > gMC.x)
				npc->direct = 0;
			else
				npc->direct = 2;

			break;

		case 52:
			if (++npc->act_wait > 3)
			{
				npc->act_no = 10;
				npc->ani_no = 0;
			}

			break;

		case 100:
			npc->ani_no = 3;
			npc->act_no = 101;
			npc->bits &= ~NPC_SHOOTABLE;
			npc->damage = 0;

			for (i = 0; i < 8; ++i)
				SetNpChar(4, npc->x + (Random(-12, 12) * 0x200), npc->y + (Random(-12, 12) * 0x200), Random(-341, 341), Random(-0x600, 0), 0, NULL, 0x100);

			break;

		case 101:
			if (npc->flag & 8)
			{
				npc->act_no = 102;
				npc->act_wait = 0;
				npc->ani_no = 2;
				PlaySoundObject(26, SOUND_MODE_PLAY);
				SetQuake(20);
			}

			break;

		case 102:
			npc->xm = (npc->xm * 8) / 9;

			if (++npc->act_wait > 50)
			{
				npc->act_no = 103;
				npc->act_wait = 0;
				npc->ani_no = 10;
			}

			break;

		case 103:
			if (++npc->act_wait > 50)
			{
				npc->ani_no = 9;
				npc->act_no = 104;
				npc->act_wait = 0;
			}

			break;

		case 104:
			if (++npc->ani_no > 10)
				npc->ani_no = 9;

			if (++npc->act_wait > 100)
			{
				npc->act_wait = 0;
				npc->ani_no = 9;
				npc->act_no = 105;
			}

			break;

		case 105:
			if (++npc->act_wait > 50)
			{
				npc->ani_wait = 0;
				npc->act_no = 106;
				npc->ani_no = 11;
			}

			break;

		case 106:
			if (++npc->ani_wait > 50)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 12)
				npc->ani_no = 12;

			break;

		case 140:
			npc->act_no = 141;
			npc->act_wait = 0;
			npc->ani_no = 12;
			PlaySoundObject(29, SOUND_MODE_PLAY);
			// Fallthrough
		case 141:
			if (++npc->ani_no > 13)
				npc->ani_no = 12;

			if (++npc->act_wait > 100)
			{
				for (i = 0; i < 4; ++i)
					SetNpChar(4, npc->x + (Random(-12, 12) * 0x200), npc->y + (Random(-12, 12) * 0x200), Random(-341, 341), Random(-0x600, 0), 0, NULL, 0x100);

				npc->cond = 0;
			}

			break;
	}

	if (npc->act_no > 100 && npc->act_no < 105 && npc->act_wait % 9 == 0)
		SetNpChar(4, npc->x + (Random(-12, 12) * 0x200), npc->y + (Random(-12, 12) * 0x200), Random(-341, 341), Random(-0x600, 0), 0, NULL, 0x100);

	npc->ym += 0x20;

	if (npc->ym > 0x5FF)
		npc->ym = 0x5FF;
	if (npc->ym < -0x5FF)
		npc->ym = -0x5FF;

	npc->x += npc->xm;
	npc->y += npc->ym;

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Toroko block projectile
void ActNpc141(NPCHAR *npc)
{
	int i;
	unsigned char deg;

	RECT rect[2] = {
		{288, 32, 304, 48},
		{304, 32, 320, 48},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->act_wait = 0;
			// Fallthrough
		case 1:
			if (npc->pNpc->direct == 0)
				npc->x = npc->pNpc->x + (10 * 0x200);
			else
				npc->x = npc->pNpc->x - (10 * 0x200);

			npc->y = npc->pNpc->y - (8 * 0x200);

			if (npc->pNpc->act_no == 24 || npc->pNpc->act_no == 52)
			{
				npc->act_no = 10;

				if (npc->pNpc->direct == 0)
					npc->x = npc->pNpc->x - (16 * 0x200);
				else
					npc->x = npc->pNpc->x + (16 * 0x200);

				npc->y = npc->pNpc->y;

				deg = GetArktan(npc->x - gMC.x, npc->y - gMC.y);
				npc->ym = GetSin(deg) * 4;
				npc->xm = GetCos(deg) * 4;

				PlaySoundObject(39, SOUND_MODE_PLAY);
			}

			break;

		case 10:
			if (npc->flag & 0xF)
			{
				npc->act_no = 20;
				npc->act_wait = 0;
				SetCaret(npc->x, npc->y, 2, 0);
				PlaySoundObject(12, SOUND_MODE_PLAY);

				for (i = 0; i < 4; ++i)
					SetNpChar(4, npc->x, npc->y, Random(-0x200, 0x200), Random(-0x200, 0x200), 0, NULL, 0x100);

				break;
			}

			npc->x += npc->xm;
			npc->y += npc->ym;

			break;

		case 20:
			npc->x += npc->xm;
			npc->y += npc->ym;

			if (++npc->act_wait > 4)
			{
				for (i = 0; i < 4; ++i)
					SetNpChar(4, npc->x, npc->y, Random(-0x200, 0x200), Random(-0x200, 0x200), 0, NULL, 0x100);

				npc->code_char = 142;
				npc->ani_no = 0;
				npc->act_no = 20;
				npc->xm = 0;
				npc->bits &= ~NPC_INVULNERABLE;
				npc->bits |= NPC_SHOOTABLE;
				npc->damage = 1;
			}

			break;
	}

	if (++npc->ani_no > 1)
		npc->ani_no = 0;

	npc->rect = rect[npc->ani_no];
}

// Flower Cub
void ActNpc142(NPCHAR *npc)
{
	RECT rect[5] = {
		{0, 128, 16, 144},
		{16, 128, 32, 144},
		{32, 128, 48, 144},
		{48, 128, 64, 144},
		{64, 128, 80, 144},
	};

	switch (npc->act_no)
	{
		case 10:
			npc->act_no = 11;
			npc->ani_no = 0;
			npc->act_wait = 0;
			// Fallthrough
		case 11:
			if (++npc->act_wait > 30)
			{
				npc->act_no = 12;
				npc->ani_no = 1;
				npc->ani_wait = 0;
			}

			break;

		case 12:
			if (++npc->ani_wait > 8)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no == 3)
			{
				npc->act_no = 20;
				npc->ym = -0x200;

				if (gMC.x < npc->x)
					npc->xm = -0x200;
				else
					npc->xm = 0x200;
			}

			break;

		case 20:
			if (npc->ym > -0x80)
				npc->ani_no = 4;
			else
				npc->ani_no = 3;

			if (npc->flag & 8)
			{
				npc->ani_no = 2;
				npc->act_no = 21;
				npc->act_wait = 0;
				npc->xm = 0;
				PlaySoundObject(23, SOUND_MODE_PLAY);
			}

			break;

		case 21:
			if (++npc->act_wait > 10)
			{
				npc->act_no = 10;
				npc->ani_no = 0;
			}

			break;
	}

	npc->ym += 0x40;

	if (npc->ym > 0x5FF)
		npc->ym = 0x5FF;
	if (npc->ym < -0x5FF)
		npc->ym = -0x5FF;

	npc->x += npc->xm;
	npc->y += npc->ym;

	npc->rect = rect[npc->ani_no];
}

// Jenka (collapsed)
void ActNpc143(NPCHAR *npc)
{
	RECT rcLeft[1] = {
		{208, 32, 224, 48}
	};

	RECT rcRight[1] = {
		{208, 48, 224, 64}
	};

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Toroko (teleporting in)
void ActNpc144(NPCHAR *npc)
{
	RECT rcLeft[5] = {
		{0, 64, 16, 80},
		{16, 64, 32, 80},
		{32, 64, 48, 80},
		{16, 64, 32, 80},
		{128, 64, 144, 80},
	};

	RECT rcRight[5] = {
		{0, 80, 16, 96},
		{16, 80, 32, 96},
		{32, 80, 48, 96},
		{16, 80, 32, 96},
		{128, 80, 144, 96},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->ani_no = 0;
			npc->ani_wait = 0;
			npc->tgt_x = npc->x;
			PlaySoundObject(29, SOUND_MODE_PLAY);
			// Fallthrough
		case 1:
			if (++npc->act_wait == 64)
			{
				npc->act_no = 2;
				npc->act_wait = 0;
			}

			break;

		case 2:
			if (++npc->ani_wait > 2)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 3)
				npc->ani_no = 2;

			if (npc->flag & 8)
			{
				npc->act_no = 4;
				npc->act_wait = 0;
				npc->ani_no = 4;
				PlaySoundObject(23, SOUND_MODE_PLAY);
			}

			break;

		case 10:
			npc->act_no = 11;
			npc->ani_no = 0;
			npc->ani_wait = 0;
			// Fallthrough
		case 11:
			if (Random(0, 120) == 10)
			{
				npc->act_no = 12;
				npc->act_wait = 0;
				npc->ani_no = 1;
			}

			break;

		case 12:
			if (++npc->act_wait > 8)
			{
				npc->act_no = 11;
				npc->ani_no = 0;
			}

			break;
	}

	if (npc->act_no > 1)
	{
		npc->ym += 0x20;
		if (npc->ym > 0x5FF)
			npc->ym = 0x5FF;

		npc->y += npc->ym;
	}

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];

	if (npc->act_no == 1)
	{
		npc->rect.bottom = npc->rect.top + npc->act_wait / 4;

		if (npc->act_wait / 2 % 2)
			npc->x = npc->tgt_x;
		else
			npc->x = npc->tgt_x + (1 * 0x200);
	}
}

// King's sword
void ActNpc145(NPCHAR *npc)
{
	RECT rcLeft[1] = {
		{96, 32, 112, 48}
	};

	RECT rcRight[1] = {
		{112, 32, 128, 48}
	};

	switch (npc->act_no)
	{
		case 0:
			if (npc->pNpc->count2 == 0)
			{
				if (npc->pNpc->direct == 0)
					npc->direct = 0;
				else
					npc->direct = 2;
			}
			else
			{
				if (npc->pNpc->direct == 0)
					npc->direct = 2;
				else
					npc->direct = 0;
			}

			if (npc->direct == 0)
				npc->x = npc->pNpc->x - (10 * 0x200);
			else
				npc->x = npc->pNpc->x + (10 * 0x200);

			npc->y = npc->pNpc->y;

			break;
	}

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Lightning
void ActNpc146(NPCHAR *npc)
{
	RECT rect[5] = {
		{0, 0, 0, 0},
		{256, 0, 272, 240},
		{272, 0, 288, 240},
		{288, 0, 304, 240},
		{304, 0, 320, 240},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;

			if (npc->direct == 2)
				SetFlash(0, 0, 2);
			// Fallthrough
		case 1:
			if (++npc->act_wait > 10)
			{
				npc->act_no = 2;
				PlaySoundObject(101, SOUND_MODE_PLAY);
			}

			break;

		case 2:
			if (++npc->ani_wait > 2)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no == 2)
				npc->damage = 10;

			if (npc->ani_no > 4)
			{
				SetDestroyNpChar(npc->x, npc->y, 0x1000, 8);
				npc->cond = 0;
			#ifdef FIX_BUGS
				return;	// The code below will use 'ani_no' to access 'rect', even though it's now too high
			#endif
			}

			break;
	}

	npc->rect = rect[npc->ani_no];
}

// Critter (purple)
void ActNpc147(NPCHAR *npc)
{
	int xm, ym;
	unsigned char deg;

	// MOD: RECT table shifted to new sprite sheet rows (top row 208-224, bottom 224-240)
	RECT rcLeft[6] = {
		{ 0, 208, 16, 224},
		{16, 208, 32, 224},
		{32, 208, 48, 224},
		{48, 208, 64, 224},
		{64, 208, 80, 224},
		{80, 208, 96, 224},
	};

	RECT rcRight[6] = {
		{ 0, 224, 16, 240},
		{16, 224, 32, 240},
		{32, 224, 48, 240},
		{48, 224, 64, 240},
		{64, 224, 80, 240},
		{80, 224, 96, 240},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->y += 3 * 0x200;
			npc->act_no = 1;
			// Fallthrough
		case 1:
			if (npc->act_wait >= 8 && npc->x - (96 * 0x200) < gMC.x && npc->x + (96 * 0x200) > gMC.x && npc->y - (96 * 0x200) < gMC.y && npc->y + (32 * 0x200) > gMC.y)
			{
				if (npc->x > gMC.x)
					npc->direct = 0;
				else
					npc->direct = 2;

				npc->ani_no = 1;
			}
			else
			{
				if (npc->act_wait < 8)
					++npc->act_wait;

				npc->ani_no = 0;
			}

			if (npc->shock)
			{
				npc->act_no = 2;
				npc->ani_no = 0;
				npc->act_wait = 0;
			}

			if (npc->act_wait >= 8 && npc->x - (48 * 0x200) < gMC.x && npc->x + (48 * 0x200) > gMC.x && npc->y - (96 * 0x200) < gMC.y && npc->y + (32 * 0x200) > gMC.y)
			{
				npc->act_no = 2;
				npc->ani_no = 0;
				npc->act_wait = 0;
			}

			break;

		case 2:
			if (++npc->act_wait > 8)
			{
				npc->act_no = 3;
				npc->ani_no = 2;
				npc->ym = -0x5FF;
				PlaySoundObject(30, SOUND_MODE_PLAY);

				if (npc->x > gMC.x)
					npc->direct = 0;
				else
					npc->direct = 2;
			}

			break;

		case 3:
			if (npc->ym > 0x100)
			{
				npc->tgt_y = npc->y;
				npc->act_no = 4;
				npc->ani_no = 3;
				npc->act_wait = 0;
				npc->act_wait = 0;	// Duplicate line
			}

			break;

		case 4:
			if (npc->x < gMC.x)
				npc->direct = 2;
			else
				npc->direct = 0;

			++npc->act_wait;

			if (npc->flag & 7 || npc->act_wait > 60)
			{
				npc->damage = 3;
				npc->act_no = 5;
				npc->ani_no = 2;
				break;
			}

			if (npc->act_wait % 4 == 1)
				PlaySoundObject(109, SOUND_MODE_PLAY);

			if (npc->flag & 8)
				npc->ym = -0x200;

			if (npc->act_wait % 30 == 6)
			{
				deg = GetArktan(npc->x - gMC.x, npc->y - gMC.y);
				deg += (unsigned char)Random(-6, 6);
				ym = GetSin(deg) * 3;
				xm = GetCos(deg) * 3;

				SetNpChar(148, npc->x, npc->y, xm, ym, 0, NULL, 0x100);
				PlaySoundObject(39, SOUND_MODE_PLAY);
			}

			if (++npc->ani_wait > 0)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 5)
				npc->ani_no = 3;

			break;

		case 5:
			if (npc->flag & 8)
			{
				npc->damage = 2;
				npc->xm = 0;
				npc->act_wait = 0;
				npc->ani_no = 0;
				npc->act_no = 1;
				PlaySoundObject(23, SOUND_MODE_PLAY);
			}

			break;
	}

	if (npc->act_no != 4)
	{
		npc->ym += 0x20;
		if (npc->ym > 0x5FF)
			npc->ym = 0x5FF;
	}
	else
	{
		if (npc->y > npc->tgt_y)
			npc->ym -= 0x10;
		else
			npc->ym += 0x10;

		if (npc->ym > 0x200)
			npc->ym = 0x200;
		if (npc->ym < -0x200)
			npc->ym = -0x200;

		if (npc->xm > 0x200)
			npc->xm = 0x200;
		if (npc->xm < -0x200)
			npc->xm = -0x200;
	}

	npc->x += npc->xm;
	npc->y += npc->ym;

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Purple Critter's projectile
void ActNpc148(NPCHAR *npc)
{
	if (npc->flag & 0xFF)
	{
		SetCaret(npc->x, npc->y, 2, 0);
		npc->cond = 0;
	}

	npc->y += npc->ym;
	npc->x += npc->xm;

	// MOD: Sprite sheet rows shifted (top 96→208, bottom 104→216)
	RECT rect_left[2] = {
		{ 96, 208, 104, 216},
		{104, 208, 112, 216},
	};

	if (++npc->ani_no > 1)
		npc->ani_no = 0;

	npc->rect = rect_left[npc->ani_no];

	if (++npc->count1 > 300)
	{
		SetCaret(npc->x, npc->y, 2, 0);
		npc->cond = 0;
	}
}


#include "MycHit.h"

// Moving block (horizontal)
// Large Moving Block / Press (Redesign Version)
void ActNpc149(NPCHAR *npc)
{
	int i;
	int max_xm;

	switch (npc->act_no)
	{
		case 0:
			// Initialize position and solid bits
			npc->x += 8 * 0x200;
			npc->y += 8 * 0x200;
			npc->bits |= NPC_SOLID_HARD;

			// [Mod] Check bit 0x100 for "Carried/Dormant" state
			if (npc->bits & 0x100)
			{
				npc->act_no = 1;
				npc->ani_no = 3;
			}
			else
			{
				npc->ani_no = 1;
				npc->act_no = (npc->direct == 0) ? 10 : 20;
			}

			npc->xm = 0;
			npc->ym = 0;
			break;

		case 1:
			// Being carried/stationary - wait for specific collision trigger
			if (JudgeHitMyCharNPC4(npc))
			{
				npc->act_no = 2;
				npc->ani_no = 2;
				npc->count1 = 0;
				SetQuake(30);
			}
			break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			// "Break free" animation sequence
			if (++npc->count1 > 5)
			{
				npc->count1 = 0;
				++npc->act_no;
				PlaySoundObject(111, SOUND_MODE_PLAY);
			}
			break;

		case 8:
			// Sequence complete, start moving
			npc->ani_no = 1;
			npc->count1 = 0;
			npc->act_no = (npc->direct == 0) ? 10 : 20;
			break;

		case 10:
			// Waiting to move Left
			npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
			npc->damage = 0;

			// Proximity detection box
			if (gMC.x < npc->x + (25 * 0x200) && 
			    gMC.x > npc->x - (400 * 0x200) && 
			    gMC.y < npc->y + (25 * 0x200) && 
			    gMC.y > npc->y - (25 * 0x200))
			{
				npc->act_no = 11;
				npc->act_wait = 0;
			}
			break;

		case 11:
			// Moving Left
			if (++npc->act_wait % 10 == 6)
				PlaySoundObject(107, SOUND_MODE_PLAY);

			if (npc->flag & 1) // Hit left wall
			{
				npc->xm = 0;
				npc->direct = 2;
				npc->act_no = 20;
				SetQuake(10);
				PlaySoundObject(26, SOUND_MODE_PLAY);

				for (i = 0; i < 4; ++i)
					SetNpChar(4, npc->x - 0x2000, npc->y + (Random(-12, 12) << 9), Random(-341, 341), Random(-0x600, 0), 0, NULL, 0x100);
			}
			else
			{
				// [Mod] Crush logic: Deal 100 damage if player is touching the left side
				if (gMC.flag & 1)
				{
					npc->bits |= NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 100;
				}
				else
				{
					npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 0;
				}
				npc->xm -= 32;
			}
			break;

		case 20:
			// Waiting to move Right
			npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
			npc->damage = 0;

			// Proximity detection box
			if (gMC.x > npc->x - (25 * 0x200) && 
			    gMC.x < npc->x + (400 * 0x200) && 
			    gMC.y < npc->y + (25 * 0x200) && 
			    gMC.y > npc->y - (25 * 0x200))
			{
				npc->act_no = 21;
				npc->act_wait = 0;
			}
			break;

		case 21:
			// Moving Right
			if (++npc->act_wait % 10 == 6)
				PlaySoundObject(107, SOUND_MODE_PLAY);

			if (npc->flag & 4) // Hit right wall
			{
				npc->xm = 0;
				npc->direct = 0;
				npc->act_no = 10;
				SetQuake(10);
				PlaySoundObject(26, SOUND_MODE_PLAY);

				for (i = 0; i < 4; ++i)
					SetNpChar(4, npc->x + 0x2000, npc->y + (Random(-12, 12) << 9), Random(-341, 341), Random(-0x600, 0), 0, NULL, 0x100);
			}
			else
			{
				// [Mod] Crush logic: Deal 100 damage if player is touching the right side
				if (gMC.flag & 4)
				{
					npc->bits |= NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 100;
				}
				else
				{
					npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 0;
				}
				npc->xm += 32;
			}
			break;
	}

	// [Mod] Speed clamping logic from end of IDA output
	max_xm = (npc->bits & 0x400) ? 0x800 : 0x200;

	if (npc->xm > max_xm) npc->xm = max_xm;
	if (npc->xm < -max_xm) npc->xm = -max_xm;

	npc->x += npc->xm;

	// [Mod] Custom RECT calculation from IDA output
	// left = 32 * (ani_no - 1) + 48
	int left_calc = 32 * (npc->ani_no - 1) + 48;
	npc->rect.left = left_calc;
	npc->rect.top = 208;
	npc->rect.right = left_calc + 32;
	npc->rect.bottom = 240;
}

// Quote (NPC form)
void ActNpc150(NPCHAR *npc)
{
	int i;

	static const RECT rcLeft[9] = {
		{0, 0, 16, 16},    // 0: Idle
		{48, 0, 64, 16},   // 1: Pain/Shock
		{144, 0, 160, 16}, // 2: Teleporting
		{16, 0, 32, 16},   // 3: Walk 1
		{0, 0, 16, 16},    // 4: Walk 2
		{32, 0, 48, 16},   // 5: Walk 3
		{0, 0, 16, 16},    // 6: Walk 4
		{160, 0, 176, 16}, // 7: Falling
		{112, 0, 128, 16}, // 8: Lying down
	};

	static const RECT rcRight[9] = {
		{0, 16, 16, 32},
		{48, 16, 64, 32},
		{144, 16, 160, 32},
		{16, 16, 32, 32},
		{0, 16, 16, 32},
		{32, 16, 48, 32},
		{0, 16, 16, 32},
		{160, 16, 176, 32},
		{112, 16, 128, 32},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->ani_no = 0;

			if (npc->direct > 10)
			{
				npc->x = gMC.x;
				npc->y = gMC.y;
				npc->direct -= 10;
			}
			break;

		case 2:
			npc->ani_no = 1;
			break;

		case 10:
			npc->act_no = 11;

			for (i = 0; i < 4; ++i)
				SetNpChar(4, npc->x, npc->y, Random(-0x155, 0x155), Random(-0x600, 0), 0, NULL, 0x100);

			PlaySoundObject(71, SOUND_MODE_PLAY);
			// Fallthrough
		case 11:
			npc->ani_no = 2;
			break;

		case 20:
			npc->act_no = 21;
			npc->act_wait = 64;
			PlaySoundObject(29, SOUND_MODE_PLAY);
			// Fallthrough
		case 21:
			if (--npc->act_wait == 0)
				npc->cond = 0;

			break;

		case 50:
			npc->act_no = 51;
			npc->ani_no = 3;
			npc->ani_wait = 0;
			// Fallthrough
		case 51:
			if (++npc->ani_wait > 4)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 6)
				npc->ani_no = 3;

			if (npc->direct == 0)
				npc->x -= 0x200;
			else
				npc->x += 0x200;

			break;

		case 60:
			npc->act_no = 61;
			npc->ani_no = 7;
			npc->tgt_x = npc->x;
			npc->tgt_y = npc->y;
			// Fallthrough
		case 61:
			// [MOD] Speed increased from 0x100 (0.5px) to 0x800 (4px)
			npc->tgt_y += 0x800; 
			npc->x = npc->tgt_x + (Random(-1, 1) * 0x200);
			npc->y = npc->tgt_y + (Random(-1, 1) * 0x200);
			break;

		case 70:
			npc->act_no = 71;
			npc->act_wait = 0;
			npc->ani_no = 3;
			npc->ani_wait = 0;
			// Fallthrough
		case 71:
			if (npc->direct == 0)
				npc->x += 0x100;
			else
				npc->x -= 0x100;

			if (++npc->ani_wait > 8)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 6)
				npc->ani_no = 3;

			break;

		case 80:
			npc->ani_no = 8;
			break;

		case 99:
		case 100:
			npc->act_no = 101;
			npc->ani_no = 3;
			npc->ani_wait = 0;
			// Fallthrough
		case 101:
			npc->ym += 0x40;

			if (npc->ym > 0x5FF)
				npc->ym = 0x5FF;

			if (npc->flag & 8)
			{
				npc->ym = 0;
				npc->act_no = 102;
			}

			npc->y += npc->ym;
			break;

		case 102:
			if (++npc->ani_wait > 8)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 6)
				npc->ani_no = 3;

			break;
	}

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];

	// Handle the dissolving effect (State 21)
	if (npc->act_no == 21)
	{
		npc->rect.bottom = npc->rect.top + (npc->act_wait / 4);

		// [MOD] New horizontal jitter effect added during dissolve
		if ((npc->act_wait / 2) % 2)
			npc->rect.left += 1;
	}

	// Use a different sprite set if wearing the Mimiga Mask
	if (gMC.equip & EQUIP_MIMIGA_MASK)
	{
		npc->rect.top += 32;
		npc->rect.bottom += 32;
	}
}

// Blue robot (standing)
void ActNpc151(NPCHAR *npc)
{
	RECT rcLeft[2] = {
		{192, 0, 208, 16},
		{208, 0, 224, 16},
	};

	RECT rcRight[2] = {
		{192, 16, 208, 32},
		{208, 16, 224, 32},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->ani_no = 0;
			npc->ani_wait = 0;
			// Fallthrough

		case 1:
			if (Random(0, 100) == 0)
			{
				npc->act_no = 2;
				npc->act_wait = 0;
				npc->ani_no = 1;
			}

			break;

		case 2:
			if (++npc->act_wait > 16)
			{
				npc->act_no = 1;
				npc->ani_no = 0;
			}

			break;
	}

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

#include "Flags.h"


void ActNpc152(NPCHAR *npc)
{
	int rect_top = 208;
	int rect_bottom = 224;
	int rect_left, rect_right;

	// Act 0: Initialization
	// Maps 'Event #' from the editor to specific internal behavior modes
	if (npc->act_no == 0)
	{
		int mode = 1;
		switch (npc->code_event)
		{
			case 1: mode = 3; break;
			case 2: mode = 4; break;
			case 3: mode = 5; break;
			case 4: mode = 6; break;
			case 5: mode = 7; break;
		}
		npc->act_no = mode;
	}

	// Trigger Logic
	// This block only runs if the NPC is not in its 70-frame cooldown (act_wait)
	if (npc->act_wait == 0)
	{
		bool triggered = false;
		int signal = npc->count2; // Signal usually passed from HitNpCharBullet

		if (signal != 0)
		{
			// Mode-based filtering of the signal
			if (npc->act_no == 3)
			{
				triggered = true; // Mode 1: Triggered by any signal
			}
			else if (npc->act_no == 4 && (signal == 13 || signal == 14 || signal == 15))
			{
				triggered = true; // Mode 2: Triggered only by specific Bullet IDs (e.g., Level 3 shots)
			}
			else if (npc->act_no == 5 && signal == 14)
			{
				triggered = true; // Mode 3: Triggered only by a very specific Bullet ID
			}
			else if (npc->act_no == 6 && signal != 0)
			{
				triggered = true; // Mode 4: Triggered by any signal
			}
			else if (npc->act_no == 7 && signal != 0)
			{
				triggered = true; // Mode 5: Triggered by any signal
			}
		}

		if (triggered)
		{
			npc->act_wait = 70; // Set cooldown

			// Toggle the Flag ID assigned in the editor (code_flag)
			if (GetNPCFlag(npc->code_flag))
				CutNPCFlag(npc->code_flag);
			else
				SetNPCFlag(npc->code_flag);

			PlaySoundObject(103, SOUND_MODE_PLAY); // Play "Flag Toggled" sound
		}
		else
		{
			// LABEL_37: Standard non-triggered sprite calculation
			rect_left = 16 * (npc->act_no - 1) + 208;
			rect_right = rect_left + 16;
			goto SetRect;
		}
	}

	// Decrement cooldown timer
	if (npc->act_wait > 0)
		npc->act_wait--;

	// Animation logic: Determine if we show the "Idle" or "Active/Blink" frame
	// The blink occurs when ani_wait is between 1 and 4
	if (--npc->ani_wait > 4)
	{
		// Standard Mode Sprite
		rect_left = 16 * (npc->act_no - 1) + 208;
		rect_right = rect_left + 16;
	}
	else
	{
		if (npc->ani_wait <= 0)
			npc->ani_wait = 10;

		// Blink Sprite (Frame used to indicate activity or "hit")
		rect_left = 224;
		rect_right = 240;
	}

SetRect:
	// If direct is set, use the second row of the sprite sheet
	if (npc->direct != 0)
	{
		rect_top = 224;
		rect_bottom = 240;
	}

	npc->rect.left = rect_left;
	npc->rect.top = rect_top;
	npc->rect.right = rect_right;
	npc->rect.bottom = rect_bottom;

	// Important: Clear signal every frame so it doesn't re-trigger
	npc->count2 = 0;
}

// NPC 152 (Switch/Flag toggle based on sub_4967E0)
void ActNpc153(NPCHAR *npc)
{
    static const RECT rcLeft[21] = {
        {0, 0, 24, 24},
        {24, 0, 48, 24},
        {48, 0, 72, 24},
        {0, 24, 24, 48},
        {72, 0, 96, 24},
        {0, 0, 24, 24},
        {96, 0, 120, 24},
        {120, 0, 144, 24},
        {144, 0, 168, 24},
        {168, 0, 192, 24},
        {192, 0, 216, 24},
        {216, 0, 240, 24},
        {240, 0, 264, 24},
        {264, 0, 288, 24},
        {0, 48, 24, 72},
        {24, 48, 48, 72},
        {48, 48, 72, 72},
        {72, 48, 96, 72},
        {288, 0, 312, 24},
        {24, 48, 48, 72},
        {96, 48, 120, 72},
    };

    static const RECT rcRight[21] = {
        {0, 24, 24, 48},
        {24, 24, 48, 48},
        {48, 24, 72, 48},
        {0, 0, 24, 24},
        {72, 24, 96, 48},
        {0, 24, 24, 48},
        {96, 24, 120, 48},
        {120, 24, 144, 48},
        {144, 24, 168, 48},
        {168, 24, 192, 48},
        {192, 24, 216, 48},
        {216, 24, 240, 48},
        {240, 24, 264, 48},
        {264, 24, 288, 48},
        {0, 72, 24, 96},
        {24, 72, 48, 96},
        {48, 72, 72, 96},
        {72, 72, 96, 96},
        {288, 24, 312, 48},
        {24, 72, 48, 96},
        {96, 72, 120, 96},
    };

    // Range Check: Only process AI if within 1 screen width/height of the player
    if (npc->x <= gMC.x + (320 * 0x200) &&
        npc->x >= gMC.x - (320 * 0x200) &&
        npc->y <= gMC.y + (240 * 0x200) &&
        npc->y >= gMC.y - (240 * 0x200))
    {
        switch (npc->act_no)
        {
            case 0:
                npc->act_no = 1;
                npc->xm = 0;
                npc->ani_no = 0;
                npc->y += 3 * 0x200;
                // Fallthrough
            case 1:
                if (Random(0, 100) == 1)
                {
                    npc->act_no = 2;
                    npc->ani_no = 1;
                    npc->act_wait = 0;
                }

                if (Random(0, 100) == 1)
                {
                    if (npc->direct == 0)
                        npc->direct = 2;
                    else
                        npc->direct = 0;
                }

                if (Random(0, 100) == 1)
                    npc->act_no = 10;
                break;

            case 2:
                npc->act_wait++;
                if (npc->act_wait > 20)
                {
                    npc->act_no = 1;
                    npc->ani_no = 0;
                }
                break;

            case 10:
                npc->act_no = 11;
                npc->act_wait = Random(25, 100);
                npc->ani_no = 2;
                npc->ani_wait = 0;
                // Fallthrough
            case 11:
                npc->ani_wait++;
                if (npc->ani_wait > 3)
                {
                    npc->ani_wait = 0;
                    npc->ani_no++;
                }

                if (npc->ani_no > 5)
                    npc->ani_no = 2;

                if (npc->direct == 0)
                    npc->xm = -0x200;
                else
                    npc->xm = 0x200;

                if (npc->act_wait == 0)
                {
                    npc->act_no = 1;
                    npc->ani_no = 0;
                    npc->xm = 0;
                }
                else
                {
                    npc->act_wait--;
                }

                if (npc->direct == 0 && (npc->flag & 1))
                {
                    npc->ani_no = 2;
                    npc->ym = -0x5FF;
                    npc->act_no = 20;
                    if (!(gMC.cond & 2))
                        PlaySoundObject(30, SOUND_MODE_PLAY);
                }
                else if (npc->direct == 2 && (npc->flag & 4))
                {
                    npc->ani_no = 2;
                    npc->ym = -0x5FF;
                    npc->act_no = 20;
                    if (!(gMC.cond & 2))
                        PlaySoundObject(30, SOUND_MODE_PLAY);
                }
                break;

            case 20:
                if (npc->direct == 0 && (npc->flag & 1))
                    npc->count1++;
                else if (npc->direct == 2 && (npc->flag & 4))
                    npc->count1++;
                else
                    npc->count1 = 0;

                if (npc->count1 > 10)
                {
                    if (npc->direct == 0)
                        npc->direct = 2;
                    else
                        npc->direct = 0;
                }

                if (npc->direct == 0)
                    npc->xm = -0x100;
                else
                    npc->xm = 0x100;

                if (npc->flag & 8)
                {
                    npc->act_no = 21;
                    npc->ani_no = 20; // 0x14
                    npc->act_wait = 0;
                    npc->xm = 0;
                    if (!(gMC.cond & 2))
                        PlaySoundObject(23, SOUND_MODE_PLAY);
                }
                break;

            case 21:
                npc->act_wait++;
                if (npc->act_wait > 10)
                {
                    npc->act_no = 1;
                    npc->ani_no = 0;
                }
                break;
        }
    }

    npc->ym += 0x40;
    if (npc->ym > 0x5FF)
        npc->ym = 0x5FF;

    npc->x += npc->xm;
    npc->y += npc->ym;

    if (npc->direct == 0)
        npc->rect = rcLeft[npc->ani_no];
    else
        npc->rect = rcRight[npc->ani_no];

    // HP Threshold transform (51 = 0x33)
    if (npc->life < 51)
    {
        npc->code_char = 154; // Transitions to Defeated/Boss phase
        npc->act_no = 0;
    }
}


const RECT grcKitL[21] = {
	{0, 0, 24, 24},
	{24, 0, 48, 24},
	{48, 0, 72, 24},
	{0, 0, 24, 24},
	{72, 0, 96, 24},
	{0, 0, 24, 24},
	{96, 0, 120, 24},
	{120, 0, 144, 24},
	{144, 0, 168, 24},
	{168, 0, 192, 24},
	{192, 0, 216, 24},
	{216, 0, 240, 24},
	{240, 0, 264, 24},
	{264, 0, 288, 24},
	{0, 48, 24, 72},
	{24, 48, 48, 72},
	{48, 48, 72, 72},
	{72, 48, 96, 72},
	{288, 0, 312, 24},
	{24, 48, 48, 72},
	{96, 48, 120, 72}
};

const RECT grcKitR[21] = {
	{0, 24, 24, 48},
	{24, 24, 48, 48},
	{48, 24, 72, 48},
	{0, 24, 24, 48},
	{72, 24, 96, 48},
	{0, 24, 24, 48},
	{96, 24, 120, 48},
	{120, 24, 144, 48},
	{144, 24, 168, 48},
	{168, 24, 192, 48},
	{192, 24, 216, 48},
	{216, 24, 240, 48},
	{240, 24, 264, 48},
	{264, 24, 288, 48},
	{0, 72, 24, 96},
	{24, 72, 48, 96},
	{48, 72, 72, 96},
	{72, 72, 96, 96},
	{288, 24, 312, 48},
	{24, 72, 48, 96},
	{96, 72, 120, 96}
};

// Gaudi (dead)
void ActNpc154(NPCHAR *npc)
{
	switch (npc->act_no)
	{
		case 0:
			npc->bits &= ~NPC_SHOOTABLE;
			npc->bits &= ~NPC_IGNORE_SOLIDITY;
			npc->damage = 0;
			npc->act_no = 1;
			npc->ani_no = 9;
			npc->ym = -0x200;

			if (npc->direct == 0)
				npc->xm = 0x100;
			else
				npc->xm = -0x100;

			PlaySoundObject(53, SOUND_MODE_PLAY);
			break;

		case 1:
			if (npc->flag & 8)
			{
				npc->ani_no = 10;
				npc->ani_wait = 0;
				npc->act_no = 2;
				npc->act_wait = 0;
			}

			break;

		case 2:
			npc->xm = (npc->xm * 8) / 9;

			if (++npc->ani_wait > 3)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 11)
				npc->ani_no = 10;

			if (++npc->act_wait > 50)
				npc->cond |= 8;

			break;
	}

	npc->ym += 0x20;
	if (npc->ym > 0x5FF)
		npc->ym = 0x5FF;

	npc->x += npc->xm;
	npc->y += npc->ym;

	if (npc->direct == 0)
		npc->rect = grcKitL[npc->ani_no];
	else
		npc->rect = grcKitR[npc->ani_no];
}
// Gaudi (flying) - NPC 155
void ActNpc155(NPCHAR *npc)
{
	unsigned char deg;
	int xm, ym;

	// [MOD] Performance/Off-screen check
	// If the NPC is more than one screen away from the player, skip AI processing
	if (npc->x > gMC.x + (320 * 0x200) || npc->x < gMC.x - (320 * 0x200) || 
	    npc->y > gMC.y + (240 * 0x200) || npc->y < gMC.y - (240 * 0x200))
	{
		return;
	}

	switch (npc->act_no)
	{
		case 0:
			// Initialize flight path using trigonometry
			deg = (unsigned char)Random(0, 0xFF);
			npc->xm = GetCos(deg);
			npc->tgt_x = npc->x + (GetCos(deg + 64) * 8);

			deg = (unsigned char)Random(0, 0xFF);
			npc->ym = GetCos(deg); // ASM uses GetCos for both, resulting in a slightly different drift
			npc->tgt_y = npc->y + (GetCos(deg + 64) * 8);

			npc->act_no = 1;
			npc->count1 = 120;
			npc->act_wait = Random(70, 150);
			npc->ani_no = 14;
			// Fallthrough

		case 1:
			// Standard flight animation (Frames 14-15)
			if (++npc->ani_no > 15)
				npc->ani_no = 14;

			if (npc->act_wait != 0)
			{
				--npc->act_wait;
			}
			else
			{
				// Switch to shooting prep
				npc->act_no = 2;
				npc->ani_no = 18;
			}
			break;

		case 2:
			// Shooting animation (Frames 18-19)
			if (++npc->ani_no > 19)
				npc->ani_no = 18;

			if (++npc->act_wait > 30)
			{
				// Aim at player
				deg = GetArktan(npc->x - gMC.x, npc->y - gMC.y);
				deg += (unsigned char)Random(-6, 6); // Add slight inaccuracy
				
				ym = GetSin(deg) * 3;
				xm = GetCos(deg) * 3;

				SetNpChar(156, npc->x, npc->y, xm, ym, 0, NULL, 0x100);

				if (!(gMC.cond & 2)) // Play sound only if player isn't hidden
					PlaySoundObject(39, SOUND_MODE_PLAY);

				// Return to cruising
				npc->act_no = 1;
				npc->act_wait = Random(70, 150);
				npc->ani_no = 14;
				npc->ani_wait = 0;
			}
			break;
	}

	// Face the player
	if (npc->x <= gMC.x)
		npc->direct = 2;
	else
		npc->direct = 0;

	// Gentle homing towards target point
	if (npc->tgt_x < npc->x) npc->xm -= 0x10;
	if (npc->tgt_x > npc->x) npc->xm += 0x10;
	if (npc->tgt_y < npc->y) npc->ym -= 0x10;
	if (npc->tgt_y > npc->y) npc->ym += 0x10;

	// Speed limit
	if (npc->xm > 0x200)  npc->xm = 0x200;
	if (npc->xm < -0x200) npc->xm = -0x200;
	if (npc->ym > 0x200)  npc->ym = 0x200;
	if (npc->ym < -0x200) npc->ym = -0x200;

	npc->x += npc->xm;
	npc->y += npc->ym;

	// Sprite selection
	if (npc->direct == 0)
		npc->rect = grcKitL[npc->ani_no];
	else
		npc->rect = grcKitR[npc->ani_no];

	// [MOD] "Wounded" Transformation Logic
	// If health is 50 or less, transform into NPC 154 (The tumbling corpse NPC)
	if (npc->life <= 50)
	{
		npc->code_char = 154;
		npc->act_no = 0;
	}
}
// Gaudi projectile
void ActNpc156(NPCHAR *npc)
{
	if (npc->flag & 0xFF)
	{
		SetCaret(npc->x, npc->y, 2, 0);
		npc->cond = 0;
	}

	npc->y += npc->ym;
	npc->x += npc->xm;

	RECT rect_left[3] = {
		{96, 112, 112, 128},
		{112, 112, 128, 128},
		{128, 112, 144, 128},
	};

	if (++npc->ani_no > 2)
		npc->ani_no = 0;

	npc->rect = rect_left[npc->ani_no];

	if (++npc->count1 > 300)
	{
		SetCaret(npc->x, npc->y, 2, 0);
		npc->cond = 0;
	}
}
// Moving block (vertical) - Modded NPC 157
void ActNpc157(NPCHAR *npc)
{
	int i;
	int speed_limit;

	switch (npc->act_no)
	{
		case 0:
			// Shift block position by 8 pixels
			npc->x += 8 * 0x200;
			npc->y += 8 * 0x200;
			npc->bits |= NPC_SOLID_HARD;

			// If Option 1 (0x100) is set, block is "dormant" until stepped on
			if (npc->bits & 0x100) // Note: Map editor bits can vary by mod
			{
				npc->act_no = 1;
				npc->ani_no = 3;
			}
			else
			{
				npc->ani_no = 1;
				if (npc->direct == 0)
					npc->act_no = 10;
				else
					npc->act_no = 20;
			}

			npc->xm = 0;
			npc->ym = 0;
			break;

		case 1:
			// Wait for player to step on top of the block
			if (JudgeHitMyCharNPC4(npc))
			{
				npc->act_no = 2;
				npc->ani_no = 2;
				npc->count1 = 0;
				SetQuake(30);
			}
			break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			// Activation timer: 6 steps of 5 frames each with a "click" sound
			if (++npc->count1 > 5)
			{
				npc->count1 = 0;
				npc->act_no++;
				PlaySoundObject(111, SOUND_MODE_PLAY);
			}
			break;

		case 8:
			// Done activating, start movement
			npc->ani_no = 1;
			npc->count1 = 0;
			if (npc->direct == 0)
				npc->act_no = 10;
			else
				npc->act_no = 20;
			break;

		case 10: // Waiting at bottom
			npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
			npc->damage = 0;

			// Detect player within a large vertical column (400 pixels up)
			if (gMC.y < npc->y + 0x3200 && gMC.y > npc->y - (400 * 0x200) &&
				gMC.x < npc->x + 0x3200 && gMC.x > npc->x - 0x3200)
			{
				npc->act_no = 11;
				npc->act_wait = 0;
			}
			break;

		case 11: // Moving Up
			if (++npc->act_wait % 10 == 6)
				PlaySoundObject(107, SOUND_MODE_PLAY);

			if (npc->flag & 2) // Hit ceiling
			{
				npc->ym = 0;
				npc->direct = 2;
				npc->act_no = 20;
				SetQuake(10);
				PlaySoundObject(26, SOUND_MODE_PLAY);

				for (i = 0; i < 4; ++i)
					SetNpChar(4, npc->x - 0x2000, npc->y + (Random(-12, 12) * 0x200), Random(-341, 341), Random(-0x600, 0), 0, NULL, 0x100);
			}
			else
			{
				// Crush Logic: If player's top is hitting a ceiling while block moves up
				if (gMC.flag & 2)
				{
					npc->bits |= NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 100;
				}
				else
				{
					npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 0;
				}
				npc->ym -= 0x20;
			}
			break;

		case 20: // Waiting at top
			npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
			npc->damage = 0;

			// Detect player within a large vertical column (400 pixels down)
			if (gMC.y > npc->y - 0x3200 && gMC.y < npc->y + (400 * 0x200) &&
				gMC.x < npc->x + 0x3200 && gMC.x > npc->x - 0x3200)
			{
				npc->act_no = 21;
				npc->act_wait = 0;
			}
			break;

		case 21: // Moving Down
			if (++npc->act_wait % 10 == 6)
				PlaySoundObject(107, SOUND_MODE_PLAY);

			if (npc->flag & 8) // Hit floor
			{
				npc->ym = 0;
				npc->direct = 0;
				npc->act_no = 10;
				SetQuake(10);
				PlaySoundObject(26, SOUND_MODE_PLAY);

				for (i = 0; i < 4; ++i)
					SetNpChar(4, npc->x + 0x2000, npc->y + (Random(-12, 12) * 0x200), Random(-341, 341), Random(-0x600, 0), 0, NULL, 0x100);
			}
			else
			{
				// Crush Logic: If player's bottom is hitting a floor while block moves down
				if (gMC.flag & 8)
				{
					npc->bits |= NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 100;
				}
				else
				{
					npc->bits &= ~NPC_REAR_AND_TOP_DONT_HURT;
					npc->damage = 0;
				}
				npc->ym += 0x20;
			}
			break;
	}

	// Speed Logic
	// If Option 0x400 (usually "Appear from Chest" bit) is set, block moves at 4px/f.
	// Otherwise, standard 1px/f.
	if (npc->bits & 0x400)
		speed_limit = 0x800;
	else
		speed_limit = 0x200;

	if (npc->ym > speed_limit) npc->ym = speed_limit;
	if (npc->ym < -speed_limit) npc->ym = -speed_limit;

	npc->y += npc->ym;

	// Visuals
	// The mod uses a specific layout on the sprite sheet starting at Y=208
	npc->rect.left = (32 * (npc->ani_no - 1)) + 48;
	npc->rect.right = npc->rect.left + 32;
	
	if (npc->direct == 0)
	{
		npc->rect.top = 208;
		npc->rect.bottom = 240;
	}
	else
	{
		npc->rect.top = 224;
		npc->rect.bottom = 240;
	}
}

// Fish Missile
void ActNpc158(NPCHAR *npc)
{
	int dir;

	RECT rect[8] = {
		{0, 224, 16, 240},
		{16, 224, 32, 240},
		{32, 224, 48, 240},
		{48, 224, 64, 240},
		{64, 224, 80, 240},
		{80, 224, 96, 240},
		{96, 224, 112, 240},
		{112, 224, 128, 240},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;

			switch (npc->direct)
			{
				case 0:
					npc->count1 = 0xA0;
					break;

				case 1:
					npc->count1 = 0xE0;
					break;

				case 2:
					npc->count1 = 0x20;
					break;

				case 3:
					npc->count1 = 0x60;
					break;
			}
			// Fallthrough
		case 1:
			npc->xm = 2 * GetCos(npc->count1);
			npc->ym = 2 * GetSin(npc->count1);
			npc->y += npc->ym;
			npc->x += npc->xm;
			dir = GetArktan(npc->x - gMC.x, npc->y - gMC.y);

			if (dir < npc->count1)
			{
				if (npc->count1 - dir < 0x80)
					--npc->count1;
				else
					++npc->count1;
			}
			else
			{
				if (dir - npc->count1 < 0x80)
					++npc->count1;
				else
					--npc->count1;
			}

			if (npc->count1 > 0xFF)
				npc->count1 -= 0x100;
			if (npc->count1 < 0)
				npc->count1 += 0x100;

			break;
	}

	if (++npc->ani_wait > 2)
	{
		npc->ani_wait = 0;
		SetCaret(npc->x, npc->y, 7, 4);
	}

	npc->ani_no = (npc->count1 + 0x10) / 0x20;

	if (npc->ani_no > 7)
		npc->ani_no = 7;

	npc->rect = rect[npc->ani_no];
}

// Monster X (defeated)
void ActNpc159(NPCHAR *npc)
{
	int i;

	RECT rect = {144, 128, 192, 200};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;

			for (i = 0; i < 8; ++i)
				SetNpChar(4, npc->x + (Random(-16, 16) * 0x200), npc->y + (Random(-16, 16) * 0x200), Random(-341, 341), Random(-341, 341), 0, NULL, 0x100);
			// Fallthrough
		case 1:
			if (++npc->act_wait > 50)
			{
				npc->act_no = 2;
				npc->xm = -0x100;
			}

			if (npc->act_wait / 2 % 2)
				npc->x += 0x200;
			else
				npc->x -= 0x200;

			break;

		case 2:
			++npc->act_wait;
			npc->ym += 0x40;

			if (npc->y > 40 * 0x10 * 0x200)
				npc->cond = 0;

			break;
	}

	npc->y += npc->ym;
	npc->x += npc->xm;

	npc->rect = rect;

	if (npc->act_wait % 8 == 1)
		SetNpChar(4, npc->x + (Random(-16, 16) * 0x200), npc->y + (Random(-16, 16) * 0x200), Random(-341, 341), Random(-341, 341), 0, NULL, 0x100);
}
