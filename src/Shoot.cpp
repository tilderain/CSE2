#include "Shoot.h"

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "Bullet.h"
#include "Caret.h"
#include "KeyControl.h"
#include "MyChar.h"
#include "MycParam.h"
#include "Sound.h"

static int empty;

void ShootBullet_Frontia1(int level)
{
	int bul_no;

	switch (level)
	{
		case 1:
			bul_no = 1;
			break;

		case 2:
			bul_no = 2;
			break;

		case 3:
			bul_no = 3;
			break;
	}

	if (CountArmsBullet(1) > 3)
		return;

	if (gKeyTrg & gKeyShot)
	{
		if (!UseArmsEnergy(1))
		{
			ChangeToFirstArms();
		}
		else
		{
			if (gMC.up)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (3 * 0x200), gMC.y - (10 * 0x200), 1);
					SetCaret(gMC.x - (3 * 0x200), gMC.y - (10 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (3 * 0x200), gMC.y - (10 * 0x200), 1);
					SetCaret(gMC.x + (3 * 0x200), gMC.y - (10 * 0x200), 3, 0);
				}
			}
			else if (gMC.down)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (3 * 0x200), gMC.y + (10 * 0x200), 3);
					SetCaret(gMC.x - (3 * 0x200), gMC.y + (10 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (3 * 0x200), gMC.y + (10 * 0x200), 3);
					SetCaret(gMC.x + (3 * 0x200), gMC.y + (10 * 0x200), 3, 0);
				}
			}
			else
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (6 * 0x200), gMC.y + (2 * 0x200), 0);
					SetCaret(gMC.x - (12 * 0x200), gMC.y + (2 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (6 * 0x200), gMC.y + (2 * 0x200), 2);
					SetCaret(gMC.x + (12 * 0x200), gMC.y + (2 * 0x200), 3, 0);
				}
			}

			PlaySoundObject(33, SOUND_MODE_PLAY);
		}
	}
}

void ShootBullet_PoleStar(int level)
{
	int bul_no;

	// The switch statement was optimized into a simple addition
	bul_no = level + 3;

	// The bullet limit was increased from 2 to 3
	if (CountArmsBullet(2) > 2)
		return;

	if (gKeyTrg & gKeyShot)
	{
		if (!UseArmsEnergy(1))
		{
			PlaySoundObject(37, SOUND_MODE_PLAY);
		}
		else
		{
			if (gMC.up)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (1 * 0x200), gMC.y - (8 * 0x200), 1);
					SetCaret(gMC.x - (1 * 0x200), gMC.y - (8 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (1 * 0x200), gMC.y - (8 * 0x200), 1);
					SetCaret(gMC.x + (1 * 0x200), gMC.y - (8 * 0x200), 3, 0);
				}
			}
			else if (gMC.down)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (1 * 0x200), gMC.y + (8 * 0x200), 3);
					SetCaret(gMC.x - (1 * 0x200), gMC.y + (8 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (1 * 0x200), gMC.y + (8 * 0x200), 3);
					SetCaret(gMC.x + (1 * 0x200), gMC.y + (8 * 0x200), 3, 0);
				}
			}
			else
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (6 * 0x200), gMC.y + (3 * 0x200), 0);
					SetCaret(gMC.x - (12 * 0x200), gMC.y + (3 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (6 * 0x200), gMC.y + (3 * 0x200), 2);
					SetCaret(gMC.x + (12 * 0x200), gMC.y + (3 * 0x200), 3, 0);
				}
			}

			if (level == 3)
				PlaySoundObject(49, SOUND_MODE_PLAY);
			else
				PlaySoundObject(32, SOUND_MODE_PLAY);
		}
	}
}


// Note: 'empty' is usually a static variable at the top of Shoot.cpp in CSE2.
extern int empty;

#include "Flags.h"

void ShootBullet_FireBall(int level)
{
	static int wait;

	if (--empty <= 0)
		empty = 0;

	if (gKeyTrg & gKeyShot)
	{
		// Calculate dynamic ammo cost
		int ammo_cost = (2 * level) + 4;
		if (GetNPCFlag(242)) ammo_cost = (2 * level) + 9;
		if (GetNPCFlag(243)) ammo_cost += 10; // Max cost is 25

		// Gate: Only fire if current ammo >= cost
		if (gArmsData[gSelectedArms].num < ammo_cost)
		{
			//PlaySoundObject(37, SOUND_MODE_PLAY); // Click
			if (empty == 0)
			{
				//SetCaret(gMC.x, gMC.y, CARET_EMPTY, DIR_LEFT);
				empty = 50;
			}
		}
		else
		{
			UseArmsEnergy(ammo_cost);

			int bul_x, bul_y, bul_dir, caret_x;

			if (gMC.up || gMC.down)
			{
				bul_dir = gMC.up ? DIR_UP : DIR_DOWN;
				bul_x = (gMC.direct == DIR_LEFT) ? gMC.x - 0x200 : gMC.x + 0x200;
				bul_y = gMC.up ? gMC.y - 0x1000 : gMC.y + 0x1000;
				caret_x = bul_x;
			}
			else
			{
				bul_dir = gMC.direct;
				bul_y = gMC.y + 0x600;
				int offset = (gMC.direct == DIR_LEFT) ? -0xC00 : 0xC00;
				bul_x = gMC.x + offset;
				caret_x = gMC.x + (offset * 2);
			}

			SetBullet(7, bul_x, bul_y, bul_dir);
			SetCaret(caret_x, bul_y, CARET_SHOOT, DIR_LEFT);

			PlaySoundObject((level == 3) ? 33 : 34, SOUND_MODE_PLAY);
		}
	}
	else if (++wait >= 2) // Passive Recharge
	{
		wait = 0;
		ChargeArmsEnergy(1);
	}
}
int gFireTimer = 0;
int gRechargeTimer = 0;
void ShootBullet_Machinegun1(int level)
{
	int bul_no = level + 9; // Lv1=10, Lv2=11, Lv3=12
	int max_bul = 12;
	int shot_count = 1;

	// Check for Multi-shot upgrades
	if (GetNPCFlag(0x234))
	{
		max_bul = 36;
		shot_count = 3;
	}
	else if (GetNPCFlag(0x233))
	{
		shot_count = 2;
	}

	bool rapid_recharge = GetNPCFlag(0x232);

	// Bullet limit check
	if (CountArmsBullet(4) > max_bul)
		return;

	if (!(gKey & gKeyShot))
	{
		// NOT FIRING: Handle Manual Recharge
		// Reset fire timer based on upgrade status
		if (rapid_recharge)
			gFireTimer = 7;
		else
			gFireTimer = 9;

		// Recharge logic (replaces vanilla auto-recharge)
		if (++gRechargeTimer > (rapid_recharge ? 5 : 15))
		{
			gRechargeTimer = 0;
			ChargeArmsEnergy(1);
		}
	}
	else
	{
		// FIRING: Handle Shot Logic
		if (++gFireTimer > 7)
		{
			gFireTimer = 0;

			if (!UseArmsEnergy(shot_count))
			{
				PlaySoundObject(37, SOUND_MODE_PLAY); // Out of ammo
				SetCaret(gMC.x, gMC.y, 16, 0);       // "Empty" caret
			}
			else
			{
				int bul_x, bul_y;
				int caret_x;
				int shot_dir = gMC.direct;

				// Determine spawn position based on player direction
				if (gMC.up)
				{
					shot_dir = 1;
					bul_x = (gMC.direct == 0) ? (gMC.x - 0x800) : (gMC.x + 0x800);
					bul_y = gMC.y - 0x1000;
					caret_x = bul_x;
				}
				else if (gMC.down)
				{
					shot_dir = 3;
					bul_x = (gMC.direct == 0) ? (gMC.x - 0x800) : (gMC.x + 0x800);
					bul_y = gMC.y + 0x1000;
					caret_x = bul_x;
				}
				else
				{
					bul_y = gMC.y + 0x600;
					if (gMC.direct == 0)
					{
						bul_x = gMC.x - 0x1200;
						caret_x = gMC.x - 0x2400; // Muzzle flash offset
					}
					else
					{
						bul_x = gMC.x + 0x1200;
						caret_x = gMC.x + 0x2400;
					}
				}

				SetCaret(caret_x, bul_y, 3, 0);
				PlaySoundObject((level == 3) ? 49 : 32, SOUND_MODE_PLAY);

				// Handle multi-bullet patterns
				if (shot_count == 1)
				{
					SetBullet(bul_no, bul_x, bul_y, shot_dir);
				}
				else if (shot_count == 2)
				{
					// Double spread
					if (!gMC.up && !gMC.down)
					{
						SetBullet(bul_no, bul_x, bul_y - 0x600, shot_dir);
						SetBullet(bul_no, bul_x, bul_y + 0x400, shot_dir);
					}
					else
					{
						SetBullet(bul_no, bul_x - 0x400, bul_y, shot_dir);
						SetBullet(bul_no, bul_x + 0x400, bul_y, shot_dir);
					}
				}
				else if (shot_count == 3)
				{
					// Triple spread
					if (!gMC.up && !gMC.down)
					{
						SetBullet(bul_no, bul_x, bul_y - 0xF00, shot_dir);
						SetBullet(bul_no, bul_x, bul_y, shot_dir);
						SetBullet(bul_no, bul_x, bul_y + 0x1400, shot_dir); // Decompilation math simplification
					}
					else
					{
						SetBullet(bul_no, bul_x - 0xA00, bul_y, shot_dir);
						SetBullet(bul_no, bul_x, bul_y, shot_dir);
						SetBullet(bul_no, bul_x + 0xA00, bul_y, shot_dir);
					}
				}
			}
		}
	}
}

// Helper to count active bullets of a specific type (Custom Engine Function)
int CountBulletsByID(int id)
{
	int count = 0;
	for (int i = 0; i < 64; ++i)
	{
		// Check if bullet is active (bit 7 of cond) and matches ID
		if ((gBul[i].cond & 0x80) && gBul[i].code_bullet == id)
			count++;
	}
	return count;
}

// Helper to get current ammo of the selected weapon
int GetSelectedWeaponAmmo()
{
	return gArmsData[gSelectedArms].num;
}

int gEmptySoundTimer = 0;
#include "Game.h"
void ShootBullet_Missile(int level, bool bSuper)
{
	int bul_no;
	int ammo_cost;
	int max_bullets = level * 2;

	// Determine base bullet type based on game flags
	if (GetNPCFlag(0xCA))
		bul_no = 13; // Missile
	else
		bul_no = 15; // Fireball/Custom

	// Bullet screen limit check
	if (CountBulletsByID(bul_no) < max_bullets)
	{
		// Handle firing modes
		if (GetNPCFlag(0xCB) && (gKeyTrg & gKeyMap))
		{
			// Special Alt-Fire Mode (Map button)
			bul_no = 14;
			ammo_cost = 5;
		}
		else
		{
			// Standard Fire Mode
			ammo_cost = 1;
			if (!(gKeyTrg & gKeyShot))
				return;
		}

		// Ammo Check
		if (GetSelectedWeaponAmmo() < ammo_cost)
		{
			if (gEmptySoundTimer == 0)
			{
				PlaySoundObject(37, SOUND_MODE_PLAY); // "Empty" click
				SetCaret(gMC.x, gMC.y, 16, 0);        // "Empty" icon
				gEmptySoundTimer = 25;                // Prevent sound spam
			}
		}
		else
		{
			// Fire the weapon
			UseArmsEnergy(ammo_cost);

			int bul_x, bul_y, caret_x;
			int shot_dir = gMC.direct;

			// Position calculation
			if (gMC.up)
			{
				shot_dir = 1;
				bul_x = (gMC.direct == 0) ? (gMC.x - 0x600) : (gMC.x + 0x600);
				bul_y = gMC.y - 0x1200;
				caret_x = bul_x;
			}
			else if (gMC.down)
			{
				shot_dir = 3;
				bul_x = (gMC.direct == 0) ? (gMC.x - 0x600) : (gMC.x + 0x600);
				bul_y = gMC.y + 0x1200;
				caret_x = bul_x;
			}
			else
			{
				bul_y = gMC.y + 0x800;
				if (gMC.direct == 0)
				{
					bul_x = gMC.x - 0xD00;
					caret_x = gMC.x - 0x1A00; // Flash spawns further out
				}
				else
				{
					bul_x = gMC.x + 0xD00;
					caret_x = gMC.x + 0x1A00;
				}
			}

			SetCaret(caret_x, bul_y, 3, 0); // Muzzle flash
			
			// Select sound based on bullet type
			if (bul_no == 15)
				PlaySoundObject(33, SOUND_MODE_PLAY);
			else
				PlaySoundObject(32, SOUND_MODE_PLAY);

			// Handle Bullet 17 (Special behavior) or standard with spread
			if (bul_no == 17)
			{
				SetBullet(17, bul_x, bul_y, shot_dir);
			}
			else
			{
				int spread_x = 0;
				int spread_y = 0;

				if (!gMC.up && !gMC.down)
				{
					// Horizontal fire: slight vertical randomization
					spread_y = Random(0x100, 0x400); 
					SetBullet(bul_no, bul_x, bul_y - spread_y, shot_dir);
				}
				else
				{
					// Vertical fire: slight horizontal randomization
					spread_x = Random(-0x200, 0x200);
					SetBullet(bul_no, bul_x + spread_x, bul_y, shot_dir);
				}
			}
		}
	}
}

// Custom variables used by the mod's new weapon logic
extern unsigned char gGrappleState; // Maps to 0x493804
extern unsigned char gCustomWeaponDir;   // Maps to 0x493805

// Replaces ShootBullet_Bubblin1
void ShootBullet_Bubblin1(void)
{
	int b_x, b_y;
	int b_dir;
	unsigned char dir_8way;

	// FUN_004946a0 is a custom wrapper for CountArmsBullet
	// If no bullets of ID 19 (Bubbler 1) are active, reset the weapon state.
	if (CountBulletNum(19) == 0)
	{
		gGrappleState = 0;
	}

	// Only allow shooting if the state is 0 and the Shoot key was just pressed
	if (gGrappleState != 3 && gGrappleState != 2 && gGrappleState != 1 && (gKeyTrg & gKeyShot))
	{
		b_dir = gMC.direct;

		// Determine spawn coordinates based on aiming direction
		if (gMC.up == 0 && gMC.down == 0)
		{
			// Shooting horizontally
			b_y = gMC.y + 0x400; // y + 2px
			
			if (gMC.direct == 0)
				b_x = -0x1000;
			else
				b_x = 0x1000;
			
			b_x = (b_x * 2) + gMC.x; // Spawn 16 pixels (0x2000) in front of Quote
		}
		else
		{
			// Shooting vertically
			if (gMC.direct == 0)
				b_x = gMC.x - 0x800; // x - 4px
			else
				b_x = gMC.x + 0x800; // x + 4px

			if (gMC.up == 0)
			{
				b_dir = 3; // Down
				b_y = gMC.y + 0x1800; // y + 12px
			}
			else
			{
				b_dir = 1; // Up
				b_y = gMC.y - 0x1800; // y - 12px
			}
		}

		// Spawn visual effects
		SetCaret(b_x, b_y, CARET_BUBBLE, b_dir);
		SetCaret(b_x, b_y, CARET_SHOOT, 0);
		
		PlaySoundObject(117, SOUND_MODE_PLAY); // 0x75
		PlaySoundObject(114, SOUND_MODE_PLAY); // 0x72

		// Minor vertical offset adjustment for horizontal shots
		if (gMC.up == 0 && gMC.down == 0)
			b_y -= 0xA00; // y - 5px

		// Fire Bullet 19
		SetBullet(19, b_x, b_y, b_dir);

		// Lock weapon state to 1
		gGrappleState = 1;

		// 8-Way Direction Logic Mapping
		// 0: Left, 1: Up, 2: Right, 3: Down
		// 4: Up-Left, 5: Up-Right, 6: Down-Right, 7: Down-Left
		if (!(gKey & gKeyLeft))
		{
			if (gKey & gKeyUp)
			{
				if (gKey & gKeyRight)
					dir_8way = 5; // Up-Right
				else
					dir_8way = 1; // Up
			}
			else if (gKey & gKeyRight)
			{
				if (gKey & gKeyDown)
					dir_8way = 6; // Down-Right
				else
					dir_8way = 2; // Right
			}
			else if (gKey & gKeyDown)
			{
				dir_8way = 3; // Down
			}
			else
			{
				dir_8way = b_dir; // Default 4-way direction
			}
		}
		else if (gKey & gKeyUp)
		{
			dir_8way = 4; // Up-Left
		}
		else if (gKey & gKeyDown)
		{
			dir_8way = 7; // Down-Left
		}
		else
		{
			dir_8way = 0; // Left
		}

		// Store the captured 8-way direction into the new global variable
		gCustomWeaponDir = dir_8way;
	}
}

void ShootBullet_Bubblin2(int level)
{
	static int wait;

	if (CountArmsBullet(7) > 15)
		return;

	level += 18;

	if (!(gKey & gKeyShot))
		gMC.rensha = 6;

	if (gKey & gKeyShot)
	{
		if (++gMC.rensha < 7)
			return;

		gMC.rensha = 0;

		if (!UseArmsEnergy(1))
		{
			PlaySoundObject(37, SOUND_MODE_PLAY);

			if (empty == 0)
			{
				SetCaret(gMC.x, gMC.y, 16, 0);
				empty = 50;
			}

			return;
		}

		if (gMC.up)
		{
			if (gMC.direct == 0)
			{
				SetBullet(level, gMC.x - (3 * 0x200), gMC.y - (8 * 0x200), 1);
				SetCaret(gMC.x - (3 * 0x200), gMC.y - (16 * 0x200), 3, 0);
			}
			else
			{
				SetBullet(level, gMC.x + (3 * 0x200), gMC.y - (8 * 0x200), 1);
				SetCaret(gMC.x + (3 * 0x200), gMC.y - (16 * 0x200), 3, 0);
			}
		}
		else if (gMC.down)
		{
			if (gMC.direct == 0)
			{
				SetBullet(level, gMC.x - (3 * 0x200), gMC.y + (8 * 0x200), 3);
				SetCaret(gMC.x - (3 * 0x200), gMC.y + (16 * 0x200), 3, 0);
			}
			else
			{
				SetBullet(level, gMC.x + (3 * 0x200), gMC.y + (8 * 0x200), 3);
				SetCaret(gMC.x + (3 * 0x200), gMC.y + (16 * 0x200), 3, 0);
			}
		}
		else
		{
			if (gMC.direct == 0)
			{
				SetBullet(level, gMC.x - (6 * 0x200), gMC.y + (3 * 0x200), 0);
				SetCaret(gMC.x - (12 * 0x200), gMC.y + (3 * 0x200), 3, 0);
			}
			else
			{
				SetBullet(level, gMC.x + (6 * 0x200), gMC.y + (3 * 0x200), 2);
				SetCaret(gMC.x + (12 * 0x200), gMC.y + (3 * 0x200), 3, 0);
			}
		}

		PlaySoundObject(48, SOUND_MODE_PLAY);
	}
	else if (++wait > 1)
	{
		wait = 0;
		ChargeArmsEnergy(1);
	}
}


// Note: DAT_0049e678 appears to be a frame counter or global timer
int gModTimer; 

#include "Map.h"


void ShootBullet_Sword(int level)
{
    int bul_no;
    int bul_x;
    int bul_y;
    int bul_dir;

    // Check if boomerang (26) already exists — bail if so
    if (CountBulletNum(26) != 0)
        return;

    if (!(gKey & gKeyShot))
    {
        // Alt-fire: boomerang (map key pressed)
        if (!(gKeyTrg & gKeyMap))
            return;

        bul_no = 26;   // 0x1a

        // Zero VelocityX only when on slope tiles AND moving fast
        if (!gMC.down
            && (gMC.flag & 0x3d) != 0
            && gMC.ani_no > 3)
        {
            gMC.xm = 0;
        }
    }
    else
    {
        // Standard fire: slash (23)
        // Bail if slash already exists
        if (CountBulletNum(23) != 0)
            return;

        bul_no = 23;   // 0x17
    }

    // Cache player state
    bul_x   = gMC.x;
    bul_y   = gMC.y;
    bul_dir = gMC.direct;   // 0=left, 2=right

    int facing_vert = gMC.up + gMC.down;   // IsFacingUp + IsFacingDown

    if (facing_vert != 0)
    {
        // Diagonal/vertical fire
        // X offset based on facing direction
        if (gMC.direct != 0)   // facing right
            bul_x += 0x600;
        else
            bul_x -= 0x600;

        if (gMC.up)
        {
            // Shooting up
            // Boomerang always gets DIR_UP(1), slash gets DIR_UP only if facing right
            // If slash AND facing left: dir stays 0 (left)
            if (bul_no == 26 || gMC.direct != 0)
                bul_dir = 1;   // DIR_UP
            // else bul_dir stays as gMC.direct (0 = left)

            bul_y -= 0x1200;
        }
        else
        {
            // Shooting down
            bul_dir = 3;   // DIR_DOWN
            bul_y  += 0x1200;
        }
    }
    else
    {
        // Horizontal fire
        bul_y -= 0x600;

        // bul_x starts as a velocity/offset, added to PlayerX later
        if (gMC.direct != 0)   // facing right
            bul_x = 0xC00;
        else
            bul_x = -0xC00;    // 0xFFFFF400

        // Add player X position (only done for horizontal, not diagonal)
        bul_x += gMC.x;
    }

    // Sound: boomerang=106(0x6a), slash=110(0x6e)
    PlaySoundObject((bul_no == 26) ? 0x6a : 0x6e, SOUND_MODE_PLAY);

    SetBullet(bul_no, bul_x, bul_y, bul_dir);
}
void ShootBullet_Nemesis(int level)
{
	int bul_no;

	switch (level)
	{
		case 1:
			bul_no = 34;
			break;

		case 2:
			bul_no = 35;
			break;

		case 3:
			bul_no = 36;
			break;
	}

	if (CountArmsBullet(12) > 1)
		return;

	if (gKeyTrg & gKeyShot)
	{
		if (!UseArmsEnergy(1))
		{
			PlaySoundObject(37, SOUND_MODE_PLAY);
		}
		else
		{
			if (gMC.up)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (1 * 0x200), gMC.y - (12 * 0x200), 1);
					SetCaret(gMC.x - (1 * 0x200), gMC.y - (12 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (1 * 0x200), gMC.y - (12 * 0x200), 1);
					SetCaret(gMC.x + (1 * 0x200), gMC.y - (12 * 0x200), 3, 0);
				}
			}
			else if (gMC.down)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (1 * 0x200), gMC.y + (12 * 0x200), 3);
					SetCaret(gMC.x - (1 * 0x200), gMC.y + (12 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (1 * 0x200), gMC.y + (12 * 0x200), 3);
					SetCaret(gMC.x + (1 * 0x200), gMC.y + (12 * 0x200), 3, 0);
				}
			}
			else
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (22 * 0x200), gMC.y + (3 * 0x200), 0);
					SetCaret(gMC.x - (16 * 0x200), gMC.y + (3 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (22 * 0x200), gMC.y + (3 * 0x200), 2);
					SetCaret(gMC.x + (16 * 0x200), gMC.y + (3 * 0x200), 3, 0);
				}
			}

			switch (level)
			{
				case 1:
					PlaySoundObject(117, SOUND_MODE_PLAY);
					break;

				case 2:
					PlaySoundObject(49, SOUND_MODE_PLAY);
					break;

				case 3:
					PlaySoundObject(60, SOUND_MODE_PLAY);
					break;
			}
		}
	}
}

int spur_charge;

void ResetSpurCharge(void)
{
	spur_charge = 0;

	if (gArmsData[gSelectedArms].code == 13)
		ZeroExpMyChar();
}

void ShootBullet_Spur(int level)
{
	int bul_no;
	BOOL bShot;
	static BOOL bMax;

	bShot = FALSE;

	if (gKey & gKeyShot)
	{
		if (gMC.equip & EQUIP_TURBOCHARGE)
			AddExpMyChar(3);
		else
			AddExpMyChar(2);

		if (++spur_charge / 2 % 2)
		{
			switch (level)
			{
				case 1:
					PlaySoundObject(59, SOUND_MODE_PLAY);
					break;

				case 2:
					PlaySoundObject(60, SOUND_MODE_PLAY);
					break;

				case 3:
					if (!IsMaxExpMyChar())
						PlaySoundObject(61, SOUND_MODE_PLAY);

					break;
			}
		}
	}
	else
	{
		if (spur_charge)
			bShot = TRUE;

		spur_charge = 0;
	}

	if (IsMaxExpMyChar())
	{
		if (!bMax)
		{
			bMax = TRUE;
			PlaySoundObject(65, SOUND_MODE_PLAY);
		}
	}
	else
	{
		bMax = FALSE;
	}

	if (!(gKey & gKeyShot))
		ZeroExpMyChar();

	switch (level)
	{
		case 1:
			bul_no = 6;
			bShot = FALSE;
			break;

		case 2:
			bul_no = 37;
			break;

		case 3:
			if (bMax)
				bul_no = 39;
			else
				bul_no = 38;

			break;
	}

	if (CountArmsBullet(13) > 0 || CountArmsBullet(14) > 0)
		return;

	if (gKeyTrg & gKeyShot || bShot)
	{
		if (!UseArmsEnergy(1))
		{
			PlaySoundObject(37, SOUND_MODE_PLAY);
		}
		else
		{
			if (gMC.up)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (1 * 0x200), gMC.y - (8 * 0x200), 1);
					SetCaret(gMC.x - (1 * 0x200), gMC.y - (8 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (1 * 0x200), gMC.y - (8 * 0x200), 1);
					SetCaret(gMC.x + (1 * 0x200), gMC.y - (8 * 0x200), 3, 0);
				}
			}
			else if (gMC.down)
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (1 * 0x200), gMC.y + (8 * 0x200), 3);
					SetCaret(gMC.x - (1 * 0x200), gMC.y + (8 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (1 * 0x200), gMC.y + (8 * 0x200), 3);
					SetCaret(gMC.x + (1 * 0x200), gMC.y + (8 * 0x200), 3, 0);
				}
			}
			else
			{
				if (gMC.direct == 0)
				{
					SetBullet(bul_no, gMC.x - (6 * 0x200), gMC.y + (3 * 0x200), 0);
					SetCaret(gMC.x - (12 * 0x200), gMC.y + (3 * 0x200), 3, 0);
				}
				else
				{
					SetBullet(bul_no, gMC.x + (6 * 0x200), gMC.y + (3 * 0x200), 2);
					SetCaret(gMC.x + (12 * 0x200), gMC.y + (3 * 0x200), 3, 0);
				}
			}

			switch (bul_no)
			{
				case 6:
					PlaySoundObject(49, SOUND_MODE_PLAY);
					break;

				case 37:
					PlaySoundObject(62, SOUND_MODE_PLAY);
					break;

				case 38:
					PlaySoundObject(63, SOUND_MODE_PLAY);
					break;

				case 39:
					PlaySoundObject(64, SOUND_MODE_PLAY);
					break;
			}
		}
	}
}

void ShootBullet(void)
{
	static int soft_rensha;

	// Decrement 'empty' (out of ammo) particle timer
	if (empty != 0)
		--empty;

	// Handle rapid-fire (rensha) cooldown
	if (soft_rensha != 0)
		--soft_rensha;

	if (gKeyTrg & gKeyShot)
	{
		if (soft_rensha != 0)
			return;

		soft_rensha = 4;
	}

	// Do not run if Quote is hidden/dead
	if (gMC.cond & 2)
		return;

	int arm_code = gArmsData[gSelectedArms].code;
	int arm_level = gArmsData[gSelectedArms].level;

	switch (arm_code)
	{
		case 1:
			ShootBullet_Frontia1(arm_level);
			break;

		case 2:
			ShootBullet_PoleStar(arm_level);
			break;

		case 3:
			ShootBullet_FireBall(arm_level);
			break;

		case 4:
			// Redirected to modded Machine Gun logic
			ShootBullet_Machinegun1(arm_level); // Address: 0x41E2E0
			break;

		case 5:
			// Redirected to modded Missile logic
			ShootBullet_Missile(arm_level, false); // Address: 0x41E700
			break;

		case 7:
			// MOD: Bubbler now only uses the Level 1 shooting function 
			// regardless of its actual level.
			ShootBullet_Bubblin1();
			break;

		case 9:
			// Standard Sword logic
			if (arm_level == 1)
				ShootBullet_Sword(1);
			else if (arm_level == 2)
				ShootBullet_Sword(2);
			else if (arm_level == 3)
				ShootBullet_Sword(3);
			break;

		case 10:
			// Redirected to new custom Super Missile logic
			//ShootBullet_SuperMissile(arm_level); // Address: 0x493A00
			break;

		case 12:
			// MOD: Nemesis replaced by the Sword.
			ShootBullet_Sword(arm_level);
			break;

		case 13:
			ShootBullet_Spur(arm_level);
			break;
	}
}