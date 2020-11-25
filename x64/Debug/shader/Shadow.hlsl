#include "Shared.hlsl"

struct VSIn
{
    float3 Position : POSITION;
    float2 TexCoord   : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent: TANGENT;
    float3 Bitangent: BITANGENT;
    uint   InstanceID : SV_InstanceID;
};

struct PSIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

PSIn shadowvs(VSIn input)
{
    PSIn output;

    float4 WorldPos = mul(float4(input.Position, 1.f), Instances[input.InstanceID].WorldMatrix); // pass pixel world position as opposed to screen space position for lighitng calculations
    output.Position = mul(WorldPos, SunViewProjectionMatrix);
    output.TexCoord = input.TexCoord;

    return output;
}

//void shadowps(PSIn input)
//{}

float4 shadowps(PSIn input) : SV_TARGET
{
    //input.Position.z /= input.Position.w;
    //return float4(input.Position.z, input.Position.z, input.Position.z, 1.f);
    return float4(0.f, 1.f, 0.f, 1.f);
}
