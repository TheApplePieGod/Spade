#include "Lighting.hlsl"

Texture2D DiffuseTex : register(t1);
Texture2D NormalTex : register(t2);

struct VSIn
{
    float3 Position : POSITION;
    float2 TexCoord   : TEXCOORD;
    float3 Normal : NORMAL;
    uint   InstanceID : SV_InstanceID;
};

struct PSIn
{
    float4 Position : SV_POSITION;
    float4 WorldPos : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
	float3 CameraPos : TEXCOORD1;
};

PSIn mainvs(VSIn input)
{
    PSIn output;

	output.WorldPos = mul(float4(input.Position, 1.f), Instances[input.InstanceID].WorldMatrix); // pass pixel world position as opposed to screen space position for lighitng calculations
	output.Position = mul(output.WorldPos, CameraViewProjectionMatrix);
	output.TexCoord = input.TexCoord;
    output.Normal = normalize(mul(float4(input.Normal, 0.f), Instances[input.InstanceID].InverseTransposeWorldMatrix).xyz);
	//output.Normal = input.Normal;
	output.CameraPos = CameraPosition;

    return output;
}

float4 mainps(PSIn input) : SV_TARGET
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

	if (TextureDiffuse)
	{
		FinalColor = DiffuseTex.Sample(Samp, input.TexCoord);  // Sample the color from the texture
		FinalColor *= DiffuseColor;
	}
	else
		FinalColor = DiffuseColor;

	if (TextureNormal)
	{

	}

	float Reflect = Reflectivity;
	if (TextureReflective)
	{

	}

	float3 ViewVector = normalize(input.CameraPos - input.WorldPos.xyz);
	float SpecularPower = 50.f;
	float3 SpecularColor = float3(0.5f, 0.5f, 0.5f);
	float3 LightVector = -SunDirection;
	float3 Halfway = normalize(LightVector + ViewVector);
	FinalColor.xyz = (1 - Reflectivity) * CalcDiffuseReflection(FinalColor.xyz, input.Normal, LightVector, SunColor);
	FinalColor.xyz += CalcSpecularReflection(input.Normal, Halfway, SpecularPower, dot(input.Normal, LightVector), SunColor, SpecularColor);
	FinalColor += SampleEnvironmentMap(input.Normal, ViewVector, Reflect);

	if (FinalColor.w <= 0.01) // dont draw the pixel if the transparency is low enough
		discard;
    return FinalColor;
}