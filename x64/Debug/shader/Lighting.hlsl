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

float4 SampleEnvironmentMap(float3 Normal, float3 ViewVector, float ReflectionAmount)
{
	float3 Reflection = reflect(-ViewVector, Normal);
	float4 RefColor = EnvironmentMap.Sample(Samp, Reflection);
	return ReflectionAmount * RefColor;
}