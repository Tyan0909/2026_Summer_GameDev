#pragma once

#include <DxLib.h>

//トラップ関連
enum class TRAP_TYPE { SPIKE = 0, MINE = 1 };
struct Trap
{
	TRAP_TYPE type;
	VECTOR pos;
	bool triggered = false;
	int lifeFrames = 0; // 残存フレーム（スパイク持続等）
	int ownerPlayerIndex = 0; // owner index in players_ (optional)

	int modelId = -1;
};