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
	// 37, 38, and 39 are the Bullet IDs for the Sword (Lv1, Lv2, Lv3).
	// If the bullet is a sword, do NOT play the "tink" sound.
	if (bul->code_bullet == 37 || bul->code_bullet == 38 || bul->code_bullet == 39)
	{
		SetCaret(bul->x, bul->y, CARET_PROJECTILE_DISSIPATION, 1);
	}
	else
	{
		// Play standard "Tink" sound for all other bullets
		PlaySoundObject(28, SOUND_MODE_PLAY);
	}

	bul->cond = 0; // Destroy the bullet
	SetCaret(bul->x, bul->y, CARET_PROJECTILE_DISSIPATION, 2);
}

int JudgeHitBulletBlock(int x, int y, BULLET *bul)
{
	int hit = 0;

	// Check if bullet overlaps the block tile
	if (bul->x - bul->blockXL < (x * 16 + 8) * 0x200 &&
		bul->x + bul->blockXL > (x * 16 - 8) * 0x200 &&
		bul->y - bul->blockYL < (y * 16 + 8) * 0x200 &&
		bul->y + bul->blockYL > (y * 16 - 8) * 0x200)
	{
		hit = 0x200;
	}

	// 0x60 = Bits for "Breaks blocks" (0x20) and "Pierces blocks" (0x40)
	if (hit && (bul->bbits & 0x60))
	{
		if (GetAttribute(x, y) == 0x43) // 0x43 is the Breakable Block attribute
		{
			// Reset timer if it's a piercing weapon (Vanilla destroyed non-piercing weapons)
			if (bul->bbits & 0x40)
				bul->count1 = 0;

			SetCaret(bul->x, bul->y, CARET_PROJECTILE_DISSIPATION, 0);
			PlaySoundObject(12, 1); // Block break sound
			
			for (int i = 0; i < 4; i++)
				SetNpChar(4, x * 0x2000, y * 0x2000, Random(-0x200, 0x200), Random(-0x200, 0x200), 0, NULL, 0x100);
			
			ShiftMapParts(x, y); // Break the block
		}
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
			// Modded solid block check for piercing bullets
			if (*atrb == 0x41 || *atrb == 0x46 || (*atrb >= 0x49 && *atrb <= 0x4F) || *atrb == 0x18 || (*atrb == 0x0A && bul->code_bullet != 19))
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
			// Modded solid block check for normal bullets
			if (*atrb == 0x41 || *atrb == 0x46 || *atrb == 0x43 || (*atrb >= 0x47 && *atrb <= 0x4F) || *atrb == 0x18 || (*atrb == 0x0A && bul->code_bullet != 19))
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
			Vanish(bul); // The mod moved Vanish() to 0x494EA0 without changing its internal operation
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
 extern unsigned char gGrappleState;
 extern int gGrappleLength;
 extern int gGrappleX;
 extern int gGrappleY;
 extern int gGrappleMomentum;

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

        gGrappleMomentum   = 0;
        gGrappleState = 2;

        int snap_x = (tx * 16 + 5) * 0x200;
        int snap_y = (ty * 16 + 5) * 0x200;
        gGrappleX = snap_x;
        gGrappleY = snap_y;
        bul->x = snap_x;
        bul->y = snap_y;

        PlaySoundObject(4,    SOUND_MODE_PLAY);
        PlaySoundObject(0x25, SOUND_MODE_PLAY);

        float dx  = (float)(snap_x - gMC.x);
        float dy  = (float)(snap_y - gMC.y);
        int dist  = (int)sqrtf(dx * dx + dy * dy);

        if (dist > 0xc000) dist = 0xc000;
        if (dist < 0x1000) dist = 0x1000;

        gGrappleLength = dist;
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

#include "Fluid.h"
void HitBulletMap(void)
{
	int i;
	int j;

	int x, y;
	int tx, ty;
	unsigned char atrb[4];
	int offx[4] = {0, 1, 0, 1};
	int offy[4] = {0, 0, 1, 1};

	for (i = 0; i < BULLET_MAX; ++i)
	{
		if (gBul[i].cond & 0x80)
		{
			// Arithmetic Shift Right equivalent to properly support negative values
			int px = gBul[i].x;
			int py = gBul[i].y;
			x = ((px >> 4) + ((px >> 4) < 0 ? 0x1ff : 0)) >> 9;
			y = ((py >> 4) + ((py >> 4) < 0 ? 0x1ff : 0)) >> 9;

			atrb[0] = GetAttribute(x, y);
			atrb[1] = GetAttribute(x + 1, y);
			atrb[2] = GetAttribute(x, y + 1);
			atrb[3] = GetAttribute(x + 1, y + 1);

			gBul[i].flag = 0;

			if (!(gBul[i].bbits & 4))
			{
				for (j = 0; j < 4; ++j)
				{
					if (!(gBul[i].cond & 0x80))
						break;

					tx = x + offx[j];
					ty = y + offy[j];

					switch (atrb[j])
					{
						// Solid blocks
						case 0x06: case 0x07: case 0x08: case 0x09:
						case 0x59: case 0x5A: case 0x5B: case 0x5C:
						case 0x5D: case 0x5E: case 0x5F:
							gBul[i].flag |= JudgeHitBulletBlock(tx, ty, &gBul[i]);
							continue;

						case 0x10: case 0x50: case 0x70:
							gBul[i].flag |= JudgeHitBulletTriangleA(tx, ty, &gBul[i]);
							continue;

						case 0x11: case 0x51: case 0x71:
							gBul[i].flag |= JudgeHitBulletTriangleB(tx, ty, &gBul[i]);
							continue;

						case 0x12: case 0x52: case 0x72:
							gBul[i].flag |= JudgeHitBulletTriangleC(tx, ty, &gBul[i]);
							continue;

						case 0x13: case 0x53: case 0x73:
							gBul[i].flag |= JudgeHitBulletTriangleD(tx, ty, &gBul[i]);
							continue;

						case 0x14: case 0x54: case 0x74:
							gBul[i].flag |= JudgeHitBulletTriangleE(tx, ty, &gBul[i]);
							continue;

						case 0x15: case 0x55: case 0x75:
							gBul[i].flag |= JudgeHitBulletTriangleF(tx, ty, &gBul[i]);
							continue;

						case 0x16: case 0x56: case 0x76:
							gBul[i].flag |= JudgeHitBulletTriangleG(tx, ty, &gBul[i]);
							continue;

						case 0x17: case 0x57: case 0x77:
							gBul[i].flag |= JudgeHitBulletTriangleH(tx, ty, &gBul[i]);
							continue;

						// Custom Hook Handler
						case 0x0A: case 0x43: case 0x47: case 0x48:
						case 0x49: case 0x4A: case 0x4B: case 0x4C:
						case 0x4D: case 0x4E: case 0x4F:
							gBul[i].flag |= FUN_00494770(tx, ty, &gBul[i]);
							continue;

						// Certain attributes short-circuit tile evaluation directly to the bounding box judge
						case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
						case 0x41: case 0x46: case 0x58: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
						case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
						case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
						case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
						case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E:
							goto hit_block_2;

						default:
							continue;
					}
				}
			}

hit_block_2:
			gBul[i].flag |= JudgeHitBulletBlock2(x, y, atrb, &gBul[i]);
			
			if (IsFluidAt(gBul[i].x, gBul[i].y))
    		{
    		    // Bullet passing through water — small continuous disturbance
    		    DisturbFluid(gBul[i].x, gBul[i].y, gBul[i].xm, gBul[i].ym, 4);
    		}
		}
	}
}