#include "Shared.hlsl"

cbuffer LightingConstantBuffer : register(b1) // p1 (switch to structured?)
{
	float3 AmbientColor;
	float SunIntensity;

	float3 SunColor;
	float padding4;

	float3 SunDirection;
	float padding5;
};

/* Calculate lighting based on light direction and diffuse color
* Light color is post attenuation/illumination
*/
float3 CalcDiffuseReflection(float3 Diffuse, float3 Normal, float3 LightVector, float3 LightColor)
{
	float3 DirectColor = LightColor * saturate(dot(Normal, LightVector));
	return ((AmbientColor + DirectColor) * Diffuse);
}

/* Calc specular
* Halfway : halfway between light vector and view vector
* NL : normal * light vector, prevents specular when light is behind surface
* Light color is post attenuation/illumination
* Higher specular power = less spread
*/
float3 CalcSpecularReflection(float3 Normal, float3 Halfway, float SpecularPower, float NL, float3 LightColor, float3 SpecularColor)
{
	float Highlight = pow(saturate(dot(Normal, Halfway)), SpecularPower) * float(NL > 0.0f);
	return (LightColor * Highlight * SpecularColor);
}

float4 SampleEnvironmentMap(float3 Normal, float3 ViewVector, float ReflectionAmount, float Mip)
{
	float3 Reflection = reflect(-ViewVector, Normal);
	float4 RefColor = EnvironmentMap.SampleLevel(Samp, Reflection, Mip);
	return ReflectionAmount * RefColor;
}

float3 FresnelSchlick(float3 F0, float HdotV)
{
	return F0 + (1 - F0) * pow(1 - HdotV, 5);
}

//// Single term for separable Schlick-GGX below.
float gaSchlickG1(float NdotL, float k)
{
	return NdotL / (NdotL * (1.0 - k) + k);
}

float3 gaSchlickGGX(float NdotL, float NdotV, float Roughness)
{
	float r = Roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(NdotL, k) * gaSchlickG1(NdotV, k);
}
//
float3 ndfGGX(float NdotH, float Roughness)
{
	float alpha = Roughness * Roughness;
	float alphaSq = alpha * alpha;

	float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (Pi * denom * denom);
}

float CalcShadowFactor(SamplerComparisonState samp, Texture2D tex, float4 shadowPos)
{
	shadowPos.xyz /= shadowPos.w;
	shadowPos.x = shadowPos.x * 0.5 + 0.5;
	shadowPos.y = -shadowPos.y * 0.5 + 0.5;

	float bias = 0.001f;
	float depth = shadowPos.z - bias;

	// Texel size.
	const float dx = 1.f / 2048.f;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		float filteredDepth = tex.SampleCmpLevelZero(samp, shadowPos.xy + offsets[i], depth).r;
		percentLit += filteredDepth;
	}

	return percentLit /= 9.0f;
}