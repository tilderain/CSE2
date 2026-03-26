#pragma once

#include "WindowsWrapper.h"

#define CONSOLE_MAX_TEXT 235
#define CONSOLE_MAX_HISTORY 256

struct DebugConsole
{
	bool visible;
	char text[CONSOLE_MAX_TEXT];

	char response[CONSOLE_MAX_TEXT];
	int response_timer;

	int text_pos;
	int blink_timer;

	int text_count;
	int text_history_pos;
	//Surely no one will use more than this
	char* text_history[CONSOLE_MAX_HISTORY];

	int debug_text_count;
	char* debug_texts[CONSOLE_MAX_HISTORY];
};

void PutConsole(void);
void InitConsole(void);
void ConsoleProc(int key);
void ClearConsole(void);
void ConsoleRespond(const char* fmt, ...);
void DebugPrint(const char* fmt, ...);

struct DebugVars
{
	bool bGodmode;
	bool bNoclip;
	bool bFastForward;
	int FastForwardTimer;

	int FrameAdvanceCount;
	bool bFrameFreeze;
	bool bFrameCanAdvance;

	bool bShowHitboxes;
};

extern DebugVars gDebug;

extern DebugConsole gConsole;