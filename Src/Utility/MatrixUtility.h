#pragma once
#include <vector>
#include <DxLib.h>

class MatrixUtility
{
	// Lerp üŒ`•âŠ®
public:
	static float Lerp(float start, float end, float t)
	{
		return start + (end - start) * t;
	}
};

