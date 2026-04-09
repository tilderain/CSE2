#include "MycHit.h"

#include "WindowsWrapper.h"

#include "Back.h"
#include "Boss.h"
#include "Caret.h"
#include "Game.h"
#include "KeyControl.h"
#include "Map.h"
#include "MyChar.h"
#include "MycParam.h"
#include "NpChar.h"
#include "Sound.h"
#include "TextScr.h"

void ResetMyCharFlag(void)
{
	gMC.flag = 0;
}

static void PutlittleStar(void)
{
	if (!(gMC.cond & 2) && gMC.ym < -0x200)
	{
		PlaySoundObject(3, SOUND_MODE_PLAY);
		SetCaret(gMC.x, gMC.y - gMC.hit.top, 13, 0);
		SetCaret(gMC.x, gMC.y - gMC.hit.top, 13, 0);
	}
}

int JudgeHitMyCharBlock(int x, int y)
{
	int hit = 0;

	// Left wall
	if (gMC.y - gMC.hit.top < (y * 0x10 + 4) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 4) * 0x200
		&& gMC.x - gMC.hit.back < (x * 0x10 + 8) * 0x200
		&& gMC.x - gMC.hit.back > x * 0x10 * 0x200)
	{
		// Clip
		gMC.x = ((x * 0x10 + 8) * 0x200) + gMC.hit.back;

		// Halt momentum
		if (gMC.xm < -0x180)
			gMC.xm = -0x180;
		if (!(gKey & gKeyLeft) && gMC.xm < 0)
			gMC.xm = 0;

		// Set that a left wall was hit
		hit |= 1;
	}

	// Right wall
	if (gMC.y - gMC.hit.top < (y * 0x10 + 4) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 4) * 0x200
		&& gMC.x + gMC.hit.back > (x * 0x10 - 8) * 0x200
		&& gMC.x + gMC.hit.back < x * 0x10 * 0x200)
	{
		// Clip
		gMC.x = ((x * 0x10 - 8) * 0x200) - gMC.hit.back;

		// Halt momentum
		if (gMC.xm > 0x180)
			gMC.xm = 0x180;
		if (!(gKey & gKeyRight) && gMC.xm > 0)
			gMC.xm = 0;

		// Set that a right wall was hit
		hit |= 4;
	}

	// Ceiling
	if (gMC.x - gMC.hit.back < (x * 0x10 + 5) * 0x200
		&& gMC.x + gMC.hit.back > (x * 0x10 - 5) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 + 8) * 0x200
		&& gMC.y - gMC.hit.top > y * 0x10 * 0x200)
	{
		// Clip
		gMC.y = ((y * 0x10 + 8) * 0x200) + gMC.hit.top;

		// Halt momentum
		if (!(gMC.cond & 2) && gMC.ym < -0x200)
			PutlittleStar();
		if (gMC.ym < 0)
			gMC.ym = 0;

		// Set that a ceiling was hit
		hit |= 2;
	}

	// Floor
	if (gMC.x - gMC.hit.back < (x * 0x10 + 5) * 0x200
		&& gMC.x + gMC.hit.back > ((x * 0x10 - 5) * 0x200)
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 8) * 0x200
		&& gMC.y + gMC.hit.bottom < y * 0x10 * 0x200)
	{
		// Clip
		gMC.y = ((y * 0x10 - 8) * 0x200) - gMC.hit.bottom;

		// Halt momentum
		if (gMC.ym > 0x400)
			PlaySoundObject(23, SOUND_MODE_PLAY);
		if (gMC.ym > 0)
			gMC.ym = 0;

		// Set that a floor was hit
		hit |= 8;
	}

	return hit;
}

int JudgeHitMyCharTriangleA(int x, int y)
{
	int hit = 0;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 * 0x200) - (gMC.x - x * 0x10 * 0x200) / 2 + 0x800
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) - ((gMC.x - x * 0x10 * 0x200) / 2) + 0x800 + gMC.hit.top;

		// Halt momentum
		if (!(gMC.cond & 2) && gMC.ym < -0x200)
			PutlittleStar();
		if (gMC.ym < 0)
			gMC.ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	return hit;
}

int JudgeHitMyCharTriangleB(int x, int y)
{
	int hit = 0;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 * 0x200) - ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) - ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800 + gMC.hit.top;

		// Halt momentum
		if (!(gMC.cond & 2) && gMC.ym < -0x200)
			PutlittleStar();
		if (gMC.ym < 0)
			gMC.ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	return hit;
}

int JudgeHitMyCharTriangleC(int x, int y)
{
	int hit = 0;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800 + gMC.hit.top;

		// Halt momentum
		if (!(gMC.cond & 2) && gMC.ym < -0x200)
			PutlittleStar();
		if (gMC.ym < 0)
			gMC.ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	return hit;
}

int JudgeHitMyCharTriangleD(int x, int y)
{
	int hit = 0;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) + 0x800
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) + 0x800 + gMC.hit.top;

		// Halt momentum
		if (!(gMC.cond & 2) && gMC.ym < -0x200)
			PutlittleStar();
		if (gMC.ym < 0)
			gMC.ym = 0;

		// Set that hit a ceiling
		hit |= 2;
	}

	return hit;
}

int JudgeHitMyCharTriangleE(int x, int y)
{
	int hit = 0;

	hit |= 0x10000;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800
		&& gMC.y - gMC.hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800 - gMC.hit.bottom;

		// Halt momentum
		if (gMC.ym > 0x400)
			PlaySoundObject(23, SOUND_MODE_PLAY);
		if (gMC.ym > 0)
			gMC.ym = 0;

		// Set that hit this slope
		hit |= 0x28;
	}

	return hit;
}

int JudgeHitMyCharTriangleF(int x, int y)
{
	int hit = 0;

	hit |= 0x20000;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) + 0x800
		&& gMC.y - gMC.hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) + ((gMC.x - x * 0x10 * 0x200) / 2) + 0x800 - gMC.hit.bottom;

		// Halt momentum
		if (gMC.ym > 0x400)
			PlaySoundObject(23, SOUND_MODE_PLAY);
		if (gMC.ym > 0)
			gMC.ym = 0;

		// Set that hit this slope
		hit |= 0x28;
	}

	return hit;
}

int JudgeHitMyCharTriangleG(int x, int y)
{
	int hit = 0;

	hit |= 0x40000;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 * 0x200) - ((gMC.x - x * 0x10 * 0x200) / 2) + 0x800
		&& gMC.y - gMC.hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) - ((gMC.x - x * 0x10 * 0x200) / 2) + 0x800 - gMC.hit.bottom;

		// Halt momentum
		if (gMC.ym > 0x400)
			PlaySoundObject(23, SOUND_MODE_PLAY);
		if (gMC.ym > 0)
			gMC.ym = 0;

		// Set that hit this slope
		hit |= 0x18;
	}

	return hit;
}

int JudgeHitMyCharTriangleH(int x, int y)
{
	int hit = 0;

	hit |= 0x80000;

	if (gMC.x < (x * 0x10 + 8) * 0x200
		&& gMC.x > (x * 0x10 - 8) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 * 0x200) - ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800
		&& gMC.y - gMC.hit.top < (y * 0x10 + 8) * 0x200)
	{
		// Clip
		gMC.y = (y * 0x10 * 0x200) - ((gMC.x - x * 0x10 * 0x200) / 2) - 0x800 - gMC.hit.bottom;

		// Halt momentum
		if (gMC.ym > 0x400)
			PlaySoundObject(23, SOUND_MODE_PLAY);
		if (gMC.ym > 0)
			gMC.ym = 0;

		// Set that hit this slope
		hit |= 0x18;
	}

	return hit;
}

int JudgeHitMyCharWater(int x, int y)
{
	int hit = 0;

	if (gMC.x - gMC.hit.back < (x * 0x10 + 5) * 0x200
		&& gMC.x + gMC.hit.back > ((x * 0x10 - 5) * 0x200)
		&& gMC.y - gMC.hit.top < ((y * 0x10 + 5) * 0x200)
		&& gMC.y + gMC.hit.bottom > y * 0x10 * 0x200)
		hit |= 0x100;

	return hit;
}

int JudgeHitMyCharDamage(int x, int y)
{
	int hit = 0;

	if (gMC.x - 0x800 < (x * 0x10 + 4) * 0x200
		&& gMC.x + 0x800 > (x * 0x10 - 4) * 0x200
		&& gMC.y - 0x800 < (y * 0x10 + 3) * 0x200
		&& gMC.y + 0x800 > (y * 0x10 - 3) * 0x200)
		hit |= 0x400;

	return hit;
}

int JudgeHitMyCharDamageW(int x, int y)
{
	int hit = 0;

	if (gMC.x - 0x800 < (x * 0x10 + 4) * 0x200
		&& gMC.x + 0x800 > (x * 0x10 - 4) * 0x200
		&& gMC.y - 0x800 < (y * 0x10 + 3) * 0x200
		&& gMC.y + 0x800 > (y * 0x10 - 3) * 0x200)
		hit |= 0xD00;

	return hit;
}

int JudgeHitMyCharVectLeft(int x, int y)
{
	int hit = 0;
	if (gMC.x - gMC.hit.back < (x * 0x10 + 6) * 0x200
		&& gMC.x + gMC.hit.back > (x * 0x10 - 6) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 + 6) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 6) * 0x200)
		hit |= 0x1000;

	return hit;
}

int JudgeHitMyCharVectUp(int x, int y)
{
	int hit = 0;
	if (gMC.x - gMC.hit.back < (x * 0x10 + 6) * 0x200
		&& gMC.x + gMC.hit.back > (x * 0x10 - 6) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 + 6) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 6) * 0x200)
		hit |= 0x2000;

	return hit;
}

int JudgeHitMyCharVectRight(int x, int y)
{
	int hit = 0;
	if (gMC.x - gMC.hit.back < (x * 0x10 + 6) * 0x200
		&& gMC.x + gMC.hit.back > (x * 0x10 - 6) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 + 6) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 6) * 0x200)
		hit |= 0x4000;

	return hit;
}

int JudgeHitMyCharVectDown(int x, int y)
{
	int hit = 0;
	if (gMC.x - gMC.hit.back < (x * 0x10 + 6) * 0x200
		&& gMC.x + gMC.hit.back > (x * 0x10 - 6) * 0x200
		&& gMC.y - gMC.hit.top < (y * 0x10 + 6) * 0x200
		&& gMC.y + gMC.hit.bottom > (y * 0x10 - 6) * 0x200)
		hit |= 0x8000;

	return hit;
}

void HitMyCharMap(void)
{
    // Floor division matching ASM arithmetic shift
    int px = gMC.x;
    int py = gMC.y;
    int tx = ((px + (px >> 31 & 0xf)) >> 4);
    tx = (tx + (tx >> 31 & 0x1ff)) >> 9;
    int ty = ((py + (py >> 31 & 0xf)) >> 4);
    ty = (ty + (ty >> 31 & 0x1ff)) >> 9;

    int offx[4] = {0, 1, 0, 1};
    int offy[4] = {0, 0, 1, 1};

    for (int i = 0; i < 4; ++i)
    {
        int cx = tx + offx[i];
        int cy = ty + offy[i];
        uint8_t atrb = (uint8_t)GetAttribute(cx, cy);

        switch (atrb)
        {
            case 0x02:
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;

            case 0x05: case 0x06: case 0x07: case 0x08: case 0x09:
            case 0x0a: case 0x18:
            case 0x41: case 0x43: case 0x46: case 0x47: case 0x48:
            case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d:
            case 0x4e: case 0x4f:
            case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d:
            case 0x5e: case 0x5f:
                gMC.flag |= JudgeHitMyCharBlock(cx, cy);
                break;

            case 0x10: case 0x50:
                gMC.flag |= JudgeHitMyCharTriangleA(cx, cy);
                break;
            case 0x11: case 0x51:
                gMC.flag |= JudgeHitMyCharTriangleB(cx, cy);
                break;
            case 0x12: case 0x52:
                gMC.flag |= JudgeHitMyCharTriangleC(cx, cy);
                break;
            case 0x13: case 0x53:
                gMC.flag |= JudgeHitMyCharTriangleD(cx, cy);
                break;
            case 0x14: case 0x54:
                gMC.flag |= JudgeHitMyCharTriangleE(cx, cy);
                break;
            case 0x15: case 0x55:
                gMC.flag |= JudgeHitMyCharTriangleF(cx, cy);
                break;
            case 0x16: case 0x56:
                gMC.flag |= JudgeHitMyCharTriangleG(cx, cy);
                break;
            case 0x17: case 0x57:
                gMC.flag |= JudgeHitMyCharTriangleH(cx, cy);
                break;

            case 0x19: case 0x42:
                gMC.flag |= JudgeHitMyCharDamage(cx, cy);
                break;

            case 0x60:
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x61:
                gMC.flag |= JudgeHitMyCharBlock(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x62:
                gMC.flag |= JudgeHitMyCharDamageW(cx, cy);
                break;

            case 0x70:
                gMC.flag |= JudgeHitMyCharTriangleA(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x71:
                gMC.flag |= JudgeHitMyCharTriangleB(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x72:
                gMC.flag |= JudgeHitMyCharTriangleC(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x73:
                gMC.flag |= JudgeHitMyCharTriangleD(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x74:
                gMC.flag |= JudgeHitMyCharTriangleE(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x75:
                gMC.flag |= JudgeHitMyCharTriangleF(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x76:
                gMC.flag |= JudgeHitMyCharTriangleG(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0x77:
                gMC.flag |= JudgeHitMyCharTriangleH(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;

            case 0x80:
                gMC.flag |= JudgeHitMyCharVectLeft(cx, cy);
                break;
            case 0x81:
                gMC.flag |= JudgeHitMyCharVectUp(cx, cy);
                break;
            case 0x82:
                gMC.flag |= JudgeHitMyCharVectRight(cx, cy);
                break;
            case 0x83:
                gMC.flag |= JudgeHitMyCharVectDown(cx, cy);
                break;

            case 0xa0:
                gMC.flag |= JudgeHitMyCharVectLeft(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0xa1:
                gMC.flag |= JudgeHitMyCharVectUp(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0xa2:
                gMC.flag |= JudgeHitMyCharVectRight(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;
            case 0xa3:
                gMC.flag |= JudgeHitMyCharVectDown(cx, cy);
                gMC.flag |= JudgeHitMyCharWater(cx, cy);
                break;

            default:
                break;
        }
    }

    if (gMC.y > gWaterY + 0x800)
        gMC.flag |= 0x100;
}
int JudgeHitMyCharNPC(NPCHAR *npc)
{
	int hit = 0;

	if (gMC.y - gMC.hit.top < npc->y + npc->hit.bottom - (3 * 0x200)
		&& gMC.y + gMC.hit.bottom > npc->y - npc->hit.top + (3 * 0x200)
		&& gMC.x - gMC.hit.back < npc->x + npc->hit.back
		&& gMC.x - gMC.hit.back > npc->x)
	{
		if (gMC.xm < 0x200)
			gMC.xm += 0x200;
		hit |= 1;
	}

	if (gMC.y - gMC.hit.top < npc->y + npc->hit.bottom - (3 * 0x200)
		&& gMC.y + gMC.hit.bottom > npc->y - npc->hit.top + (3 * 0x200)
		&& gMC.x + gMC.hit.back - 0x200 > npc->x - npc->hit.back
		&& gMC.x + gMC.hit.back - 0x200 < npc->x)
	{
		if (gMC.xm > -0x200)
			gMC.xm -= 0x200;
		hit |= 4;
	}

	if (gMC.x - gMC.hit.back < npc->x + npc->hit.back - (3 * 0x200)
		&& gMC.x + gMC.hit.back > npc->x - npc->hit.back + (3 * 0x200)
		&& gMC.y - gMC.hit.top < npc->y + npc->hit.bottom
		&& gMC.y - gMC.hit.top > npc->y)
	{
		if (gMC.ym < 0)
			gMC.ym = 0;
		hit |= 2;
	}

	if (gMC.x - gMC.hit.back < npc->x + npc->hit.back - (3 * 0x200)
		&& gMC.x + gMC.hit.back > npc->x - npc->hit.back + (3 * 0x200)
		&& gMC.y + gMC.hit.bottom > npc->y - npc->hit.top
		&& gMC.hit.bottom + gMC.y < npc->y + (3 * 0x200))
	{
		if (npc->bits & NPC_BOUNCY)
		{
			gMC.ym = npc->ym - 0x200;
			hit |= 8;
		}
		else if (!(gMC.flag & 8) && gMC.ym > npc->ym)
		{
			gMC.y = npc->y - npc->hit.top - gMC.hit.bottom + 0x200;
			gMC.ym = npc->ym;
			gMC.x += npc->xm;
			hit |= 8;
		}
	}

	return hit;
}

unsigned char JudgeHitMyCharNPC3(NPCHAR *npc)
{
	if (npc->direct == 0)
	{
		if (gMC.x + (2 * 0x200) > npc->x - npc->hit.front
			&& gMC.x - (2 * 0x200) < npc->x + npc->hit.back
			&& gMC.y + (2 * 0x200) > npc->y - npc->hit.top
			&& gMC.y - (2 * 0x200) < npc->y + npc->hit.bottom)
			return 1;
	}
	else
	{
		if (gMC.x + (2 * 0x200) > npc->x - npc->hit.back
			&& gMC.x - (2 * 0x200) < npc->x + npc->hit.front
			&& gMC.y + (2 * 0x200) > npc->y - npc->hit.top
			&& gMC.y - (2 * 0x200) < npc->y + npc->hit.bottom)
			return 1;
	}

	return 0;
}

int JudgeHitMyCharNPC4(NPCHAR *npc)
{
	// TODO: comment this
	int hit = 0;

	float fy1;
	float fx1;
	float fx2;
	float fy2;

	if (npc->x > gMC.x)
		fx1 = (float)(npc->x - gMC.x);
	else
		fx1 = (float)(gMC.x - npc->x);

	if (npc->y > gMC.y)
		fy1 = (float)(npc->y - gMC.y);
	else
		fy1 = (float)(gMC.y - npc->y);

	fx2 = (float)npc->hit.back;
	fy2 = (float)npc->hit.top;

	if (fx1 == 0.0f)
		fx1 = 1.0f;
	if (fx2 == 0.0f)
		fx2 = 1.0f;

	if (fy1 / fx1 > fy2 / fx2)
	{
		if (gMC.x - gMC.hit.back < npc->x + npc->hit.back && gMC.x + gMC.hit.back > npc->x - npc->hit.back)
		{
			if (gMC.y - gMC.hit.top < npc->y + npc->hit.bottom && gMC.y - gMC.hit.top > npc->y)
			{
				if (gMC.ym < npc->ym)
				{
					gMC.y = npc->y + npc->hit.bottom + gMC.hit.top + 0x200;
					gMC.ym = npc->ym;
				}
				else
				{
					if (gMC.ym < 0)
						gMC.ym = 0;
				}

				hit |= 2;
			}

			if (gMC.y + gMC.hit.bottom > npc->y - npc->hit.top && gMC.hit.bottom + gMC.y < npc->y + (3 * 0x200))
			{
				if (gMC.ym - npc->ym > 2 * 0x200)
					PlaySoundObject(23, SOUND_MODE_PLAY);

				if (gMC.unit == 1)
				{
					gMC.y = npc->y - npc->hit.top - gMC.hit.bottom + 0x200;
					hit |= 8;
				}
				else if (npc->bits & NPC_BOUNCY)
				{
					gMC.ym = npc->ym - 0x200;
					hit |= 8;
				}
				else if (!(gMC.flag & 8) && gMC.ym > npc->ym)
				{
					gMC.y = npc->y - npc->hit.top - gMC.hit.bottom + 0x200;
					gMC.ym = npc->ym;
					gMC.x += npc->xm;
					hit |= 8;
				}
			}
		}
	}
	else
	{
		if (gMC.y - gMC.hit.top < npc->y + npc->hit.bottom && gMC.y + gMC.hit.bottom > npc->y - npc->hit.top)
		{
			if (gMC.x - gMC.hit.back < npc->x + npc->hit.back && gMC.x - gMC.hit.back > npc->x)
			{
				if (gMC.xm < npc->xm)
					gMC.xm = npc->xm;

				gMC.x = npc->x + npc->hit.back + gMC.hit.back;

				hit |= 1;
			}

			if (gMC.x + gMC.hit.back > npc->x - npc->hit.back && gMC.hit.back + gMC.x < npc->x)
			{
				if (gMC.xm > npc->xm)
					gMC.xm = npc->xm;

				gMC.x = npc->x - npc->hit.back - gMC.hit.back;

				hit |= 4;
			}
		}
	}

	return hit;
}
// Handles the remaining collision types (Hearts, Damage, Scripts)
void HandleModdedNPCPickups(int hit, int i)
{
	// Heart Drop (Code 0x57 / 87)
	if (hit && gNPC[i].code_char == 87)
	{
		PlaySoundObject(20, SOUND_MODE_PLAY);
		AddLifeMyChar(gNPC[i].exp);
		gNPC[i].cond = 0; // Destroy drop
	}

	// Script Execution (Touch)
	// If the NPC has the "Run Script on Touch" bit (0x100)
	if (!(g_GameFlags & 4) && hit && (gNPC[i].bits & NPC_EVENT_WHEN_TOUCHED))
	{
		StartTextScript(gNPC[i].code_event);
	}

	// Player Damage Collision
	if ((g_GameFlags & 2) && !(gNPC[i].bits & NPC_INTERACTABLE))
	{
		// If NPC only hurts when touched on specific sides (e.g. spikes)
		if (gNPC[i].bits & NPC_REAR_AND_TOP_DONT_HURT)
		{
			if (hit & 4 && gNPC[i].xm < 0) DamageMyChar(gNPC[i].damage);
			if (hit & 1 && gNPC[i].xm > 0) DamageMyChar(gNPC[i].damage);
			if (hit & 8 && gNPC[i].ym < 0) DamageMyChar(gNPC[i].damage);
			if (hit & 2 && gNPC[i].ym > 0) DamageMyChar(gNPC[i].damage);
		}
		else
		{
			// Normal enemy damage
			if (hit && gNPC[i].damage != 0 && !(g_GameFlags & 4))
			{
				DamageMyChar(gNPC[i].damage);
			}
		}
	}

	// [MOD] Script Execution (Interact / Down Key)
	// The modder explicitly added logic to force Quote to stop moving (xm = 0)
	// when he interacts with an NPC, preventing sliding during dialogue.
	if (!(g_GameFlags & 4) && hit && (gMC.cond & 1) && (gNPC[i].bits & NPC_INTERACTABLE))
	{
		StartTextScript(gNPC[i].code_event);
		gMC.xm = 0; // [MOD] Stop Quote's momentum instantly
		gMC.ques = 0;
	}

	// Find the next active NPC to process (to return to the main loop)
	while (++i < NPC_MAX)
	{
		if (gNPC[i].cond & 0x80)
		{
			// Reset the loop state for the next NPC and jump back into the main loop logic
			// (In C, this is handled naturally by returning to the 'for' loop. In ASM, 
			// the modder had to manually reconstruct the loop mechanics).
			return; 
		}
	}

	if (gMC.ques)
		SetCaret(gMC.x, gMC.y, CARET_QUESTION_MARK, DIR_LEFT);
}
#include "ArmsItem.h"
void AddCustomWeaponAmmo(int amount, int ammo_amount)
{
	int i;
	for (i = 0; i < 8; ++i)
		if (gArmsData[i].code == 4) break;

	if (i == 8)
	{
		for (i = 0; i < 8; ++i)
			if (gArmsData[i].code == 10) break;
		if (i == 8) return;
	}

	gArmsData[i].num += ammo_amount;
	if (gArmsData[i].max_num < gArmsData[i].num)
		gArmsData[i].num = gArmsData[i].max_num; // FIXED: I previously wrote this line backwards!
}
void HitMyCharNpChar(void)
{
	int hit;
	int i;

	if (!(gMC.cond & 0x80) || gMC.cond & 2)
		return;

	for (i = 0; i < NPC_MAX; ++i)
	{
		// If NPC is not active, skip it
		if (!(gNPC[i].cond & 0x80))
			continue;

		// 1. Calculate Physical Collision
		if (gNPC[i].bits & NPC_SOLID_SOFT)
		{
			hit = JudgeHitMyCharNPC(&gNPC[i]);
			gMC.flag |= hit;
		}
		else if (gNPC[i].bits & NPC_SOLID_HARD)
		{
			hit = JudgeHitMyCharNPC4(&gNPC[i]);
			gMC.flag |= hit;
		}
		else
		{
			hit = JudgeHitMyCharNPC3(&gNPC[i]);
		}

		// 2. Item Pickups
		// If we hit an item, we delete it and MUST 'continue' to the next NPC
		// so we don't accidentally take damage from a "dead" item.
		if (hit)
		{
			if (gNPC[i].code_char == 1) // EXP
			{
				PlaySoundObject(14, SOUND_MODE_PLAY);
				AddExpMyChar(gNPC[i].exp);
				gNPC[i].cond = 0;
				continue; 
			}
			
			if (gNPC[i].code_char == 76) // [MOD] Custom Ammo Pickup
			{
				PlaySoundObject(37, SOUND_MODE_PLAY);
				AddCustomWeaponAmmo(gNPC[i].code_event, gNPC[i].exp);
				gNPC[i].cond = 0;
				continue;
			}
			
			if (gNPC[i].code_char == 86) // Missile Refill
			{
				PlaySoundObject(42, SOUND_MODE_PLAY);
				AddBulletMyChar(gNPC[i].code_event, gNPC[i].exp);
				gNPC[i].cond = 0;
				continue;
			}
			
			if (gNPC[i].code_char == 87) // Heart
			{
				PlaySoundObject(20, SOUND_MODE_PLAY);
				AddLifeMyChar(gNPC[i].exp);
				gNPC[i].cond = 0;
				continue;
			}
		}

		// 3. Touch Scripts
		if (!(g_GameFlags & 4) && hit && (gNPC[i].bits & NPC_EVENT_WHEN_TOUCHED))
		{
			StartTextScript(gNPC[i].code_event);
		}

		// 4. Enemy Damage
		// Only run if the NPC wasn't an item (which would have hit a 'continue' above)
		if ((g_GameFlags & 2) && !(gNPC[i].bits & NPC_INTERACTABLE))
		{
			if (gNPC[i].bits & NPC_REAR_AND_TOP_DONT_HURT)
			{
				if (hit & 4 && gNPC[i].xm < 0) DamageMyChar(gNPC[i].damage);
				if (hit & 1 && gNPC[i].xm > 0) DamageMyChar(gNPC[i].damage);
				if (hit & 8 && gNPC[i].ym < 0) DamageMyChar(gNPC[i].damage);
				if (hit & 2 && gNPC[i].ym > 0) DamageMyChar(gNPC[i].damage);
			}
			else if (hit && gNPC[i].damage != 0 && !(g_GameFlags & 4))
			{
				DamageMyChar(gNPC[i].damage);
			}
		}

		// 5. Interaction Scripts (Pressing Down)
		if (!(g_GameFlags & 4) && hit && (gMC.cond & 1))
		{
			if (gNPC[i].bits & NPC_INTERACTABLE)
			{
				StartTextScript(gNPC[i].code_event);
				gMC.xm = 0;   // [MOD] Stop Quote's momentum instantly
				gMC.ques = 0; // Clear the question mark
			}
		}
	}

	if (gMC.ques)
		SetCaret(gMC.x, gMC.y, 9, 0);
}
// HitMyCharBoss (Modded)
void HitMyCharBoss(void)
{
	int b;
	int hit;

	// Only process if the player is active and not hidden
	if (!(gMC.cond & 0x80) || (gMC.cond & 2))
		return;

	// Loop through all 20 possible boss parts
	for (b = 0; b < 20; ++b) // 20 is BOSS_MAX
	{
		// Skip inactive boss parts
		if (!(gBoss[b].cond & 0x80))
			continue;

		// 1. Calculate Collision
		if (!(gBoss[b].bits & NPC_IGNORE_SOLIDITY))
		{
			if (!(gBoss[b].bits & NPC_SOLID_SOFT))
			{
				hit = JudgeHitMyCharNPC3(&gBoss[b]);
			}
			else
			{
				// Hard Collision (Solid Block)
				hit = JudgeHitMyCharNPC4(&gBoss[b]);
				gMC.flag |= hit;
			}
		}
		else
		{
			// Alternate Hard Collision
			hit = JudgeHitMyCharNPC(&gBoss[b]);
			gMC.flag |= hit;
		}

		// 2. Touch Scripts
		// If the boss part has the "Run Script on Touch" bit (0x100)
		if (!(g_GameFlags & 4) && hit && (gBoss[b].bits & NPC_EVENT_WHEN_TOUCHED))
		{
			StartTextScript(gBoss[b].code_event);
			gMC.ques = FALSE; // Hide interact icon
		}

		// 3. Player Damage
		if (!(gBoss[b].bits & NPC_REAR_AND_TOP_DONT_HURT))
		{
			// Normal enemy damage
			if (hit && gBoss[b].damage != 0 && !(g_GameFlags & 4))
			{
				DamageMyChar(gBoss[b].damage);
			}
		}
		else
		{
			// Directional damage (only hurts if hit from the front)
			// Note: The modder removed the Y-axis damage checks here, 
			// unlike what they did for normal NPCs!
			if ((hit & 4) && gBoss[b].xm < 0)
				DamageMyChar(gBoss[b].damage);
			if ((hit & 1) && gBoss[b].xm > 0)
				DamageMyChar(gBoss[b].damage);
		}

		// 4. [MOD] Interaction Scripts (Pressing Down)
		// The modder explicitly added logic to force Quote to stop moving (xm = 0)
		// when he interacts with a boss part, mirroring the NPC interaction fix.
		if (!(g_GameFlags & 4) && hit && (gMC.cond & 1))
		{
			if (gBoss[b].bits & NPC_INTERACTABLE)
			{
				StartTextScript(gBoss[b].code_event);
				gMC.xm = 0;       // [MOD] Stop Quote's momentum instantly when talking
				gMC.ques = FALSE; // Clear the question mark
			}
		}
	}

	// Draw Interact Indicator if applicable
	if (gMC.ques)
		SetCaret(gMC.x, gMC.y, CARET_QUESTION_MARK, DIR_LEFT);
}