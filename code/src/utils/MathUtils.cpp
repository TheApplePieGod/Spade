#include "pch.h"
#include "MathUtils.h"

v3& v3::operator=(const v4& A)
{
	x = A.x;
	y = A.y;
	z = A.z;
	return *this;
}