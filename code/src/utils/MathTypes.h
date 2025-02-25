#pragma once
#include "Basic.h"
#include <math.h>

union v2
{
	struct
	{
		f32 x, y;
	};
	struct
	{
		f32 u, v;
	};
	f32 E[2];
};

union v4;
union v3
{
	struct
	{
		f32 x, y, z;
	};
	struct
	{
		f32 u, v, w;
	};
	struct
	{
		f32 r, g, b;
	};
	f32 E[3];

	v3& operator=(const v4& A);
};

struct directions
{
	static inline v3 Up = v3{ 0.f, 1.0f, 0.f };
	static inline v3 Down = v3{ 0.f, -1.0f, 0.f };
	static inline v3 Left = v3{ -1.f, 0.0f, 0.f };
	static inline v3 Right = v3{ 1.f, 0.0f, 0.f };
	static inline v3 Forward = v3{ 0.f, 0.0f, 1.f };
	static inline v3 Backward = v3{ 0.f, 0.0f, -1.f };
};

typedef v3 rotator;

// transform order: scale, rotate (degrees), translate
struct transform
{
	transform(v3 _Location, rotator _Rotation, v3 _Scale)
	{
		Location = _Location;
		Rotation = _Rotation;
		Scale = _Scale;
	}

	transform(v3 _Location)
	{
		Location = _Location;
	}

	transform() = default;

	v3 Location = { 0.f, 0.f, 0.f };
	rotator Rotation = { 0.f, 0.f, 0.f };
	v3 Scale = { 1.f, 1.f, 1.f };
};

union v4
{
	struct
	{
		union
		{
			v3 xyz;
			struct
			{
				f32 x, y, z;
			};
		};

		f32 w;
	};
	struct
	{
		union
		{
			v3 rgb;
			struct
			{
				f32 r, g, b;
			};
		};

		f32 a;
	};
	struct
	{
		v2 xy;
		f32 Ignored0_;
		f32 Ignored1_;
	};
	struct
	{
		f32 Ignored2_;
		v2 yz;
		f32 Ignored3_;
	};
	struct
	{
		f32 Ignored4_;
		f32 Ignored5_;
		v2 zw;
	};
	f32 E[4];
};

union matrix4x4
{
	struct
	{
		f32 m11;
		f32 m12;
		f32 m13;
		f32 m14;

		f32 m21;
		f32 m22;
		f32 m23;
		f32 m24;

		f32 m31;
		f32 m32;
		f32 m33;
		f32 m34;

		f32 m41;
		f32 m42;
		f32 m43;
		f32 m44;

	};
	struct
	{
		v4 row1;
		v4 row2;
		v4 row3;
		v4 row4;

	};

	inline v4 col1()
	{
		return v4{ m11, m21, m31, m41 };
	};

	inline v4 col2()
	{
		return v4{ m12, m22, m32, m42 };
	};

	inline v4 col3()
	{
		return v4{ m13, m23, m33, m43 };
	};

	inline v4 col4()
	{
		return v4{ m14, m24, m34, m44 };
	};
};

struct colors
{
	static inline v4 White = v4{ 1.f, 1.f, 1.f, 1.f };
	static inline v4 Gray = v4{ 0.5f, 0.5f, 0.5f, 1.f };
	static inline v4 Black = v4{ 0.f, 0.f, 0.f, 1.f };
	static inline v4 Translucent = v4{ 1.f, 1.f, 1.f, 0.4f };
	static inline v4 Red = v4{ 1.f, 0.f, 0.f, 1.f };
	static inline v4 Green = v4{ 0.f, 1.f, 0.f, 1.f };
	static inline v4 Blue = v4{ 0.f, 0.f, 1.f, 1.f };
	static inline v4 Yellow = v4{ 1.f, 1.f, 0.f, 1.f };
	static inline v4 Purple = v4{ 1.f, 0.f, 1.f, 1.f };
	static inline v4 Orange = v4{ 1.f, 0.4f, 0.f, 1.f };
};

// Standard struct used in rendering
union vertex
{
	vertex(f32 X, f32 Y, f32 Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	vertex(f32 X, f32 Y, f32 Z, f32 U, f32 V)
	{
		x = X;
		y = Y;
		z = Z;
		u = U;
		v = V;
	}

	vertex(f32 X, f32 Y, f32 Z, f32 U, f32 V, f32 NX, f32 NY, f32 NZ)
	{
		x = X;
		y = Y;
		z = Z;
		u = U;
		v = V;
		nx = NX;
		ny = NY;
		nz = NZ;
	}

	vertex() = default;

	struct
	{
		// Position in x/y plane
		f32 x, y, z;

		// UV coordinates
		f32 u, v;

		// Normals
		f32 nx, ny, nz;

		// Tangents
		f32 tx, ty, tz;

		// Bitangents
		f32 bx, by, bz;
	};

	struct
	{
		// Position in x/y plane
		v3 Position;

		// UV coordinates
		v2 UV;

		// Normals
		v3 Normal;

		// Tangents
		v3 Tangent;

		// Bitangents
		v3 Bitangent;
	};
};