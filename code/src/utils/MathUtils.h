#pragma once
#include "MathTypes.h"

inline f32 Length(v3 A)
{
	return sqrt((A.x * A.x) + (A.y * A.y) + (A.z * A.z));
}

inline v3 Normalize(v3 A)
{
	v3 Result;
	f32 length = Length(A);
	Result.x = A.x / length;
	Result.y = A.y / length;
	Result.z = A.z / length;
	return(Result);
}

inline v3 CrossProduct(v3 A, v3 B)
{
	v3 Result;
	Result.x = (A.y * B.z) - (A.z * B.y);
	Result.y = (A.z * B.x) - (A.x * B.z);
	Result.z = (A.x * B.y) - (A.y * B.x);
	return(Result);
}