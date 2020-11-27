#include "Lighting.hlsl"

Texture2D ShadowMap : register(t4);

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

void shadowps(PSIn input)
{}

//float4 shadowps(PSIn input) : SV_TARGET
//{
//    //input.Position.z /= input.Position.w;
//    //return float4(input.Position.z, input.Position.z, input.Position.z, 1.f);
//    return float4(0.f, 1.f, 0.f, 1.f);
//}

PSIn variancevs(VSIn input)
{
    PSIn output;

    output.Position = float4(input.Position, 1.f);
    output.TexCoord = input.TexCoord;

    return output;
}

float4 varianceps(PSIn input) : SV_TARGET
{
    // guassian blur
    float w = 1.f / 2048.f; // texmap size
    float h = 1.f / 2048.f;

    int width = 21;
    int center = width / 2;

    float depth = 0.f;
    for (int x = -center; x <= center; x++)
    {
        for (int y = -center; y <= center; y++)
        {
            float sampleDepth = ShadowMap.Sample(Samp, saturate(input.TexCoord + float2(x * w, y * h)));
            int offset = (y + center) * width + (x + center);
            float g = kernel[offset];
            depth += sampleDepth * g;
        }
    }

    //float depth = ShadowMap.Sample(Samp, input.TexCoord);
    float2 moments;
    moments.x = depth;

    float dx = ddx(depth);
    float dy = ddy(depth);
    moments.y = depth * depth + 0.25 * (dx * dx + dy * dy);

    return float4(moments.xy, 0.f, 1.f);
}