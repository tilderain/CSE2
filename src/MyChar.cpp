// THIS IS DECOMPILED PROPRIETARY CODE - USE AT YOUR OWN RISK.
//
// The original code belongs to Daisuke "Pixel" Amaya.
//
// Modifications and custom code are under the MIT licence.
// See LICENCE.txt for details.

#include "MyChar.h"

#include <stddef.h>
#include <string.h>

#include "WindowsWrapper.h"

#include "ArmsItem.h"
#include "Caret.h"
#include "CommonDefines.h"
#include "Draw.h"
#include "Flags.h"
#include "Game.h"
#include "KeyControl.h"
#include "MycParam.h"
#include "NpChar.h"
#include "Sound.h"
#include "Star.h"
#include "TextScr.h"
#include "ValueView.h"

MYCHAR gMC;

void InitMyChar(void)
{
	memset(&gMC, 0, sizeof(MYCHAR));
	gMC.cond = 0x80;
	gMC.direct = 2;

	gMC.view.back = 8 * 0x200;
	gMC.view.top = 8 * 0x200;
	gMC.view.front = 8 * 0x200;
	gMC.view.bottom = 8 * 0x200;

	gMC.hit.back = 5 * 0x200;
	gMC.hit.top = 8 * 0x200;
	gMC.hit.front = 5 * 0x200;
	gMC.hit.bottom = 8 * 0x200;

	gMC.life = 5;
	gMC.max_life = 5;
	gMC.unit = 0;

	// This is initialized with the values the game uses in vanilla
	gMC.physics_normal.max_dash = 0x32C;
	gMC.physics_normal.max_move = 0x5FF;
	gMC.physics_normal.gravity1 = 0x50;
	gMC.physics_normal.gravity2 = 0x20;
	gMC.physics_normal.dash1 = 0x55;
	gMC.physics_normal.dash2 = 0x20;
	gMC.physics_normal.resist = 0x33;
	gMC.physics_normal.jump = 0x500;

	gMC.physics_underwater.max_dash = 0x196;
	gMC.physics_underwater.max_move = 0x2FF;
	gMC.physics_underwater.gravity1 = 0x28;
	gMC.physics_underwater.gravity2 = 0x10;
	gMC.physics_underwater.dash1 = 0x2A;
	gMC.physics_underwater.dash2 = 0x10;
	gMC.physics_underwater.resist = 0x19;
	gMC.physics_underwater.jump = 0x280;

	gMC.no_splash_or_air_limit_underwater = 0;
}

void AnimationMyChar(BOOL bKey)
{
	RECT rcLeft[12] = {
		{0, 0, 16, 16},
		{16, 0, 32, 16},
		{0, 0, 16, 16},
		{32, 0, 48, 16},
		{0, 0, 16, 16},
		{48, 0, 64, 16},
		{64, 0, 80, 16},
		{48, 0, 64, 16},
		{80, 0, 96, 16},
		{48, 0, 64, 16},
		{96, 0, 112, 16},
		{112, 0, 128, 16},
	};

	RECT rcRight[12] = {
		{0, 16, 16, 32},
		{16, 16, 32, 32},
		{0, 16, 16, 32},
		{32, 16, 48, 32},
		{0, 16, 16, 32},
		{48, 16, 64, 32},
		{64, 16, 80, 32},
		{48, 16, 64, 32},
		{80, 16, 96, 32},
		{48, 16, 64, 32},
		{96, 16, 112, 32},
		{112, 16, 128, 32},
	};

	if (gMC.cond & 2)
		return;

	if (gMC.flag & 8)
	{
		if (gMC.cond & 1)
		{
			gMC.ani_no = 11;
		}
		else if (gKey & gKeyUp && gKey & (gKeyLeft | gKeyRight) && bKey)
		{
			gMC.cond |= 4;

			if (++gMC.ani_wait > 4)
			{
				gMC.ani_wait = 0;

				if (++gMC.ani_no == 7 || gMC.ani_no == 9)
					PlaySoundObject(24, SOUND_MODE_PLAY);
			}

			if (gMC.ani_no > 9 || gMC.ani_no < 6)
				gMC.ani_no = 6;
		}
		else if (gKey & (gKeyLeft | gKeyRight) && bKey)
		{
			gMC.cond |= 4;

			if (++gMC.ani_wait > 4)
			{
				gMC.ani_wait = 0;

				if (++gMC.ani_no == 2 || gMC.ani_no == 4)
					PlaySoundObject(24, SOUND_MODE_PLAY);
			}

			if (gMC.ani_no > 4 || gMC.ani_no < 1)
				gMC.ani_no = 1;
		}
		else if (gKey & gKeyUp && bKey)
		{
			if (gMC.cond & 4)
				PlaySoundObject(24, SOUND_MODE_PLAY);

			gMC.cond &= ~4;
			gMC.ani_no = 5;
		}
		else
		{
			if (gMC.cond & 4)
				PlaySoundObject(24, SOUND_MODE_PLAY);

			gMC.cond &= ~4;
			gMC.ani_no = 0;
		}
	}
	else if (gMC.up)
	{
		gMC.ani_no = 6;
	}
	else if (gMC.down)
	{
		gMC.ani_no = 10;
	}
	else
	{
		if (gMC.ym > 0)
			gMC.ani_no = 1;
		else
			gMC.ani_no = 3;
	}

	if (gMC.direct == 0)
		gMC.rect = rcLeft[gMC.ani_no];
	else
		gMC.rect = rcRight[gMC.ani_no];
}

void ShowMyChar(BOOL bShow)
{
	if (bShow)
		gMC.cond &= ~2;
	else
		gMC.cond |= 2;
}

#include "Backends/Rendering.h"
extern char gGrappleState;   // byte_493804
void PutMyChar(int fx, int fy)
{
	int i;
	int arms_offset_y;
	int hand_x, hand_y;
	int diff_x, diff_y;
	int step_x, step_y;
	int draw_x, draw_y;

	// New global variables for the Grapple/Tether system
	// These correspond to the addresses found in the mod's code cave

	extern unsigned char gCustomWeaponDir;     // byte_493805
	extern int gGrappleX;                // dword_4937F8
	extern int gGrappleY;                // dword_4937FC
	extern int gGrappleLength;             // dword_493818


	float flicker = (float)Random(100, 100); 

    DrawLight(gMC.x, gMC.y, fx, fy, flicker, 255, 200, 150, 128);

	// Standard conditions: Don't draw if Quote is inactive or hidden
	if (!(gMC.cond & 0x80) || gMC.cond & 2)
		return;

	// [MOD] Grapple Orientation Logic
	// Force Quote to look at/aim toward the anchor point if State is 2 (Attached)
	if (gGrappleState == 2 && gSelectedArms == 5)
	{
		int centerX = gGrappleX + (gGrappleLength / 2);

		if (gMC.x < centerX && centerX - gGrappleLength < gMC.x)
		{
			// Directly above or below the anchor
			gMC.down = (gMC.y <= gGrappleY);
			gMC.up = !gMC.down;
			gMC.ani_no = gMC.down ? 10 : 6;
		}
		else
		{
			// Aiming diagonally or horizontally toward the anchor
			gMC.down = (gMC.y <= gGrappleY);
			gMC.up = !gMC.down;
			gMC.direct = (gMC.x < gGrappleX) ? 2 : 0;
			gMC.ani_no = 6;
		}
	}

	// Weapon Sprite Calculation
	gMC.rect_arms.left = (gArmsData[gSelectedArms].code % 13) * 24;
	gMC.rect_arms.right = gMC.rect_arms.left + 24;
	gMC.rect_arms.top = (gArmsData[gSelectedArms].code / 13) * 96;

	// [MOD] If Grapple is active, shift to the specialized weapon row (+96)
	if (gSelectedArms == 5 && gGrappleState != 0)
		gMC.rect_arms.top += 96;

	gMC.rect_arms.bottom = gMC.rect_arms.top + 16;

	// Standard Direction/Aiming logic using modded 24x96 per-weapon layout
	if (gMC.direct == 2)
	{
		gMC.rect_arms.top += 16;
		gMC.rect_arms.bottom += 16;
	}

	if (gMC.up)
	{
		arms_offset_y = -4;
		gMC.rect_arms.top += 32;
		gMC.rect_arms.bottom += 32;
	}
	else if (gMC.down)
	{
		arms_offset_y = 4;
		gMC.rect_arms.top += 64;
		gMC.rect_arms.bottom += 64;
	}
	else
	{
		arms_offset_y = 0;
	}

	// Nudge weapon 1px if stepping
	if (gMC.ani_no == 1 || gMC.ani_no == 3 || gMC.ani_no == 6 || gMC.ani_no == 8)
		++gMC.rect_arms.top;

	// Draw Weapon
	if (gMC.direct == 0)
		PutBitmap3(&grcGame,
			SubpixelToScreenCoord(gMC.x - gMC.view.front) - SubpixelToScreenCoord(fx) - PixelToScreenCoord(8),
			SubpixelToScreenCoord(gMC.y - gMC.view.top) - SubpixelToScreenCoord(fy) + PixelToScreenCoord(arms_offset_y),
			&gMC.rect_arms, SURFACE_ID_ARMS);
	else
		PutBitmap3(&grcGame,
			SubpixelToScreenCoord(gMC.x - gMC.view.front) - SubpixelToScreenCoord(fx),
			SubpixelToScreenCoord(gMC.y - gMC.view.top) - SubpixelToScreenCoord(fy) + PixelToScreenCoord(arms_offset_y),
			&gMC.rect_arms, SURFACE_ID_ARMS);

	// Handle Damage Flash
	if (gMC.shock / 2 % 2)
		return;

	// Draw Player Body
	RECT rect = gMC.rect;
	rect.top += 32 * gMIMCurrentNum;
	rect.bottom += 32 * gMIMCurrentNum;

	if (gMC.equip & EQUIP_MIMIGA_MASK)
	{
		rect.top += 32;
		rect.bottom += 32;
	}

	PutBitmap3(&grcGame,
		SubpixelToScreenCoord(gMC.x - gMC.view.front) - SubpixelToScreenCoord(fx),
		SubpixelToScreenCoord(gMC.y - gMC.view.top) - SubpixelToScreenCoord(fy),
		&rect, SURFACE_ID_MY_CHAR);

	// Draw Air Tank / Bubbles
	RECT rcBubble[2] = { {56, 96, 80, 120}, {80, 96, 104, 120} };
	++gMC.bubble;
	if ((gMC.equip & EQUIP_AIR_TANK && gMC.flag & 0x100) || gMC.unit == 1)
	{
		PutBitmap3(&grcGame,
			SubpixelToScreenCoord(gMC.x) - PixelToScreenCoord(12) - SubpixelToScreenCoord(fx),
			SubpixelToScreenCoord(gMC.y) - PixelToScreenCoord(12) - SubpixelToScreenCoord(fy),
			&rcBubble[(gMC.bubble / 2) % 2], SURFACE_ID_CARET);
	}

	// [MOD] Grapple Rope Rendering
	if (gGrappleState != 0)
	{
		// 1. Determine "Hand" position (Where rope connects to the gun)
		if (gSelectedArms == 5)
		{
			if (gMC.up == 0 && gMC.down == 0)
			{
				// Horizontal
				draw_y = gMC.y + 0x1000;
				draw_x = (gMC.direct == 0) ? -0x400 : 0xC00;
				draw_x = (draw_x * 2) + gMC.x;
			}
			else
			{
				// Vertical/Diagonal
				draw_x = gMC.x + ((gMC.direct == 0) ? 0x400 : 0x1200);
				draw_y = gMC.y + (gMC.up ? -0x600 : 0x1800);
			}
		}
		else
		{
			// Default fallback for other weapons
			draw_x = gMC.x + gMC.view.front;
			draw_y = gMC.y + gMC.view.bottom;
		}

		// 2. Linear Interpolation for 32 Rope Segments
		diff_x = gGrappleX - draw_x;
		diff_y = gGrappleY - draw_y;
		
		step_x = diff_x / 32;
		step_y = diff_y / 32;

		// 0xC0, 0x18, 0xC6, 0x1E
		RECT rcRope = { 192, 24, 198, 30 }; 

		for (i = 0; i < 32; ++i)
		{
			// Increment segment position
			draw_x += step_x;
			draw_y += step_y;

			PutBitmap3(&grcGame,
				SubpixelToScreenCoord(draw_x - gMC.view.front) - SubpixelToScreenCoord(fx),
				SubpixelToScreenCoord(draw_y - gMC.view.top) - SubpixelToScreenCoord(fy),
				&rcRope, SURFACE_ID_MY_CHAR);
		}
	}
}
#include <math.h>

// Modder-added Global Variables (mapped to addresses in the 0x49xxxx range)
extern char gGrappleState;      // DAT_00493804 (0 = Normal, 2 = Grappling)
int gGrappleX;          // DAT_004937f8 (Pivot X coordinate)
int gGrappleY;          // DAT_004937fc (Pivot Y coordinate)
int gGrappleLength;     // DAT_00493818 (Length of the rope)
int gGrappleMomentum;   // DAT_0049381c (Angular velocity / swing momentum)

// Replaces the vanilla ActMyChar_Normal

void ActMyChar_Normal(BOOL bKey)
{
	int max_dash;
	int max_move;
	int gravity1;
	int gravity2;
	int jump;
	int dash1;
	int dash2;
	int resist;

	int a, x;
	if (gDebug.bNoclip)
	{
		if (gKey & gKeyLeft)
		{
			gMC.x -= 0x1000;
			gMC.direct = 0;
		}
		if (gKey & gKeyRight)
		{
			gMC.x += 0x1000;
			gMC.direct = 2;
		}
		if (gKey & gKeyUp)
		{
			gMC.y -= 0x1000;
		}
		if (gKey & gKeyDown)
		{
			gMC.y += 0x1000;
		}

		gMC.tgt_x = gMC.x;
		gMC.tgt_y = gMC.y;

		return;
	}
	// Do not run physics if dead or hidden
	if (gMC.cond & 2)
		return;

	// --- 1. Base Physics Constants Setup ---
	if (gMC.flag & 0x100) // Underwater
	{
		max_dash = 0x196;
		gravity1 = 0x28;  // <--- FIXED
		gravity2 = 0x10;  // <--- FIXED
		jump     = 0x280;
		dash1    = 0x2A;
		dash2    = 0x10;
		resist   = 0x19;
	}
	else // On Land / In Air
	{
		max_dash = 0x32C;
		gravity1 = 0x50;  // <--- FIXED: Normal gravity pull
		gravity2 = 0x20;  // <--- FIXED: Reduced gravity pull while jump is held
		jump     = 0x500; // <--- Normal jump power
		dash1    = 0x55;
		dash2    = 0x20;
		resist   = 0x33;
	}

	// MOD: If Grappling, massively increase maximum dash and jump limits
	if (gGrappleState == 2)
	{
		max_dash <<= 4; // Shift left 4 (x16 max speed limit)
		jump     <<= 2; // Shift left 2 (x4 jump strength limit)
	}

	// MOD: Check for a specific custom NPC Flag (170 / 0xAA) to give a speed buff
	if (GetNPCFlag(0xAA))
	{
		jump += 0x1B0;
	}

	gMC.ques = FALSE;

	// If can't control player, stop boosting
	if (!bKey)
		gMC.boost_sw = 0;

	// --- 2. Ground vs Air Movement ---
	if (!(gMC.flag & 8) && !(gMC.flag & 0x10) && !(gMC.flag & 0x20))
	{
		// IN AIR
		if (bKey)
		{
			if (gMC.equip & (EQUIP_BOOSTER_0_8 | EQUIP_BOOSTER_2_0) && gKeyTrg & gKeyJump && gMC.boost_cnt != 0)
			{
				if (gMC.equip & EQUIP_BOOSTER_0_8)
				{
					gMC.boost_sw = 1;
					if (gMC.ym > 0x100)
						gMC.ym /= 2;
				}

				if (gMC.equip & EQUIP_BOOSTER_2_0)
				{
					if (gKey & gKeyUp)
					{
						gMC.boost_sw = 2;
						gMC.xm = 0;
						gMC.ym = -0x5FF;
					}
					else if (gKey & gKeyLeft)
					{
						gMC.boost_sw = 1;
						gMC.ym = 0;
						gMC.xm = -0x5FF;
					}
					else if (gKey & gKeyRight)
					{
						gMC.boost_sw = 1;
						gMC.ym = 0;
						gMC.xm = 0x5FF;
					}
					else if (gKey & gKeyDown)
					{
						gMC.boost_sw = 3;
						gMC.xm = 0;
						gMC.ym = 0x5FF;
					}
					else
					{
						gMC.boost_sw = 2;
						gMC.xm = 0;
						gMC.ym = -0x5FF;
					}
				}
			}

			// Move left and right in air
			if (gKey & gKeyLeft && gMC.xm > -max_dash)
				gMC.xm -= dash2;
			if (gKey & gKeyRight && gMC.xm < max_dash)
				gMC.xm += dash2;

			if (gKey & gKeyLeft)
				gMC.direct = 0;
			if (gKey & gKeyRight)
				gMC.direct = 2;
		}

		// Slow down when stopped boosting (Booster 2.0)
		if (gMC.equip & EQUIP_BOOSTER_2_0 && gMC.boost_sw != 0 && (!(gKey & gKeyJump) || gMC.boost_cnt == 0))
		{
			if (gMC.boost_sw == 1)
				gMC.xm /= 2;
			else if (gMC.boost_sw == 2)
				gMC.ym /= 2;
		}

		// Stop boosting
		if (gMC.boost_cnt == 0 || !(gKey & gKeyJump))
			gMC.boost_sw = 0;
	}
	else
	{
		// ON GROUND
		gMC.boost_sw = 0;

		// Refuel booster
		if (!(gMC.equip & EQUIP_BOOSTER_0_8))
		{
			if (!(gMC.equip & EQUIP_BOOSTER_2_0))
				gMC.boost_cnt = 0;
			else
				gMC.boost_cnt = 50;
		}
		else
		{
			gMC.boost_cnt = 50;
		}

		if (bKey)
		{
			if (gKeyTrg == gKeyDown && gKey == gKeyDown && !(gMC.cond & 1) && !(g_GameFlags & 4))
			{
				gMC.cond |= 1;
				gMC.ques = TRUE;
			}
			else if (gKey != gKeyDown)
			{
				// Ground Acceleration
				if (gKey & gKeyLeft && gMC.xm > -max_dash)
					gMC.xm -= dash1;
				if (gKey & gKeyRight && gMC.xm < max_dash)
					gMC.xm += dash1;

				if (gKey & gKeyLeft)
					gMC.direct = 0;
				if (gKey & gKeyRight)
					gMC.direct = 2;
			}
		}

		// Friction
		if (!(gMC.cond & 0x20))
		{
			if (gMC.xm < 0)
			{
				if (gMC.xm > -resist)
					gMC.xm = 0;
				else
					gMC.xm += resist;
			}
			if (gMC.xm > 0)
			{
				if (gMC.xm < resist)
					gMC.xm = 0;
				else
					gMC.xm -= resist;
			}
		}
	}

	// --- 3. Jumping & Vertical Forces ---
	if (bKey)
	{
		gMC.up = (gKey & gKeyUp);
		gMC.down = (gKey & gKeyDown) && !(gMC.flag & 8);

		if (gKeyTrg & gKeyJump && (gMC.flag & 8 || gMC.flag & 0x10 || gMC.flag & 0x20) && !(gMC.flag & 0x2000))
		{
			gMC.ym = -jump;
			PlaySoundObject(15, SOUND_MODE_PLAY);
		}
	}

	// Stop interacting when moved
	if (bKey && gKey & (gKeyLeft | gKeyRight | gKeyUp | gKeyJump | gKeyShot))
		gMC.cond &= ~1;

	// Booster losing fuel
	if (gMC.boost_sw != 0 && gMC.boost_cnt != 0)
		--gMC.boost_cnt;

	// Wind / current forces
	if (gMC.flag & 0x1000) gMC.xm -= 0x88;
	if (gMC.flag & 0x2000) gMC.ym -= 0x80;
	if (gMC.flag & 0x4000) gMC.xm += 0x88;
	if (gMC.flag & 0x8000) gMC.ym += 0x55;

	// Gravity and Booster Physics
	if (!(gMC.equip & EQUIP_BOOSTER_2_0) || gMC.boost_sw == 0)
	{
		if (!(gMC.flag & 0x2000))
		{
			if (!(gMC.equip & EQUIP_BOOSTER_0_8) || gMC.boost_sw == 0 || gMC.ym < -0x3FF)
			{
				if (gMC.ym < 0 && bKey && gKey & gKeyJump)
					gMC.ym += gravity2;
				else
					gMC.ym += gravity1;
			}
			else
			{
				// Booster 0.8 / Mimiga Mask float
				gMC.ym -= 0x20;
				if (gMC.boost_cnt % 3 == 0)
				{
					SetCaret(gMC.x, gMC.y + (gMC.hit.bottom / 2), 7, 3);
					PlaySoundObject(113, SOUND_MODE_PLAY);
				}
				if (gMC.flag & 2)
					gMC.ym = 0x200;
			}
		}
		else
		{
			gMC.ym += gravity1;
		}
	}
	else if (gMC.boost_sw == 1)
	{
		if (gMC.flag & 5) gMC.ym = -0x100;

		if (gMC.direct == 0) gMC.xm -= 0x20;
		if (gMC.direct == 2) gMC.xm += 0x20;

		if (gKeyTrg & gKeyJump || gMC.boost_cnt % 3 == 1)
		{
			if (gMC.direct == 0) SetCaret(gMC.x + 0x400, gMC.y + 0x400, 7, 2);
			if (gMC.direct == 2) SetCaret(gMC.x - 0x400, gMC.y + 0x400, 7, 0);
			PlaySoundObject(113, SOUND_MODE_PLAY);
		}
	}
	else if (gMC.boost_sw == 2)
	{
		gMC.ym -= 0x20;
		if (gKeyTrg & gKeyJump || gMC.boost_cnt % 3 == 1)
		{
			SetCaret(gMC.x, gMC.y + 0xC00, 7, 3);
			PlaySoundObject(113, SOUND_MODE_PLAY);
		}
	}
	else if (gMC.boost_sw == 3 && (gKeyTrg & gKeyJump || gMC.boost_cnt % 3 == 1))
	{
		SetCaret(gMC.x, gMC.y - 0xC00, 7, 1);
		PlaySoundObject(113, SOUND_MODE_PLAY);
	}

	// Keep player on slopes
	if (!bKey || !(gKey & gKeyJump))
	{
		if (gMC.flag & 0x10 && gMC.xm < 0) gMC.ym = -gMC.xm;
		if (gMC.flag & 0x20 && gMC.xm > 0) gMC.ym = gMC.xm;
		if (gMC.flag & 8 && gMC.flag & 0x80000 && gMC.xm < 0) gMC.ym = 0x400;
		if (gMC.flag & 8 && gMC.flag & 0x10000 && gMC.xm > 0) gMC.ym = 0x400;
		if (gMC.flag & 8 && gMC.flag & 0x20000 && gMC.flag & 0x40000) gMC.ym = 0x400;
	}

	// Base Speed Limits
	if (!(gMC.flag & 0x100) || (gMC.flag & 0xF000))
	{
		if (gMC.xm < -0x5FF) gMC.xm = -0x5FF;
		if (gMC.xm >  0x5FF) gMC.xm =  0x5FF;
		if (gMC.ym < -0x7FF) gMC.ym = -0x7FF;
		if (gMC.ym >  0x5FF) gMC.ym =  0x5FF;
	}
	else
	{
		if (gMC.xm < -0x2FF) gMC.xm = -0x2FF;
		if (gMC.xm >  0x2FF) gMC.xm =  0x2FF;
		if (gMC.ym < -0x4FF) gMC.ym = -0x4FF;
		if (gMC.ym >  0x2FF) gMC.ym =  0x2FF;
	}

	// Splashing
	if (!gMC.sprash && gMC.flag & 0x100)
	{
		if (!(gMC.flag & 8) && gMC.ym > 0x200)
		{
			for (a = 0; a < 8; ++a)
				SetNpChar(73, gMC.x + (Random(-8, 8) * 0x200), gMC.y, gMC.xm + Random(-0x200, 0x80), Random(-0x200, 0x200), 0, NULL, 0);
			PlaySoundObject(56, SOUND_MODE_PLAY);
		}
		else if (gMC.xm > 0x200 || gMC.xm < -0x200)
		{
			for (a = 0; a < 8; ++a)
				SetNpChar(73, gMC.x + (Random(-8, 8) * 0x200), gMC.y, gMC.xm + Random(-0x200, 0x200), 0, 0, NULL, 0);
			PlaySoundObject(56, SOUND_MODE_PLAY);
		}
		gMC.sprash = TRUE;
	}
	if (!(gMC.flag & 0x100)) gMC.sprash = FALSE;

	// Spike Damage
	if (gMC.flag & 0x400)
		DamageMyChar(10);

	// Camera Management
	if (gMC.direct == 0)
	{
		gMC.index_x -= 0x200;
		if (gMC.index_x < -0x8000) gMC.index_x = -0x8000;
	}
	else
	{
		gMC.index_x += 0x200;
		if (gMC.index_x > 0x8000) gMC.index_x = 0x8000;
	}
	if (!(gKey & gKeyUp) || !bKey)
	{
		if (!(gKey & gKeyDown) || !bKey)
		{
			if (gMC.index_y > 0x200) gMC.index_y -= 0x200;
			if (gMC.index_y < -0x200) gMC.index_y += 0x200;
		}
		else
		{
			gMC.index_y += 0x200;
			if (gMC.index_y > 0x8000) gMC.index_y = 0x8000;
		}
	}
	else
	{
		gMC.index_y -= 0x200;
		if (gMC.index_y < -0x8000) gMC.index_y = -0x8000;
	}

	// --- 4. MOD: THE GRAPPLING HOOK PHYSICS ENGINE ---
	if (gGrappleState == 2)
	{
		// Weapon 5 (formerly Missile Launcher) extends/retracts rope
		if (gSelectedArms == 5)
		{
			int half_len = gGrappleLength / 2;

			// Extending Rope
			if (gKey & gKeyDown)
			{
				if (gMC.x < gGrappleX + half_len && gMC.x > gGrappleX - gGrappleLength)
				{
					if (gMC.y > gGrappleY)
					{
						if (!(gMC.flag & 8)) gGrappleLength += 0x400; // Blocked by floor
					}
					else if (!(gMC.flag & 2))
					{
						gGrappleLength += 0x400; // Blocked by ceiling
					}
				}
				else if (gMC.x < gGrappleX)
				{
					if (!(gMC.flag & 1)) gGrappleLength += 0x400; // Blocked by left wall
				}
				else if (!(gMC.flag & 4))
				{
					gGrappleLength += 0x400; // Blocked by right wall
				}
			}

			// Retracting Rope
			if (gKey & gKeyUp)
			{
				if (gMC.x < gGrappleX + half_len && gMC.x > gGrappleX - gGrappleLength)
				{
					if (gMC.y > gGrappleY)
					{
						if (!(gMC.flag & 2)) gGrappleLength -= 0x400;
					}
					else if (!(gMC.flag & 8))
					{
						gGrappleLength -= 0x400;
					}
				}
				else if (gMC.x < gGrappleX)
				{
					if (!(gMC.flag & 4)) gGrappleLength -= 0x400;
				}
				else if (!(gMC.flag & 1))
				{
					gGrappleLength -= 0x400;
				}
			}
		}

		// Clamp Rope Length limits
		if (gGrappleLength < 0x1001)  gGrappleLength = 0x1000;
		else if (gGrappleLength > 0xBFFF) gGrappleLength = 0xC000;

		// Calculate anticipated next position
		int next_x = (gMC.xm > resist || gMC.xm < -resist) ? gMC.x + gMC.xm : gMC.x;
		int next_y = gMC.y + gMC.ym;

		// Find distance to pivot using Pythagorean theorem (FPU math converted to C)
		float fdx = (float)(next_x - gGrappleX);
		float fdy = (float)(next_y - gGrappleY);
		int dist_to_pivot = (int)sqrt((fdx * fdx) + (fdy * fdy));

		// Check if player has hit the end of the rope tension
		if (dist_to_pivot >= gGrappleLength)
		{
			// Snap to perimeter of the circle
			float angle = atan2(fdy, fdx);
			
			// Constrain positional velocity to keep the player on the rope boundary
			gMC.xm = (int)(cos(angle) * gGrappleLength) - (gMC.x - gGrappleX);
			gMC.ym = (int)(sin(angle) * gGrappleLength) - (gMC.y - gGrappleY);

			int mom_y = 1;
			int mom_x = -1;
			int accel = gGrappleMomentum;

			// Add Swing Momentum via Input (Below pivot point)
			if (gMC.y > gGrappleY)
			{
				int half_len = gGrappleLength / 2;
				if (gMC.x < gGrappleX + half_len && gMC.x > gGrappleX - gGrappleLength)
				{
					if (!(gKey & gKeyJump)) accel -= (accel / 5);
					else accel += (accel / 3);
				}

				if (!(gKey & gKeyLeft))
				{
					if (gKey & gKeyRight)
					{
						if (gMC.xm < 1) accel -= 9;
						else accel += 0x3C;
					}
				}
				else if (gMC.xm < 0) accel += 0x3C;
				else accel -= 9;
			}

			// Determine Tangent direction depending on quadrant
			if (gMC.y < gGrappleY - (gGrappleLength / 2))
			{
				if (!(gKey & gKeyLeft) && ((gKey & gKeyRight) || gMC.xm > 0))
				{
					mom_y = 1;
					mom_x = -1;
				}
				else
				{
					mom_y = -1;
					mom_x = 1;
				}
			}
			else if (!(gKey & gKeyLeft) && ((gKey & gKeyRight) || gMC.xm < 1))
			{
				mom_y = -1;
				mom_x = 1;
			}
			else
			{
				mom_y = 1;
				mom_x = -1;
			}

			// Apply damped momentum as a tangential velocity vector
			gGrappleMomentum = accel / 50;
			gMC.ym += gGrappleMomentum * mom_y;
			gMC.xm += gGrappleMomentum * mom_x;
		}
		else
		{
			// Rope has slack: Provide a mild pull toward the pivot point
			gMC.xm += (gGrappleX - next_x) / 100;
			gMC.ym += (gGrappleY - next_y) / 100;
		}
	}

	// Apply Final Calculated Velocities
	gMC.tgt_x = gMC.x + gMC.index_x;
	gMC.tgt_y = gMC.y + gMC.index_y;

	gMC.x += gMC.xm;
	gMC.y += gMC.ym;
}

void ActMyChar_Stream(BOOL bKey)
{
	gMC.up = FALSE;
	gMC.down = FALSE;

	if (bKey)
	{
		if (gKey & (gKeyLeft | gKeyRight))
		{
			if (gKey & gKeyLeft)
				gMC.xm -= 0x100;

			if (gKey & gKeyRight)
				gMC.xm += 0x100;
		}
		else if (gMC.xm < 0x80 && gMC.xm > -0x80)
		{
			gMC.xm = 0;
		}
		else if (gMC.xm > 0)
		{
			gMC.xm -= 0x80;
		}
		else if (gMC.xm < 0)
		{
			gMC.xm += 0x80;
		}

		if (gKey & (gKeyUp | gKeyDown))
		{
			if (gKey & gKeyUp)
				gMC.ym -= 0x100;

			if (gKey & gKeyDown)
				gMC.ym += 0x100;
		}
		else if (gMC.ym < 0x80 && gMC.ym > -0x80)
		{
			gMC.ym = 0;
		}
		else if (gMC.ym > 0)
		{
			gMC.ym -= 0x80;
		}
		else if (gMC.ym < 0)
		{
			gMC.ym += 0x80;
		}
	}
	else
	{
		if (gMC.xm < 0x80 && gMC.xm > -0x40)
			gMC.xm = 0;
		else if (gMC.xm > 0)
			gMC.xm -= 0x80;
		else if (gMC.xm < 0)
			gMC.xm += 0x80;

		if (gMC.ym < 0x80 && gMC.ym > -0x40)
			gMC.ym = 0;
		else if (gMC.ym > 0)
			gMC.ym -= 0x80;
		else if (gMC.ym < 0)
			gMC.ym += 0x80;
	}

	if (gMC.ym < -0x200 && gMC.flag & 2)
		SetCaret(gMC.x, gMC.y - gMC.hit.top, 13, 5);
	if (gMC.ym > 0x200 && gMC.flag & 8)
		SetCaret(gMC.x, gMC.y + gMC.hit.bottom, 13, 5);

	if (gMC.xm > 0x400)
		gMC.xm = 0x400;
	if (gMC.xm < -0x400)
		gMC.xm = -0x400;

	if (gMC.ym > 0x400)
		gMC.ym = 0x400;
	if (gMC.ym < -0x400)
		gMC.ym = -0x400;

	if ((gKey & (gKeyLeft | gKeyUp)) == (gKeyLeft | gKeyUp))
	{
		if (gMC.xm < -780)
			gMC.xm = -780;
		if (gMC.ym < -780)
			gMC.ym = -780;
	}

	if ((gKey & (gKeyRight | gKeyUp)) == (gKeyRight | gKeyUp))
	{
		if (gMC.xm > 780)
			gMC.xm = 780;
		if (gMC.ym < -780)
			gMC.ym = -780;
	}

	if ((gKey & (gKeyLeft | gKeyDown)) == (gKeyLeft | gKeyDown))
	{
		if (gMC.xm < -780)
			gMC.xm = -780;
		if (gMC.ym > 780)
			gMC.ym = 780;
	}

	if ((gKey & (gKeyRight | gKeyDown)) == (gKeyRight | gKeyDown))
	{
		if (gMC.xm > 780)
			gMC.xm = 780;
		if (gMC.ym > 780)
			gMC.ym = 780;
	}

	gMC.x += gMC.xm;
	gMC.y += gMC.ym;
}

void AirProcess(void)
{
	if (gMC.equip & EQUIP_AIR_TANK)
	{
		gMC.air = 1000;
		gMC.air_get = 0;
	}
	else
	{
		if (!(gMC.flag & 0x100) || gMC.no_splash_or_air_limit_underwater)
		{
			gMC.air = 1000;
		}
		else
		{
			if (--gMC.air <= 0)
			{
				if (GetNPCFlag(4000))
				{
					// Core cutscene
					StartTextScript(1100);
				}
				else
				{
					// Drown
					StartTextScript(41);

					if (gMC.direct == 0)
						SetCaret(gMC.x, gMC.y, 8, 0);
					else
						SetCaret(gMC.x, gMC.y, 8, 2);

					gMC.cond &= ~0x80;
				}
			}
		}

		if (gMC.flag & 0x100)
		{
			gMC.air_get = 60;
		}
		else
		{
			if (gMC.air_get != 0)
				--gMC.air_get;
		}
	}
}

void ActMyChar(BOOL bKey)
{
	if (!(gMC.cond & 0x80))
		return;

	if (gMC.exp_wait != 0)
		--gMC.exp_wait;

	if (gMC.shock != 0)
	{
		--gMC.shock;
	}
	else if (gMC.exp_count != 0)
	{
		SetValueView(&gMC.x, &gMC.y, gMC.exp_count);
		gMC.exp_count = 0;
	}

	switch (gMC.unit)
	{
		case 0:
			if (!(g_GameFlags & 4) && bKey)
				AirProcess();

			ActMyChar_Normal(bKey);
			break;

		case 1:
			ActMyChar_Stream(bKey);
			break;
	}

	gMC.cond &= ~0x20;
}

void GetMyCharPosition(int *x, int *y)
{
	*x = gMC.x;
	*y = gMC.y;
}

void SetMyCharPosition(int x, int y)
{
	gMC.x = x;
	gMC.y = y;
	gMC.tgt_x = gMC.x;
	gMC.tgt_y = gMC.y;
	gMC.index_x = 0;
	gMC.index_y = 0;
	gMC.xm = 0;
	gMC.ym = 0;
	gMC.cond &= ~1;
	InitStar();
}

void MoveMyChar(int x, int y)
{
	gMC.x = x;
	gMC.y = y;
}

void ZeroMyCharXMove(void)
{
	gMC.xm = 0;
}

int GetUnitMyChar(void)
{
	return gMC.unit;
}

void SetMyCharDirect(unsigned char dir)
{
	int i;

	if (dir == 3)
	{
		gMC.cond |= 1;
	}
	else
	{
		gMC.cond &= ~1;

		if (dir < 10)
		{
			gMC.direct = dir;
		}
		else
		{
			for (i = 0; i < NPC_MAX; ++i)
				if (gNPC[i].code_event == dir)
					break;

			if (i == NPC_MAX)
				return;

			if (gMC.x > gNPC[i].x)
				gMC.direct = 0;
			else
				gMC.direct = 2;
		}
	}

	gMC.xm = 0;
	AnimationMyChar(FALSE);
}

void ChangeMyUnit(unsigned char a)
{
	gMC.unit = a;
}

void PitMyChar(void)
{
	gMC.y += 2 * 0x10 * 0x200;	// Shove player two tiles down. I wonder what this was meant for?
}

void EquipItem(int flag, BOOL b)
{
	if (b)
		gMC.equip |= flag;
	else
		gMC.equip &= ~flag;
}

void ResetCheck(void)
{
	gMC.cond &= ~1;
}

static int noise_no;
static unsigned int noise_freq;

void SetNoise(int no, int freq)
{
	noise_freq = freq;
	noise_no = no;

	switch (noise_no)
	{
		case 1:
			ChangeSoundFrequency(40, noise_freq);
			ChangeSoundFrequency(41, noise_freq + 100);
			PlaySoundObject(40, SOUND_MODE_PLAY_LOOP);
			PlaySoundObject(41, SOUND_MODE_PLAY_LOOP);
			break;

		case 2:
			PlaySoundObject(58, SOUND_MODE_PLAY_LOOP);
			break;
	}
}

void CutNoise(void)
{
	noise_no = 0;
	PlaySoundObject(40, SOUND_MODE_STOP);
	PlaySoundObject(41, SOUND_MODE_STOP);
	PlaySoundObject(58, SOUND_MODE_STOP);
}

void ResetNoise(void)
{
	switch (noise_no)
	{
		case 1:
			ChangeSoundFrequency(40, noise_freq);
			ChangeSoundFrequency(41, noise_freq + 100);
			PlaySoundObject(40, SOUND_MODE_PLAY_LOOP);
			PlaySoundObject(41, SOUND_MODE_PLAY_LOOP);
			break;

		case 2:
			PlaySoundObject(58, SOUND_MODE_PLAY_LOOP);
			break;
	}
}

void SleepNoise(void)
{
	PlaySoundObject(40, SOUND_MODE_STOP);
	PlaySoundObject(41, SOUND_MODE_STOP);
	PlaySoundObject(58, SOUND_MODE_STOP);
}
