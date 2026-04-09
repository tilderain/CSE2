#include "Profile.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "WindowsWrapper.h"

#include "Backends/Misc.h"
#include "ArmsItem.h"
#include "BossLife.h"
#include "Fade.h"
#include "File.h"
#include "Flags.h"
#include "Frame.h"
#include "Game.h"
#include "Main.h"
#include "MiniMap.h"
#include "MyChar.h"
#include "NpChar.h"
#include "SelStage.h"
#include "Stage.h"
#include "Star.h"
#include "TextScr.h"
#include "ValueView.h"

const char* const gDefaultName = "Profile.dat";
const char* const gProfileCode = "Do041220";
const char* const gProfileCodeExtra = "CSE2E   20200304";

BOOL IsProfile(void)
{
	std::string path = gModulePath + '/' + gDefaultName;

	FILE *file = fopen(path.c_str(), "rb");
	if (file == NULL)
		return FALSE;

	fclose(file);
	return TRUE;
}

BOOL SaveProfile(const char *name)
{
	FILE *fp;
	PROFILE profile;
	const char *FLAG = "FLAG";

	std::string path;

	// Get path
	if (name != NULL)
		path = gModulePath + '/' + name;
	else
		path = gModulePath + '/' + gDefaultName;

	// Open file
	fp = fopen(path.c_str(), "wb");
	if (fp == NULL)
		return FALSE;

	// Set up profile
	memset(&profile, 0, sizeof(PROFILE));
	memcpy(profile.code, gProfileCode, sizeof(profile.code));
	memcpy(profile.FLAG, FLAG, sizeof(profile.FLAG));
	profile.stage = gStageNo;
	profile.music = gMusicNo;
	profile.x = gMC.x;
	profile.y = gMC.y;
	profile.direct = gMC.direct;
	profile.max_life = gMC.max_life;
	profile.life = gMC.life;
	profile.star = gMC.star;
	profile.select_arms = gSelectedArms;
	profile.select_item = gSelectedItem;
	profile.equip = gMC.equip;
	profile.unit = gMC.unit;
	profile.counter = gCounter;
	memcpy(profile.arms, gArmsData, sizeof(profile.arms));
	memcpy(profile.items, gItemData, sizeof(profile.items));
	memcpy(profile.permitstage, gPermitStage, sizeof(profile.permitstage));
	memcpy(profile.permit_mapping, gMapping, sizeof(profile.permit_mapping));
	memcpy(profile.flags, gFlagNPC, sizeof(profile.flags));
	// Custom
	memcpy(profile.extra_code, gProfileCodeExtra, sizeof(profile.extra_code));
	profile.MIMCurrentNum = gMIMCurrentNum;
	profile.physics_normal = gMC.physics_normal;
	profile.physics_underwater = gMC.physics_underwater;
	profile.no_splash_or_air_limit_underwater = gMC.no_splash_or_air_limit_underwater;

	// Write to file
	fwrite(profile.code, 8, 1, fp);
	File_WriteLE32(profile.stage, fp);
	File_WriteLE32(profile.music, fp);
	File_WriteLE32(profile.x, fp);
	File_WriteLE32(profile.y, fp);
	File_WriteLE32(profile.direct, fp);
	File_WriteLE16(profile.max_life, fp);
	File_WriteLE16(profile.star, fp);
	File_WriteLE16(profile.life, fp);
	File_WriteLE16(profile.a, fp);
	File_WriteLE32(profile.select_arms, fp);
	File_WriteLE32(profile.select_item, fp);
	File_WriteLE32(profile.equip, fp);
	File_WriteLE32(profile.unit, fp);
	File_WriteLE32(profile.counter, fp);
	for (int arm = 0; arm < 8; arm++)
	{
		File_WriteLE32(profile.arms[arm].code, fp);
		File_WriteLE32(profile.arms[arm].level, fp);
		File_WriteLE32(profile.arms[arm].exp, fp);
		File_WriteLE32(profile.arms[arm].max_num, fp);
		File_WriteLE32(profile.arms[arm].num, fp);
	}
	for (int item = 0; item < 32; item++)
		File_WriteLE32(profile.items[item].code, fp);
	for (int stage = 0; stage < 8; stage++)
	{
		File_WriteLE32(profile.permitstage[stage].index, fp);
		File_WriteLE32(profile.permitstage[stage].event, fp);
	}
	fwrite(profile.permit_mapping, 0x80, 1, fp);
	fwrite(FLAG, 4, 1, fp);
	fwrite(profile.flags, 1000, 1, fp);
	// Custom
	fwrite(gProfileCodeExtra, 0x10, 1, fp);
	File_WriteLE32(profile.MIMCurrentNum, fp);
	File_WriteLE32(profile.physics_normal.max_dash, fp);
	File_WriteLE32(profile.physics_normal.max_move, fp);
	File_WriteLE32(profile.physics_normal.gravity1, fp);
	File_WriteLE32(profile.physics_normal.gravity2, fp);
	File_WriteLE32(profile.physics_normal.dash1, fp);
	File_WriteLE32(profile.physics_normal.dash2, fp);
	File_WriteLE32(profile.physics_normal.resist, fp);
	File_WriteLE32(profile.physics_normal.jump, fp);
	File_WriteLE32(profile.physics_underwater.max_dash, fp);
	File_WriteLE32(profile.physics_underwater.max_move, fp);
	File_WriteLE32(profile.physics_underwater.gravity1, fp);
	File_WriteLE32(profile.physics_underwater.gravity2, fp);
	File_WriteLE32(profile.physics_underwater.dash1, fp);
	File_WriteLE32(profile.physics_underwater.dash2, fp);
	File_WriteLE32(profile.physics_underwater.resist, fp);
	File_WriteLE32(profile.physics_underwater.jump, fp);
	File_WriteLE32(profile.no_splash_or_air_limit_underwater, fp);

	fclose(fp);
	return TRUE;
}

// Modded LoadProfile (sub_41D255)
// Matches the "Cave Story Redesign" binary exactly.
BOOL LoadProfile(const char *name)
{
	int i;
	FILE *fp;
	unsigned char buffer[0x604]; // Exactly 1540 bytes
	
	// Headers and Magic Numbers
	const char *head = "Do041220";
	
	// New global variables for the mod
	extern unsigned char gGrappleState;   // 0x493804
	extern int gCurlyShoot_wait;         // 0x49E1E4

	// [MOD] Loading a game resets the Grapple/Jetpack state immediately
	gGrappleState = 0;

	PROFILE profile;
	std::string path;

	// Get path
	if (name != NULL)
		path = name;
	else
		path = gModulePath + '/' + gDefaultName;

	// Open file
	fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
		return FALSE;

	if (fp == NULL)
		return FALSE;

	// Check header code ("Do041220")
	unsigned char check_head[8];
	fread(check_head, 8, 1, fp);
	if (memcmp(check_head, head, 8) != 0)
	{
		fclose(fp);
		return FALSE;
	}

	// Read entire valid data block
	fseek(fp, 0, SEEK_SET);
	memset(buffer, 0, 0x604);
	if (fread(buffer, 0x604, 1, fp) != 1)
	{
		fclose(fp);
		return FALSE;
	}
	fclose(fp);

	// Map buffer to Global Variables
	// Offsets derived from the ASM's local stack assignments
	gSelectedArms = *(int*)(buffer + 0x24);
	gSelectedItem = *(int*)(buffer + 0x28);
	gCounter      = *(int*)(buffer + 0x34);

	memcpy(gArmsData,    buffer + 0x38,  0xA0);
	memcpy(gItemData,    buffer + 0xD8,  0x80);
	memcpy(gPermitStage, buffer + 0x158, 0x40);
	memcpy(gMapping,     buffer + 0x198, 0x80);
	memcpy(gFlagNPC,     buffer + 0x21C, 1000);

	// Load Stage and Music
	ChangeMusic((MusicID)*(int*)(buffer + 0x0C));
	InitMyChar();

	// TransferStage(stage, event, x, y)
	if (!TransferStage(*(int*)(buffer + 0x08), 0, 0, 1))
		return FALSE;

	// Set Character Properties
	gMC.equip    = *(int*)(buffer + 0x2C);
	gMC.unit     = *(int*)(buffer + 0x30);
	gMC.direct   = *(int*)(buffer + 0x18);
	gMC.max_life = *(short*)(buffer + 0x1C);
	gMC.life     = *(short*)(buffer + 0x20);
	gMC.star     = *(short*)(buffer + 0x1E);
	
	gMC.cond = 0x80;
	gMC.air = 1000;
	gMC.lifeBr = gMC.life;
	gMC.x = *(int*)(buffer + 0x10);
	gMC.y = *(int*)(buffer + 0x14);

	// [MOD] Custom Weapon Rect Logic
	// Redesign uses a 10-column layout for the Arms spritesheet
	// and 32px height increments.
	int arms_code = gArmsData[gSelectedArms].code;
	gMC.rect_arms.left   = (arms_code % 10) * 24;
	gMC.rect_arms.right  = gMC.rect_arms.left + 24;
	gMC.rect_arms.top    = (arms_code / 10) * 32;
	gMC.rect_arms.bottom = gMC.rect_arms.top + 16;

	// Post-Load Maintenance
	ClearFade();
	SetFrameMyChar();
	SetFrameTargetMyChar(16);
	InitBossLife();
	CutNoise();
	InitStar();
	ClearValueView();

	// [MOD] Custom reset for a specific boss/cutscene variable
	gCurlyShoot_wait = 0;

	return TRUE;
}

BOOL InitializeGame(void)
{
	InitMyChar();
	gSelectedArms = 0;
	gSelectedItem = 0;
	gCounter = 0;
	ClearArmsData();
	ClearItemData();
	ClearPermitStage();
	StartMapping();
	InitFlags();
	if (!TransferStage(102, 200, 41, 27))
	{
#ifdef JAPANESE
		Backend_ShowMessageBox("エラー", "ステージの読み込みに失敗");
#else
		Backend_ShowMessageBox("Error", "Failed to load stage");
#endif

		return FALSE;
	}

	ClearFade();
	SetFrameMyChar();
	SetFrameTargetMyChar(16);
	InitBossLife();
	CutNoise();
	ClearValueView();
	gCurlyShoot_wait = 0;
	SetFadeMask();
	SetFrameTargetMyChar(16);
	return TRUE;
}
