#pragma once
#include "MathTypes.h"

inline f32 Length(v3 A)
{
	return sqrt((A.x * A.x) + (A.y * A.y) + (A.z * A.z));
}

inline f32 Length(v2 A)
{
	return sqrt((A.x * A.x) + (A.y * A.y));
}

inline f32 Lerp(f32 Min, f32 Max, f32 Alpha)
{
	return Min + (Max - Min) * Alpha;
}

inline f32 InverseLerp(f32 Min, f32 Max, f32 Value)
{
	if (abs(Max - Min) < 0.00001) return Min;
	return (Value - Min) / (Max - Min);
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

inline v2 Normalize(v2 A)
{
	v2 Result;
	f32 length = Length(A);
	Result.x = A.x / length;
	Result.y = A.y / length;
	return(Result);
}

inline vertex Normalize(const vertex& A)
{
	vertex Result = A;
	v3 V3A = v3{ A.x, A.y, A.z };
	f32 length = Length(V3A);
	Result.x = V3A.x / length;
	Result.y = V3A.y / length;
	Result.z = V3A.z / length;
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
operator*(v3 A, f32 B)
{
	v3 Result;

	Result.x = A.x * B;
	Result.y = A.y * B;
	Result.z = A.z * B;

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

inline v2
operator/(v2 A, f32 B)
{
	v2 Result;

	Result.x = A.x / B;
	Result.y = A.y / B;

	return(Result);
}

inline v2
operator-(v2 A, v2 B)
{
	v2 Result;

	Result.x = A.x - B.x;
	Result.y = A.y - B.y;

	return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
	v2 Result;

	Result.x = A.x + B.x;
	Result.y = A.y + B.y;

	return(Result);
}

inline vertex
Midpoint(const vertex& A, const vertex& B)
{
	vertex Result;
	Result.x = (A.x + B.x) * 0.5f;
	Result.y = (A.y + B.y) * 0.5f;
	Result.z = (A.z + B.z) * 0.5f;
	Result.u = (A.u + B.u) * 0.5f;
	Result.v = (A.v + B.v) * 0.5f;
	Result.nx = (A.nx + B.nx) * 0.5f;
	Result.ny = (A.ny + B.ny) * 0.5f;
	Result.nz = (A.nz + B.nz) * 0.5f;
	return Result;
}

inline v3
Midpoint(const v3& A, const v3& B, const v3& C)
{
	v3 Mid = 0.5f * (A + B);
	Mid = 0.5f * (Mid + C);
	return Mid;
}

inline v3
Midpoint(const vertex& A, const vertex& B, const vertex& C)
{
	v3 Point1 = v3{ A.x, A.y, A.z };
	v3 Point2 = v3{ B.x, B.y, B.z };
	v3 Point3 = v3{ C.x, C.y, C.z };
	return Midpoint(Point1, Point2, Point3);
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

inline v4
operator*(matrix4x4 A, v4 B)
{
	v4 Result;

	Result.x = DotProduct(A.row1, B);
	Result.y = DotProduct(A.row2, B);
	Result.z = DotProduct(A.row3, B);
	Result.w = DotProduct(A.row4, B);

	return Result;


}

inline v3 Transform(v3 Point, matrix4x4 Matrix)
{
	v3 result;
	v4 temp = { Point.x, Point.y, Point.z, 1.f };
	v4 temp2;

	temp2.x = temp.x * Matrix.m11 + temp.y * Matrix.m21 + temp.z * Matrix.m31 + temp.w * Matrix.m41;
	temp2.y = temp.x * Matrix.m12 + temp.y * Matrix.m22 + temp.z * Matrix.m32 + temp.w * Matrix.m42;
	temp2.z = temp.x * Matrix.m13 + temp.y * Matrix.m23 + temp.z * Matrix.m33 + temp.w * Matrix.m43;
	temp2.w = temp.x * Matrix.m14 + temp.y * Matrix.m24 + temp.z * Matrix.m34 + temp.w * Matrix.m44;

	result.x = temp2.x / temp2.w;	//view projection matrices make use of the W component
	result.y = temp2.y / temp2.w;
	result.z = temp2.z / temp2.w;

	return result;
}

inline v3 TransformTransposed(v3 Point, matrix4x4 Matrix)
{
	v3 result;
	v4 temp = { Point.x, Point.y, Point.z, 1.f };
	v4 temp2;

	temp2.x = temp.x * Matrix.m11 + temp.y * Matrix.m12 + temp.z * Matrix.m13 + temp.w * Matrix.m14;
	temp2.y = temp.x * Matrix.m21 + temp.y * Matrix.m22 + temp.z * Matrix.m23 + temp.w * Matrix.m24;
	temp2.z = temp.x * Matrix.m31 + temp.y * Matrix.m32 + temp.z * Matrix.m33 + temp.w * Matrix.m34;
	temp2.w = temp.x * Matrix.m41 + temp.y * Matrix.m42 + temp.z * Matrix.m43 + temp.w * Matrix.m44;

	result.x = temp2.x / temp2.w;	//view projection matrices make use of the W component
	result.y = temp2.y / temp2.w;
	result.z = temp2.z / temp2.w;

	return result;
}

//inline matrix4x4
//Inverse(const matrix4x4& A)
//{
//
//}