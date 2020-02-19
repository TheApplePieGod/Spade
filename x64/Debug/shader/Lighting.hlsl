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