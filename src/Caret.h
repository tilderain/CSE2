#pragma once

#include "WindowsWrapper.h"

#define CARET_MAX 0x40
enum
{
	CARET_NULL = 0,
	CARET_BUBBLE = 1,
	CARET_PROJECTILE_DISSIPATION = 2,
	CARET_SHOOT = 3,
	CARET_SNAKE_AFTERIMAGE = 4,
	CARET_ZZZ = 5,
	CARET_SNAKE_AFTERIMAGE_DUPLICATE = 6,
	CARET_EXHAUST = 7,
	CARET_DROWNED_QUOTE = 8,
	CARET_QUESTION_MARK = 9,
	CARET_LEVEL_UP = 10,
	CARET_HURT_PARTICLES = 11,
	CARET_EXPLOSION = 12,
	CARET_TINY_PARTICLES = 13,
	CARET_UNKNOWN = 14,
	CARET_PROJECTILE_DISSIPATION_TINY = 15,
	CARET_EMPTY = 16,
	CARET_PUSH_JUMP_KEY = 17
};
struct CARET_TABLE
{
	int view_left;
	int view_top;
};

struct CARET
{
	int cond;
	int code;
	int direct;
	int x;
	int y;
	int xm;
	int ym;
	int act_no;
	int act_wait;
	int ani_no;
	int ani_wait;
	int view_left;
	int view_top;
	RECT rect;
};

extern CARET gCrt[CARET_MAX];
extern CARET_TABLE gCaretTable[18];

void InitCaret(void);
void ActCaret(void);
void PutCaret(int fx, int fy);
void SetCaret(int x, int y, int code, int dir);
