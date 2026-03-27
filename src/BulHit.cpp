#include "BulHit.h"

#include <stddef.h>

#include "WindowsWrapper.h"

#include "Bullet.h"
#include "Caret.h"
#include "Game.h"
#include "Map.h"
#include "NpChar.h"
#include "Sound.h"

static void Vanish(BULLET *bul)
{
	if (bul->code_bullet != 37 && bul->code_bullet != 38 && bul->code_bullet != 39)
		PlaySoundObject(28, SOUND_MODE_PLAY);
	else
		SetCaret(bul->x, bul->y, 2, 1);

	bul->cond = 0;
	SetCaret(bul->x, bul->y, 2, 2);
}

int JudgeHitBulletBlock(int x, int y, BULLET *bul)
{
	int i;
	int hit = 0;
	if (bul->x - bul->blockXL < ((x * 16) + 8) * 0x200
		&& bul->x + bul->blockXL > ((x * 16) - 8) * 0x200
		&& bul->y - bul->blockYL < ((y * 16) + 8) * 0x200
		&& bul->y + bul->blockYL > ((y * 16) - 8) * 0x200)
		hit |= 0x200;

	if (hit && bul->bbits & 0x60 && GetAttribute(x, y) == 0x43)
	{
		if (!(bul->bbits & 0x40))
			bul->cond = 0;

		SetCaret(bul->x, bul->y, 2, 0);
		PlaySoundObject(12, SOUND_MODE_PLAY);

		for (i = 0; i < 4; ++i)
			SetNpChar(4, x * 0x200 * 0x10, y * 0x200 * 0x10, Random(-0x200, 0x200), Random(-0x200, 0x200), 0, NULL, 0x100);

		ShiftMapParts(x, y);
	}

	return hit;
}

int JudgeHitBulletBlock2(int x, int y, unsigned char *atrb, BULLET *bul)
{
	BOOL block[4];
	int workX, workY;
	int hit, i;

	hit = 0;

	if (bul->bbits & 0x40)
	{
		for (i = 0; i < 4; ++i)
		{
			if (*atrb == 0x41 || *atrb == 0x61)
				block[i] = TRUE;
			else
				block[i] = FALSE;

			++atrb;
		}
	}
	else
	{
		for (i = 0; i < 4; ++i)
		{
			if (*atrb == 0x41 || *atrb == 0x43 || *atrb == 0x61)
				block[i] = TRUE;
			else
				block[i] = FALSE;

			++atrb;
		}
	}

	workX = ((x * 16) + 8) * 0x200;
	workY = ((y * 16) + 8) * 0x200;

	// Left wall
	if (block[0] && block[2])
	{
		if (bul->x - bul->blockXL < workX)
			hit |= 1;
	}
	else if (block[0] && !block[2])
	{
		if (bul->x - bul->blockXL < workX && bul->y - bul->blockYL < workY - (3 * 0x200))
			hit |= 1;
	}
	else if (!block[0] && block[2])
	{
		if (bul->x - bul->blockXL < workX && bul->y + bul->blockYL > workY + (3 * 0x200))
			hit |= 1;
	}

	// Right wall
	if (block[1] && block[3])
	{
		if (bul->x + bul->blockXL > workX)
			hit |= 4;
	}
	else if (block[1] && !block[3])
	{
		if (bul->x + bul->blockXL > workX && bul->y - bul->blockYL < workY - (3 * 0x200))
			hit |= 4;
	}
	else if (!block[1] && block[3])
	{
		if (bul->x + bul->blockXL > workX && bul->y + bul->blockYL > workY + (3 * 0x200))
			hit |= 4;
	}

	// Ceiling
	if (block[0] && block[1])
	{
		if (bul->y - bul->blockYL < workY)
			hit |= 2;
	}
	else if (block[0] && !block[1])
	{
		if (bul->y - bul->blockYL < workY && bul->x - bul->blockXL < workX - (3 * 0x200))
			hit |= 2;
	}
	else if (!block[0] && block[1])
	{
		if (bul->y - bul->blockYL < workY && bul->x + bul->blockXL > workX + (3 * 0x200))
			hit |= 2;
	}

	// Ground
	if (block[2] && block[3])
	{
		if (bul->y + bul->blockYL > workY)
			hit |= 8;
	}
	else if (block[2] && !block[3])
	{
		if (bul->y + bul->blockYL > workY && bul->x - bul->blockXL < workX - (3 * 0x200))
			hit |= 8;
	}
	else if (!block[2] && block[3])
	{
		if (bul->y + bul->blockYL > workY && bul->x + bul->blockXL > workX + (3 * 0x200))
			hit |= 8;
	}

	// Clip
	if (bul->bbits & 8)
	{
		if (hit & 1)
			bul->x = workX + bul->blockXL;
		else if (hit & 4)
			bul->x = workX - bul->blockXL;
		else if (hit & 2)
			bul->y = workY + bul->blockYL;
		else if (hit & 8)
			bul->y = workY - bul->blockYL;
	}
	else
	{
		if (hit & 0xF)
			Vanish(bul);
	}

	return hit;
}

int JudgeHitBulletTriangleA(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x > ((x * 16) - 8) * 0x200
		&& bul->y - (2 * 0x200) < ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) + (4 * 0x200)
		&& bul->y + (2 * 0x200) > ((y * 16) - 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) + (6 * 0x200);
		else
			Vanish(bul);

		hit |= 0x82;
	}

	return hit;
}

int JudgeHitBulletTriangleB(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x > ((x * 16) - 8) * 0x200
		&& bul->y - (2 * 0x200) < ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) - (4 * 0x200)
		&& bul->y + (2 * 0x200) > ((y * 16) - 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) - (2 * 0x200);
		else
			Vanish(bul);

		hit |= 0x82;
	}

	return hit;
}

int JudgeHitBulletTriangleC(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x > ((x * 16) - 8) * 0x200
		&& bul->y - (2 * 0x200) < ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) - (4 * 0x200)
		&& bul->y + (2 * 0x200) > ((y * 16) - 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) - (2 * 0x200);
		else
			Vanish(bul);

		hit |= 0x42;
	}

	return hit;
}

int JudgeHitBulletTriangleD(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x > ((x * 16) - 8) * 0x200
		&& bul->y - (2 * 0x200) < ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) + (4 * 0x200)
		&& bul->y + (2 * 0x200) > ((y * 16) - 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) + (6 * 0x200);
		else
			Vanish(bul);

		hit |= 0x42;
	}

	return hit;
}

int JudgeHitBulletTriangleE(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x - (1 * 0x200) > ((x * 16) - 8) * 0x200
		&& bul->y + (2 * 0x200) > ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) - (4 * 0x200)
		&& bul->y - (2 * 0x200) < ((y * 16) + 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) - (6 * 0x200);
		else
			Vanish(bul);

		hit |= 0x28;
	}

	return hit;
}

int JudgeHitBulletTriangleF(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x > ((x * 16) - 8) * 0x200
		&& bul->y + (2 * 0x200) > ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) + (4 * 0x200)
		&& bul->y - (2 * 0x200) < ((y * 16) + 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) + ((bul->x - ((x * 16) * 0x200)) / 2) + (2 * 0x200);
		else
			Vanish(bul);

		hit |= 0x28;
	}

	return hit;
}

int JudgeHitBulletTriangleG(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x > ((x * 16) - 8) * 0x200
		&& bul->y + (2 * 0x200) > ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) + (4 * 0x200)
		&& bul->y - (2 * 0x200) < ((y * 16) + 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) + (2 * 0x200);
		else
			Vanish(bul);

		hit |= 0x18;
	}

	return hit;
}

int JudgeHitBulletTriangleH(int x, int y, BULLET *bul)
{
	int hit = 0;
	if (bul->x < ((x * 16) + 8) * 0x200
		&& bul->x > ((x * 16) - 8) * 0x200
		&& bul->y + (2 * 0x200) > ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) - (4 * 0x200)
		&& bul->y - (2 * 0x200) < ((y * 16) + 8) * 0x200)
	{
		if (bul->bbits & 8)
			bul->y = ((y * 16) * 0x200) - ((bul->x - ((x * 16) * 0x200)) / 2) - (6 * 0x200);
		else
			Vanish(bul);

		hit |= 0x18;
	}

	return hit;
}



// Mod-specific Global Variables (Likely for a Harpoon or Grapple weapon)
 unsigned char gGrappleState;
 int gGrappleDist;
 int gGrappleTgtX;
 int gGrappleTgtY;
 int gGrappleUnk;

 #include "Map.h"
 #include "MyChar.h"
 #include <cmath>
int FUN_00494770(int tx, int ty, BULLET *bul)
{
    int hit = 0;

    if (   bul->x - bul->blockXL < (tx * 16 + 8) * 0x200
        && bul->x + bul->blockXL > (tx * 16 - 8) * 0x200
        && bul->y - bul->blockYL < (ty * 16 + 8) * 0x200
        && bul->y + bul->blockYL > (ty * 16 - 8) * 0x200)
    {
        hit = 0x200;
    }

    if (hit == 0)
        return 0;

    if ((bul->bbits & 0xe3) == 0)
        return hit;

    unsigned char atrb = (unsigned char)GetAttribute(tx, ty);
    int snd      = 0;
    int particles = 0;

    if (atrb == 0x0a)
    {
        if (bul->code_bullet != 19 || gGrappleState != 1)
            return hit;

        gGrappleUnk   = 0;
        gGrappleState = 2;

        int snap_x = (tx * 16 + 5) * 0x200;
        int snap_y = (ty * 16 + 5) * 0x200;
        gGrappleTgtX = snap_x;
        gGrappleTgtY = snap_y;
        bul->x = snap_x;
        bul->y = snap_y;

        PlaySoundObject(4,    SOUND_MODE_PLAY);
        PlaySoundObject(0x25, SOUND_MODE_PLAY);

        float dx  = (float)(snap_x - gMC.x);
        float dy  = (float)(snap_y - gMC.y);
        int dist  = (int)sqrtf(dx * dx + dy * dy);

        if (dist > 0xc000) dist = 0xc000;
        if (dist < 0x1000) dist = 0x1000;

        gGrappleDist = dist;
        return hit;
    }
    else if (atrb == 0x43)
    {
        // bbits | 0x60 is always nonzero — unconditional
        if (bul->bbits & 0x40)
            bul->count1 = 0;    // NOT act_no — count1 at +0x4c

        snd       = 12;
        particles = 4;
    }
    else if (atrb == 0x47)
    {
        if (!(bul->bbits & 0x80))
            return hit;

        if (bul->bbits & 0x40)
            bul->count1 = 0;    // NOT act_no — count1 at +0x4c

        snd       = 117;
        particles = 8;
    }
    else if (atrb == 0x48)
    {
        if (!(bul->bbits & 0x80) || !(bul->bbits & 0x40))
            return hit;

        snd       = 44;
        particles = 0;
        bul->life = 1;    // NOT ani_no — life at +0x5c
    }
    else if (atrb == 0x4f)
    {
        if (!(bul->bbits & 0x02))
            return hit;

        bul->life = 0;    // NOT ani_no — life at +0x5c
        ShiftMapParts(tx, ty);
        // falls through to SetCaret etc
    }
    else
    {
        // Non-destructible tile
        if (bul->code_bullet == 7)
            bul->count1 -= 2;
        else if (bul->code_bullet != 8)
            bul->flag = 0xf;

        return hit;
    }

    // Common destruction path
    SetCaret(bul->x, bul->y, 2, 0);

    if (snd != 0)
        PlaySoundObject(snd, SOUND_MODE_PLAY);

    for (int i = 0; i < particles; ++i)
    {
        int vx = Random(-0x200, 0x200);
        int vy = Random(-0x200, 0x200);
        SetNpChar(4, tx << 13, ty << 13, vx, vy, 0, 0, 0x100);
    }

    ShiftMapParts(tx, ty);

    // Chain-drill only for tile 0x47 AND bullet code 14
    if (atrb == 0x47 && bul->code_bullet == 0x0e)
    {
        for (int i = 0; i < 3; ++i)
        {
            if ((unsigned char)GetAttribute(tx, ty) != 0x47)
                break;
            ShiftMapParts(tx, ty);
        }

        // Push bullet back based on direction — only inside this block
        switch (bul->direct)
        {
            case 0: bul->xm += 0x200; break;   // left  → push right
            case 1: bul->ym += 0x200; break;   // up    → push down
            case 2: bul->xm -= 0x200; break;   // right → push left
            case 3: bul->ym -= 0x200; break;   // down  → push up
        }
    }

    return hit;
}

void HitBulletMap(void)
{
    for (int i = 0; i < BULLET_MAX; ++i)
    {
        if (!(gBul[i].cond & 0x80))
            continue;

        // Signed floor division — matches ASM arithmetic shift rounding
        // equivalent to floor(x / 16) / 512 = floor(x / 8192)
        int px = gBul[i].x;
        int py = gBul[i].y;
        int x = ((px >> 4) + ((px >> 4) < 0 ? 0x1ff : 0)) >> 9;
        int y = ((py >> 4) + ((py >> 4) < 0 ? 0x1ff : 0)) >> 9;

        // Surrounding tile offsets — order: TL, TR, BL, BR
        // offx[] = {0,1,0,1}, offy[] = {0,0,1,1}
        // stored as local_38[0..3] = offx, local_38[4..7] = offy

        unsigned char atrb[4];
        atrb[0] = GetAttribute(x,     y    );
        atrb[1] = GetAttribute(x + 1, y    );
        atrb[2] = GetAttribute(x,     y + 1);
        atrb[3] = GetAttribute(x + 1, y + 1);

        // offx[j] = j & 1, offy[j] = j >> 1
        // i.e. j=0: (0,0), j=1: (1,0), j=2: (0,1), j=3: (1,1)

        gBul[i].flag = 0;

        if (gBul[i].bbits & 4)
        {
            // Ignore-wall flag set — skip per-tile collision,
            // only run the full-block judge
            gBul[i].flag |= JudgeHitBulletBlock2(x, y, atrb, &gBul[i]);
        }
        else
        {
            for (int j = 0; j < 4; ++j)
            {
                if (!(gBul[i].cond & 0x80))
                    break;

                int tx = x + (j & 1);
                int ty = y + (j >> 1);

                switch (atrb[j])
                {
                    // Solid blocks
                    case 0x06: case 0x07: case 0x08: case 0x09:
                    case 0x59: case 0x5a: case 0x5b: case 0x5c:
                    case 0x5d: case 0x5e: case 0x5f:
                        gBul[i].flag |= JudgeHitBulletBlock(tx, ty, &gBul[i]);
                        break;

                    // New slope type — FUN_00494770
                    case 0x0a:
                    case 0x43: case 0x47: case 0x48: case 0x49:
                    case 0x4a: case 0x4b: case 0x4c: case 0x4d:
                    case 0x4e: case 0x4f:
                        FUN_00494770(tx, ty, &gBul[i]);
                        gBul[i].flag |= gBul[i].flag;  // extraout_EAX — return via eax
                        break;

                    // Triangle slopes A-H, each with 3 tile variants
                    case 0x10: case 0x50: case 0x70:
                        gBul[i].flag |= JudgeHitBulletTriangleA(tx, ty, &gBul[i]);
                        break;

                    case 0x11: case 0x51: case 0x71:
                        gBul[i].flag |= JudgeHitBulletTriangleB(tx, ty, &gBul[i]);
                        break;

                    case 0x12: case 0x52: case 0x72:
                        gBul[i].flag |= JudgeHitBulletTriangleC(tx, ty, &gBul[i]);
                        break;

                    case 0x13: case 0x53: case 0x73:
                        gBul[i].flag |= JudgeHitBulletTriangleD(tx, ty, &gBul[i]);
                        break;

                    case 0x14: case 0x54: case 0x74:
                        gBul[i].flag |= JudgeHitBulletTriangleE(tx, ty, &gBul[i]);
                        break;

                    case 0x15: case 0x55: case 0x75:
                        gBul[i].flag |= JudgeHitBulletTriangleF(tx, ty, &gBul[i]);
                        break;

                    case 0x16: case 0x56: case 0x76:
                        gBul[i].flag |= JudgeHitBulletTriangleG(tx, ty, &gBul[i]);
                        break;

                    case 0x17: case 0x57: case 0x77:
                        gBul[i].flag |= JudgeHitBulletTriangleH(tx, ty, &gBul[i]);
                        break;

                    // All other tiles (passthrough, water, etc.)
                    // fall to default JudgeHitBulletBlock2 below
                    default:
                        gBul[i].flag |= JudgeHitBulletBlock2(x, y, atrb, &gBul[i]);
                        break;
                }
            }
        }
    }
}
