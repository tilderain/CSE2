#include "MycParam.h"

#include <stddef.h>
#include <stdio.h>
#include <string>

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "CommonDefines.h"
#include "Caret.h"
#include "Draw.h"
#include "File.h"
#include "Game.h"
#include "Main.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Sound.h"
#include "TextScr.h"
#include "ValueView.h"

ARMS_LEVEL gArmsLevelTable[14] =
{
    {{30,  40, 16}},   // 49366C: 1E, 28, 10
    {{33,  25, 17}},   // 493678: 21, 19, 11
    {{48,  40, 32}},   // 493684: 30, 28, 20
    {{100, 75, 50}},   // 493690: 64, 4B, 32
    {{50,  50, 50}},   // 49369C: 32, 32, 32
    {{50,  100, 75}},  // 4936A8: 32, 64, 4B  (note: 32 not 50 -- wait, 0x32 = 50)
    {{25,  40, 60}},   // 4936B4: 19, 28, 3C
    {{80,  10, 20}},   // 4936C0: 50, 0A, 14
    {{100, 30, 60}},   // 4936CC: 64, 1E, 3C
    {{15,  20, 40}},   // 4936D8: 0F, 14, 28
    {{80,  30, 60}},   // 4936E4: 50, 1E, 3C
    {{10,  1,  1}},    // 4936F0: 0A, 01, 01
    {{1,   10, 20}},   // 4936FC: 01, 0A, 14
};

// Modded AddExpMyChar
void AddExpMyChar(int x)
{
	int lv = gArmsData[gSelectedArms].level - 1;
	int arms_code = gArmsData[gSelectedArms].code;

	gArmsData[gSelectedArms].exp += x;

	// [MOD] New logic: Handle losing EXP (Leveling Down)
	if (x < 0)
	{
		// Iterate backwards through levels if EXP is negative
		for (; lv > -1; --lv)
		{
			if (gArmsData[gSelectedArms].exp <= 0)
			{
				--gArmsData[gSelectedArms].level;
				int new_lv_idx = gArmsData[gSelectedArms].level - 1;

				// Set EXP to the max of the previous level, 
				// then apply the remaining negative overflow
				gArmsData[gSelectedArms].exp = gArmsLevelTable[arms_code].exp[new_lv_idx] + gArmsData[gSelectedArms].exp;
				
				if (gArmsData[gSelectedArms].exp < 0)
					gArmsData[gSelectedArms].exp = 0;

				PlaySoundObject(29, SOUND_MODE_PLAY); // 0x1D Level Down sound
				SetCaret(gMC.x, gMC.y, 10, 1);       // Caret 10, Dir 1 (Level Down "cloud")
			}
		}
	}
	// Logic for Max Level (unchanged from vanilla logic, but thresholds might differ in mod)
	else if (lv == 2)
	{
		if (gArmsData[gSelectedArms].exp >= gArmsLevelTable[arms_code].exp[lv])
		{
			gArmsData[gSelectedArms].exp = gArmsLevelTable[arms_code].exp[lv];

			if (gMC.equip & EQUIP_WHIMSICAL_STAR)
			{
				if (gMC.star < 3)
					++gMC.star;
			}
		}
	}
	// Logic for Leveling Up (Standard)
	else
	{
		for (; lv < 2; ++lv)
		{
			if (gArmsData[gSelectedArms].exp >= gArmsLevelTable[arms_code].exp[lv])
			{
				++gArmsData[gSelectedArms].level;
				gArmsData[gSelectedArms].exp = 0;

				// Don't play level-up sound/caret for the Spur (ID 13)
				if (gArmsData[gSelectedArms].code != 13)
				{
					PlaySoundObject(27, SOUND_MODE_PLAY); // 0x1B Level Up sound
					SetCaret(gMC.x, gMC.y, 10, 0);       // Caret 10, Dir 0 (Level Up "cloud")
				}
			}
		}
	}

	// UI Feedback
	if (gArmsData[gSelectedArms].code == 13) // Spur
	{
		gMC.exp_wait = 10;
	}
	else
	{
		// [MOD] Negative x will now correctly reduce the orange floating EXP number
		gMC.exp_count += x;
		gMC.exp_wait = 30; // 0x1E
	}
}

void ZeroExpMyChar(void)
{
	gArmsData[gSelectedArms].level = 1;
	gArmsData[gSelectedArms].exp = 0;
}

BOOL IsMaxExpMyChar(void)
{
	int arms_code;

	if (gArmsData[gSelectedArms].level == 3)
	{
		arms_code = gArmsData[gSelectedArms].code;

		if (gArmsData[gSelectedArms].exp >= gArmsLevelTable[arms_code].exp[2])
			return TRUE;
	}

	return FALSE;
}
#include "Flags.h"
// Custom Mod Function: Handles Player Death and the "Second Wind" Item
void HandlePlayerDeath(void)
{
	// 0x1518 = 5400. Flag 5400 checks if the player owns the Revive Item.
	if (GetNPCFlag(5400)) 
	{
		// 0x1522 = 5410. Flag 5410 checks if the Revive Item was already consumed.
		if (!GetNPCFlag(5410)) 
		{
			// Player survives! Clamp health to 0 to prevent negative overflow.
			// The TSC script will manually refill the player's health.
			gMC.life = 0;
			goto TriggerScript;
		}
	}

	// Normal Death Sequence (Item missing or already used)
	PlaySoundObject(17, SOUND_MODE_PLAY);             // 0x11: Player death sound
	SetDestroyNpChar(gMC.x, gMC.y, 0x1400, 64);       // 0x40: Spawn 64 smoke clouds

TriggerScript:
	// Start Event 40 (0x28). 
	// If reviving, this script heals the player and sets Flag 5410.
	// If dying, this script shows the Game Over screen.
	StartTextScript(40); 
}

void DamageMyChar(int damage)
{
	// (Vanilla bug preserved: bitwise OR instead of bitwise AND)
	if (!(g_GameFlags | 2))
		return;

	if (gMC.shock)
		return;

	// Damage player
	PlaySoundObject(16, SOUND_MODE_PLAY);
	gMC.cond &= ~1;
	gMC.shock = 128;

	if (gMC.unit != 1)
	{
		gMC.ym = -0x400;
	}

	gMC.life -= (short)damage;

	// Lose a whimsical star
	if (gMC.equip & EQUIP_WHIMSICAL_STAR && gMC.star > 0)
		gMC.star = (short)gMC.star - 1;

	// Lose experience (Halved if Arms Barrier is equipped)
	if (gMC.equip & EQUIP_ARMS_BARRIER)
		gArmsData[gSelectedArms].exp -= damage;
	else
		gArmsData[gSelectedArms].exp -= damage * 2;

	// Handle level-downs from massive damage
	while (gArmsData[gSelectedArms].exp < 0)
	{
		if (gArmsData[gSelectedArms].level > 1)
		{
			--gArmsData[gSelectedArms].level;

			int lv = gArmsData[gSelectedArms].level - 1;
			int arms_code = gArmsData[gSelectedArms].code;

			gArmsData[gSelectedArms].exp = gArmsLevelTable[arms_code].exp[lv] + gArmsData[gSelectedArms].exp;

			if (gMC.life > 0 && gArmsData[gSelectedArms].code != 13)
				SetCaret(gMC.x, gMC.y, CARET_LEVEL_UP, DIR_RIGHT); // 10, 2
		}
		else
		{
			gArmsData[gSelectedArms].exp = 0;
		}
	}

	// Tell player how much damage was taken
	SetValueView(&gMC.x, &gMC.y, -damage);

	// [MOD] Hook to Code Cave
	if (gMC.life <= 0)
	{
		HandlePlayerDeath(); // Jumps to FUN_00493970
	}
}


void ZeroArmsEnergy_All(void)
{
	int a;

	for (a = 0; a < ARMS_MAX; a++)
	{
		gArmsData[a].level = 1;
		gArmsData[a].exp = 0;
	}
}

void AddBulletMyChar(int no, int val)
{
	int a;

	// Loop to find Missile Launcher
	a = 0;
	while (a < ARMS_MAX && gArmsData[a].code != 5)
		++a;

	// MOD: The secondary check for Super Missiles (ID 10) was entirely NOP'd / removed
	// The original `if (a == ARMS_MAX)` nested block is gone.

	// If no Missile Launcher was found, abort
	if (a == ARMS_MAX)
		return;

	// Add ammo to the Missile Launcher
	gArmsData[a].num += val;

	// Clamp to max ammo
	if (gArmsData[a].num > gArmsData[a].max_num)
		gArmsData[a].num = gArmsData[a].max_num;
}

void AddLifeMyChar(int x)
{
	gMC.life += (short)x;
	if (gMC.life > gMC.max_life)
		gMC.life = gMC.max_life;
	gMC.lifeBr = gMC.life;
}

void AddMaxLifeMyChar(int val)
{
	gMC.max_life += (short)val;
	if (gMC.max_life > 232)
		gMC.max_life = 232;
	gMC.life += (short)val;
	gMC.lifeBr = gMC.life;
}

void PutArmsEnergy(BOOL flash)
{
	static unsigned char add_flash;

	RECT rcPer = {72, 48, 80, 56};
	RECT rcLv = {80, 80, 96, 88};
	RECT rcView = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
	RECT rcNone = {80, 48, 96, 56};

	if (gArmsEnergyX > 16)
		gArmsEnergyX -= 2;
	if (gArmsEnergyX < 16)
		gArmsEnergyX += 2;

	// Draw max ammo
	if (gArmsData[gSelectedArms].max_num)
	{
		PutNumber4(gArmsEnergyX + 32, 16, gArmsData[gSelectedArms].num, FALSE);
		PutNumber4(gArmsEnergyX + 32, 24, gArmsData[gSelectedArms].max_num, FALSE);
	}
	else
	{
		PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX + 48), PixelToScreenCoord(16), &rcNone, SURFACE_ID_TEXT_BOX, true);
		PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX + 48), PixelToScreenCoord(24), &rcNone, SURFACE_ID_TEXT_BOX, true);
	}

	// Draw experience and ammo
	if (flash == TRUE && (gMC.shock / 2) % 2)
		return;

	PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX + 32), PixelToScreenCoord(24), &rcPer, SURFACE_ID_TEXT_BOX, true);
	PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX), PixelToScreenCoord(32), &rcLv, SURFACE_ID_TEXT_BOX, true);
	PutNumber4(gArmsEnergyX - 8, 32, gArmsData[gSelectedArms].level, FALSE);

	RECT rcExpBox = {0, 72, 40, 80};
	RECT rcExpVal = {0, 80, 0, 88};
	RECT rcExpMax = {40, 72, 80, 80};
	RECT rcExpFlash = {40, 80, 80, 88};

	int lv = gArmsData[gSelectedArms].level - 1;

#ifdef FIX_BUGS
	// When the player has no weapons, the default level is 0, which becomes -1.
	// Catch it, and set it to 0 instead, so the following array-accesses aren't
	// out-of-bounds.
	if (lv < 0)
		lv = 0;
#endif

	int arms_code = gArmsData[gSelectedArms].code;
	int exp_now = gArmsData[gSelectedArms].exp;
	int exp_next = gArmsLevelTable[arms_code].exp[lv];

	PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX + 24), PixelToScreenCoord(32), &rcExpBox, SURFACE_ID_TEXT_BOX, true);

	if (lv == 2 && gArmsData[gSelectedArms].exp == gArmsLevelTable[arms_code].exp[lv])
	{
		PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX + 24), PixelToScreenCoord(32), &rcExpMax, SURFACE_ID_TEXT_BOX, true);
	}
	else
	{
		if (exp_next != 0)
			rcExpVal.right += (exp_now * 40) / exp_next;
		else
			rcExpVal.right = 0;

		PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX + 24), PixelToScreenCoord(32), &rcExpVal, SURFACE_ID_TEXT_BOX, true);
	}

	if (gMC.exp_wait && ((add_flash++ / 2) % 2))
		PutBitmap3(&rcView, PixelToScreenCoord(gArmsEnergyX + 24), PixelToScreenCoord(32), &rcExpFlash, SURFACE_ID_TEXT_BOX, true);
}

void PutActiveArmsList(void)
{
	int x;
	int a;
	RECT rect = {0, 0, 0, 16};

	int arms_num = 0;
	while (gArmsData[arms_num].code != 0)
		++arms_num;

	if (arms_num == 0)
		return;

	for (a = 0; a < arms_num; ++a)
	{
		// Get X position to draw at
		x = ((a - gSelectedArms) * 16) + gArmsEnergyX;

		if (x < 8)
			x += 48 + (arms_num * 16);
		else if (x >= 24)
			x += 48;

		if (x >= 72 + ((arms_num - 1) * 16))
			x -= 48 + (arms_num * 16);
		if (x < 72 && x >= 24)
			x -= 48;

		// Draw icon
		rect.left = gArmsData[a].code * 16;
		rect.right = rect.left + 16;
		PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(16), &rect, SURFACE_ID_ARMS_IMAGE, true);
	}
}

void PutMyLife(BOOL flash)
{
	RECT rcCase = {0, 40, 232, 48};
	RECT rcLife = {0, 24, 232, 32};
	RECT rcBr = {0, 32, 232, 40};

	if (flash == TRUE && gMC.shock / 2 % 2)
		return;

	if (gMC.lifeBr < gMC.life)
		gMC.lifeBr = gMC.life;

	if (gMC.lifeBr > gMC.life)
	{
		if (++gMC.lifeBr_count > 30)
			--gMC.lifeBr;
	}
	else
	{
		gMC.lifeBr_count = 0;
	}

	// Draw bar
	rcCase.right = 64;
	rcLife.right = ((gMC.life * 40) / gMC.max_life) - 1;
	rcBr.right = ((gMC.lifeBr * 40) / gMC.max_life) - 1;

	PutBitmap3(&grcGame, PixelToScreenCoord(16), PixelToScreenCoord(40), &rcCase, SURFACE_ID_TEXT_BOX, true);
	PutBitmap3(&grcGame, PixelToScreenCoord(40), PixelToScreenCoord(40), &rcBr, SURFACE_ID_TEXT_BOX, true);
	PutBitmap3(&grcGame, PixelToScreenCoord(40), PixelToScreenCoord(40), &rcLife, SURFACE_ID_TEXT_BOX, true);
	PutNumber4(8, 40, gMC.lifeBr, FALSE);
}

void PutMyAir(int x, int y)
{
	RECT rcAir[2] = {
		{112, 72, 144, 80},
		{112, 80, 144, 88},
	};

	if (gMC.no_splash_or_air_limit_underwater)
		return;

	if (gMC.equip & EQUIP_AIR_TANK)
		return;

	if (gMC.air_get != 0)
	{
		// Draw how much air is left
		if (gMC.air_get % 6 < 4)
			PutNumber4(x + 32, y, gMC.air / 10, FALSE);

		// Draw "AIR" text
		if (gMC.air % 30 > 10)
			PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcAir[0], SURFACE_ID_TEXT_BOX);
		else
			PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcAir[1], SURFACE_ID_TEXT_BOX);
	}
}

static int time_count;

void PutTimeCounter(int x, int y)
{
	RECT rcTime[3] = {
		{112, 104, 120, 112},
		{120, 104, 128, 112},
		{128, 104, 160, 112},
	};

	if (gMC.equip & EQUIP_NIKUMARU_COUNTER)
	{
		// Draw clock and increase time
		if (g_GameFlags & 2)
		{
			if (time_count < 100 * 60 * (gb60fps ? 60 : 50))	// 100 minutes
				++time_count;

			if (time_count % 30 > 10)
				PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcTime[0], SURFACE_ID_TEXT_BOX);
			else
				PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcTime[1], SURFACE_ID_TEXT_BOX);
		}
		else
		{
			PutBitmap3(&grcGame, PixelToScreenCoord(x), PixelToScreenCoord(y), &rcTime[0], SURFACE_ID_TEXT_BOX);
		}

		// Draw time
		if (gb60fps)
		{
			PutNumber4(x,		y, time_count / (60 * 60),	FALSE);
			PutNumber4(x + 20,	y, time_count / 60 % 60,	TRUE);
			PutNumber4(x + 32,	y, time_count / 6 % 10,		FALSE);
		}
		else
		{
			PutNumber4(x,		y, time_count / (60 * 50),	FALSE);
			PutNumber4(x + 20,	y, time_count / 50 % 60,	TRUE);
			PutNumber4(x + 32,	y, time_count / 5 % 10,		FALSE);
		}
		PutBitmap3(&grcGame, PixelToScreenCoord(x + 30), PixelToScreenCoord(y), &rcTime[2], SURFACE_ID_TEXT_BOX);
	}
	else
	{
		time_count = 0;
	}
}

BOOL SaveTimeCounter(void)
{
	int i;
	unsigned char p[4];
	REC rec;
	FILE *fp;
	std::string path;

	// Quit if player doesn't have the Nikumaru Counter
	if (!(gMC.equip & EQUIP_NIKUMARU_COUNTER))
		return TRUE;

	// Get last time
	path = gModulePath + "/290.rec";

	fp = fopen(path.c_str(), "rb");
	if (fp != NULL)
	{
		// Read data
		rec.counter[0] = File_ReadLE32(fp);
		rec.counter[1] = File_ReadLE32(fp);
		rec.counter[2] = File_ReadLE32(fp);
		rec.counter[3] = File_ReadLE32(fp);
		rec.random[0] = fgetc(fp);
		rec.random[1] = fgetc(fp);
		rec.random[2] = fgetc(fp);
		rec.random[3] = fgetc(fp);
		fclose(fp);

		p[0] = (unsigned char)(rec.counter[0] - rec.random[0]);
		p[1] = (unsigned char)((rec.counter[0] >> 8) - rec.random[0]);
		p[2] = (unsigned char)((rec.counter[0] >> 16) - rec.random[0]);
		p[3] = (unsigned char)((rec.counter[0] >> 24) - rec.random[0] / 2);

		rec.counter[0] = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

		// If this is faster than our new time, quit
		if (rec.counter[0] < time_count)
			return TRUE;
	}

	// Save new time
	for (i = 0; i < 4; ++i)
	{
		rec.counter[i] = time_count;
		rec.random[i] = Random(0, 250) + i;

		p[0] = (unsigned char)(rec.counter[i] + rec.random[i]);
		p[1] = (unsigned char)((rec.counter[i] >> 8) + rec.random[i]);
		p[2] = (unsigned char)((rec.counter[i] >> 16) + rec.random[i]);
		p[3] = (unsigned char)((rec.counter[i] >> 24) + rec.random[i] / 2);

		rec.counter[i] = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
	}

	fp = fopen(path.c_str(), "wb");
	if (fp == NULL)
		return FALSE;

	File_WriteLE32(rec.counter[0], fp);
	File_WriteLE32(rec.counter[1], fp);
	File_WriteLE32(rec.counter[2], fp);
	File_WriteLE32(rec.counter[3], fp);
	fputc(rec.random[0], fp);
	fputc(rec.random[1], fp);
	fputc(rec.random[2], fp);
	fputc(rec.random[3], fp);

	fclose(fp);
	return TRUE;
}

int LoadTimeCounter(void)
{
	int i;
	unsigned char p[4];
	REC rec;
	FILE *fp;
	std::string path;

	// Open file
	path = gModulePath + "/290.rec";

	fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
		return 0;

	// Read data
	rec.counter[0] = File_ReadLE32(fp);
	rec.counter[1] = File_ReadLE32(fp);
	rec.counter[2] = File_ReadLE32(fp);
	rec.counter[3] = File_ReadLE32(fp);
	rec.random[0] = fgetc(fp);
	rec.random[1] = fgetc(fp);
	rec.random[2] = fgetc(fp);
	rec.random[3] = fgetc(fp);
	fclose(fp);

	// Decode from checksum
	for (i = 0; i < 4; ++i)
	{
		p[0] = (unsigned char)(rec.counter[i] - rec.random[i]);
		p[1] = (unsigned char)((rec.counter[i] >> 8) - rec.random[i]);
		p[2] = (unsigned char)((rec.counter[i] >> 16) - rec.random[i]);
		p[3] = (unsigned char)((rec.counter[i] >> 24) - rec.random[i] / 2);

		rec.counter[i] = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
	}

	// Verify checksum's result
	if (rec.counter[0] != rec.counter[1] || rec.counter[0] != rec.counter[2])
	{
		time_count = 0;
		return 0;
	}

	time_count = rec.counter[0];
	return time_count;
}
