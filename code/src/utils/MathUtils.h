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

inline f32
DotProduct(v3 A, v3 B)
{
	f32 Result = A.x * B.x + A.y * B.y + A.z * B.z;

	return(Result);
}

inline f32
DotProduct(v4 A, v4 B)
{
	f32 Result = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;

	return(Result);
}

inline v3
operator+(v3 A, v3 B)
{
	v3 Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return(Result);
}

inline v3
operator-(v3 A, v3 B)
{
	v3 Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return(Result);
}

inline v3
operator*(f32 A, v3 B)
{
	v3 Result;

	Result.x = A * B.x;
	Result.y = A * B.y;
	Result.z = A * B.z;

	return(Result);
}

inline v3
operator*(v3 A, v3 B)
{
	v3 Result;

	Result.x = A.x * B.x;
	Result.y = A.y * B.y;
	Result.z = A.z * B.z;

	return(Result);
}

inline v3&
operator+=(v3& A, v3 B)
{
	A = A + B;
	return A;
}

inline transform
operator+(transform A, transform B)
{
	return { A.Location + B.Location, A.Rotation + B.Rotation, A.Scale + B.Scale };
}

inline matrix4x4
operator*(matrix4x4 A, matrix4x4 B)
{
	matrix4x4 Result;

	// Row 1
	Result.m11 = DotProduct(A.row1, B.col1());
	Result.m12 = DotProduct(A.row1, B.col2());
	Result.m13 = DotProduct(A.row1, B.col3());
	Result.m14 = DotProduct(A.row1, B.col4());

	// Row 2
	Result.m21 = DotProduct(A.row2, B.col1());
	Result.m22 = DotProduct(A.row2, B.col2());
	Result.m23 = DotProduct(A.row2, B.col3());
	Result.m24 = DotProduct(A.row2, B.col4());

	// Row 3
	Result.m31 = DotProduct(A.row3, B.col1());
	Result.m32 = DotProduct(A.row3, B.col2());
	Result.m33 = DotProduct(A.row3, B.col3());
	Result.m34 = DotProduct(A.row3, B.col4());

	// Row 4
	Result.m41 = DotProduct(A.row4, B.col1());
	Result.m42 = DotProduct(A.row4, B.col2());
	Result.m43 = DotProduct(A.row4, B.col3());
	Result.m44 = DotProduct(A.row4, B.col4());

	return Result;
}

inline matrix4x4
Inverse(const matrix4x4& A)
{

}