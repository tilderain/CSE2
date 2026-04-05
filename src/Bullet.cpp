#include "Bullet.h"

#include <stddef.h>
#include <string.h>

#include "WindowsWrapper.h"

#include "Draw.h"
#include "Caret.h"
#include "Game.h"
#include "KeyControl.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Sound.h"

BULLET gBul[BULLET_MAX];

#include "Bullet.h"
#include "NpChar.h"
#include "MyChar.h"
#include "Sound.h"
#include "Caret.h"
#include "Map.h"
#include "Triangle.h"
#include "Flags.h"
#include "ArmsItem.h"
#include "KeyControl.h"
#include "Random.h"

// Mod-specific Global Variables used for the Custom Bubbler logic
unsigned char gCustomWeaponState = 0;
unsigned char gCustomWeaponDir = 0;


void ActBullet_FireBall_Meteor(BULLET *bul)
{
	bul->y += 0x300;
	bul->direct = DIR_LEFT;

	if (++bul->ani_wait > 2)
	{
		bul->ani_wait = 0;
		++bul->ani_no;
	}

	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
	}
	else
	{
		if (bul->ani_no > 4)
			bul->ani_no = 0;

		// Reconstructed from the assembly's local array
		RECT rect_meteor[5] = {
			{192, 16, 200, 32},
			{200, 16, 208, 32},
			{208, 16, 216, 32},
			{216, 16, 224, 32},
			{224, 16, 232, 32},
		};

		bul->rect = rect_meteor[bul->ani_no];
	}
}

//-----------------------------------------------------
// Fireball Level 3 Explosion (Bullet 9)
//-----------------------------------------------------

void ActBullet_FireBall_Explosion(BULLET *bul)
{
	// FUN_00494360(3) -> Dynamically get Fireball weapon level
	int level = 0;
	for (int i = 0; i < ARMS_MAX; ++i)
	{
		if (gArmsData[i].code == 3)
		{
			level = gArmsData[i].level;
			break;
		}
	}

	// 004943cd: MOV [ECX + 0x58], EAX -> bul->damage = level
	bul->damage = level;

	if (bul->act_no == 0)
	{
		bul->act_no = 1;
		bul->act_wait = 12;
		PlaySoundObject(71, SOUND_MODE_PLAY);  // 0x47 SND_EXPLOSION
		PlaySoundObject(114, SOUND_MODE_PLAY); // 0x72 SND_IRONH_DAMAGE
	}
	else if (bul->act_no == 1)
	{
		if (bul->act_wait % 3 == 0)
		{
			int spread = (level * 8) + 5;
			
			// The assembly calculates Y randomness first, then X randomness
			int rand_y = Random(-spread, spread) * 0x200;
			int rand_x = Random(-spread, spread) * 0x200;
			
			// SetDestroyNpCharUp(x, y, w, num)
			// Assembly passes bul->enemyXL (0x60) for width and bul->damage (0x58) for count
			SetDestroyNpCharUp(bul->x + rand_x, bul->y + rand_y, bul->enemyXL, bul->damage);
		}

		if (bul->act_wait % 2 == 0)
		{
			bul->damage = 0; // Temporarily disable damage to prevent multi-hit melting
		}

		bul->act_wait--;

		if (bul->act_wait < 0)
		{
			bul->cond = 0;
		}
	}
}


//-----------------------------------------------------
// Machine Gun (Bullet 10, 11, 12)
//-----------------------------------------------------
void ActBullet_MachineGun(BULLET *bul, int level)
{
	// RECT tables for the three levels of the Machine Gun
	// These are typically defined in the function or as static data
	RECT rect_lv1[4] = {
		{64, 0, 80, 16}, {80, 0, 96, 16}, {96, 0, 112, 16}, {112, 0, 128, 16}
	};
	RECT rect_lv2[4] = {
		{64, 16, 80, 32}, {80, 16, 96, 32}, {96, 16, 112, 32}, {112, 16, 128, 32}
	};
	RECT rect_lv3[4] = {
		{64, 32, 80, 48}, {80, 32, 96, 48}, {96, 32, 112, 48}, {112, 32, 128, 48}
	};

	// Lifespan management
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	// Initializing bullet state
	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		int speed = 0x1000; // 8.0 pixels per frame
		int spread;

		// MOD: Dynamic accuracy check based on NPC Flags (563 and 564)
		if (GetNPCFlag(0x233))
			spread = 0x10;  // High accuracy
		else if (GetNPCFlag(0x234))
			spread = 0x50;  // Medium accuracy
		else
			spread = 0x90;  // Default low accuracy (wobbly)

		// Set velocity based on direction and randomized spread
		switch (bul->direct)
		{
			case 0: // Left
				bul->xm = -speed;
				bul->ym = Random(-spread, spread);
				break;
			case 1: // Up
				bul->ym = -speed;
				bul->xm = Random(-spread, spread);
				break;
			case 2: // Right
				bul->xm = speed;
				bul->ym = Random(-spread, spread);
				break;
			case 3: // Down
				bul->ym = speed;
				bul->xm = Random(-spread, spread);
				break;
		}
	}
	else
	{
		// Constant movement
		bul->x += bul->xm;
		bul->y += bul->ym;

		// Level-specific behaviors and rendering
		if (level == 1)
		{
			bul->rect = rect_lv1[bul->direct];
		}
		else if (level == 2)
		{
			bul->rect = rect_lv2[bul->direct];

			// Spawns "Trail" effects (NPC 127)
			// If firing vertically, set effect direction to 1, else 0
			if (bul->direct == 1 || bul->direct == 3)
				SetNpChar(127, bul->x, bul->y, 0, 0, 1, NULL, 0x100);
			else
				SetNpChar(127, bul->x, bul->y, 0, 0, 0, NULL, 0x100);
		}
		else if (level == 3)
		{
			bul->rect = rect_lv3[bul->direct];

			// Spawns "Trail" effects (NPC 128) following the bullet direction
			SetNpChar(128, bul->x, bul->y, 0, 0, bul->direct, NULL, 0x100);
		}
	}
}

//-----------------------------------------------------
// Custom Bubbler logic (Bullet 19, 20, 21)
//-----------------------------------------------------
void ActBullet_CustomBubbler(BULLET *bul)
{
	int wpn_lvl = 0;
	for (int i = 0; i < ARMS_MAX; ++i)
	{
		if (gArmsData[i].code == 7) // 7 = Bubbler
		{
			wpn_lvl = gArmsData[i].level;
			break;
		}
	}

	bul->damage = (wpn_lvl + 3) / 2;
	bul->life_count = wpn_lvl * 2 + 10;

	if (gCustomWeaponState == 1)
	{
		if (bul->act_no == 1)
		{
			if ((bul->flag & 0xF) == 0)
			{
				bul->count1++;
				if (bul->count1 < bul->life_count)
				{
					bul->act_wait--;
					if (bul->act_wait < 1)
					{
						bul->act_wait = 5;
						PlaySoundObject(100, SOUND_MODE_PLAY); // 100 SND_BUBBLER_LAUNCH
					}
					bul->x += bul->xm;
					bul->y += bul->ym;
					
					RECT rc = {0, 48, 16, 64};
					rc.left += gCustomWeaponDir * 16;
					rc.right += gCustomWeaponDir * 16;
					bul->rect = rc;
					return;
				}
			}
			else
			{
				PlaySoundObject(31, SOUND_MODE_PLAY); // 0x1F SND_PROJECTILE_HIT
				SetCaret(bul->x, bul->y, CARET_SHOOT, DIR_LEFT);
			}
			
			gCustomWeaponState = 3;
		}
		else
		{
			int xm = 0, ym = 0;
			
			switch (gCustomWeaponDir)
			{
				case 0: xm = -0xF00; break;
				case 1: ym = -0xF00; break;
				case 2: xm =  0xF00; break;
				case 3: ym =  0xF00; break;
				case 4: xm = -0xD00; ym = -0xD00; break;
				case 5: xm =  0xD00; ym = -0xD00; break;
				case 6: xm =  0xD00; ym =  0xD00; break;
				case 7: xm = -0xD00; ym =  0xD00; break;
			}
			
			bul->xm = xm;
			bul->ym = ym;
			bul->act_no = 1;
		}
		return;
	}
	
	if (gCustomWeaponState != 2)
	{
		if (gCustomWeaponState == 3)
			gCustomWeaponState = 0;
		else
			bul->cond = 0;
			
		return;
	}
	
	bul->life = 100;
	
	if (gSelectedArms == 5)
	{
		if (!(gKey & gKeyShot)) { /* Do nothing */ }
		else if (gKey & gKeyJump)
		{
			// Acts like a jetpack!
			gMC.xm += gMC.xm / 5;
			gMC.ym += gMC.ym / 7;
		}
	}
	else if (!(gKey & gKeyJump)) { /* Do nothing */ }
	else
	{
		gCustomWeaponState = 3;
	}
	
	RECT rcEmpty = {0, 0, 0, 0};
	bul->rect = rcEmpty;
}



void InitBullet(void)
{
	// Identical to ClearBullet
	int i;

	for (i = 0; i < BULLET_MAX; ++i)
		gBul[i].cond = 0;
}

// [Mod] Custom Replacement for CountArmsBullet
// Instead of checking active bullets by their literal `code_bullet` (which ranges 
// from 1 to 45), this custom routine mathematically calculates the base "Weapon ID"
// from the bullet code.
int CountWeaponShotOccurrences(int weapon_id)
{
	int count = 0;
	int i;

	for (i = 0; i < BULLET_MAX; ++i)
	{
		// If the bullet is currently active (cond & 0x80)
		if (gBul[i].cond & 0x80)
		{
			// In vanilla Cave Story, every weapon has exactly 3 levels.
			// Therefore, the bullet IDs for a weapon are grouped sequentially in threes.
			// Example: 
			// Snake Level 1 = 1, Level 2 = 2, Level 3 = 3
			// Polar Star Level 1 = 4, Level 2 = 5, Level 3 = 6
			// Fireball Level 1 = 7, Level 2 = 8, Level 3 = 9
			//
			// This math formula: (code_bullet + 2) / 3
			// maps those 3-level groupings back down to a single base "Weapon ID".
			// (e.g., bullets 4, 5, and 6 will all evaluate to Weapon ID 2).
			
			if ((gBul[i].code_bullet + 2) / 3 == weapon_id)
				++count;
		}
	}

	return count;
}

int CountArmsBullet(int arms_code)
{
	int i;
	int count = 0;

	for (i = 0; i < BULLET_MAX; ++i)
		if (gBul[i].cond & 0x80 && (gBul[i].code_bullet + 2) / 3 == arms_code)
			++count;

	return count;
}

int CountBulletNum(int bullet_code)
{
	int i;
	int count = 0;

	for (i = 0; i < BULLET_MAX; ++i)
		if (gBul[i].cond & 0x80 && gBul[i].code_bullet == bullet_code)
			++count;

	return count;
}

void DeleteBullet(int code)
{
	int i;
	int count = 0;	// Guessed name. This is unused, and was optimised out of the Linux port.

	for (i = 0; i < BULLET_MAX; ++i)
		if (gBul[i].cond & 0x80 && (gBul[i].code_bullet + 2) / 3 == code)
				gBul[i].cond = 0;
}

void ClearBullet(void)
{
	// Identical to InitBullet
	int i;

	for (i = 0; i < BULLET_MAX; ++i)
		gBul[i].cond = 0;
}

void PutBullet(int fx, int fy)
{
	int i;
	int x, y;

	for (i = 0; i < BULLET_MAX; ++i)
	{
		if (gBul[i].cond & 0x80)
		{
			switch (gBul[i].direct)
			{
				case 0:
					x = gBul[i].x - gBul[i].view.front;
					y = gBul[i].y - gBul[i].view.top;
					break;
				case 1:
					x = gBul[i].x - gBul[i].view.top;
					y = gBul[i].y - gBul[i].view.front;
					break;
				case 2:
					x = gBul[i].x - gBul[i].view.back;
					y = gBul[i].y - gBul[i].view.top;
					break;
				case 3:
					x = gBul[i].x - gBul[i].view.top;
					y = gBul[i].y - gBul[i].view.back;
					break;
			}

			PutBitmap3(&grcGame, SubpixelToScreenCoord(x) - SubpixelToScreenCoord(fx), SubpixelToScreenCoord(y) - SubpixelToScreenCoord(fy), &gBul[i].rect, SURFACE_ID_BULLET);
		}
	}
}
BULLET_TABLE gBulTbl[46] = {
    // [0] Null
    {0, 0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0}},

    // [1] Snake lv1  — 0x48F074
    // 04 01 | 14000000 | 24000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {4, 1, 20, 36, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [2] Snake lv2  — 0x48F0A0
    // 06 01 | 17000000 | 24000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {6, 1, 23, 36, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [3] Snake lv3  — 0x48F0CC
    // 08 01 | 1E000000 | 24000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {8, 1, 30, 36, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [4] Polar Star lv1  — 0x48F0F8
    // 01 02 | 10000000 | 20000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {1, 2, 16, 32, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [5] Polar Star lv2  — 0x48F124
    // 03 01 | 12000000 | 20000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {3, 1, 18, 32, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [6] Polar Star lv3  — 0x48F150
    // 02 02 | 14000000 | 20000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {2, 2, 20, 32, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [7] Fireball lv1  — 0x48F17C
    // 01 03 | 78000000 | 2A000000 | 08000000 | 08000000 | 08000000 | 06000000 | 08000000 | 08000000 | 08000000 | 08000000
    {1, 3, 120, 42, 8, 8, 8, 6, {8, 8, 8, 8}},

    // [8] Fireball lv2  — 0x48F1A8
    // 01 05 | 3C000000 | 0A000000 | 04000000 | 06000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000 | 08000000
    {1, 5, 60, 10, 4, 6, 2, 8, {8, 8, 8, 8}},

    // [9] Fireball lv3  — 0x48F1D4
    // 04 64 | 64000000 | 5A000000 | 0C000000 | 0C000000 | 00000000 | 00000000 | 08000000 | 08000000 | 08000000 | 08000000
    {4, 100, 100, 90, 12, 12, 0, 0, {8, 8, 8, 8}},

    // [10] Machine Gun lv1  — 0x48F200
    // 04 01 | 20000000 | 20000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {4, 1, 32, 32, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [11] Machine Gun lv2  — 0x48F22C
    // 06 01 | 20000000 | 20000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {6, 1, 32, 32, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [12] Machine Gun lv3  — 0x48F258
    // 08 01 | 20000000 | 20000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {8, 1, 32, 32, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [13] Missile lv1  — 0x48F284
    // 05 5B | 64000000 | A8000000 | 05000000 | 05000000 | 03000000 | 03000000 | 08000000 | 08000000 | 08000000 | 08000000
    {5, 91, 100, 168, 5, 5, 3, 3, {8, 8, 8, 8}},

    // [14] Missile lv2  — 0x48F2B0
    // 05 5D | 64000000 | E8000000 | 08000000 | 08000000 | 06000000 | 06000000 | 08000000 | 08000000 | 08000000 | 08000000
    {5, 93, 100, 232, 8, 8, 6, 6, {8, 8, 8, 8}},

    // [15] Missile lv3  — 0x48F2DC
    // 05 5A | 32000000 | A8000000 | 04000000 | 04000000 | 02000000 | 02000000 | 08000000 | 08000000 | 08000000 | 08000000
    {5, 90, 50, 168, 4, 4, 2, 2, {8, 8, 8, 8}},

    // [16] Missile explosion lv1  — 0x48F308
    // 03 64 | 64000000 | BC000000 | 0E000000 | 0E000000 | 00000000 | 00000000 | 08000000 | 08000000 | 08000000 | 08000000
    {3, 100, 100, 188, 14, 14, 0, 0, {8, 8, 8, 8}},

    // [17] Missile explosion lv2  — 0x48F334
    // 0D 64 | 64000000 | FC000000 | 1E000000 | 1E000000 | 00000000 | 00000000 | 08000000 | 08000000 | 08000000 | 08000000
    {13, 100, 100, 252, 30, 30, 0, 0, {8, 8, 8, 8}},

    // [18] Missile explosion lv3  — 0x48F360
    // 03 64 | 64000000 | 9C000000 | 0A000000 | 0A000000 | 00000000 | 00000000 | 08000000 | 08000000 | 08000000 | 08000000
    {3, 100, 100, 156, 10, 10, 0, 0, {8, 8, 8, 8}},

    // [19] Bubbler lv1  — 0x48F38C
    // 01 64 | 28000000 | 68000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000
    {1, 100, 40, 104, 4, 4, 4, 4, {4, 4, 4, 4}},

    // [20] Bubbler lv2  — 0x48F3B8
    // 02 64 | 32000000 | 68000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000
    {2, 100, 50, 104, 4, 4, 4, 4, {4, 4, 4, 4}},

    // [21] Bubbler lv3  — 0x48F3E4
    // 03 64 | 3C000000 | 68000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000 | 04000000
    {3, 100, 60, 104, 4, 4, 4, 4, {4, 4, 4, 4}},

    // [22] Blade lv1  — 0x48F410
    // 02 01 | 20000000 | 20000000 | 04000000 | 04000000 | 02000000 | 02000000 | 04000000 | 04000000 | 04000000 | 04000000
    {2, 1, 32, 32, 4, 4, 2, 2, {4, 4, 4, 4}},

    // [23] Blade lv2  — 0x48F43C
    // 01 64 | 00000000 | 68000000 | 12000000 | 14000000 | 10000000 | 10000000 | 0C000000 | 0C000000 | 0C000000 | 0C000000
    {1, 100, 0, 104, 18, 20, 16, 16, {12, 12, 12, 12}},

    // [24] Blade lv3  — 0x48F468
    // E7 01 | 02000000 | 04000000 | 06000000 | 04000000 | 06000000 | 04000000 | 00000000 | 00000000 | 00000000 | 00000000
    // E7 = 231 as unsigned, -25 as signed char
    {-25, 1, 2, 4, 6, 4, 6, 4, {0, 0, 0, 0}},

    // [25] Falling spike  — 0x48F494
    // 01 64 | 04000000 | 68000000 | 08000000 | 08000000 | 08000000 | 08000000 | 08000000 | 08000000 | 08000000 | 08000000
    {1, 100, 4, 104, 8, 8, 8, 8, {8, 8, 8, 8}},

    // [26] Super Missile lv1  — 0x48F4C0
    // 02 64 | 32000000 | 68000000 | 08000000 | 08000000 | 0C000000 | 0C000000 | 0C000000 | 0C000000 | 0C000000 | 0C000000
    {2, 100, 50, 104, 8, 8, 12, 12, {12, 12, 12, 12}},

    // [27] Super Missile lv2  — 0x48F4EC
    // 01 64 | 32000000 | 68000000 | 08000000 | 08000000 | 0C000000 | 0C000000 | 0C000000 | 0C000000 | 0C000000 | 0C000000
    {1, 100, 50, 104, 8, 8, 12, 12, {12, 12, 12, 12}},

    // [28] Super Missile lv3  — 0x48F518
    // 0A 0A | 1E000000 | 20000000 | 04000000 | 04000000 | 00000000 | 00000000 | 08000000 | 08000000 | 08000000 | 08000000
    {10, 10, 30, 32, 4, 4, 0, 0, {8, 8, 8, 8}},

    // [29] Super Missile explosion lv1  — 0x48F544
    // 14 0A | 28000000 | 20000000 | 08000000 | 08000000 | 00000000 | 00000000 | 08000000 | 08000000 | 08000000 | 08000000
    {20, 10, 40, 32, 8, 8, 0, 0, {8, 8, 8, 8}},

    // [30] Super Missile explosion lv2  — 0x48F570
    // 0A 0A | 28000000 | 20000000 | 06000000 | 06000000 | 00000000 | 00000000 | 08000000 | 08000000 | 08000000 | 08000000
    {10, 10, 40, 32, 6, 6, 0, 0, {8, 8, 8, 8}},

    // [31] Super Missile explosion lv3  — 0x48F59C
    // 01 14 | 64000000 | 14000000 | 18000000 | 18000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000
    {1, 20, 100, 20, 24, 24, 0, 0, {0, 0, 0, 0}},

    // [32] Nemesis lv1  — 0x48F5C8
    // 01 14 | 64000000 | 14000000 | 20000000 | 20000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000
    {1, 20, 100, 20, 32, 32, 0, 0, {0, 0, 0, 0}},

    // [33] Nemesis lv2  — 0x48F5F4
    // 01 14 | 64000000 | 14000000 | 20000000 | 20000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000
    {1, 20, 100, 20, 32, 32, 0, 0, {0, 0, 0, 0}},

    // [34] Spur lv1  — 0x48F620
    // 04 04 | 14000000 | 20000000 | 04000000 | 04000000 | 03000000 | 03000000 | 08000000 | 08000000 | 18000000 | 08000000
    {4, 4, 20, 32, 4, 4, 3, 3, {8, 8, 24, 8}},

    // [35] Spur lv2  — 0x48F64C
    // 04 02 | 14000000 | 20000000 | 02000000 | 02000000 | 02000000 | 02000000 | 08000000 | 08000000 | 18000000 | 08000000
    {4, 2, 20, 32, 2, 2, 2, 2, {8, 8, 24, 8}},

    // [36] Spur lv3  — 0x48F678
    // 01 01 | 14000000 | 20000000 | 02000000 | 02000000 | 02000000 | 02000000 | 08000000 | 08000000 | 18000000 | 08000000
    {1, 1, 20, 32, 2, 2, 2, 2, {8, 8, 24, 8}},

    // [37] Spur trail lv1  — 0x48F6A4
    // 04 04 | 1E000000 | 40000000 | 06000000 | 06000000 | 03000000 | 03000000 | 08000000 | 08000000 | 08000000 | 08000000
    {4, 4, 30, 64, 6, 6, 3, 3, {8, 8, 8, 8}},

    // [38] Spur trail lv2  — 0x48F6D0
    // 08 08 | 1E000000 | 40000000 | 06000000 | 06000000 | 03000000 | 03000000 | 08000000 | 08000000 | 08000000 | 08000000
    {8, 8, 30, 64, 6, 6, 3, 3, {8, 8, 8, 8}},

    // [39] Spur trail lv3  — 0x48F6FC
    // 0C 0C | 1E000000 | 40000000 | 06000000 | 06000000 | 03000000 | 03000000 | 08000000 | 08000000 | 08000000 | 08000000
    {12, 12, 30, 64, 6, 6, 3, 3, {8, 8, 8, 8}},

    // [40] Curly Nemesis  — 0x48F728
    // 03 64 | 1E000000 | 20000000 | 06000000 | 06000000 | 03000000 | 03000000 | 04000000 | 04000000 | 04000000 | 04000000
    {3, 100, 30, 32, 6, 6, 3, 3, {4, 4, 4, 4}},

    // [41] Curly Nemesis lv2  — 0x48F754
    // 06 64 | 1E000000 | 20000000 | 06000000 | 06000000 | 03000000 | 03000000 | 04000000 | 04000000 | 04000000 | 04000000
    {6, 100, 30, 32, 6, 6, 3, 3, {4, 4, 4, 4}},

    // [42] Curly Nemesis lv3  — 0x48F780
    // 0B 64 | 1E000000 | 20000000 | 06000000 | 06000000 | 03000000 | 03000000 | 04000000 | 04000000 | 04000000 | 04000000
    {11, 100, 30, 32, 6, 6, 3, 3, {4, 4, 4, 4}},

    // [43] Whimsical star  — 0x48F7AC
    // 04 04 | 14000000 | 20000000 | 04000000 | 04000000 | 03000000 | 03000000 | 08000000 | 08000000 | 18000000 | 08000000
    {4, 4, 20, 32, 4, 4, 3, 3, {8, 8, 24, 8}},

    // [44] Screen nuke  — 0x48F7D8
    // 00 04 | 04000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000 | 00000000
    {0, 4, 4, 0, 0, 0, 0, 0, {0, 0, 0, 0}},

    // [45] Whimsical star (second slot?)  — 0x48F804
    // 01 01 | 01000000 | 24000000 | 01000000 | 01000000 | 01000000 | 01000000 | 01000000 | 01000000 | 01000000 | 01000000
    {1, 1, 1, 36, 1, 1, 1, 1, {1, 1, 1, 1}},
};
void SetBullet(int no, int x, int y, int dir)
{
	int i = 0;
	while (i < BULLET_MAX && gBul[i].cond & 0x80)
		++i;

	if (i >= BULLET_MAX)
		return;

	memset(&gBul[i], 0, sizeof(BULLET));
	gBul[i].code_bullet = no;
	gBul[i].cond = 0x80;
	gBul[i].direct = dir;
	gBul[i].damage = gBulTbl[no].damage;
	gBul[i].life = gBulTbl[no].life;
	gBul[i].life_count = gBulTbl[no].life_count;
	gBul[i].bbits = gBulTbl[no].bbits;
	gBul[i].enemyXL = gBulTbl[no].enemyXL * 0x200;
	gBul[i].enemyYL = gBulTbl[no].enemyYL * 0x200;
	gBul[i].blockXL = gBulTbl[no].blockXL * 0x200;
	gBul[i].blockYL = gBulTbl[no].blockYL * 0x200;
	gBul[i].view.back = gBulTbl[no].view.back * 0x200;
	gBul[i].view.front = gBulTbl[no].view.front * 0x200;
	gBul[i].view.top = gBulTbl[no].view.top * 0x200;
	gBul[i].view.bottom = gBulTbl[no].view.bottom * 0x200;
	gBul[i].x = x;
	gBul[i].y = y;
}

void ActBullet_Frontia1(BULLET *bul)
{
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	if (bul->act_no == 0)
	{
		bul->ani_no = Random(0, 2);
		bul->act_no = 1;

		switch (bul->direct)
		{
			case 0:
				bul->xm = -0x600;
				break;
			case 1:
				bul->ym = -0x600;
				break;
			case 2:
				bul->xm = 0x600;
				break;
			case 3:
				bul->ym = 0x600;
				break;
		}
	}
	else
	{
		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	if (++bul->ani_wait > 0)
	{
		bul->ani_wait = 0;
		++bul->ani_no;
	}

	if (bul->ani_no > 3)
		bul->ani_no = 0;

	RECT rcLeft[4] = {
		{136, 80, 152, 80},
		{120, 80, 136, 96},
		{136, 64, 152, 80},
		{120, 64, 136, 80},
	};

	RECT rcRight[4] = {
		{120, 64, 136, 80},
		{136, 64, 152, 80},
		{120, 80, 136, 96},
		{136, 80, 152, 80},
	};

	if (bul->direct == 0)
		bul->rect = rcLeft[bul->ani_no];
	else
		bul->rect = rcRight[bul->ani_no];
}

void ActBullet_Frontia2(BULLET *bul, int level)
{
	static unsigned int inc;

	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	if (bul->act_no == 0)
	{
		bul->ani_no = Random(0, 2);
		bul->act_no = 1;

		switch (bul->direct)
		{
			case 0:
				bul->xm = -0x200;
				break;
			case 1:
				bul->ym = -0x200;
				break;
			case 2:
				bul->xm = 0x200;
				break;
			case 3:
				bul->ym = 0x200;
				break;
		}

		++inc;

		switch (bul->direct)
		{
			case 0:
			case 2:
				if (inc % 2)
					bul->ym = 0x400;
				else
					bul->ym = -0x400;

				break;

			case 1:
			case 3:
				if (inc % 2)
					bul->xm = 0x400;
				else
					bul->xm = -0x400;

				break;
		}
	}
	else
	{
		switch (bul->direct)
		{
			case 0:
				bul->xm -= 0x80;
				break;
			case 1:
				bul->ym -= 0x80;
				break;
			case 2:
				bul->xm += 0x80;
				break;
			case 3:
				bul->ym += 0x80;
				break;
		}

		switch (bul->direct)
		{
			case 0:
			case 2:
				if (bul->count1 % 5 == 2)
				{
					if (bul->ym < 0)
						bul->ym = 0x400;
					else
						bul->ym = -0x400;
				}

				break;

			case 1u:
			case 3u:
				if (bul->count1 % 5 == 2)
				{
					if (bul->xm < 0)
						bul->xm = 0x400;
					else
						bul->xm = -0x400;
				}

				break;
		}

		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	if (++bul->ani_wait > 0)
	{
		bul->ani_wait = 0;
		++bul->ani_no;
	}

	if (bul->ani_no > 2)
		bul->ani_no = 0;

	RECT rect[3] = {
		{192, 16, 208, 32},
		{208, 16, 224, 32},
		{224, 16, 240, 32},
	};

	bul->rect = rect[bul->ani_no];

	if (level == 2)
		SetNpChar(129, bul->x, bul->y, 0, -0x200, bul->ani_no, NULL, 0x100);
	else
		SetNpChar(129, bul->x, bul->y, 0, -0x200, bul->ani_no + 3, NULL, 0x100);
}

void ActBullet_PoleStar(BULLET *bul, int level)
{
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		// Set speed
		switch (bul->direct)
		{
			case 0:
				bul->xm = -0x1000;
				break;
			case 1:
				bul->ym = -0x1000;
				break;
			case 2:
				bul->xm = 0x1000;
				break;
			case 3:
				bul->ym = 0x1000;
				break;
		}

		// Set hitbox
		switch (level)
		{
			case 1:
				switch (bul->direct)
				{
					case 0:
						bul->enemyYL = 0x400;
						break;
					case 1:
						bul->enemyXL = 0x400;
						break;
					case 2:
						bul->enemyYL = 0x400;
						break;
					case 3:
						bul->enemyXL = 0x400;
						break;
				}
				break;
			case 2:
				switch (bul->direct)
				{
					case 0:
						bul->enemyYL = 0x800;
						break;
					case 1:
						bul->enemyXL = 0x800;
						break;
					case 2:
						bul->enemyYL = 0x800;
						break;
					case 3:
						bul->enemyXL = 0x800;
						break;
				}
				break;
		}
	}
	else
	{
		// Move
		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	RECT rect1[2] = {
		{128, 32, 144, 48},
		{144, 32, 160, 48},
	};

	RECT rect2[2] = {
		{160, 32, 176, 48},
		{176, 32, 192, 48},
	};

	RECT rect3[2] = {
		{128, 48, 144, 64},
		{144, 48, 160, 64},
	};

	//Set framerect
	switch (level)
	{
		case 1:
			if (bul->direct == 1 || bul->direct == 3)
				bul->rect = rect1[1];
			else
				bul->rect = rect1[0];

			break;

		case 2:
			if (bul->direct == 1 || bul->direct == 3)
				bul->rect = rect2[1];
			else
				bul->rect = rect2[0];

			break;

		case 3:
			if (bul->direct == 1 || bul->direct == 3)
				bul->rect = rect3[1];
			else
				bul->rect = rect3[0];

			break;
	}
}
#include "Map.h"
#include "Flags.h"
#include "ArmsItem.h"


void ActBullet_FireBall(BULLET *bul, int level)
{
	BOOL bBreak;

	// The mod dynamically fetches the Fireball's level (Weapon ID 3) directly 
	// from the inventory, ignoring the 'level' parameter.
	int real_level = 0;
	for (int i = 0; i < ARMS_MAX; ++i)
	{
		if (gArmsData[i].code == 3)
		{
			real_level = gArmsData[i].level;
			break;
		}
	}

	bul->damage = real_level + 1;
	bul->count1 += (4 - real_level);

	if (bul->count1 > bul->life_count || bul->life <= 1 || !(bul->cond & 0x80))
	{
		if (bul->act_no == 2)
		{
			PlaySoundObject(70, SOUND_MODE_PLAY); // 0x46 (SND_LITTLE_CRASH)
			
			// FIXED: Uses Caret 12 (Flash/Star) instead of 2 (Dissipate Puff)
			SetCaret(bul->x, bul->y, 12, 1); 
			bul->cond = 0;
			return;
		}

		bul->act_no = 2; // Transition to death animation on the next frame
		
		// Mod Flag 243: Spawn a level 3 Fireball explosion (Bullet 9) on death
		if (GetNPCFlag(243))
			SetBullet(9, bul->x, bul->y, DIR_RIGHT);

		return;
	}

	// Check environment attribute at current position
	unsigned char atrb = GetAttribute(bul->x / 0x10 / 0x200, bul->y / 0x10 / 0x200);
	
	if (atrb == 0x03 || (atrb >= 0x60 && atrb <= 0x62) || (atrb >= 0x70 && atrb <= 0x77) || (atrb >= 0xA0 && atrb <= 0xA3))
	{
		bul->count1 += 6;
		bul->act_no = 2; // Hit a wall, trigger dissipation
	}

	bBreak = FALSE;
	if (bul->flag & COLL_CEILING && bul->flag & COLL_GROUND)
		bBreak = TRUE;
	if (bul->flag & COLL_LEFT_WALL && bul->flag & COLL_RIGHT_WALL)
		bBreak = TRUE;

	// Bounce off walls
	if (bul->direct == DIR_LEFT && bul->flag & COLL_LEFT_WALL)
		bul->direct = DIR_RIGHT;
	if (bul->direct == DIR_RIGHT && bul->flag & COLL_RIGHT_WALL)
		bul->direct = DIR_LEFT;

	// Anti-stuck behavior
	if (bBreak)
	{
		bul->y += 0x1000;
		bul->ym = -bul->ym;
	}

	// Initialization
	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		switch (bul->direct)
		{
			case DIR_LEFT:
				bul->xm = -0x500;
				break;

			case DIR_UP:
				bul->xm = gMC.xm;
				if (gMC.xm < 0)
					bul->direct = DIR_LEFT;
				else
					bul->direct = DIR_RIGHT;

				if (gMC.direct == DIR_LEFT)
					bul->xm -= 0x80;
				else
					bul->xm += 0x80;

				bul->ym = -0x900;
				break;

			case DIR_RIGHT:
				bul->xm = 0x500;
				break;

			case DIR_DOWN:
				bul->xm = gMC.xm;
				if (gMC.xm < 0)
					bul->direct = DIR_LEFT;
				else
					bul->direct = DIR_RIGHT;

				bul->ym = 0x600;
				break;
		}
	}
	else
	{
		// Physics
		if (bul->flag & COLL_GROUND)
			bul->ym = -0x4B0;
		else if (bul->flag & COLL_LEFT_WALL)
			bul->xm = 0x600;
		else if (bul->flag & COLL_RIGHT_WALL)
			bul->xm = -0x600;
		else if (bul->flag & COLL_CEILING)
			bul->ym = 0x4B0;

		bul->ym += 0x55;
		if (bul->ym > 0x4B0)
			bul->ym = 0x4B0;

		bul->x += bul->xm;
		bul->y += bul->ym;

		if (bul->flag & 0x0D) // COLL_GROUND | COLL_LEFT_WALL | COLL_RIGHT_WALL
			PlaySoundObject(34, SOUND_MODE_PLAY); // SND_FIRE_BALL_BOUNCE (0x22)
	}

	// Mod Flag 242: Meteor Storm (Spawns Bullet 8)
	if (GetNPCFlag(242))
	{
		++bul->count2;
		int limit;

		if (real_level == 3)
			limit = 7;
		else if (real_level == 2)
			limit = 9;
		else
			limit = 11;

		if (bul->count2 >= limit)
		{
			bul->count2 = 0;
			
			// Only the X-axis is randomized in the assembly for the meteor spawn.
			int rand_x = Random(-0x500, 0x500);
			SetBullet(8, bul->x + rand_x, bul->y - 0xA00, DIR_LEFT);
		}
	}

	// FIXED: The EXACT array values dumped directly from the raw assembly registers!
	// These frame rectangles point to custom regions of the modder's Bullet.bmp file.
	RECT rect_left[3] = {
		{144, 16, 160, 32},
		{128, 16, 144, 32},
		{176, 0, 192, 16},
	};

	RECT rect_right[3] = {
		{160, 0, 176, 16},
		{144, 0, 160, 16},
		{128, 0, 144, 16},
	};

	if (++bul->ani_no > 2)
		bul->ani_no = 0;

	if (bul->direct == DIR_LEFT)
		bul->rect = rect_left[bul->ani_no];
	else
		bul->rect = rect_right[bul->ani_no];

	// Trail spawner (NPC 129)
	if (bul->xm <= 0)
		SetNpChar(129, bul->x, bul->y, 0, -0x200, bul->ani_no, NULL, 0x100);
	else
		SetNpChar(129, bul->x, bul->y, 0, -0x20, bul->ani_no + 3, NULL, 0x100);

	// Mod Flag 243: Sprite bounds shifting
	if (GetNPCFlag(243))
	{
		bul->rect.left -= 8;
		bul->rect.right -= 8;
		bul->rect.top += 0x60;
		bul->rect.bottom += 0x60;
	}
}

#include "Frame.h"

void ActBullet_Missile(BULLET *bul, int level)
{
	bool bHit = false;

	// Update lifespan counter
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	// MOD: Explosion Trigger Logic
	// Explode if the 'life' timer (offset 0x5C) is below 90 (0x5A)
	if (bul->life < 90)
		bHit = true;

	// Explode on wall collision (Standard Flags)
	if (bul->direct == 0 && (bul->flag & 0x01)) bHit = true; // Left
	if (bul->direct == 1 && (bul->flag & 0x02)) bHit = true; // Up
	if (bul->direct == 2 && (bul->flag & 0x04)) bHit = true; // Right
	if (bul->direct == 3 && (bul->flag & 0x08)) bHit = true; // Down

	// MOD: Explode on special collision flags (NPCs or custom tiles)
	if (bul->direct == 0 && (bul->flag & 0x80)) bHit = true;
	if (bul->direct == 0 && (bul->flag & 0x20)) bHit = true;
	if (bul->direct == 2 && (bul->flag & 0x40)) bHit = true;
	if (bul->direct == 2 && (bul->flag & 0x10)) bHit = true;

	if (bHit)
	{
		// Spawn the appropriate explosion (Bullet IDs 16, 17, or 18)
		SetBullet(level + 15, bul->x, bul->y, 0);
		bul->cond = 0;
		return; // Exit immediately on explosion
	}

	// Action 0: Initialization
	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		// Store initial axis to calculate the "swerve" or "wobble"
		switch (bul->direct)
		{
			case 0:
			case 2:
				bul->tgt_y = bul->y;
				break;
			case 1:
			case 3:
				bul->tgt_x = bul->x;
				break;
		}

		if (level == 3)
		{
			// Level 3 unique movement patterns
			switch (bul->direct)
			{
				case 0: // Left
				case 2: // Right
					bul->ym = (gMC.y < bul->y) ? 0x100 : -0x100;
					bul->xm = Random(-0x400, 0x400);
					break;
				case 1: // Up
				case 3: // Down
					bul->xm = (gMC.x < bul->x) ? 0x120 : -0x120;
					bul->ym = Random(-0x150, 0x500);
					break;
			}

			// MOD: Acceleration variability
			// Uses a global counter (0x49BC9C) to determine this specific missile's "weight"
			static int gMissileSwerveCycle = 0;
			gMissileSwerveCycle++;
			
			int cycle = gMissileSwerveCycle % 3;
			if (cycle == 0)      bul->count2 = 0x80; // Fast acceleration
			else if (cycle == 1) bul->count2 = 0x40; // Medium acceleration
			else                 bul->count2 = 0x33; // Slow/Heavy acceleration
		}
		else
		{
			bul->count2 = 0x80; // Standard acceleration for Lv1 and Lv2
		}
	}
	else if (bul->act_no == 1)
	{
		// Apply thrust based on acceleration value (stored in count2)
		switch (bul->direct)
		{
			case 0: bul->xm -= bul->count2; break;
			case 1: bul->ym -= bul->count2; break;
			case 2: bul->xm += bul->count2; break;
			case 3: bul->ym += bul->count2; break;
		}

		// Level 3 Steering logic (Steer back toward the original axis)
		if (level == 3)
		{
			if (bul->direct == 0 || bul->direct == 2)
			{
				if (bul->y < bul->tgt_y) bul->ym += 0x15;
				else                     bul->ym -= 0x15;
			}
			else
			{
				if (bul->x < bul->tgt_x) bul->xm += 0x15;
				else                     bul->xm -= 0x15;
			}
		}

		// Apply speed caps (8.0 pixels per frame)
		if (bul->xm < -0x1000) bul->xm = -0x1000;
		if (bul->xm >  0x1000) bul->xm =  0x1000;
		if (bul->ym < -0x1000) bul->ym = -0x1000;
		if (bul->ym >  0x1000) bul->ym =  0x1000;

		// Update position
		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	// Smoke Trail: Generate a caret every 3 frames
	if (++bul->ani_wait > 2)
	{
		bul->ani_wait = 0;
		switch (bul->direct)
		{
			case 0: SetCaret(bul->x + 0x1000, bul->y, 7, 2); break;
			case 1: SetCaret(bul->x, bul->y + 0x1000, 7, 3); break;
			case 2: SetCaret(bul->x - 0x1000, bul->y, 7, 0); break;
			case 3: SetCaret(bul->x, bul->y - 0x1000, 7, 1); break;
		}
	}

	// Animation Frames (RECT definitions)
	static const RECT rect_lv1[4] = {
		{0, 0, 16, 16}, {16, 0, 32, 16}, {32, 0, 48, 16}, {48, 0, 64, 16}
	};
	static const RECT rect_lv2[4] = {
		{0, 16, 16, 32}, {16, 16, 32, 32}, {32, 16, 48, 32}, {48, 16, 64, 32}
	};
	static const RECT rect_lv3[4] = {
		{0, 32, 16, 48}, {16, 32, 32, 48}, {32, 32, 48, 48}, {48, 32, 64, 48}
	};

	// Set frame based on level and direction
	if (level == 1)      bul->rect = rect_lv1[bul->direct];
	else if (level == 2) bul->rect = rect_lv2[bul->direct];
	else if (level == 3) bul->rect = rect_lv3[bul->direct];
}

void ActBullet_Bom(BULLET *bul, int level)
{
	// Initialization
	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		if (level == 1)
		{
			bul->count1 = 10;
			PlaySoundObject(44, SOUND_MODE_PLAY);
		}
		else if (level == 2)
		{
			bul->count1 = 10;
			PlaySoundObject(35, SOUND_MODE_PLAY);
			// MOD: Level 2 Missile explosions now cause screen quake
			SetQuake(Random(14, 20)); 
			PlaySoundObject(115, SOUND_MODE_PLAY);
		}
		else // Level 3
		{
			bul->count1 = 5;
			PlaySoundObject(72, SOUND_MODE_PLAY);
		}
	}

	// Spawn particle effects based on level density
	if (level == 1)
	{
		if (bul->count1 % 3 == 0)
		{
			int off_x = Random(-16, 16) * 0x200;
			int off_y = Random(-16, 16) * 0x200;
			SetDestroyNpCharUp(bul->x + off_x, bul->y + off_y, bul->enemyXL, 5);
		}
	}
	else if (level == 2)
	{
		if (bul->count1 % 2 == 0)
		{
			int off_x = Random(-32, 32) * 0x200;
			int off_y = Random(-32, 32) * 0x200;
			SetDestroyNpCharUp(bul->x + off_x, bul->y + off_y, bul->enemyXL, 10);
		}
	}
	else if (level == 3)
	{
		if (bul->count1 % 4 == 0)
		{
			int off_x = Random(-5, 5) * 0x200;
			int off_y = Random(-5, 5) * 0x200;
			SetDestroyNpCharUp(bul->x + off_x, bul->y + off_y, bul->enemyXL, 2);
		}
	}

	// Countdown and despawn
	if (--bul->count1 < 0)
	{
		bul->cond = 0;
	}
}

void ActBullet_Bubblin1(BULLET *bul)
{
	if (bul->flag & 0x2FF)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 2, 0);
		return;
	}

	switch (bul->act_no)
	{
		case 0:
			bul->act_no = 1;

			switch (bul->direct)
			{
				case 0:
					bul->xm = -0x600;
					break;
				case 2:
					bul->xm = 0x600;
					break;
				case 1:
					bul->ym = -0x600;
					break;
				case 3:
					bul->ym = 0x600;
					break;
			}

			break;
	}

	switch (bul->direct)
	{
		case 0:
			bul->xm += 0x2A;
			break;
		case 2:
			bul->xm -= 0x2A;
			break;
		case 1:
			bul->ym += 0x2A;
			break;
		case 3:
			bul->ym -= 0x2A;
			break;
	}

	bul->x += bul->xm;
	bul->y += bul->ym;

	if (++bul->act_wait > 40)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 15, 0);
	}

	RECT rect[4] = {
		{192, 0, 200, 8},
		{200, 0, 208, 8},
		{208, 0, 216, 8},
		{216, 0, 224, 8},
	};

	if (++bul->ani_wait > 3)
	{
		bul->ani_wait = 0;
		++bul->ani_no;
	}

	if (bul->ani_no > 3)
		bul->ani_no = 3;

	bul->rect = rect[bul->ani_no];
}

void ActBullet_Bubblin2(BULLET *bul)
{
	BOOL bDelete = FALSE;

	if (bul->direct == 0 && bul->flag & 1)
		bDelete = TRUE;
	if (bul->direct == 2 && bul->flag & 4)
		bDelete = TRUE;
	if (bul->direct == 1 && bul->flag & 2)
		bDelete = TRUE;
	if (bul->direct == 3 && bul->flag & 8)
		bDelete = TRUE;

	if (bDelete)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 2, 0);
		return;
	}

	switch (bul->act_no)
	{
		case 0:
			bul->act_no = 1;

			switch (bul->direct)
			{
				case 0:
					bul->xm = -0x600;
					bul->ym = Random(-0x100, 0x100);
					break;
				case 2:
					bul->xm = 0x600;
					bul->ym = Random(-0x100, 0x100);
					break;
				case 1:
					bul->ym = -0x600;
					bul->xm = Random(-0x100, 0x100);
					break;
				case 3:
					bul->ym = 0x600;
					bul->xm = Random(-0x100, 0x100);
					break;
			}

			break;
	}

	switch (bul->direct)
	{
		case 0:
			bul->xm += 0x10;
			break;
		case 2:
			bul->xm -= 0x10;
			break;
		case 1:
			bul->ym += 0x10;
			break;
		case 3:
			bul->ym -= 0x10;
			break;
	}

	bul->x += bul->xm;
	bul->y += bul->ym;

	if (++bul->act_wait > 60)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 15, 0);
	}

	RECT rect[4] = {
		{192, 8, 200, 16},
		{200, 8, 208, 16},
		{208, 8, 216, 16},
		{216, 8, 224, 16},
	};

	if (++bul->ani_wait > 3)
	{
		bul->ani_wait = 0;
		++bul->ani_no;
	}

	if (bul->ani_no > 3)
		bul->ani_no = 3;

	bul->rect = rect[bul->ani_no];
}

void ActBullet_Bubblin3(BULLET *bul)
{
	if (++bul->act_wait > 100 || !(gKey & gKeyShot))
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 2, 0);
		PlaySoundObject(100, SOUND_MODE_PLAY);

		if (gMC.up)
			SetBullet(22, bul->x, bul->y, 1);
		else if (gMC.down)
			SetBullet(22, bul->x, bul->y, 3);
		else
			SetBullet(22, bul->x, bul->y, gMC.direct);

		return;
	}

	switch (bul->act_no)
	{
		case 0:
			bul->act_no = 1;

			switch (bul->direct)
			{
				case 0:
					bul->xm = Random(-0x400, -0x200);
					bul->ym = (Random(-4, 4) * 0x200) / 2;
					break;
				case 2:
					bul->xm = Random(0x200, 0x400);
					bul->ym = (Random(-4, 4) * 0x200) / 2;
					break;
				case 1:
					bul->ym = Random(-0x400, -0x200);
					bul->xm = (Random(-4, 4) * 0x200) / 2;
					break;
				case 3:
					bul->ym = Random(0x80, 0x100);
					bul->xm = (Random(-4, 4) * 0x200) / 2;
					break;
			}

			break;
	}

	if (bul->x < gMC.x)
		bul->xm += 0x20;
	if (bul->x > gMC.x)
		bul->xm -= 0x20;

	if (bul->y < gMC.y)
		bul->ym += 0x20;
	if (bul->y > gMC.y)
		bul->ym -= 0x20;

	if (bul->xm < 0 && bul->flag & 1)
		bul->xm = 0x400;
	if (bul->xm > 0 && bul->flag & 4)
		bul->xm = -0x400;

	if (bul->ym < 0 && bul->flag & 2)
		bul->ym = 0x400;
	if (bul->ym > 0 && bul->flag & 8)
		bul->ym = -0x400;

	bul->x += bul->xm;
	bul->y += bul->ym;

	RECT rect[4] = {
		{240, 16, 248, 24},
		{248, 16, 256, 24},
		{240, 24, 248, 32},
		{248, 24, 256, 32},
	};

	if (++bul->ani_wait > 3)
	{
		bul->ani_wait = 0;
		++bul->ani_no;
	}

	if (bul->ani_no > 3)
		bul->ani_no = 3;

	bul->rect = rect[bul->ani_no];
}

void ActBullet_Spine(BULLET *bul)
{
	if (++bul->count1 > bul->life_count || bul->flag & 8)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		switch (bul->direct)
		{
			case 0:
				bul->xm = (-Random(10, 16) * 0x200) / 2;
				break;
			case 1:
				bul->ym = (-Random(10, 16) * 0x200) / 2;
				break;
			case 2:
				bul->xm = (Random(10, 16) * 0x200) / 2;
				break;
			case 3:
				bul->ym = (Random(10, 16) * 0x200) / 2;
				break;
		}
	}
	else
	{
		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	if (++bul->ani_wait > 1)
	{
		bul->ani_wait = 0;
		++bul->ani_no;
	}

	if (bul->ani_no > 1)
		bul->ani_no = 0;

	RECT rcLeft[2] = {
		{224, 0, 232, 8},
		{232, 0, 240, 8},
	};

	RECT rcRight[2] = {
		{224, 0, 232, 8},
		{232, 0, 240, 8},
	};

	RECT rcDown[2] = {
		{224, 8, 232, 16},
		{232, 8, 240, 16},
	};

	switch (bul->direct)
	{
		case 0:
			bul->rect = rcLeft[bul->ani_no];
			break;
		case 1:
			bul->rect = rcDown[bul->ani_no];
			break;
		case 2:
			bul->rect = rcRight[bul->ani_no];
			break;
		case 3:
			bul->rect = rcDown[bul->ani_no];
			break;
	}
}
void ActBullet_Sword1(BULLET *bul)
{
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, CARET_SHOOT, DIR_LEFT);
		return;
	}

	// Mod: After 3 frames, the blade can pass through solid walls
	if (bul->count1 == 3)
		bul->bbits &= ~NPC_SOLID_HARD;

	// Play swoosh sound
	if (bul->count1 % 5 == 1)
		PlaySoundObject(34, SOUND_MODE_PLAY); // 0x22

	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		switch (bul->direct)
		{
			case DIR_LEFT:  bul->xm = -0x800; break;
			case DIR_UP:    bul->ym = -0x800; break;
			case DIR_RIGHT: bul->xm =  0x800; break;
			case DIR_DOWN:  bul->ym =  0x800; break;
		}
	}
	else
	{
		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	// Restored RECTs from the assembly's local variables
	static const RECT rect_l[] = {
		{0, 48, 16, 64}, {16, 48, 32, 64}, {32, 48, 48, 64}, {48, 48, 64, 64}
	};
	static const RECT rect_r[] = {
		{64, 48, 80, 64}, {80, 48, 96, 64}, {96, 48, 112, 64}, {112, 48, 128, 64}
	};

	if (++bul->ani_wait > 1)
	{
		bul->ani_wait = 0;
		if (++bul->ani_no > 3)
			bul->ani_no = 0;
	}

	if (bul->direct == DIR_LEFT)
		bul->rect = rect_l[bul->ani_no];
	else
		bul->rect = rect_r[bul->ani_no];
}
void ActBullet_Sword2(BULLET *bul)
{
    int bul_x = bul->x;
    int bul_y = bul->y;
    int mc_x  = gMC.x;
    int mc_y  = gMC.y;

    if (bul->act_no != 2)
    {
        if (bul->act_no == 0)
        {
            bul->act_wait = 0;
            bul->act_no   = 1;

            int level = 0;
			for (int i = 0; i < ARMS_MAX; ++i)
			if (gArmsData[i].code == 9) { level = gArmsData[i].level; break;  }

            bul->life_count = (level * 25 + 25) * 2;
            bul->damage     = (level + 3) / 2;

            bul->xm = 0;
            bul->ym = 0;
            switch (bul->direct)
            {
                case 0: bul->xm = -0x800; break;
                case 1: bul->ym = -0x800; break;
                case 2: bul->xm =  0x800; break;
                case 3: bul->ym =  0x800; break;
            }
            goto move;
        }

        // act_no == 1: check wall collision
        if (bul->flag & (0x1 | 0x2 | 0x4 | 0x8))
        {
            SetCaret(bul->x + bul->ym, bul->y + bul->xm, 3, 0);
            PlaySoundObject(0x6d, SOUND_MODE_PLAY);
            // fall through to act_no = 2
        }
        else if (bul->count1 < bul->life_count)
        {
            bul->count1++;

            if (gKey & KEY_MAP)
            {
                if (gKey & KEY_MAP)
                {
                    int steer_x = bul->xm;
                    int steer_y = bul->ym;

                    if (gKey & KEY_LEFT)  steer_x -= 0x400;
                    if (gKey & KEY_UP)    steer_y -= 0x400;
                    if (gKey & KEY_RIGHT) steer_x += 0x400;
                    if (gKey & KEY_DOWN)  steer_y += 0x400;

                    bul->xm /= 2;
                    bul->ym /= 2;

                    steer_x += bul->xm;
                    steer_y += bul->ym;

                    if (steer_x >  0x600) steer_x =  0x600;
                    if (steer_x < -0x600) steer_x = -0x600;
                    if (steer_y >  0x600) steer_y =  0x600;
                    if (steer_y < -0x600) steer_y = -0x600;

                    bul->xm = steer_x;
                    bul->ym = steer_y;
                }
                goto move;
            }
            else if ((gKeyTrg & gKeyShot)
                     && gArmsData[gSelectedArms].code == 9)
            {
                // fall through to act_no = 2
            }
            else
            {
                goto move;
            }
        }
        // count1 >= life_count: fall through to act_no = 2

        bul->act_no = 2;
        // NOPs were here — original set bbits to ignore walls during return
        // Restore: set the ignore-wall flag so homing phase passes through geometry
        bul->bbits |= 0x4;
    }

    // act_no == 2: homing
    {
        bul->count1 = 0x10;

        int dist_x = bul_x - mc_x;
        int dist_y = bul_y - mc_y;
        if (dist_x < 0) dist_x = -dist_x;
        if (dist_y < 0) dist_y = -dist_y;

        if (((dist_x + dist_y) >> 1) <= 0x1000)
        {
            bul->cond = 0;
            SetCaret(bul->x, bul->y, 4, 0);
            return;
        }

        bul->xm /= 2;
        bul->ym /= 2;

        int hx = (dist_x > 0x400) ? 0x400 : (dist_x < 0) ? 0 : dist_x;
        int hy = (dist_y > 0x400) ? 0x400 : (dist_y < 0) ? 0 : dist_y;

        if (mc_x > bul_x) bul->xm += hx; else bul->xm -= hx;
        if (mc_y > bul_y) bul->ym += hy; else bul->ym -= hy;
    }

move:
    bul->act_wait++;
    if (bul->act_wait >= 7)
    {
        bul->act_wait = 0;
        PlaySoundObject(0x6e, SOUND_MODE_PLAY);
    }

    bul->x += bul->xm;
    bul->y += bul->ym;

    static const RECT rect_left[4] = {
        {0xA0, 0x30, 0xB8, 0x48},
        {0xB8, 0x30, 0xD0, 0x48},
        {0xD0, 0x30, 0xE8, 0x48},
        {0xE8, 0x30, 0x00, 0x48},
    };
    static const RECT rect_right[4] = {
        {0xA0, 0x48, 0xB8, 0x60},
        {0xB8, 0x48, 0xD0, 0x60},
        {0xD0, 0x48, 0xE8, 0x60},
        {0xE8, 0x48, 0x00, 0x60},
    };

    bul->ani_wait++;
    if (bul->ani_wait > 1)
    {
        bul->ani_wait = 0;
        bul->ani_no++;
    }
    if (bul->ani_no > 3) bul->ani_no = 0;

    if (bul->direct == 0)
        bul->rect = rect_left[bul->ani_no];
    else
        bul->rect = rect_right[bul->ani_no];
}

void ActBullet_Sword3(BULLET *bul)
{
	RECT rcLeft[2] = {
		{272, 0, 296, 24},
		{296, 0, 320, 24},
	};

	RECT rcUp[2] = {
		{272, 48, 296, 72},
		{296, 0, 320, 24},
	};

	RECT rcRight[2] = {
		{272, 24, 296, 48},
		{296, 24, 320, 48},
	};

	RECT rcDown[2] = {
		{296, 48, 320, 72},
		{296, 24, 320, 48},
	};

	switch (bul->act_no)
	{
		case 0:
			bul->act_no = 1;
			bul->xm = 0;
			bul->ym = 0;
			// Fallthrough
		case 1:
			switch (bul->direct)
			{
				case 0:
					bul->xm = -0x800;
					break;
				case 1:
					bul->ym = -0x800;
					break;
				case 2:
					bul->xm = 0x800;
					break;
				case 3:
					bul->ym = 0x800;
					break;
			}

			if (bul->life != 100)
			{
				bul->act_no = 2;
				bul->ani_no = 1;
				bul->damage = -1;
				bul->act_wait = 0;
			}

			if (++bul->act_wait % 4 == 1)
			{
				PlaySoundObject(106, SOUND_MODE_PLAY);

				if (++bul->count1 % 2)
					SetBullet(23, bul->x, bul->y, 0);
				else
					SetBullet(23, bul->x, bul->y, 2);
			}

			if (++bul->count1 == 5)
				bul->bbits &= ~4;

			if (bul->count1 > bul->life_count)
			{
				bul->cond = 0;
				SetCaret(bul->x, bul->y, 3, 0);
				return;
			}

			break;

		case 2:
			bul->xm = 0;
			bul->ym = 0;
			++bul->act_wait;

			if (Random(-1, 1) == 0)
			{
				PlaySoundObject(106, SOUND_MODE_PLAY);

				if (Random(0, 1) % 2)
					SetBullet(23, bul->x + (Random(-0x40, 0x40) * 0x200), bul->y + (Random(-0x40, 0x40) * 0x200), 0);
				else
					SetBullet(23, bul->x + (Random(-0x40, 0x40) * 0x200), bul->y + (Random(-0x40, 0x40) * 0x200), 2);
			}

			if (bul->act_wait > 50)
				bul->cond = 0;
	}

	bul->x += bul->xm;
	bul->y += bul->ym;

	switch (bul->direct)
	{
		case 0:
			bul->rect = rcLeft[bul->ani_no];
			break;
		case 1:
			bul->rect = rcUp[bul->ani_no];
			break;
		case 2:
			bul->rect = rcRight[bul->ani_no];
			break;
		case 3:
			bul->rect = rcDown[bul->ani_no];
			break;
	}

	if (bul->act_wait % 2)
		bul->rect.right = 0;
}


// Global toggle used to alternate slash animations/positions 
unsigned char gSwordState; 


void ActBullet_Edge(BULLET *bul)
{
	// Initialization
	if (bul->act_no != 1)
	{
		bul->act_no = 1;
		bul->act_wait = 4; // Damage frequency timer
		
		// Alternates the sword "state" toggle every time a new slash is created
		if (gSwordState == 0)
			gSwordState = 1;
		else
			gSwordState = 0;
	}

	// Pin the bullet to Quote's current position
	bul->x = gMC.x;
	bul->y = gMC.y;

	// Position the slash in front of Quote based on aiming direction
	if (gMC.down)
	{
		// Aiming Down
		if (gMC.direct == DIR_LEFT)
		{
			bul->x -= 0x1000; // 8 pixels left
			bul->y += 0x2200; // 17 pixels down
		}
		else
		{
			bul->x += 0x1000; // 8 pixels right
			bul->y += 0x2800; // 20 pixels down
		}
	}
	else if (gMC.up)
	{
		// Aiming Up
		if (gMC.direct == DIR_LEFT)
		{
			bul->x -= 0x1400; // 10 pixels left
			bul->y -= 0x2200; // 17 pixels up
		}
		else
		{
			bul->x += 0x1400; // 10 pixels right
			bul->y -= 0x2800; // 20 pixels up
		}
	}
	else
	{
		// Aiming Horizontally
		if (gMC.direct == DIR_LEFT)
		{
			bul->x -= 0x2000; // 16 pixels left
			bul->y -= 0x400;  // 2 pixels up
		}
		else
		{
			bul->x += 0x2000; // 16 pixels right
			bul->y -= 0x400;  // 2 pixels up
		}
	}

	// Logic: Animation Speed scaling by Weapon Level (Weapon ID 9)
	bul->count1++; 
	int level = 0;
	for (int i = 0; i < ARMS_MAX; ++i)
		if (gArmsData[i].code == 9) { level = gArmsData[i].level; break; }

	// Assembly logic: (3 - level) < count1 determines frame advance speed
	// Level 3 slashes move frames much faster than Level 1.
	if ((3 - level) < bul->count1)
	{
		bul->count1 = 0;
		bul->ani_no++;
	}

	// Logic: Damage / Hitbox frequency
	if (--bul->act_wait == 0)
	{
		if (bul->damage != 0)
		{
			bul->act_wait = 4;
			bul->damage = 0;
		}
		else
		{
			// Damage scaling formula from ASM: (level * level) + 8
			bul->damage = (level * level) + 8;
			bul->act_wait = 1;
		}
	}
	else
	{
		bul->damage = 0;
	}

	// Destroy slash after 5 frames of animation
	if (bul->ani_no > 4)
	{
		bul->cond = 0;
		return;
	}

	// Correct RECT Tables (Restored from assembly stack logic)
	// These frames are 24x24 pixels wide.
	static const RECT rcLeft[] = {
		{0, 64, 24, 88}, {24, 64, 48, 88}, {48, 64, 72, 88}, {72, 64, 96, 88}, {96, 64, 120, 88}
	};
	static const RECT rcRight[] = {
		{0, 88, 24, 112}, {24, 88, 48, 112}, {48, 88, 72, 112}, {72, 88, 96, 112}, {96, 88, 120, 112}
	};
	static const RECT rcUp[] = {
		{0, 112, 24, 136}, {24, 112, 48, 136}, {48, 112, 72, 136}, {72, 112, 96, 136}, {96, 112, 120, 136}
	};
	static const RECT rcDown[] = {
		{0, 136, 24, 160}, {24, 136, 48, 160}, {48, 136, 72, 160}, {72, 136, 96, 160}, {96, 136, 120, 160}
	};

	// Determine which RECT set to use based on aiming and gSwordState
	if (gMC.down)
	{
		bul->rect = (gMC.direct == DIR_LEFT) ? rcDown[bul->ani_no] : rcDown[bul->ani_no];
		// Assembly note: Downwards slashes use the rcDown set directly
	}
	else if (!gMC.up)
	{
		// Horizontal: Alternates between High and Low slash animations
		if (gMC.direct == DIR_LEFT)
			bul->rect = (gSwordState != 0) ? rcDown[bul->ani_no] : rcLeft[bul->ani_no];
		else
			bul->rect = (gSwordState != 0) ? rcDown[bul->ani_no] : rcRight[bul->ani_no];
	}
	else
	{
		// Upwards slashes
		bul->rect = rcUp[bul->ani_no];
	}
}

void ActBullet_Drop(BULLET *bul)
{
	RECT rc = {0, 0, 0, 0};

	if (++bul->act_wait > 2)
		bul->cond = 0;

	bul->rect = rc;
}

void ActBullet_SuperMissile(BULLET *bul, int level)
{
	BOOL bHit;

	static unsigned int inc;

	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	bHit = FALSE;

	if (bul->life != 10)
		bHit = TRUE;
	if (bul->direct == 0 && bul->flag & 1)
		bHit = TRUE;
	if (bul->direct == 2 && bul->flag & 4)
		bHit = TRUE;
	if (bul->direct == 1 && bul->flag & 2)
		bHit = TRUE;
	if (bul->direct == 3 && bul->flag & 8)
		bHit = TRUE;
	if (bul->direct == 0 && bul->flag & 0x80)
		bHit = TRUE;
	if (bul->direct == 0 && bul->flag & 0x20)
		bHit = TRUE;
	if (bul->direct == 2 && bul->flag & 0x40)
		bHit = TRUE;
	if (bul->direct == 2 && bul->flag & 0x10)
		bHit = TRUE;

	if (bHit)
	{
		SetBullet(level + 30, bul->x, bul->y, 0);
		bul->cond = 0;
	}

	switch (bul->act_no)
	{
		case 0:
			bul->act_no = 1;

			switch (bul->direct)
			{
				case 0:
				case 2:
					bul->tgt_y = bul->y;
					bul->enemyXL = 0x1000;
					bul->blockXL = 0x1000;
					break;

				case 1:
				case 3:
					bul->tgt_x = bul->x;
					bul->enemyYL = 0x1000;
					bul->blockYL = 0x1000;
					break;
			}

			if (level == 3)
			{
				switch (bul->direct)
				{
					case 0:
					case 2:
						if (bul->y > gMC.y)
							bul->ym = 0x100;
						else
							bul->ym = -0x100;

						bul->xm = Random(-0x200, 0x200);
						break;

					case 1:
					case 3:
						if (bul->x > gMC.x)
							bul->xm = 0x100;
						else
							bul->xm = -0x100;

						bul->ym = Random(-0x200, 0x200);
						break;
				}

				switch (++inc % 3)
				{
					case 0:
						bul->ani_no = 0x200;
						break;
					case 1:
						bul->ani_no = 0x100;
						break;
					case 2:
						bul->ani_no = 0xAA;
						break;
				}
			}
			else
			{
				bul->ani_no = 0x200;
			}
			// Fallthrough
		case 1:
			switch (bul->direct)
			{
				case 0:
					bul->xm += -bul->ani_no;
					break;
				case 1:
					bul->ym += -bul->ani_no;
					break;
				case 2:
					bul->xm += bul->ani_no;
					break;
				case 3:
					bul->ym += bul->ani_no;
					break;
			}

			if (level == 3)
			{
				switch (bul->direct)
				{
					case 0:
					case 2:
						if (bul->y < bul->tgt_y)
							bul->ym += 0x40;
						else
							bul->ym -= 0x40;

						break;
					case 1:
					case 3:
						if (bul->x < bul->tgt_x)
							bul->xm += 0x40;
						else
							bul->xm -= 0x40;

						break;
				}
			}

			if (bul->xm < -0x1400)
				bul->xm = -0x1400;
			if (bul->xm > 0x1400)
				bul->xm = 0x1400;

			if (bul->ym < -0x1400)
				bul->ym = -0x1400;
			if (bul->ym > 0x1400)
				bul->ym = 0x1400;

			bul->x += bul->xm;
			bul->y += bul->ym;

			break;
	}

	if (++bul->count2 > 2)
	{
		bul->count2 = 0;

		switch (bul->direct)
		{
			case 0:
				SetCaret(bul->x + (8 * 0x200), bul->y, 7, 2);
				break;
			case 1:
				SetCaret(bul->x, bul->y + (8 * 0x200), 7, 3);
				break;
			case 2:
				SetCaret(bul->x - (8 * 0x200), bul->y, 7, 0);
				break;
			case 3:
				SetCaret(bul->x, bul->y - (8 * 0x200), 7, 1);
				break;
		}
	}

	RECT rect1[4] = {
		{120, 96, 136, 112},
		{136, 96, 152, 112},
		{152, 96, 168, 112},
		{168, 96, 184, 112},
	};

	RECT rect2[4] = {
		{184, 96, 200, 112},
		{200, 96, 216, 112},
		{216, 96, 232, 112},
		{232, 96, 248, 112},
	};

	switch (level)
	{
		case 1:
			bul->rect = rect1[bul->direct];
			break;
		case 2:
			bul->rect = rect2[bul->direct];
			break;
		case 3:
			bul->rect = rect1[bul->direct];
			break;
	}
}

void ActBullet_SuperBom(BULLET *bul, int level)
{
	switch (bul->act_no)
	{
		case 0:
			bul->act_no = 1;

			switch (level)
			{
				case 1:
					bul->act_wait = 10;
					break;
				case 2:
					bul->act_wait = 14;
					break;
				case 3:
					bul->act_wait = 6;
					break;
			}

			PlaySoundObject(44, SOUND_MODE_PLAY);
			// Fallthrough
		case 1:
			switch (level)
			{
				case 1:
					if (bul->act_wait % 3 == 0)
						SetDestroyNpCharUp(bul->x + (Random(-16, 16) * 0x200), bul->y + (Random(-16, 16) * 0x200), bul->enemyXL, 2);
					break;
				case 2:
					if (bul->act_wait % 3 == 0)
						SetDestroyNpCharUp(bul->x + (Random(-32, 32) * 0x200), bul->y + (Random(-32, 32) * 0x200), bul->enemyXL, 2);
					break;
				case 3:
					if (bul->act_wait % 3 == 0)
						SetDestroyNpCharUp(bul->x + (Random(-40, 40) * 0x200), bul->y + (Random(-40, 40) * 0x200), bul->enemyXL, 2);
					break;
			}

			if (--bul->act_wait < 0)
				bul->cond = 0;

			break;
	}
}

void ActBullet_Nemesis(BULLET *bul, int level)
{
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	if (bul->act_no == 0)
	{
		bul->act_no = 1;
		bul->count1 = 0;

		switch (bul->direct)
		{
			case 0:
				bul->xm = -0x1000;
				break;
			case 1:
				bul->ym = -0x1000;
				break;
			case 2:
				bul->xm = 0x1000;
				break;
			case 3:
				bul->ym = 0x1000;
				break;
		}

		switch (level)
		{
			case 3:
				bul->xm /= 3;
				bul->ym /= 3;
				break;
		}
	}
	else
	{
		if (level == 1 && bul->count1 % 4 == 1)
		{
			switch (bul->direct)
			{
				case 0:
					SetNpChar(4, bul->x, bul->y, -0x200, Random(-0x200, 0x200), 2, NULL, 0x100);
					break;
				case 1:
					SetNpChar(4, bul->x, bul->y, Random(-0x200, 0x200), -0x200, 2, NULL, 0x100);
					break;
				case 2:
					SetNpChar(4, bul->x, bul->y, 0x200, Random(-0x200, 0x200), 2, NULL, 0x100);
					break;
				case 3:
					SetNpChar(4, bul->x, bul->y, Random(-0x200, 0x200), 0x200, 2, NULL, 0x100);
					break;
			}
		}

		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	if (++bul->ani_no > 1)
		bul->ani_no = 0;

	RECT rcL[2] = {
		{0, 112, 32, 128},
		{0, 128, 32, 144},
	};

	RECT rcU[2] = {
		{32, 112, 48, 144},
		{48, 112, 64, 144},
	};

	RECT rcR[2] = {
		{64, 112, 96, 128},
		{64, 128, 96, 144},
	};

	RECT rcD[2] = {
		{96, 112, 112, 144},
		{112, 112, 128, 144},
	};

	switch (bul->direct)
	{
		case 0:
			bul->rect = rcL[bul->ani_no];
			break;
		case 1:
			bul->rect = rcU[bul->ani_no];
			break;
		case 2:
			bul->rect = rcR[bul->ani_no];
			break;
		case 3:
			bul->rect = rcD[bul->ani_no];
			break;
	}

	bul->rect.top += ((level - 1) / 2) * 32;
	bul->rect.bottom += ((level - 1) / 2) * 32;
	bul->rect.left += ((level - 1) % 2) * 128;
	bul->rect.right += ((level - 1) % 2) * 128;
}

void ActBullet_Spur(BULLET *bul, int level)
{
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		SetCaret(bul->x, bul->y, 3, 0);
		return;
	}

	if (bul->damage && bul->life != 100)
		bul->damage = 0;

	if (bul->act_no == 0)
	{
		bul->act_no = 1;

		switch (bul->direct)
		{
			case 0:
				bul->xm = -0x1000;
				break;
			case 1:
				bul->ym = -0x1000;
				break;
			case 2:
				bul->xm = 0x1000;
				break;
			case 3:
				bul->ym = 0x1000;
				break;
		}

		switch (level)
		{
			case 1:
				switch (bul->direct)
				{
					case 0:
						bul->enemyYL = 0x400;
						break;
					case 1:
						bul->enemyXL = 0x400;
						break;
					case 2:
						bul->enemyYL = 0x400;
						break;
					case 3:
						bul->enemyXL = 0x400;
						break;
				}

				break;

			case 2:
				switch (bul->direct)
				{
					case 0:
						bul->enemyYL = 0x800;
						break;
					case 1:
						bul->enemyXL = 0x800;
						break;
					case 2:
						bul->enemyYL = 0x800;
						break;
					case 3:
						bul->enemyXL = 0x800;
						break;
				}

				break;
		}
	}
	else
	{
		bul->x += bul->xm;
		bul->y += bul->ym;
	}

	RECT rect1[2] = {
		{128, 32, 144, 48},
		{144, 32, 160, 48},
	};

	RECT rect2[2] = {
		{160, 32, 176, 48},
		{176, 32, 192, 48},
	};

	RECT rect3[2] = {
		{128, 48, 144, 64},
		{144, 48, 160, 64},
	};

	bul->damage = bul->life;

	switch (level)
	{
		case 1:
			if (bul->direct == 1 || bul->direct == 3)
				bul->rect = rect1[1];
			else
				bul->rect = rect1[0];

			break;

		case 2:
			if (bul->direct == 1 || bul->direct == 3)
				bul->rect = rect2[1];
			else
				bul->rect = rect2[0];

			break;

		case 3:
			if (bul->direct == 1 || bul->direct == 3)
				bul->rect = rect3[1];
			else
				bul->rect = rect3[0];

			break;
	}

	SetBullet(39 + level, bul->x, bul->y, bul->direct);
}

void ActBullet_SpurTail(BULLET *bul, int level)
{
	if (++bul->count1 > 20)
		bul->ani_no = bul->count1 - 20;

	if (bul->ani_no > 2)
	{
		bul->cond = 0;
	#ifdef FIX_BUGS
		return;	// Avoid accessing the RECT arrays with an out-of-bounds index
	#endif
	}

	if (bul->damage && bul->life != 100)
		bul->damage = 0;

	RECT rc_h_lv1[3] = {
		{192, 32, 200, 40},
		{200, 32, 208, 40},
		{208, 32, 216, 40},
	};

	RECT rc_v_lv1[3] = {
		{192, 40, 200, 48},
		{200, 40, 208, 48},
		{208, 40, 216, 48},
	};

	RECT rc_h_lv2[3] = {
		{216, 32, 224, 40},
		{224, 32, 232, 40},
		{232, 32, 240, 40},
	};

	RECT rc_v_lv2[3] = {
		{216, 40, 224, 48},
		{224, 40, 232, 48},
		{232, 40, 240, 48},
	};

	RECT rc_h_lv3[3] = {
		{240, 32, 248, 40},
		{248, 32, 256, 40},
		{256, 32, 264, 40},
	};

	RECT rc_v_lv3[3] = {
		{240, 32, 248, 40},
		{248, 32, 256, 40},
		{256, 32, 264, 40},
	};

	switch (level)
	{
		case 1:
			if (bul->direct == 0 || bul->direct == 2)
				bul->rect = rc_h_lv1[bul->ani_no];
			else
				bul->rect = rc_v_lv1[bul->ani_no];

			break;

		case 2:
			if (bul->direct == 0 || bul->direct == 2)
				bul->rect = rc_h_lv2[bul->ani_no];
			else
				bul->rect = rc_v_lv2[bul->ani_no];

			break;

		case 3:
			if (bul->direct == 0 || bul->direct == 2)
				bul->rect = rc_h_lv3[bul->ani_no];
			else
				bul->rect = rc_v_lv3[bul->ani_no];

			break;
	}
}

void ActBullet_EnemyClear(BULLET *bul)
{
	if (++bul->count1 > bul->life_count)
	{
		bul->cond = 0;
		return;
	}

	bul->damage = 10000;
	bul->enemyXL = 0xC8000;
	bul->enemyYL = 0xC8000;
}

void ActBullet_Star(BULLET *bul)
{
	if (++bul->count1 > bul->life_count)
		bul->cond = 0;
}

//-----------------------------------------------------
// Master ActBullet function
//-----------------------------------------------------
void ActBullet(void)
{
	int i;

	for (i = 0; i < BULLET_MAX; ++i)
	{
		if (gBul[i].cond & 0x80)
		{
			if (gBul[i].life < 1)
			{
				gBul[i].cond = 0;
				continue;
			}

			// Map directly to `code_bullet` values defined in gBulTbl
			switch (gBul[i].code_bullet)
			{
				case 1: ActBullet_Frontia1(&gBul[i]); break;
				case 2: ActBullet_Frontia2(&gBul[i], 2); break;
				case 3: ActBullet_Frontia2(&gBul[i], 3); break;

				case 4: ActBullet_PoleStar(&gBul[i], 1); break;
				case 5: ActBullet_PoleStar(&gBul[i], 2); break;
				case 6: ActBullet_PoleStar(&gBul[i], 3); break;
				// Custom Modded Fireball
				case 7: ActBullet_FireBall(&gBul[i], 0); break;
				case 8: ActBullet_FireBall_Meteor(&gBul[i]); break;
				case 9: ActBullet_FireBall_Explosion(&gBul[i]); break;

				// Custom Modded Machine Gun
				case 10: ActBullet_MachineGun(&gBul[i], 1); break;
				case 11: ActBullet_MachineGun(&gBul[i], 2); break;
				case 12: ActBullet_MachineGun(&gBul[i], 3); break;

				case 13: ActBullet_Missile(&gBul[i], 1); break;
				case 14: ActBullet_Missile(&gBul[i], 2); break;
				case 15: ActBullet_Missile(&gBul[i], 3); break;

				case 16: ActBullet_Bom(&gBul[i], 1); break;
				case 17: ActBullet_Bom(&gBul[i], 2); break;
				case 18: ActBullet_Bom(&gBul[i], 3); break;

				// Custom Modded Bubbler
				case 19: ActBullet_CustomBubbler(&gBul[i]); break;
				case 20: ActBullet_CustomBubbler(&gBul[i]); break;
				case 21: ActBullet_CustomBubbler(&gBul[i]); break;

				case 22: ActBullet_Spine(&gBul[i]); break;

				case 23: ActBullet_Edge(&gBul[i]); break;

				// The mod replaces the Drop Spike (24) with Sword Level 1 logic
				case 24: ActBullet_Sword1(&gBul[i]); break; 
				case 25: ActBullet_Sword1(&gBul[i]); break;
				case 26: ActBullet_Sword2(&gBul[i]); break;

				case 31: ActBullet_SuperBom(&gBul[i], 1); break;
				case 32: ActBullet_SuperBom(&gBul[i], 2); break;
				case 33: ActBullet_SuperBom(&gBul[i], 3); break;

				case 34: ActBullet_Nemesis(&gBul[i], 1); break;
				case 35: ActBullet_Nemesis(&gBul[i], 2); break;
				case 36: ActBullet_Nemesis(&gBul[i], 3); break;

				case 37: ActBullet_Spur(&gBul[i], 1); break;
				case 38: ActBullet_Spur(&gBul[i], 2); break;
				case 39: ActBullet_Spur(&gBul[i], 3); break;

				case 40: ActBullet_SpurTail(&gBul[i], 1); break;
				case 41: ActBullet_SpurTail(&gBul[i], 2); break;
				case 42: ActBullet_SpurTail(&gBul[i], 3); break;

				case 43: ActBullet_Nemesis(&gBul[i], 1); break;
				case 44: ActBullet_EnemyClear(&gBul[i]); break;
				case 45: ActBullet_Star(&gBul[i]); break;
			}
		}
	}
}
BOOL IsActiveSomeBullet(void)
{
	int i;

	for (i = 0; i < BULLET_MAX; ++i)
	{
		if (gBul[i].cond & 0x80)
		{
			switch (gBul[i].code_bullet)
			{
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
				case 18:
				case 23:
				case 25:
				case 26:
				case 27:
				case 28:
				case 29:
				case 30:
				case 31:
				case 32:
				case 33:
					return TRUE;
			}
		}
	}

	return FALSE;
}
