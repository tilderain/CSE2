#include "NpcAct.h"

#include <stddef.h>

#include "WindowsWrapper.h"

#include "Back.h"
#include "Bullet.h"
#include "Caret.h"
#include "Flags.h"
#include "Frame.h"
#include "Game.h"
#include "MyChar.h"
#include "NpChar.h"
#include "NpcHit.h"
#include "Sound.h"
#include "Triangle.h"

// Curly AI
void ActNpc180(NPCHAR *npc)
{
	int xx, yy;

	RECT rcLeft[11] = {
		{0, 96, 16, 112},
		{16, 96, 32, 112},
		{0, 96, 16, 112},
		{32, 96, 48, 112},
		{0, 96, 16, 112},
		{48, 96, 64, 112},
		{64, 96, 80, 112},
		{48, 96, 64, 112},
		{80, 96, 96, 112},
		{48, 96, 64, 112},
		{144, 96, 160, 112},
	};

	RECT rcRight[11] = {
		{0, 112, 16, 128},
		{16, 112, 32, 128},
		{0, 112, 16, 128},
		{32, 112, 48, 128},
		{0, 112, 16, 128},
		{48, 112, 64, 128},
		{64, 112, 80, 128},
		{48, 112, 64, 128},
		{80, 112, 96, 128},
		{48, 112, 64, 128},
		{144, 112, 160, 128},
	};

	if (npc->y < gMC.y - (10 * 0x10 * 0x200))
	{
		if (npc->y < 16 * 0x10 * 0x200)
		{
			npc->tgt_x = 320 * 0x10 * 0x200;
			npc->tgt_y = npc->y;
		}
		else
		{
			npc->tgt_x = 0;
			npc->tgt_y = npc->y;
		}
	}
	else
	{
		if (gCurlyShoot_wait != 0)
		{
			npc->tgt_x = gCurlyShoot_x;
			npc->tgt_y = gCurlyShoot_y;
		}
		else
		{
			npc->tgt_x = gMC.x;
			npc->tgt_y = gMC.y;
		}
	}

	if (npc->xm < 0 && npc->flag & 1)
		npc->xm = 0;
	if (npc->xm > 0 && npc->flag & 4)
		npc->xm = 0;

	switch (npc->act_no)
	{
		case 20:
			npc->x = gMC.x;
			npc->y = gMC.y;
			npc->act_no = 100;
			npc->ani_no = 0;
			SetNpChar(183, 0, 0, 0, 0, 0, npc, 0x100);

			if (GetNPCFlag(563))
				SetNpChar(182, 0, 0, 0, 0, 0, npc, 0x100);
			else
				SetNpChar(181, 0, 0, 0, 0, 0, npc, 0x100);

			break;

		case 40:
			npc->act_no = 41;
			npc->act_wait = 0;
			npc->ani_no = 10;
			// Fallthrough
		case 41:
			if (++npc->act_wait == 750)
			{
				npc->bits &= ~NPC_INTERACTABLE;
				npc->ani_no = 0;
			}

			if (npc->act_wait > 1000)
			{
				npc->act_no = 100;
				npc->ani_no = 0;
				SetNpChar(183, 0, 0, 0, 0, 0, npc, 0x100);

				if (GetNPCFlag(563))
					SetNpChar(182, 0, 0, 0, 0, 0, npc, 0x100);
				else
					SetNpChar(181, 0, 0, 0, 0, 0, npc, 0x100);
			}

			break;

		case 100:
			npc->ani_no = 0;
			npc->xm = (npc->xm * 7) / 8;
			npc->count1 = 0;

			if (npc->x > npc->tgt_x + (16 * 0x200))
			{
				npc->act_no = 200;
				npc->ani_no = 1;
				npc->direct = 0;
				npc->act_wait = Random(20, 60);
			}
			else if (npc->x < npc->tgt_x - (16 * 0x200))
			{
				npc->act_no = 300;
				npc->ani_no = 1;
				npc->direct = 2;
				npc->act_wait = Random(20, 60);
			}

			break;

		case 200:
			npc->xm -= 0x20;
			npc->direct = 0;

			if (npc->flag & 1)
				++npc->count1;
			else
				npc->count1 = 0;

			break;

		case 210:
			npc->xm -= 0x20;
			npc->direct = 0;

			if (npc->flag & 8)
				npc->act_no = 100;

			break;

		case 300:
			npc->xm += 0x20;
			npc->direct = 2;

			if (npc->flag & 4)
				++npc->count1;
			else
				npc->count1 = 0;

			break;

		case 310:
			npc->xm += 0x20;
			npc->direct = 2;

			if (npc->flag & 8)
				npc->act_no = 100;

			break;
	}

	if (gCurlyShoot_wait != 0)
		--gCurlyShoot_wait;

	if (gCurlyShoot_wait == 70)
		npc->count2 = 10;

	if (gCurlyShoot_wait == 60 && npc->flag & 8 && Random(0, 2))
	{
		npc->count1 = 0;
		npc->ym = -0x600;
		npc->ani_no = 1;
		PlaySoundObject(15, SOUND_MODE_PLAY);

		if (npc->x > npc->tgt_x)
			npc->act_no = 210;
		else
			npc->act_no = 310;
	}

	xx = npc->x - npc->tgt_x;
	yy = npc->y - npc->tgt_y;

	if (xx < 0)
		xx *= -1;

	if (npc->act_no == 100)
	{
		if (xx + (2 * 0x200) < yy)
			npc->ani_no = 5;
		else
			npc->ani_no = 0;
	}

	if (npc->act_no == 210 || npc->act_no == 310)
	{
		if (xx + (2 * 0x200) < yy)
			npc->ani_no = 6;
		else
			npc->ani_no = 1;
	}

	if (npc->act_no == 200 || npc->act_no == 300)
	{
		++npc->ani_wait;

		if (xx + (2 * 0x200) < yy)
			npc->ani_no = 6 + (npc->ani_wait / 4 % 4);
		else
			npc->ani_no = 1 + (npc->ani_wait / 4 % 4);

		if (npc->act_wait)
		{
			--npc->act_wait;

#ifdef FIX_BUGS
			// I assume this is what was intended
			if (npc->flag & 8 && npc->count1 > 10)
#else
			if (npc->flag && 8 && npc->count1 > 10)
#endif
			{
				npc->count1 = 0;
				npc->ym = -0x600;
				npc->act_no += 10;
				npc->ani_no = 1;
				PlaySoundObject(15, SOUND_MODE_PLAY);
			}
		}
		else
		{
			npc->act_no = 100;
			npc->ani_no = 0;
		}
	}

	if (npc->act_no >= 100 && npc->act_no < 500)
	{
		if (npc->x < gMC.x - (80 * 0x200) || npc->x > gMC.x + (80 * 0x200))
		{
#ifdef FIX_BUGS
			if (npc->flag & 5)
#else
			if (npc->flag && 5)
#endif
				npc->ym += 0x10;
			else
				npc->ym += 0x33;
		}
		else
		{
			npc->ym += 0x33;
		}
	}

	if (npc->xm > 0x300)
		npc->xm = 0x300;
	if (npc->xm < -0x300)
		npc->xm = -0x300;

	if (npc->ym > 0x5FF)
		npc->ym = 0x5FF;

	npc->x += npc->xm;
	npc->y += npc->ym;

	if (npc->act_no >= 100 && !(npc->flag & 8))
	{
		switch (npc->ani_no)
		{
			case 1000:
				break;

			default:
				if (xx + (2 * 0x200) < yy)
					npc->ani_no = 6;
				else
					npc->ani_no = 1;

				break;
		}
	}

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Curly AI Machine Gun
void ActNpc181(NPCHAR *npc)
{
	// [Mod] All horizontal coordinates shifted left by 64 (0x40) 
	// as per "sprite x -0x40" checklist item.
	RECT rcLeft[2] = {
		{152, 152, 168, 168},
		{168, 152, 184, 168},
	};

	RECT rcRight[2] = {
		{152, 168, 168, 184},
		{168, 168, 184, 184},
	};

	if (npc->pNpc == NULL)
		return;

	// Attachment and Animation Logic
	if (npc->pNpc->ani_no < 5)
	{
		// Walking/Idle stance
		npc->direct = (npc->pNpc->direct == 0) ? 0 : 2;
		npc->x = (npc->direct == 0) ? npc->pNpc->x - (8 * 0x200) : npc->pNpc->x + (8 * 0x200);
		npc->y = npc->pNpc->y;
		npc->ani_no = 0;
	}
	else
	{
		// Firing/Upper stance
		npc->direct = (npc->pNpc->direct == 0) ? 0 : 2;
		npc->x = npc->pNpc->x;
		npc->y = npc->pNpc->y - (10 * 0x200);
		npc->ani_no = 1;
	}

	// Visual "bobbing" while walking
	if (npc->pNpc->ani_no == 1 || npc->pNpc->ani_no == 3 || npc->pNpc->ani_no == 6 || npc->pNpc->ani_no == 8)
		npc->y -= 1 * 0x200;

	switch (npc->act_no)
	{
		case 0:
			// Wait for signal from parent to start shooting
			if (npc->pNpc->count2 == 10)
			{
				npc->pNpc->count2 = 0;
				npc->act_no = 10;
				npc->act_wait = 0;
			}
			break;

		case 10:
			// Rapid fire sequence
			if (++npc->act_wait % 6 == 1)
			{
				if (npc->ani_no == 0)
				{
					// Shooting Forward
					int offset_x = (npc->direct == 0) ? -(4 * 0x200) : (4 * 0x200);
					SetBullet(12, npc->x + offset_x, npc->y + (3 * 0x200), npc->direct);
					SetCaret(npc->x + offset_x, npc->y + (3 * 0x200), 3, 0);
				}
				else
				{
					// Shooting Upward
					int offset_x = (npc->direct == 0) ? -(2 * 0x200) : (2 * 0x200);
					SetBullet(12, npc->x + offset_x, npc->y - (4 * 0x200), 1);
					SetCaret(npc->x + offset_x, npc->y - (4 * 0x200), 3, 0);
				}
			}

			if (npc->act_wait == 60)
				npc->act_no = 0;
			break;
	}

	// Render with new modded RECTs
	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Curly AI Polar Star
void ActNpc182(NPCHAR *npc)
{
	RECT rcLeft[2] = {
		{184, 152, 200, 168},
		{200, 152, 216, 168},
	};

	RECT rcRight[2] = {
		{184, 168, 200, 184},
		{200, 168, 216, 184},
	};

	if (npc->pNpc == NULL)
		return;

	if (npc->pNpc->ani_no < 5)
	{
		if (npc->pNpc->direct == 0)
		{
			npc->direct = 0;
			npc->x = npc->pNpc->x - (8 * 0x200);
		}
		else
		{
			npc->direct = 2;
			npc->x = npc->pNpc->x + (8 * 0x200);
		}

		npc->y = npc->pNpc->y;
		npc->ani_no = 0;
	}
	else
	{
		if (npc->pNpc->direct == 0)
		{
			npc->direct = 0;
			npc->x = npc->pNpc->x;
		}
		else
		{
			npc->direct = 2;
			npc->x = npc->pNpc->x;
		}

		npc->y = npc->pNpc->y - (10 * 0x200);
		npc->ani_no = 1;
	}

	if (npc->pNpc->ani_no == 1 || npc->pNpc->ani_no == 3 || npc->pNpc->ani_no == 6 || npc->pNpc->ani_no == 8)
		npc->y -= 1 * 0x200;

	switch (npc->act_no)
	{
		case 0:
			if (npc->pNpc->count2 == 10)
			{
				npc->pNpc->count2 = 0;
				npc->act_no = 10;
				npc->act_wait = 0;
			}

			break;

		case 10:
			if (++npc->act_wait % 12 == 1)
			{
				if (npc->ani_no == 0)
				{
					if (npc->direct == 0)
					{
						SetBullet(6, npc->x - (4 * 0x200), npc->y + (3 * 0x200), 0);
						SetCaret(npc->x - (4 * 0x200), npc->y + (3 * 0x200), 3, 0);
					}
					else
					{
						SetBullet(6, npc->x + (4 * 0x200), npc->y + (3 * 0x200), 2);
						SetCaret(npc->x + (4 * 0x200), npc->y + (3 * 0x200), 3, 0);
					}
				}
				else
				{
					if (npc->direct == 0)
					{
						SetBullet(6, npc->x - (2 * 0x200), npc->y - (4 * 0x200), 1);
						SetCaret(npc->x - (2 * 0x200), npc->y - (4 * 0x200), 3, 0);
					}
					else
					{
						SetBullet(6, npc->x + (2 * 0x200), npc->y - (4 * 0x200), 1);
						SetCaret(npc->x + (2 * 0x200), npc->y - (4 * 0x200), 3, 0);
					}
				}
			}

			if (npc->act_wait == 60)
				npc->act_no = 0;

			break;
	}

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Curly Air Tank Bubble
void ActNpc183(NPCHAR *npc)
{
	RECT rect[2] = {
		{56, 96, 80, 120},
		{80, 96, 104, 120},
	};

	if (npc->pNpc == NULL)
		return;

	switch (npc->act_no)
	{
		case 0:
			npc->x = npc->pNpc->x;
			npc->y = npc->pNpc->y;
			npc->act_no = 1;
			break;
	}

	npc->x += (npc->pNpc->x - npc->x) / 2;
	npc->y += (npc->pNpc->y - npc->y) / 2;

	if (++npc->ani_wait > 1)
	{
		npc->ani_wait = 0;
		++npc->ani_no;
	}

	if (npc->ani_no > 1)
		npc->ani_no = 0;

	if (npc->pNpc->flag & 0x100)
		npc->rect = rect[npc->ani_no];
	else
		npc->rect.right = 0;
}

// Big Shutter - NPC 184 (Simplified Mod Version)
void ActNpc184(NPCHAR *npc)
{
	static const RECT rect[4] = {
		{0, 64, 32, 96},
		{32, 64, 64, 96},
		{64, 64, 96, 96},
		{32, 64, 64, 96},
	};

	// Act 0: Initialization
	if (npc->act_no != 1)
	{
		npc->x += 8 * 0x200;
		npc->y += 8 * 0x200;
		npc->act_no = 1;
	}

	// Simple looping animation
	if (++npc->ani_wait > 10)
	{
		npc->ani_wait = 0;
		++npc->ani_no;
	}

	if (npc->ani_no > 3)
		npc->ani_no = 0;

	npc->rect = rect[npc->ani_no];
}

#include "Map.h"

// Small Shutter replacement
void ActNpc185(NPCHAR *npc)
{
    // Determine the target state based on the script flag
    int flag_state = GetNPCFlag(npc->code_flag);
    
    // If bit 0x1000 (4096) is set, invert the open/closed logic
    if (npc->bits & 0x1000)
        flag_state ^= 1;
	int tile_y;
    switch (npc->act_no)
    {
        case 0:
            if (npc->code_flag == 0)
            {
                npc->cond = 0;
                break;
            }

            // Note: The assembly sets count2 to 1 here if bit 0x100 is set, but it 
            // gets unconditionally overwritten by the raycast below. Included for parity.
            if (npc->bits & 0x100)
                npc->count2 = 1;

            // Determine extension/retraction speed based on bit 0x400
            if (npc->bits & 0x400)
                npc->count1 = 1; // Fast
            else
                npc->count1 = 3; // Slow

            // Determine initial visual state based on flag
            if (flag_state == 1)
            {
                npc->act_no = 3;
                npc->ani_no = 1;
            }
            else
            {
                npc->act_no = 1;
                npc->ani_no = 0;
            }

            // Raycast downward to find the floor (limit 16 tiles)
            // x/y >> 13 is equivalent to x/y / 0x2000 (converts subpixels to tile coords)
             tile_y = npc->y / 0x2000;
            int i;
            for (i = 0; i < 16; ++i)
            {
                unsigned char attr = GetAttribute(npc->x / 0x2000, ++tile_y);
                
                // Stop at solid block (0x05), spikes (0x18), or solid-water (0x41)
                if (attr == 0x05 || attr == 0x18 || attr == 0x41)
                    break;
            }

            // count2 stores the total pixel length to the floor
            npc->count2 = (tile_y * 0x2000) - npc->y;
            
            // Set collision bits: Solid (0x01) + Bouncy (0x02) + SolidTop (0x04) + SolidRear (0x40)
            npc->bits |= 0x47;
            npc->direct = 3;

            // If starting extended, set collision height immediately
            if (npc->ani_no != 0)
                npc->hit.bottom = npc->count2;

            break;

        case 1: // Idle Retracted
            if (npc->ani_no != flag_state)
            {
                npc->ani_no = flag_state;
                npc->act_no = 2;
                npc->act_wait = npc->count1;
                PlaySoundObject(39, SOUND_MODE_PLAY);
            }
            break;

        case 2: // Extending
            npc->act_wait--;
            if (npc->act_wait == 0)
            {
                PlaySoundObject(46, SOUND_MODE_PLAY);
                npc->hit.bottom += 0x400; // Grow by 2 pixels

                if (npc->hit.bottom >= npc->count2)
                {
                    npc->act_no = 3;
                }
                else
                {
                    npc->act_wait = npc->count1;
                }
            }
            break;

        case 3: // Idle Extended
            if (npc->ani_no != flag_state)
            {
                npc->ani_no = flag_state;
                npc->act_no = 4;
                npc->act_wait = npc->count1;
                PlaySoundObject(39, SOUND_MODE_PLAY);
            }
            break;

        case 4: // Retracting
            npc->act_wait--;
            if (npc->act_wait == 0)
            {
                PlaySoundObject(46, SOUND_MODE_PLAY);
                npc->hit.bottom -= 0x400; // Shrink by 2 pixels

                if (npc->hit.bottom <= 0x1000) // Minimum 8 pixels
                {
                    npc->act_no = 1;
                }
                else
                {
                    npc->act_wait = npc->count1;
                }
            }
            break;
    }

    // Synchronize the rendering view height to match the dynamic collision height
    npc->view.bottom = npc->hit.bottom;
}
// Lift block - NPC 186
void ActNpc186(NPCHAR *npc)
{
	static const RECT rc[4] = {
		{48, 48, 64, 64},
		{64, 48, 80, 64},
		{80, 48, 96, 64},
		{64, 48, 80, 64},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			// Fallthrough
		case 1:
			break;

		case 10:
			npc->act_no = 11;
			npc->ani_no = 1;
			npc->act_wait = 0;
			// [MOD] Instead of NPC_IGNORE_SOLIDITY (8), it sets NPC_SOLID_SOFT (1)
			npc->bits |= NPC_SOLID_SOFT; 
			// Fallthrough
		case 11:
			// [MOD] Speed increased from 128 (0x80) to 384 (0x180)
			switch (npc->direct)
			{
				case 0: npc->xm = -384; break; // Left
				case 1: npc->ym = -384; break; // Up
				case 2: npc->xm = 384;  break; // Right
				case 3: npc->ym = 384;  break; // Down
			}

			npc->x += npc->xm;
			npc->y += npc->ym;
			++npc->act_wait;
			break;
	}

	// Animation remains vanilla (6 FPS rotation)
	if (++npc->ani_wait > 10)
	{
		npc->ani_wait = 0;
		++npc->ani_no;
	}

	if (npc->ani_no > 3)
		npc->ani_no = 0;

	npc->rect = rc[npc->ani_no];
}


// Fuzz Core - NPC 187 (Modded Version)
void ActNpc187(NPCHAR *npc)
{
	int i;

	static const RECT rcLeft[2] = {
		{224, 104, 256, 136},
		{256, 104, 288, 136},
	};

	static const RECT rcRight[2] = {
		{224, 136, 256, 168},
		{256, 136, 288, 168},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->tgt_x = npc->x;
			npc->tgt_y = npc->y;
			npc->count1 = 120;
			npc->act_wait = Random(0, 50);

			// Spawn 5 Fuzz children (NPC 188)
			for (i = 0; i < 5; ++i)
				SetNpChar(188, 0, 0, 0, 0, 51 * i, npc, 0x100);
			
			// Fallthrough
		case 1:
			if (++npc->act_wait > 49)
			{
				npc->act_wait = 0;
				npc->act_no = 2;
				npc->ym = 0x300;
			}
			break;

		case 2:
			npc->count1 += 4;

			// Face the player
			if (gMC.x < npc->x)
				npc->direct = 0;
			else
				npc->direct = 2;

			// Vertical homing logic toward the origin point (tgt_y)
			if (npc->tgt_y < npc->y) npc->ym -= 0x10;
			if (npc->tgt_y > npc->y) npc->ym += 0x10;

			// Clamp vertical speed
			if (npc->ym > 0x355)  npc->ym = 0x355;
			if (npc->ym < -0x355) npc->ym = -0x355;

			break;
	}

	npc->x += npc->xm;
	npc->y += npc->ym;

	// [MOD] New Death/Transformation Logic
	// If health is below 51, check for custom flag 0xAF0 (2800)
	if (npc->life < 51)
	{
		npc->life = 0;
		if (GetNPCFlag(0xAF0))
		{
			// Spawn NPC 166 (The modded bouncing debris object)
			SetNpChar(166, npc->x, npc->y, 0, 0, 0, NULL, 0x100);
			
			// FUN_00493e45 is a custom mod function (replaces LoseNpChar/VanishNpChar)
			LoseNpChar(npc, 1); 
			
			npc->cond = 0; // Destroy the core
			return;        // Exit to prevent rect assignment
		}
	}

	// Standard animation
	if (++npc->ani_wait > 2)
	{
		npc->ani_wait = 0;
		if (++npc->ani_no > 1)
			npc->ani_no = 0;
	}

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}
// Fuzz
void ActNpc188(NPCHAR *npc)
{
	unsigned char deg;

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			// npc->direct is used to store the starting angle offset (0-255)
			npc->count1 = npc->direct;
			break;

		case 1:
			// [Mod] Check if parent (Droll) is alive and the correct ID (0xBB / 187)
			if (npc->pNpc->code_char == 187 && npc->pNpc->cond & 0x80)
			{
				// [Mod] Read parent's current damage value into child's count2
				// This allows the projectiles to inherit the boss's current power.
				npc->count2 = npc->pNpc->damage;
				
				// [Mod] Bits logic: npc->bits &= 4 (Disables shootable/invuln, keeps only solid-bit)
				npc->bits &= 4;

				// Orbit logic
				deg = (unsigned char)(npc->pNpc->count1 + npc->count1);
				npc->x = npc->pNpc->x + (GetSin(deg) * 20); // 0x14
				npc->y = npc->pNpc->y + (GetCos(deg) * 32); // 0x20
			}
			else
			{
				// [Mod] Parent is dead: Child becomes an active threat
				// Restore damage from saved count2 value
				npc->damage = npc->count2;
				
				// [Mod] Toggle Bits: bits ^= 0x8020 (Toggles SHOOTABLE and Bit 15)
				npc->bits ^= 0x8020;

				// [Mod] Scatter velocity significantly increased from ±0x200 to ±0x600
				npc->xm = Random(-0x600, 0x600);
				npc->ym = Random(-0x600, 0x600);
				npc->act_no = 10;
			}
			break;

		case 10:
			// Homing behavior (Accelerate towards player)
			if (gMC.x < npc->x) npc->xm -= 0x20;
			else npc->xm += 0x20;

			if (gMC.y < npc->y) npc->ym -= 0x20;
			else npc->ym += 0x20;

			// [Mod] Velocity clamping
			if (npc->xm > 0x800) npc->xm = 0x800;
			if (npc->xm < -0x800) npc->xm = -0x800;

			if (npc->ym > 0x200) npc->ym = 0x200;
			if (npc->ym < -0x200) npc->ym = -0x200;

			npc->x += npc->xm;
			npc->y += npc->ym;
			break;
	}

	// [Mod] Always face the player
	if (gMC.x < npc->x) npc->direct = 0;
	else npc->direct = 2;

	// Animation logic
	if (++npc->ani_wait > 2)
	{
		npc->ani_wait = 0;
		++npc->ani_no;
	}
	if (npc->ani_no > 1) npc->ani_no = 0;

	// [Mod] RECT Tables verified against ASM stack initialization
	RECT rcLeft[2] = {
		{288, 104, 304, 120},
		{304, 104, 320, 120},
	};

	RECT rcRight[2] = {
		{288, 120, 304, 136},
		{304, 120, 320, 136},
	};

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}
// Unused homing flame object (possibly related to the Core?)
void ActNpc189(NPCHAR *npc)
{
	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->xm = -0x40;
			// Fallthrough
		case 1:
			npc->y += npc->ym;

			if (++npc->act_wait > 0x100)
				npc->act_no = 10;

			break;

		case 10:
			if (gMC.x < npc->x)
				npc->xm -= 8;
			else
				npc->xm += 8;

			if (gMC.y < npc->y)
				npc->ym -= 8;
			else
				npc->ym += 8;

			if (npc->xm > 0x400)
				npc->xm = 0x400;
			if (npc->xm < -0x400)
				npc->xm = -0x400;

			if (npc->ym > 0x400)
				npc->ym = 0x400;
			if (npc->ym < -0x400)
				npc->ym = -0x400;

			npc->x += npc->xm;
			npc->y += npc->ym;

			break;
	}

	if (gMC.x < npc->x)
		npc->direct = 0;
	else
		npc->direct = 2;

	if (++npc->ani_wait > 2)
	{
		npc->ani_wait = 0;
		++npc->ani_no;
	}

	if (npc->ani_no > 2)
		npc->ani_no = 0;

	RECT rect[3] = {
		{224, 184, 232, 200},
		{232, 184, 240, 200},
		{240, 184, 248, 200},
	};

	npc->rect = rect[npc->ani_no];
}

// Broken robot
void ActNpc190(NPCHAR *npc)
{
	RECT rect[2] = {
		{192, 32, 208, 48},
		{208, 32, 224, 48},
	};

	int i;

	switch (npc->act_no)
	{
		case 0:
			npc->ani_no = 0;
			break;

		case 10:
			PlaySoundObject(72, SOUND_MODE_PLAY);

			for (i = 0; i < 8; ++i)
				SetNpChar(4, npc->x, npc->y + (Random(-8, 8) * 0x200), Random(-8, -2) * 0x200, Random(-3, 3) * 0x200, 0, NULL, 0x100);

			npc->cond = 0;
			break;

		case 20:
			if (++npc->ani_wait > 10)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 1)
				npc->ani_no = 0;

			break;
	}

	npc->rect = rect[npc->ani_no];
}

// Water level
void ActNpc191(NPCHAR *npc)
{
	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 10;
			npc->tgt_y = npc->y;
			npc->ym = 0x200;
			// Fallthrough
		case 10:
			if (npc->y < npc->tgt_y)
				npc->ym += 4;
			else
				npc->ym -= 4;

			if (npc->ym < -0x100)
				npc->ym = -0x100;
			if (npc->ym > 0x100)
				npc->ym = 0x100;

			npc->y += npc->ym;
			break;

		case 20:
			npc->act_no = 21;
			npc->act_wait = 0;
			// Fallthrough
		case 21:
			if (npc->y < npc->tgt_y)
				npc->ym += 4;
			else
				npc->ym -= 4;

			if (npc->ym < -0x200)
				npc->ym = -0x200;
			if (npc->ym > 0x200)
				npc->ym = 0x200;

			npc->y += npc->ym;

			if (++npc->act_wait > 1000)
				npc->act_no = 22;

			break;

		case 22:
			if (npc->y < 0)
				npc->ym += 4;
			else
				npc->ym -= 4;

			if (npc->ym < -0x200)
				npc->ym = -0x200;
			if (npc->ym > 0x200)
				npc->ym = 0x200;

			npc->y += npc->ym;

			if (npc->y < 64 * 0x200 || gSuperYpos != 0)
			{
				npc->act_no = 21;
				npc->act_wait = 0;
			}
			break;

		case 30:
			if (npc->y < 0)
				npc->ym += 4;
			else
				npc->ym -= 4;

			if (npc->ym < -0x200)
				npc->ym = -0x200;
			if (npc->ym > 0x100)
				npc->ym = 0x100;

			npc->y += npc->ym;
			break;
	}

	gWaterY = npc->y;

	npc->rect.right = 0;
	npc->rect.bottom = 0;
}

// Scooter
void ActNpc192(NPCHAR *npc)
{
	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->view.back = 16 * 0x200;
			npc->view.front = 16 * 0x200;
			npc->view.top = 8 * 0x200;
			npc->view.bottom = 8 * 0x200;
			break;

		case 10:
			npc->act_no = 11;
			npc->ani_no = 1;
			npc->view.top = 16 * 0x200;
			npc->view.bottom = 16 * 0x200;
			npc->y -= 5 * 0x200;
			break;

		case 20:
			npc->act_no = 21;
			npc->act_wait = 1;
			npc->tgt_x = npc->x;
			npc->tgt_y = npc->y;
			// Fallthrough
		case 21:
			npc->x = npc->tgt_x + (Random(-1, 1) * 0x200);
			npc->y = npc->tgt_y + (Random(-1, 1) * 0x200);

			if (++npc->act_wait > 30)
				npc->act_no = 30;

			break;

		case 30:
			npc->act_no = 31;
			npc->act_wait = 1;
			npc->xm = -0x800;
			npc->x = npc->tgt_x;
			npc->y = npc->tgt_y;
			PlaySoundObject(44, SOUND_MODE_PLAY);
			// Fallthrough
		case 31:
			npc->xm += 0x20;
			npc->x += npc->xm;
			++npc->act_wait;
			npc->y = npc->tgt_y + (Random(-1, 1) * 0x200);

			if (npc->act_wait > 10)
				npc->direct = 2;

			if (npc->act_wait > 200)
				npc->act_no = 40;

			break;

		case 40:
			npc->act_no = 41;
			npc->act_wait = 2;
			npc->direct = 0;
			npc->y -= 48 * 0x200;
			npc->xm = -0x1000;
			// Fallthrough
		case 41:
			npc->x += npc->xm;
			npc->y += npc->ym;

			npc->act_wait += 2;

			if (npc->act_wait > 1200)
				npc->cond = 0;

			break;
	}

	if (npc->act_wait % 4 == 0 && npc->act_no >= 20)
	{
		PlaySoundObject(34, SOUND_MODE_PLAY);

		if (npc->direct == 0)
			SetCaret(npc->x + (10 * 0x200), npc->y + (10 * 0x200), 7, 2);
		else
			SetCaret(npc->x - (10 * 0x200), npc->y + (10 * 0x200), 7, 0);
	}

	RECT rcLeft[2] = {
		{224, 64, 256, 80},
		{256, 64, 288, 96},
	};

	RECT rcRight[2] = {
		{224, 80, 256, 96},
		{288, 64, 320, 96},
	};

	if (npc->direct == 0)
		npc->rect = rcLeft[npc->ani_no];
	else
		npc->rect = rcRight[npc->ani_no];
}

// Scooter (broken)
void ActNpc193(NPCHAR *npc)
{
	RECT rc = {256, 96, 320, 112};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->y = npc->y;	// This line probably isn't accurate to the original source code, but it produces the same assembly
			npc->x += 24 * 0x200;
			break;
	}

	npc->rect = rc;
}

// Blue robot (broken)
void ActNpc194(NPCHAR *npc)
{
	RECT rc = {192, 120, 224, 128};

	if (npc->act_no == 0)
	{
		npc->act_no = 1;
		npc->y += 4 * 0x200;
	}

	npc->rect = rc;
}

// Grate
void ActNpc195(NPCHAR *npc)
{
	RECT rc = {112, 64, 128, 80};
	npc->rect = rc;
}

// Ironhead motion wall
void ActNpc196(NPCHAR *npc)
{
	RECT rcLeft = {112, 64, 144, 80};
	RECT rcRight = {112, 80, 144, 96};

	npc->x -= 6 * 0x200;

	if (npc->x <= 19 * 0x10 * 0x200)
		npc->x += 22 * 0x10 * 0x200;

	if (npc->direct == 0)
		npc->rect = rcLeft;
	else
		npc->rect = rcRight;
}

// Porcupine Fish
void ActNpc197(NPCHAR *npc)
{
	RECT rc[4] = {
		{0, 0, 16, 16},
		{16, 0, 32, 16},
		{32, 0, 48, 16},
		{48, 0, 64, 16},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 10;
			npc->ani_wait = 0;
			npc->ym = Random(-0x200, 0x200);
			npc->xm = 0x800;
			// Fallthrough
		case 10:
			if (++npc->ani_wait > 2)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 1)
				npc->ani_no = 0;

			if (npc->xm < 0)
			{
				npc->damage = 3;
				npc->act_no = 20;
			}

			break;

		case 20:
			npc->damage = 3;

			if (++npc->ani_wait > 0)
			{
				npc->ani_wait = 0;
				++npc->ani_no;
			}

			if (npc->ani_no > 3)
				npc->ani_no = 2;

			if (npc->x < 48 * 0x200)
			{
				npc->destroy_voice = 0;
				// MOD: Call new lose function 0x493e45 instead of LoseNpChar
				LoseNpChar(npc, TRUE);
			}

			break;
	}

	if (npc->flag & 2)
		npc->ym = 0x200;
	if (npc->flag & 8)
		npc->ym = -0x200;

	npc->xm -= 12;

	npc->x += npc->xm;
	npc->y += npc->ym;

	npc->rect = rc[npc->ani_no];
}

// Ironhead projectile
void ActNpc198(NPCHAR *npc)
{
	RECT rcRight[3] = {
		{208, 48, 224, 72},
		{224, 48, 240, 72},
		{240, 48, 256, 72},
	};

	switch (npc->act_no)
	{
		case 0:
			if (++npc->act_wait > 20)
			{
				npc->act_no = 1;
				npc->xm = 0;
				npc->ym = 0;
				npc->count1 = 0;
			}

			break;

		case 1:
			npc->xm += 0x20;
			break;
	}

	if (++npc->ani_wait > 0)
	{
		npc->ani_wait = 0;
		++npc->ani_no;
	}

	if (npc->ani_no > 2)
		npc->ani_no = 0;

	npc->x += npc->xm;
	npc->y += npc->ym;

	npc->rect = rcRight[npc->ani_no];

	if (++npc->count1 > 100)
		npc->cond = 0;

	if (npc->count1 % 4 == 1)
		PlaySoundObject(46, SOUND_MODE_PLAY);
}

// Water/wind particles
void ActNpc199(NPCHAR *npc)
{
	RECT rect[5] = {
		{72, 16, 74, 18},
		{74, 16, 76, 18},
		{76, 16, 78, 18},
		{78, 16, 80, 18},
		{80, 16, 82, 18},
	};

	switch (npc->act_no)
	{
		case 0:
			npc->act_no = 1;
			npc->ani_no = Random(0, 2);

			switch (npc->direct)
			{
				case 0:
					npc->xm = -1;
					break;

				case 1:
					npc->ym = -1;
					break;

				case 2:
					npc->xm = 1;
					break;

				case 3:
					npc->ym = 1;
					break;
			}

			npc->xm *= (Random(4, 8) * 0x200) / 2;
			npc->ym *= (Random(4, 8) * 0x200) / 2;
			break;
	}

	if (++npc->ani_wait > 6)
	{
		npc->ani_wait = 0;
		++npc->ani_no;
	}

	if (npc->ani_no > 4)
	{
		npc->cond = 0;
	#ifdef FIX_BUGS
		return;	// The code below will use 'ani_no' to access 'rect', even though it's now too high
	#endif
	}

	npc->x += npc->xm;
	npc->y += npc->ym;

	npc->rect = rect[npc->ani_no];
}
