#include "Shared.hlsl"

sampler Samp : register(s0);

Texture2D DiffuseTex : register(t0);
Texture2D NormalTex : register(t1);

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
};

PSIn mainvs(VSIn input)
{
    PSIn output;

	output.WorldPos = mul(float4(input.Position, 1.f), Instances[input.InstanceID].WorldMatrix); // pass pixel world position as opposed to screen space position for lighitng calculations
	output.Position = mul(output.WorldPos, ViewProjectionMatrix);
	output.TexCoord = input.TexCoord;
    output.Normal = input.Normal;

    return output;
}

float4 mainps(PSIn input) : SV_TARGET
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

	if (TextureDiffuse)
	{
		FinalColor = DiffuseTex.Sample(Samp, input.TexCoord);  // Sample the color from the texture
		if (FinalColor.w <= 0.01) // dont draw the pixel if the transparency is low enough
			discard;
		FinalColor *= DiffuseColor;
	}
	else
		FinalColor = DiffuseColor;

	if (TextureNormal)
	{

	}

    return FinalColor;
}