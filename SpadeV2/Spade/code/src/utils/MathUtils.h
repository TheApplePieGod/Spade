#pragma once
#include "Basic.h"

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
struct vertex
{
	// Position in x/y plane
	f32 x, y, z;

	// Color
	f32 r, g, b, a;

	// UV coordinates
	f32 u, v;

	// Normals
	f32 nx, ny, nz;
};

//const v4 colors::White = v4{1.f, 1.f, 1.f, 1.f};
//const v4 colors::Gray = v4{0.5f, 0.5f, 0.5f, 1.f};
//const v4 colors::Black = v4{0.f, 0.f, 0.f, 1.f};
//const v4 colors::Translucent = v4{1.f, 1.f, 1.f, 0.4f};
//const v4 colors::Red = v4{1.f, 0.f, 0.f, 1.f};
//const v4 colors::Green = v4{0.f, 1.f, 0.f, 1.f};
//const v4 colors::Blue = v4{0.f, 0.f, 1.f, 1.f};
//const v4 colors::Yellow = v4{1.f, 1.f, 0.f, 1.f};
//const v4 colors::Purple = v4{1.f, 0.f, 1.f, 1.f};
//const v4 colors::Orange = v4{1.f, 0.4f, 0.f, 1.f};