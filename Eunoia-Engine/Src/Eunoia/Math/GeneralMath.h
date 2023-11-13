#pragma once

#include "../Common.h"
#include <cstdlib>

#define EU_PI 3.14159265359f
#define EU_2PI (EU_PI * 2.0f)
#define EU_HALF_PI (EU_PI / 2.0f)
#define EU_SQRT2 1.41421356237f

#define EU_FLOOR(A) ((int)(A))
#define EU_CEIL(A) (A) != EU_FLOOR(A) ? (EU_FLOOR(A) + 1.0f) : (A)
#define EU_ROUND(A) (A) >= EU_FLOOR(A) + 0.5 ? (EU_FLOOR(A) + 1.0f) : EU_FLOOR(A)

#define EU_ABS(A) ((A) >= 0) ? (A) : (-(A))

#define EU_MIN(A, B) (((A) < (B)) ? (A) : (B))
#define EU_MAX(A, B) (((A) > (B)) ? (A) : (B))
#define EU_CLAMP(Min, Max, Value) EU_MAX((Min), EU_MIN((Max), (Value)))

#define EU_LERP(A, B, T) ((A) * (1 - (T)) + (B) * (T))

#define EU_DEG_TO_RAD(deg) (((deg) * EU_PI) / 180.0f)
#define EU_RAD_TO_DEG(rad) (((rad) * 180.0f) / EU_PI)

#define EU_RANDOM_FLOAT(Min, Max) Eunoia::RandomFloat((Min), (Max))
#define EU_RANDOM_FLOAT_NORM EU_RANDOM_FLOAT(0.0f, 1.0f)
#define EU_RANDOM_INT(Min, Max) Eunoia::RandomInt((Min), (Max))

namespace Eunoia {

	inline static r32 RandomFloat(r32 min, r32 max)
	{
		return (r32)rand() / (r32)(RAND_MAX / (max - min)) + min;
	}

	inline static s32 RandomInt(s32 min, s32 max)
	{
		return 0;
	}

}