#include "Shared.hlsl"

struct VSIn
{
	float3 Position : POSITION;
	float2 TexCoord   : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent: TANGENT;
	uint   InstanceID : SV_InstanceID;
};

struct PSIn
{
	float4 Position : SV_POSITION;
	float3 LocalPosition : TEXCOORD;
};

PSIn skyboxvs(VSIn input)
{
	PSIn output;

	output.Position = mul(mul(float4(input.Position, 1.f), CameraWorldMatrix), CameraViewProjectionMatrix).xyww;
	output.LocalPosition = input.Position.xyz;

	return output;
}

float4 skyboxps(PSIn input) : SV_TARGET
{
	return EnvironmentMap.Sample(Samp, input.LocalPosition);
	//return float4(1.f,1.f,1.f,1.f);
}