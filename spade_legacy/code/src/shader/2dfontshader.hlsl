sampler Samp : register(s0);
Texture2D <float4> FontAtlas : register(t0);

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProj;
    //matrix test;
    float3 RenderLocation; // Location of any actors/components
    uint IsFont;
    float3 Scale;
    uint   HasTexture;
    float4 InColor;     // Solid Color
    //float padding;
    //uint   IsInstanced;
};
struct VSIn
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 TexCoord   : TEXCOORD;
    float3 Normal : NORMAL;
    uint   InstanceID : SV_InstanceID;
};

struct PSIn
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord   : TEXCOORD;
    uint HasTexture : TEXCOORD1;
    uint IsFont : TEXCOORD2;
};

PSIn mainvs(VSIn input)
{
    PSIn output;

    output.Position = mul(mul(float4(input.Position, 1.f), World), ViewProj);
    //output.Position = float4(input.Position, 1.f);
    output.Color = InColor;
    output.TexCoord = input.TexCoord;
    output.HasTexture = HasTexture;
    output.IsFont = IsFont;

    return output;
}

float4 mainps(PSIn input) : SV_Target
{
    float4 SampleColor;
    if (input.HasTexture)
    {
        SampleColor = FontAtlas.Sample(Samp, input.TexCoord);
        if (SampleColor.w == 0)
            discard;
        if (input.IsFont)
            SampleColor.xyz = 1;
    }
    else
        SampleColor.xyzw = 1;

    return SampleColor * input.Color;// * InColor;
}