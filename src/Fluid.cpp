// Fluid.cpp
// Cellular Automata fluid simulation for CSE2.
//
// Design goals:
//   - True ping-pong double buffer (reads old, writes new — no same-frame contamination)
//   - Correct bottom-to-top iteration for falling cells
//   - Correct top-to-bottom iteration for rising cells (steam/gas)
//   - Randomized L/R priority per row to prevent directional bias
//   - Precomputed solid mask rebuilt only on map load or tile destruction
//   - Run-length encoded draw calls to minimize CortBox overhead
//   - Multiple cell types with distinct physics rules
//   - Optional light system bridge via FLUID_LIGHT_INTEGRATION

#include "Fluid.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "WindowsWrapper.h"
#include "Draw.h"
#include "Map.h"

#include "CommonDefines.h"

// ─── Fast local RNG (xorshift32) ─────────────────────────────────────────────
// Using this instead of Cave Story's Random() to avoid touching global RNG state
// and because we call it thousands of times per frame.
static unsigned int fluid_rng_state = 0x12345678;

static inline unsigned int FluidRand()
{
	fluid_rng_state ^= fluid_rng_state << 13;
	fluid_rng_state ^= fluid_rng_state >> 17;
	fluid_rng_state ^= fluid_rng_state << 5;
	return fluid_rng_state;
}

static inline int FluidRandBool()
{
	return (FluidRand() >> 16) & 1;
}

// ─── Grid storage ─────────────────────────────────────────────────────────────
int gFluidWidth  = 0;
int gFluidHeight = 0;

static FluidCell *gFluidGrid = NULL;   // current frame (read-only during update)
static FluidCell *gFluidNext = NULL;   // next frame    (write-only during update)
static bool      *gSolidMask = NULL;   // precomputed per-cell solidity

// ─── Coordinate helpers ───────────────────────────────────────────────────────
#define CELL(grid, x, y) ((grid)[(y) * gFluidWidth + (x)])
#define IN_BOUNDS(x, y)  ((x) >= 0 && (x) < gFluidWidth && (y) >= 0 && (y) < gFluidHeight)

static inline int SubToCell(int subpixel)
{
	// subpixel → pixel → cell
	return (subpixel / 0x200) / FLUID_CELL_SIZE;
}

static inline bool SolidAt(int cx, int cy)
{
	if (!IN_BOUNDS(cx, cy)) return true;
	return gSolidMask[cy * gFluidWidth + cx];
}

static inline bool NextEmpty(int cx, int cy)
{
	if (!IN_BOUNDS(cx, cy)) return false;
	return CELL(gFluidNext, cx, cy).type == FLUID_EMPTY;
}
// Add this after any MoveCell call in UpdateWaterCell:
static void WakeNeighbors(int x, int y)
{
	const int neighbors[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
	for (int n = 0; n < 4; n++)
	{
		int nx = x + neighbors[n][0];
		int ny = y + neighbors[n][1];
		if (!IN_BOUNDS(nx, ny)) continue;
		if (CELL(gFluidGrid, nx, ny).type == FLUID_WATER)
			CELL(gFluidNext, nx, ny).life = 0; // wake it up
	}
}
static void MoveCell(int from_x, int from_y, int to_x, int to_y)
{
	CELL(gFluidNext, to_x, to_y)         = CELL(gFluidGrid, from_x, from_y);
	CELL(gFluidNext, to_x, to_y).life    = 0; // destination starts unsettled
	CELL(gFluidNext, from_x, from_y).type   = FLUID_EMPTY;
	CELL(gFluidNext, from_x, from_y).amount = 0;
	CELL(gFluidNext, from_x, from_y).temp   = 0;
	CELL(gFluidNext, from_x, from_y).life   = 0;
	WakeNeighbors(from_x, from_y);
}
// ─── Solid mask ───────────────────────────────────────────────────────────────
// Called once on map load and again when tiles are destroyed.

void RebuildFluidSolidMask()
{
	if (!gSolidMask) return;

	for (int cy = 0; cy < gFluidHeight; cy++)
	{
		for (int cx = 0; cx < gFluidWidth; cx++)
		{
			int tx = (cx * FLUID_CELL_SIZE) / 16;
			int ty = (cy * FLUID_CELL_SIZE) / 16;

			if (tx < 0 || tx >= gMap.width || ty < 0 || ty >= gMap.length)
			{
				gSolidMask[cy * gFluidWidth + cx] = true;
				continue;
			}

			unsigned char atrb = GetAttribute(tx, ty);

			// Solid tile types — extend this list to match your tile set
			bool solid = false;

			// Full blocks
			if (atrb == 0x05 || atrb == 0x41 || atrb == 0x43)
				solid = true;

			// Block ranges
			if ((atrb >= 0x06 && atrb <= 0x0A))
				solid = true;
			if ((atrb >= 0x47 && atrb <= 0x4F))
				solid = true;
			if ((atrb >= 0x59 && atrb <= 0x5F))
				solid = true;

			// Slopes — treat as solid for fluid (fluid doesn't slide slopes gracefully)
			if ((atrb >= 0x10 && atrb <= 0x17))
				solid = true;
			if ((atrb >= 0x50 && atrb <= 0x57))
				solid = true;
			if ((atrb >= 0x70 && atrb <= 0x77))
				solid = true;

			gSolidMask[cy * gFluidWidth + cx] = solid;
		}
	}
}

void RebuildFluidSolidMaskRegion(int tx, int ty, int radius_tiles)
{
	// Convert tile region to cell region and rebuild only that area
	int cx_start = ((tx - radius_tiles) * 16) / FLUID_CELL_SIZE;
	int cy_start = ((ty - radius_tiles) * 16) / FLUID_CELL_SIZE;
	int cx_end   = ((tx + radius_tiles + 1) * 16) / FLUID_CELL_SIZE;
	int cy_end   = ((ty + radius_tiles + 1) * 16) / FLUID_CELL_SIZE;

	if (cx_start < 0) cx_start = 0;
	if (cy_start < 0) cy_start = 0;
	if (cx_end > gFluidWidth)  cx_end = gFluidWidth;
	if (cy_end > gFluidHeight) cy_end = gFluidHeight;

	for (int cy = cy_start; cy < cy_end; cy++)
	{
		for (int cx = cx_start; cx < cx_end; cx++)
		{
			int mtx = (cx * FLUID_CELL_SIZE) / 16;
			int mty = (cy * FLUID_CELL_SIZE) / 16;

			if (mtx < 0 || mtx >= gMap.width || mty < 0 || mty >= gMap.length)
			{
				gSolidMask[cy * gFluidWidth + cx] = true;
				continue;
			}

			unsigned char atrb = GetAttribute(mtx, mty);
			bool solid = false;

			if (atrb == 0x05 || atrb == 0x41 || atrb == 0x43) solid = true;
			if (atrb >= 0x06 && atrb <= 0x0A)  solid = true;
			if (atrb >= 0x47 && atrb <= 0x4F)  solid = true;
			if (atrb >= 0x59 && atrb <= 0x5F)  solid = true;
			if (atrb >= 0x10 && atrb <= 0x17)  solid = true;
			if (atrb >= 0x50 && atrb <= 0x57)  solid = true;
			if (atrb >= 0x70 && atrb <= 0x77)  solid = true;

			gSolidMask[cy * gFluidWidth + cx] = solid;
		}
	}
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void InitFluidSystem()
{
	gFluidGrid  = NULL;
	gFluidNext  = NULL;
	gSolidMask  = NULL;
	gFluidWidth = 0;
	gFluidHeight = 0;
	fluid_rng_state = (unsigned int)time(NULL) ^ 0xDEADBEEF;
}

void ResetFluidSystem(int map_width, int map_length)
{
	FreeFluidSystem();

	gFluidWidth  = (map_width  * 16) / FLUID_CELL_SIZE;
	gFluidHeight = (map_length * 16) / FLUID_CELL_SIZE;

	int count = gFluidWidth * gFluidHeight;

	gFluidGrid = (FluidCell*)calloc(count, sizeof(FluidCell));
	gFluidNext = (FluidCell*)calloc(count, sizeof(FluidCell));
	gSolidMask = (bool*)calloc(count, sizeof(bool));

	RebuildFluidSolidMask();
}

void FreeFluidSystem()
{
	if (gFluidGrid) { free(gFluidGrid); gFluidGrid = NULL; }
	if (gFluidNext) { free(gFluidNext); gFluidNext = NULL; }
	if (gSolidMask) { free(gSolidMask); gSolidMask = NULL; }
	gFluidWidth  = 0;
	gFluidHeight = 0;
}

// ─── Cell update rules ────────────────────────────────────────────────────────

static void UpdateWaterCell(int x, int y)
{
	// 1. Fall straight down
	if (!SolidAt(x, y + 1) && NextEmpty(x, y + 1))
	{
		WakeNeighbors(x, y);
		MoveCell(x, y, x, y + 1);
		return;
	}

	// 2. Fall diagonally
	{
		int dx = FluidRandBool() ? 1 : -1;
		for (int attempt = 0; attempt < 2; attempt++)
		{
			int nx = x + dx;
			if (!SolidAt(nx, y) && !SolidAt(nx, y + 1) &&
			    NextEmpty(nx, y) && NextEmpty(nx, y + 1))
			{
				WakeNeighbors(x, y);
				MoveCell(x, y, nx, y + 1);
				return;
			}
			dx = -dx;
		}
	}

	// 3. Horizontal flow
{
    unsigned char settle = CELL(gFluidGrid, x, y).life;
    if (settle >= 6)
        return;

    bool can_left  = IN_BOUNDS(x - 1, y) && !SolidAt(x - 1, y) && NextEmpty(x - 1, y);
    bool can_right = IN_BOUNDS(x + 1, y) && !SolidAt(x + 1, y) && NextEmpty(x + 1, y);

    if (!can_left && !can_right)
    {
        CELL(gFluidNext, x, y).life = settle + 1;
        return;
    }

    int dx;
    if (can_left && can_right)
    {
        // Count empty space in each direction to find which side
        // has more room — flow toward the lower/emptier side
        int space_left = 0, space_right = 0;
        for (int s = 1; s <= FLUID_MAX_SPREAD * 4; s++)
        {
            if (IN_BOUNDS(x - s, y) && !SolidAt(x - s, y) && CELL(gFluidGrid, x - s, y).type == FLUID_EMPTY)
                space_left++;
            else
                break;
        }
        for (int s = 1; s <= FLUID_MAX_SPREAD * 4; s++)
        {
            if (IN_BOUNDS(x + s, y) && !SolidAt(x + s, y) && CELL(gFluidGrid, x + s, y).type == FLUID_EMPTY)
                space_right++;
            else
                break;
        }

        if (space_left == space_right)
            dx = FluidRandBool() ? 1 : -1; // genuine tie — only case we use RNG
        else
            dx = (space_left > space_right) ? -1 : 1; // flow toward more space
    }
    else if (can_left)
        dx = -1;
    else
        dx = 1;

    for (int s = 1; s <= FLUID_MAX_SPREAD; s++)
    {
        int nx = x + dx * s;
        if (!IN_BOUNDS(nx, y) || SolidAt(nx, y)) break;
        if (NextEmpty(nx, y))
        {
            WakeNeighbors(x, y);
            MoveCell(x, y, nx, y);
            return;
        }
    }

    CELL(gFluidNext, x, y).life = settle + 2;
}
}


// Disturb fluid around a point — clears settle counters so
// nearby cells start flowing again, creating a wave effect
void DisturbFluid(int sub_x, int sub_y, int sub_vx, int sub_vy, int radius_cells)
{
    if (!gFluidGrid) return;

    int cx = SubToCell(sub_x);
    int cy = SubToCell(sub_y);

    int dx = (sub_vx > 0) ? 1 : (sub_vx < 0) ? -1 : 0;

    // ── Step 1: Eject water upward in a column at the impact point ───────────
    // Find all water cells in the column at cx and lift them up
    // The higher the column the bigger the visible splash
    int eject_height = radius_cells * 2;

    for (int ry = 0; ry >= -eject_height; ry--)
    {
        int ny = cy + ry;
        if (!IN_BOUNDS(cx, ny)) continue;
        if (CELL(gFluidGrid, cx, ny).type != FLUID_WATER) continue;

        // Find the highest empty cell above this water cell
        for (int up = 1; up <= eject_height; up++)
        {
            int target_y = ny - up;
            if (!IN_BOUNDS(cx, target_y)) break;
            if (SolidAt(cx, target_y)) break;

            if (CELL(gFluidGrid, cx, target_y).type == FLUID_EMPTY)
            {
                // Lift this water cell up — skipping cells creates
                // a visible gap that gravity then collapses into a wave
                CELL(gFluidGrid, cx, target_y) = CELL(gFluidGrid, cx, ny);
                CELL(gFluidGrid, cx, target_y).life = 0;
                CELL(gFluidGrid, cx, ny).type   = FLUID_EMPTY;
                CELL(gFluidGrid, cx, ny).amount = 0;
                CELL(gFluidGrid, cx, ny).life   = 0;
                break;
            }
        }
    }

    // ── Step 2: Eject water sideways in both directions from impact ──────────
    // Spread ejected cells outward — wider radius = bigger wave
    for (int side = -1; side <= 1; side += 2)
    {
        for (int r = 1; r <= radius_cells; r++)
        {
            int nx = cx + side * r;
            int ny = cy;

            // Find water surface in this column
            for (int scan = 0; scan <= radius_cells; scan++)
            {
                if (!IN_BOUNDS(nx, ny + scan)) break;
                if (CELL(gFluidGrid, nx, ny + scan).type != FLUID_WATER) continue;

                // Eject upward and outward — height decreases with distance
                int height = (radius_cells - r) + 1;
                int target_y = ny + scan - height;

                if (!IN_BOUNDS(nx, target_y)) break;
                if (SolidAt(nx, target_y)) break;

                if (CELL(gFluidGrid, nx, target_y).type == FLUID_EMPTY)
                {
                    CELL(gFluidGrid, nx, target_y) = CELL(gFluidGrid, nx, ny + scan);
                    CELL(gFluidGrid, nx, target_y).life = 0;
                    CELL(gFluidGrid, nx, ny + scan).type   = FLUID_EMPTY;
                    CELL(gFluidGrid, nx, ny + scan).amount = 0;
                    CELL(gFluidGrid, nx, ny + scan).life   = 0;
                }
                break;
            }
        }
    }

    // ── Step 3: Wake entire radius so cells flow to fill the gaps ───────────
    for (int ry = -radius_cells - 2; ry <= radius_cells + 2; ry++)
    {
        for (int rx = -radius_cells - 2; rx <= radius_cells + 2; rx++)
        {
            int nx = cx + rx;
            int ny = cy + ry;
            if (!IN_BOUNDS(nx, ny)) continue;
            if (CELL(gFluidGrid, nx, ny).type == FLUID_WATER)
                CELL(gFluidGrid, nx, ny).life = 0;
        }
    }
}
static void UpdateLavaCell(int x, int y)
{
	// Lava behaves like water but slower (only spreads 1 cell per frame)
	// and generates steam when adjacent to water

	// Check for adjacent water — convert both to steam
	const int neighbors[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};
	for (int n = 0; n < 4; n++)
	{
		int nx = x + neighbors[n][0];
		int ny = y + neighbors[n][1];
		if (!IN_BOUNDS(nx, ny)) continue;
		if (CELL(gFluidGrid, nx, ny).type == FLUID_WATER)
		{
			// Steam burst upward from contact point
			FluidCell steam;
			steam.type   = FLUID_STEAM;
			steam.amount = 200;
			steam.temp   = 220;
			steam.life   = 60 + (FluidRand() % 40);

			// Place steam above contact if possible
			if (IN_BOUNDS(nx, ny - 1) && NextEmpty(nx, ny - 1))
				CELL(gFluidNext, nx, ny - 1) = steam;
			else if (IN_BOUNDS(x, y - 1) && NextEmpty(x, y - 1))
				CELL(gFluidNext, x, y - 1) = steam;

			// Consume the water cell
			CELL(gFluidNext, nx, ny).type = FLUID_EMPTY;

			// Lava cools slightly at contact
			CELL(gFluidNext, x, y).temp =
				CELL(gFluidGrid, x, y).temp > 20
				? CELL(gFluidGrid, x, y).temp - 10 : 0;
			return;
		}
	}

	// Fall down
	if (!SolidAt(x, y + 1) && NextEmpty(x, y + 1))
	{
		MoveCell(x, y, x, y + 1);
		return;
	}

	// Diagonal slide
	{
		int dx = FluidRandBool() ? 1 : -1;
		for (int attempt = 0; attempt < 2; attempt++)
		{
			int nx = x + dx;
			if (!SolidAt(nx, y) && !SolidAt(nx, y + 1) &&
			    NextEmpty(nx, y) && NextEmpty(nx, y + 1))
			{
				MoveCell(x, y, nx, y + 1);
				return;
			}
			dx = -dx;
		}
	}

	// Slow horizontal spread (lava is viscous — max 1 cell)
	{
		int dx = FluidRandBool() ? 1 : -1;
		for (int attempt = 0; attempt < 2; attempt++)
		{
			int nx = x + dx;
			if (IN_BOUNDS(nx, y) && !SolidAt(nx, y) && NextEmpty(nx, y))
			{
				// Lava only spreads 50% of the time horizontally
				if (FluidRandBool())
				{
					MoveCell(x, y, nx, y);
					return;
				}
			}
			dx = -dx;
		}
	}
}

static void UpdateSteamCell(int x, int y)
{
	FluidCell *c = &CELL(gFluidGrid, x, y);

	// Decay life
	unsigned char new_life = c->life > 0 ? c->life - 1 : 0;
	if (new_life == 0)
	{
		CELL(gFluidNext, x, y).type = FLUID_EMPTY;
		return;
	}
	CELL(gFluidNext, x, y).life = new_life;

	// Rise upward
	if (!SolidAt(x, y - 1) && NextEmpty(x, y - 1))
	{
		MoveCell(x, y, x, y - 1);
		CELL(gFluidNext, x, y - 1).life = new_life;
		return;
	}

	// Spread sideways at ceiling
	{
		int dx = FluidRandBool() ? 1 : -1;
		for (int attempt = 0; attempt < 2; attempt++)
		{
			int nx = x + dx;
			if (IN_BOUNDS(nx, y) && !SolidAt(nx, y) && NextEmpty(nx, y))
			{
				MoveCell(x, y, nx, y);
				CELL(gFluidNext, nx, y).life = new_life;
				return;
			}
			dx = -dx;
		}
	}
}

static void UpdateGasCell(int x, int y)
{
	FluidCell *c = &CELL(gFluidGrid, x, y);

	unsigned char new_life = c->life > 0 ? c->life - 1 : 0;
	if (new_life == 0)
	{
		CELL(gFluidNext, x, y).type = FLUID_EMPTY;
		return;
	}
	CELL(gFluidNext, x, y).life = new_life;

	// Rise, but more erratic than steam
	if (!SolidAt(x, y - 1) && NextEmpty(x, y - 1) && FluidRandBool())
	{
		MoveCell(x, y, x, y - 1);
		CELL(gFluidNext, x, y - 1).life = new_life;
		return;
	}

	// Wide horizontal spread (gas disperses aggressively)
	{
		int dx = FluidRandBool() ? 1 : -1;
		for (int attempt = 0; attempt < 2; attempt++)
		{
			for (int s = 1; s <= 3; s++)
			{
				int nx = x + dx * s;
				if (!IN_BOUNDS(nx, y) || SolidAt(nx, y)) break;
				if (NextEmpty(nx, y))
				{
					MoveCell(x, y, nx, y);
					CELL(gFluidNext, nx, y).life = new_life;
					return;
				}
			}
			dx = -dx;
		}
	}
}

static void UpdateRubbleCell(int x, int y)
{
	// Rubble falls like sand — no horizontal flow, no spread
	if (!SolidAt(x, y + 1) && NextEmpty(x, y + 1))
	{
		MoveCell(x, y, x, y + 1);
		return;
	}

	// Pile diagonally
	{
		int dx = FluidRandBool() ? 1 : -1;
		for (int attempt = 0; attempt < 2; attempt++)
		{
			int nx = x + dx;
			if (!SolidAt(nx, y) && !SolidAt(nx, y + 1) &&
			    NextEmpty(nx, y + 1))
			{
				MoveCell(x, y, nx, y + 1);
				return;
			}
			dx = -dx;
		}
	}
}

// ─── Main update ──────────────────────────────────────────────────────────────

void UpdateFluid()
{
	if (!gFluidGrid) return;

	// Copy current state to next — cells that don't move stay where they are.
	// This is correct ping-pong: UpdateXCell reads from gFluidGrid, writes to gFluidNext.
	memcpy(gFluidNext, gFluidGrid, gFluidWidth * gFluidHeight * sizeof(FluidCell));

	// ── Falling cells: bottom to top ─────────────────────────────────────────
	for (int y = gFluidHeight - 2; y >= 0; y--)
	{
		// Randomize horizontal scan direction per row
		int dir    = FluidRandBool() ? 1 : -1;
		int start  = (dir == 1) ? 0 : gFluidWidth - 1;
		int end    = (dir == 1) ? gFluidWidth : -1;

		for (int x = start; x != end; x += dir)
		{
			// Only process cells that exist in the READ buffer
			// and haven't already been consumed this frame
			unsigned char type = CELL(gFluidGrid, x, y).type;

			// Skip if this cell was already moved (next buffer shows it empty)
			if (CELL(gFluidNext, x, y).type == FLUID_EMPTY && type != FLUID_EMPTY)
				continue;

			switch (type)
			{
				case FLUID_WATER:  UpdateWaterCell(x, y);  break;
				case FLUID_LAVA:   UpdateLavaCell(x, y);   break;
				case FLUID_RUBBLE: UpdateRubbleCell(x, y); break;
				default: break;
			}
		}
	}

	// ── Rising cells: top to bottom ──────────────────────────────────────────
	for (int y = 1; y < gFluidHeight; y++)
	{
		int dir   = FluidRandBool() ? 1 : -1;
		int start = (dir == 1) ? 0 : gFluidWidth - 1;
		int end   = (dir == 1) ? gFluidWidth : -1;

		for (int x = start; x != end; x += dir)
		{
			unsigned char type = CELL(gFluidGrid, x, y).type;
			if (CELL(gFluidNext, x, y).type == FLUID_EMPTY && type != FLUID_EMPTY)
				continue;

			switch (type)
			{
				case FLUID_STEAM: UpdateSteamCell(x, y); break;
				case FLUID_GAS:   UpdateGasCell(x, y);   break;
				default: break;
			}
		}
	}

	// Swap buffers — next becomes current
	FluidCell *tmp = gFluidGrid;
	gFluidGrid     = gFluidNext;
	gFluidNext     = tmp;
}

// ─── Drawing ──────────────────────────────────────────────────────────────────

// Water color:   deep blue,  140/255 alpha
// Lava color:    orange-red, 200/255 alpha (mostly opaque)
// Steam color:   pale white, 80/255 alpha  (very transparent)
// Gas color:     sickly green, 100/255 alpha

static const unsigned long kFluidColor[6] = {
	0x000000,           // EMPTY    (unused)
	RGB(0x30, 0x70, 0xFF), // WATER
	RGB(0xFF, 0x50, 0x10), // LAVA
	RGB(0xCC, 0xDD, 0xFF), // STEAM
	RGB(0x40, 0xCC, 0x40), // GAS
	RGB(0x88, 0x66, 0x44), // RUBBLE
};

static const unsigned char kFluidAlpha[6] = {
	0,   // EMPTY
	140, // WATER
	200, // LAVA
	80,  // STEAM
	100, // GAS
	220, // RUBBLE
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
void DrawFluid(int cam_x, int cam_y)
{
	if (!gFluidGrid) return;

	const int cell_sub = FLUID_CELL_SIZE * 0x200;

	int start_cx = MAX(0, ((cam_x / 0x200) + 8) / FLUID_CELL_SIZE);
	int start_cy = MAX(0, ((cam_y / 0x200) + 8) / FLUID_CELL_SIZE);
	int end_cx   = MIN(gFluidWidth,  start_cx + (WINDOW_WIDTH  / FLUID_CELL_SIZE) + 2);
	int end_cy   = MIN(gFluidHeight, start_cy + (WINDOW_HEIGHT / FLUID_CELL_SIZE) + 2);

	RECT rect;

	for (int pass = FLUID_WATER; pass <= FLUID_RUBBLE; pass++)
	{
		unsigned long color = kFluidColor[pass];
		unsigned char alpha = kFluidAlpha[pass];

		for (int cx = start_cx; cx < end_cx; cx++)
		{
			// Scan each column bottom-up to find the highest water cell
			// then draw a solid filled rect from that point to the bottom
			// This makes surface dancing invisible because the column
			// below the surface is always drawn solid regardless of
			// whether individual cells are flickering
			int top_cy    = -1;
			int bottom_cy = -1;

			for (int cy = end_cy - 1; cy >= start_cy; cy--)
			{
				if (CELL(gFluidGrid, cx, cy).type == (unsigned char)pass)
				{
					if (bottom_cy == -1) bottom_cy = cy;
					top_cy = cy;
				}
				else if (bottom_cy != -1)
				{
					// Gap in the column — draw what we have and reset
					// (handles columns with solid tiles breaking them up)
					rect.left   = PixelToScreenCoord((cx            * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_x);
					rect.top    = PixelToScreenCoord((top_cy        * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_y);
					rect.right  = PixelToScreenCoord(((cx + 1)      * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_x);
					rect.bottom = PixelToScreenCoord(((bottom_cy + 1) * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_y);

					CortBoxAlpha(&rect, color, alpha);
					top_cy    = -1;
					bottom_cy = -1;
				}
			}

			// Draw remaining column segment if we hit the top of the scan range
			if (top_cy != -1 && bottom_cy != -1)
			{
				rect.left   = PixelToScreenCoord((cx              * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_x);
				rect.top    = PixelToScreenCoord((top_cy          * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_y);
				rect.right  = PixelToScreenCoord(((cx + 1)        * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_x);
				rect.bottom = PixelToScreenCoord(((bottom_cy + 1) * FLUID_CELL_SIZE) - 8) - SubpixelToScreenCoord(cam_y);

				CortBoxAlpha(&rect, color, alpha);
			}
		}
	}
}
// ─── Spawn / query ────────────────────────────────────────────────────────────

void AddFluid(int sub_x, int sub_y, int type, int amount)
{
	if (!gFluidGrid) return;

	int cx = SubToCell(sub_x);
	int cy = SubToCell(sub_y);
	int r  = amount / 2;

	FluidCell cell;
	cell.type   = (unsigned char)type;
	cell.amount = 255;
	cell.temp   = (type == FLUID_LAVA) ? 220 : 0;
	cell.life   = (type == FLUID_STEAM || type == FLUID_GAS) ? 80 + (FluidRand() % 40) : 255;

	for (int dy = -r; dy <= r; dy++)
	{
		for (int dx = -r; dx <= r; dx++)
		{
			int nx = cx + dx;
			int ny = cy + dy;
			if (!IN_BOUNDS(nx, ny)) continue;
			if (SolidAt(nx, ny)) continue;
			if (CELL(gFluidGrid, nx, ny).type != FLUID_EMPTY) continue;
			CELL(gFluidGrid, nx, ny) = cell;
		}
	}
}

int GetFluidTypeAt(int sub_x, int sub_y)
{
	if (!gFluidGrid) return FLUID_EMPTY;

	int cx = SubToCell(sub_x);
	int cy = SubToCell(sub_y);

	if (!IN_BOUNDS(cx, cy)) return FLUID_EMPTY;
	return CELL(gFluidGrid, cx, cy).type;
}

bool IsFluidAt(int sub_x, int sub_y)
{
	return GetFluidTypeAt(sub_x, sub_y) != FLUID_EMPTY;
}

// ─── Light system bridge ──────────────────────────────────────────────────────
#ifdef FLUID_LIGHT_INTEGRATION

void FluidContributeToLight(float *light_r, float *light_g, float *light_b,
                             int light_w, int light_h)
{
	if (!gFluidGrid) return;

	float sx = (float)light_w / gFluidWidth;
	float sy = (float)light_h / gFluidHeight;

	for (int cy = 0; cy < gFluidHeight; cy++)
	{
		for (int cx = 0; cx < gFluidWidth; cx++)
		{
			FluidCell *c = &CELL(gFluidGrid, cx, cy);
			if (c->type == FLUID_EMPTY) continue;

			int lx = (int)(cx * sx);
			int ly = (int)(cy * sy);
			if (lx >= light_w || ly >= light_h) continue;
			int li = lx + ly * light_w;

			switch (c->type)
			{
				case FLUID_LAVA:
				{
					float intensity = c->temp / 255.0f;
					light_r[li] += 0.85f * intensity;
					light_g[li] += 0.20f * intensity;
					if (light_r[li] > 1.0f) light_r[li] = 1.0f;
					if (light_g[li] > 1.0f) light_g[li] = 1.0f;
					break;
				}
				case FLUID_STEAM:
				{
					float density = c->life / 255.0f;
					light_r[li] += 0.12f * density;
					light_g[li] += 0.12f * density;
					light_b[li] += 0.15f * density;
					if (light_r[li] > 1.0f) light_r[li] = 1.0f;
					if (light_g[li] > 1.0f) light_g[li] = 1.0f;
					if (light_b[li] > 1.0f) light_b[li] = 1.0f;
					break;
				}
				case FLUID_GAS:
				{
					float density = c->life / 255.0f;
					light_r[li] *= (1.0f - density * 0.45f);
					light_g[li] += 0.08f * density;
					light_b[li] *= (1.0f - density * 0.55f);
					break;
				}
				case FLUID_WATER:
				{
					light_r[li] *= 0.88f;
					light_g[li] *= 0.92f;
					light_b[li] += 0.04f;
					if (light_b[li] > 1.0f) light_b[li] = 1.0f;
					break;
				}
				default:
					break;
			}
		}
	}
}

#endif // FLUID_LIGHT_INTEGRATION