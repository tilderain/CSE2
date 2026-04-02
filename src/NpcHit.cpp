#include "NpcHit.h"

#include "WindowsWrapper.h"

#include "Back.h"
#include "Bullet.h"
#include "Caret.h"
#include "Flags.h"
#include "Game.h"
#include "Map.h"
#include "MyChar.h"
#include "NpChar.h"
#include "Sound.h"
#include "TextScr.h"
#include "ValueView.h"

void JadgeHitNpCharBlock(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->y - npc->hit.top < (y * 0x10 + 5) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 5) * 0x200
		&& npc->x - npc->hit.back < (x * 0x10 + 8) * 0x200
		&& npc->x - npc->hit.back > x * 0x10 * 0x200)
	{
		npc->x = ((x * 0x10 + 8) * 0x200) + npc->hit.back;
		hit |= 1;
	}

	if (npc->y - npc->hit.top < (y * 0x10 + 5) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 5) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 8) * 0x200
		&& npc->x + npc->hit.back < x * 0x10 * 0x200)
	{
		npc->x = ((x * 0x10 - 8) * 0x200) - npc->hit.back;
		hit |= 4;
	}

	if (npc->x - npc->hit.back < (x * 0x10 + 5) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 5) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200
		&& npc->y - npc->hit.top > y * 0x10 * 0x200)
	{
		npc->y = ((y * 0x10 + 8) * 0x200) + npc->hit.top;
		npc->ym = 0;
		hit |= 2;
	}

	if (npc->x - npc->hit.back < (x * 0x10 + 5) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 5) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom < y * 0x10 * 0x200)
	{
		npc->y = ((y * 0x10 - 8) * 0x200) - npc->hit.bottom;
		npc->ym = 0;
		hit |= 8;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleA(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleB(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleC(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleD(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y + npc->hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 + npc->hit.top;

		// Halt momentum
		if (npc->ym < 0)
			npc->ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleE(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x10000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x28;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleF(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x20000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x >= (x * 0x10 - 8) * 0x200	// Note that this function uses '>='. I'm not sure if this is a bug.
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) + ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x28;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleG(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x40000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) + 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x18;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharTriangleH(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	hit |= 0x80000;

	if (npc->x < (x * 0x10 + 8) * 0x200
		&& npc->x > (x * 0x10 - 8) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800
		&& npc->y - npc->hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		npc->y = (y * 0x10 * 0x200) - ((npc->x - (x * 0x10 * 0x200)) / 2) - 0x800 - npc->hit.bottom;

		// Halt momentum
		if (npc->ym > 0)
			npc->ym = 0;

		// Set that hit this slope
		hit |= 0x18;
	}

	npc->flag |= hit;
}

void JudgeHitNpCharWater(NPCHAR *npc, int x, int y)
{
	int hit = 0;

	if (npc->x - npc->hit.back < (x * 0x10 + 6) * 0x200
		&& npc->x + npc->hit.back > (x * 0x10 - 6) * 0x200
		&& npc->y - npc->hit.top < (y * 0x10 + 6) * 0x200
		&& npc->y + npc->hit.bottom > (y * 0x10 - 6) * 0x200)
		hit |= 0x100;

	npc->flag |= hit;
}

void HitNpCharMap(void)
{
	int x, y;
	int judg;

	int offx[9];
	int offy[9];

	int i, j;

	offx[0] = 0;
	offx[1] = 1;
	offx[2] = 0;
	offx[3] = 1;
	offx[4] = 2;
	offx[5] = 2;
	offx[6] = 2;
	offx[7] = 0;
	offx[8] = 1;

	offy[0] = 0;
	offy[1] = 0;
	offy[2] = 1;
	offy[3] = 1;
	offy[4] = 0;
	offy[5] = 1;
	offy[6] = 2;
	offy[7] = 2;
	offy[8] = 2;

	for (i = 0; i < NPC_MAX; ++i)
	{
		if (!(gNPC[i].cond & 0x80))
			continue;

		if (gNPC[i].bits & NPC_IGNORE_SOLIDITY)
			continue;

		if (gNPC[i].size >= 3)
		{
			judg = 9;
			x = (gNPC[i].x - 0x1000) / 0x10 / 0x200;
			y = (gNPC[i].y - 0x1000) / 0x10 / 0x200;
		}
		else
		{
			judg = 4;
			x = gNPC[i].x / 0x10 / 0x200;
			y = gNPC[i].y / 0x10 / 0x200;
		}

		gNPC[i].flag = 0;

		for (j = 0; j < judg; ++j)
		{
			switch (GetAttribute(x + offx[j], y + offy[j]))
			{
				// No NPC block
				case 0x44:
					if (gNPC[i].bits & NPC_IGNORE_TILE_44)
						break;
					// Fallthrough
				// Block
				case 0x03:
				case 0x05:
				case 0x41:
				case 0x43:
					JadgeHitNpCharBlock(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				// Slopes
				case 0x50:
					JudgeHitNpCharTriangleA(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x51:
					JudgeHitNpCharTriangleB(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x52:
					JudgeHitNpCharTriangleC(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x53:
					JudgeHitNpCharTriangleD(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x54:
					JudgeHitNpCharTriangleE(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x55:
					JudgeHitNpCharTriangleF(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x56:
					JudgeHitNpCharTriangleG(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x57:
					JudgeHitNpCharTriangleH(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				// Water
				case 0x02:
				case 0x60:
				case 0x62:
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				// Water block
				case 0x04:
				case 0x61:
				case 0x64:
					JadgeHitNpCharBlock(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				// Water slopes
				case 0x70:
					JudgeHitNpCharTriangleA(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x71:
					JudgeHitNpCharTriangleB(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x72:
					JudgeHitNpCharTriangleC(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x73:
					JudgeHitNpCharTriangleD(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x74:
					JudgeHitNpCharTriangleE(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x75:
					JudgeHitNpCharTriangleF(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x76:
					JudgeHitNpCharTriangleG(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0x77:
					JudgeHitNpCharTriangleH(&gNPC[i], x + offx[j], y + offy[j]);
					JudgeHitNpCharWater(&gNPC[i], x + offx[j], y + offy[j]);
					break;

				case 0xA0:
					gNPC[i].flag |= 0x100;
					// Fallthrough
				case 0x80:
					gNPC[i].flag |= 0x1000;
					break;

				case 0xA1:
					gNPC[i].flag |= 0x100;
					// Fallthrough
				case 0x81:
					gNPC[i].flag |= 0x2000;
					break;

				case 0xA2:
					gNPC[i].flag |= 0x100;
					// Fallthrough
				case 0x82:
					gNPC[i].flag |= 0x4000;
					break;

				case 0xA3:
					gNPC[i].flag |= 0x100;
					// Fallthrough
				case 0x83:
					gNPC[i].flag |= 0x8000;
					break;
			}

			if (gNPC[i].y > gWaterY + 0x800)
				gNPC[i].flag |= 0x100;
		}
	}
}

#include "ArmsItem.h"
#include "NpcTbl.h"
#include <cstring>
#include "NpChar.h"
// Check if a specific weapon (by code) has less than its maximum ammo
bool IsAmmoNotFull(int code)
{
	for (int i = 0; i < 8; ++i)
	{
		// Look for the weapon code in the player's inventory
		if (gArmsData[i].code == code)
		{
			// Return true if current ammo (num) is less than max ammo (max_num)
			if (gArmsData[i].num < gArmsData[i].max_num)
				return true;
		}
	}

	return false;
}

bool SpawnSpecificAmmoDrop(int x, int y, int amount)
{
	int ammo_weapons[8];
	int count = 0;

	// Search inventory for Missile Launchers (ID 4)
	for (int i = 0; i < 8; ++i)
	{
		if (gArmsData[i].code == 4 || gArmsData[i].code == 5) // Normal or Super Missile
		{
			ammo_weapons[count++] = gArmsData[i].code;
		}
	}

	if (count == 0)
		return false;

	// Randomly pick one of the found ammo-consuming weapons
	int target_weapon = ammo_weapons[Random(0, 10 * count) % count];

	// Find an empty NPC slot for the drop (scanning reserved slots 256-512)
	int slot = 256;
	while (slot < 512 && gNPC[slot].cond & 0x80)
		slot++;

	if (slot == 512)
		return false;

	// Initialize the Ammo Pickup NPC (Type 76)
	NPCHAR *drop = &gNPC[slot];
	memset(drop, 0, sizeof(NPCHAR));
	drop->cond = 0x80;
	drop->code_char = 76; // Missile/Ammo pickup NPC type
	drop->x = x;
	drop->y = y;
	drop->code_event = target_weapon;
	drop->exp = amount;
	
	// Copy base parameters from the NPC table
	drop->bits = gNpcTable[drop->code_char].bits;
	
	SetUniqueParameter(drop);

	return true;
}
void LoseNpChar(NPCHAR *npc, BOOL bVanish)
{
	// 1. Play the destruction sound assigned to this NPC
	PlaySoundObject(npc->destroy_voice, 1);

	// 2. Spawn explosion particles based on size category
	// count values (3, 7, 12) are standard for small, medium, and large effects
	switch (npc->size)
	{
		case 1:
			SetDestroyNpChar(npc->x, npc->y, npc->view.back, 3);
			break;
		case 2:
			SetDestroyNpChar(npc->x, npc->y, npc->view.back, 7);
			break;
		case 3:
			SetDestroyNpChar(npc->x, npc->y, npc->view.back, 12);
			break;
	}

	// 3. Item Drop Logic
	if (npc->exp != 0)
	{
		int ran = Random(1, 100);

		if (ran <= 30) // 30% chance for Health
		{
			// If player health is full, fallback to EXP
			if (gMC.life >= gMC.max_life)
			{
				SetExpObjects(npc->x, npc->y, npc->exp);
			}
			else
			{
				// Drop better hearts based on enemy's EXP value
				if (npc->exp <= 10)
					SetLifeObject(npc->x, npc->y, 2);
				else if (npc->exp <= 25)
					SetLifeObject(npc->x, npc->y, 6);
				else
					SetLifeObject(npc->x, npc->y, 18);
			}
		}
		else if (ran <= 55) // 25% chance for Super Missile Ammo (Weapon 5)
		{
			if (IsAmmoNotFull(5))
			{
				// In the decomp, 'flag' was used, but this usually maps to 'exp' for drop quantity
				int ammo_amount = (npc->exp <= 10) ? 2 : 5;
				if (!SetBulletObject(npc->x, npc->y, ammo_amount))
					SetExpObjects(npc->x, npc->y, npc->exp);
			}
			else
			{
				SetExpObjects(npc->x, npc->y, npc->exp);
			}
		}
		else if (ran <= 85) // 30% chance for Normal Missile Ammo (Weapon 4)
		{
			if (IsAmmoNotFull(4))
			{
				// Calls sub_494090 which spawns the custom NPC 76 ammo drop
				bool success;
				if (npc->exp <= 15)
					success = SpawnSpecificAmmoDrop(npc->x, npc->y, 25);
				else
					success = SpawnSpecificAmmoDrop(npc->x, npc->y, 100);

				if (!success)
					SetExpObjects(npc->x, npc->y, npc->exp);
			}
			else
			{
				SetExpObjects(npc->x, npc->y, npc->exp);
			}
		}
		else // 15% chance for guaranteed EXP
		{
			SetExpObjects(npc->x, npc->y, npc->exp);
		}
	}

	// 4. Cleanup and Damage View
	// If bit 0x8000 is set, show the floating damage text
	if (npc->bits & 0x8000)
	{
		if (npc->damage_view != 0)
			SetValueView(&npc->x, &npc->y, npc->damage_view);
		
		if (bVanish)
			VanishNpChar(npc);
	}
	else
	{
		// Hard-delete the NPC by clearing its condition
		npc->cond = 0;
	}
}
#include "Boss.h"
void HitNpCharBullet(void)
{
	for (int n = 0; n < 512; ++n)
	{
		NPCHAR *npc = &gNPC[n];

		// NPC must be active. 
		// If NPC is shootable (0x20) and Invincible (0x2000), it's ignored by bullets.
		if (!(npc->cond & 0x80) || ((npc->bits & 0x20) && (npc->bits & 0x2000)))
			continue;

		for (int b = 0; b < 64; ++b)
		{
			BULLET *bul = &gBul[b];

			// Bullet must be active and have a valid damage value
			if (!(bul->cond & 0x80) || bul->damage == -1)
				continue;

			bool bHit = false;

			// 1. Check Collision with Shootable NPCs
			if (npc->bits & 0x20)
			{
				if (npc->x - npc->hit.back < bul->x + bul->enemyXL &&
					npc->x + npc->hit.back > bul->x - bul->enemyXL &&
					npc->y - npc->hit.top < bul->y + bul->enemyYL &&
					npc->y + npc->hit.bottom > bul->y - bul->enemyYL)
				{
					bHit = true;
				}
			}
			// 2. Check Collision with Invulnerable/Solid parts
			else if (npc->bits & 0x04)
			{
				if (npc->x - npc->hit.back < bul->x + bul->blockXL &&
					npc->x + npc->hit.back > bul->x - bul->blockXL &&
					npc->y - npc->hit.top < bul->y + bul->blockYL &&
					npc->y + npc->hit.bottom > bul->y - bul->blockYL)
				{
					bHit = true;
				}
			}

			if (bHit)
			{
				// --- BRANCH A: DAMAGEABLE NPC ---
				if (npc->bits & 0x20)
				{
					npc->life -= bul->damage;

					if (npc->life >= 1)
					{
						// Hit reaction (jitter and carets)
						if (npc->shock < 14)
						{
							for (int k = npc->size + 1; k > 0; --k)
							{
								// Position caret halfway between bullet and NPC
								SetCaret((bul->x + npc->x) / 2, (bul->y + npc->y) / 2, 11, 0);
							}
							PlaySoundObject(npc->hit_voice, 1);
							npc->shock = 16;
						}

						// Handle Boss-Life Link (Bit 0x8000)
						if (npc->bits & 0x8000)
							npc->damage_view -= bul->damage;
					}
					else
					{
						// Fatal Hit
						npc->life = 0;

						if (npc->bits & 0x8000)
							npc->damage_view -= bul->damage;

						// Run TSC Event if Flag 0x200 is set
						if ((gMC.cond & 0x80) && (npc->bits & 0x200))
							StartTextScript(npc->code_event);
						else
							npc->cond |= 0x08; // Mark for CustomVanish
					}

					// Bullet health is ONLY decremented when hitting vulnerable NPCs
					if (bul->life > 0)
						bul->life--;
					
					continue;
				}

				// --- BRANCH B: INVULNERABLE NPC / SPECIAL LOGIC ---
				
				// 1. Magnetic Pickup Logic (Whimsical Star/Blade pull)
				if (bul->code_bullet == 19)
				{
					if (npc->code_char == 76 || npc->code_char == 86 || npc->code_char == 87)
					{
						npc->x = gMC.x;
						npc->y = gMC.y;
						goto hit_npc_bullet_listener_checks;
					}
				}
				else if (bul->code_bullet == 13 || bul->code_bullet == 14 || bul->code_bullet == 15 || 
						 bul->code_bullet == 28 || bul->code_bullet == 29 || bul->code_bullet == 30) 
				{
					// High-level weapons bypass bounce/invulnerability logic completely
					goto hit_npc_bullet_listener_checks;
				}
				else if (npc->code_char == 76 || npc->code_char == 86 || npc->code_char == 87)
				{
					// XP/Health Pickups don't block normal projectiles
					goto hit_npc_bullet_listener_checks;
				}

				// 2. Bullet Destruction/Bounce
				if (!(bul->bbits & 0x10)) // If not piercing
				{
					if (!(bul->bbits & 0x08)) // If not indestructible
						bul->cond = 0;

					// Bubbler Bounce
					if (bul->code_bullet == 7)
					{
						bul->direct = (bul->direct == 0) ? 2 : 0;
						bul->xm = -bul->xm;
						bul->ym = -bul->ym;
						PlaySoundObject(34, 1);
					}
					else if (bul->code_bullet == 8)
					{
						bul->cond = 0;
					}
					// Transition special projectiles to frame 15 (poof/dust ani_no)
					else if (bul->code_bullet == 19 || bul->code_bullet == 23 || bul->code_bullet == 26)
					{
						bul->ani_no = 15;
					}
					else
					{
						// Standard wall hit effect
						if (bul->bbits & 0x10)
						{
							bul->cond = 0;
							SetCaret(bul->x, bul->y, 2, 1);
						}
						else
						{
							SetCaret(bul->x, bul->y, 2, 2);
						}
						PlaySoundObject(28, 1);
					}
				}

			hit_npc_bullet_listener_checks:
				// 3. Signal Listener Logic
				if (npc->code_char == 152)
				{
					// Send the bullet ID to the trigger NPC
					if (npc->count2 == 0)
						npc->count2 = bul->code_bullet;
				}
				else if (npc->code_char == 303)
				{
					npc->shock = 1;
				}
			}
		}

		// Run death routine if marked
		if (npc->cond & 8)
			LoseNpChar(npc, 1);
	}
}