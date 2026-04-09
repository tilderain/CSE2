#pragma once

// ─── Configuration ────────────────────────────────────────────────────────────
// Cell size in pixels. 4 = one cell per 4x4 pixels (4 cells per tile).
// Smaller = more detail, more CPU. 2 is viable on modern hardware.
#define FLUID_CELL_SIZE 2

// Maximum spread distance per frame for horizontal flow.
// Keep at 2 — prevents teleporting water while still feeling fast.
#define FLUID_MAX_SPREAD 2

// ─── Cell Types ───────────────────────────────────────────────────────────────
#define FLUID_EMPTY  0
#define FLUID_WATER  1
#define FLUID_LAVA   2
#define FLUID_STEAM  3
#define FLUID_GAS    4
#define FLUID_RUBBLE 5

// ─── Cell Structure ───────────────────────────────────────────────────────────
typedef struct FluidCell
{
	unsigned char type;     // FLUID_* type constant
	unsigned char amount;   // 1-255 quantity (for pressure/depth future use)
	unsigned char temp;     // temperature: drives lava glow, steam generation
	unsigned char life;     // gas/steam countdown to dissipation; 255 = permanent
} FluidCell;

// ─── Grid dimensions (set after ResetFluidSystem) ────────────────────────────
extern int gFluidWidth;
extern int gFluidHeight;

// ─── Lifecycle ────────────────────────────────────────────────────────────────
void InitFluidSystem();
void ResetFluidSystem(int map_width, int map_length);
void FreeFluidSystem();

// ─── Per-frame ────────────────────────────────────────────────────────────────
void UpdateFluid();
void DrawFluid(int cam_x, int cam_y);

// ─── Spawn / query ────────────────────────────────────────────────────────────

// Spawn fluid at a subpixel coordinate.
// amount = radius in cells around the spawn point.
void AddFluid(int sub_x, int sub_y, int type, int amount);

// Returns the cell type at a subpixel coordinate (FLUID_EMPTY if out of bounds).
int  GetFluidTypeAt(int sub_x, int sub_y);

// True if there is any non-empty fluid at this subpixel coordinate.
bool IsFluidAt(int sub_x, int sub_y);

// ─── Map mask (call after tile changes e.g. destruction) ─────────────────────
void RebuildFluidSolidMask();
void RebuildFluidSolidMaskRegion(int tx, int ty, int radius_tiles);

// ─── Light system bridge ──────────────────────────────────────────────────────
// Call after ClearLight() and before AddLight() each frame.
// Writes lava glow, steam scatter, gas attenuation into light_r/g/b arrays.
// Only active when FLUID_LIGHT_INTEGRATION is defined in your build.
#ifdef FLUID_LIGHT_INTEGRATION
void FluidContributeToLight(float *light_r, float *light_g, float *light_b,
                             int light_w, int light_h);
#endif