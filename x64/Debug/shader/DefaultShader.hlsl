#include "Shared.hlsl"

sampler Samp : register(s1);
Texture2D <float4> ModelTex : register(t0);// register more textures based on if the material is texture based or not

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
    float4 WorldPos : POSITION;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};

//input : top left vert then bottom right vert
[maxvertexcount(4)]
void maings(line PSIn input[2], inout TriangleStream<PSIn> OutputStream)
{
    float4 a = input[0].Position; // top left;
    float4 b = input[1].Position; // bottom right
    float4 c = float4(b.x, a.y, a.z, a.w); // top right;
    float4 d = float4(a.x, b.y, a.z, a.w); // bottom left

    float2 aTex = input[0].TexCoord;
    float2 cTex = float2(input[1].TexCoord.x, input[0].TexCoord.y);

    PSIn OutputVert;
    OutputVert.Normal = input[0].Normal;

    // bottom left
    OutputVert.WorldPos = mul(d, World);
    OutputVert.Position = mul(OutputVert.WorldPos, ViewProj);
    OutputVert.Color = input[1].Color;
    OutputVert.TexCoord = float2(input[0].TexCoord.x, input[1].TexCoord.y);
    OutputStream.Append(OutputVert);

    // top left
    OutputVert.WorldPos = mul(a, World);
    OutputVert.Position = mul(OutputVert.WorldPos, ViewProj);
    OutputVert.Color = input[0].Color;
    OutputVert.TexCoord = aTex;
    OutputStream.Append(OutputVert);

    // bottom right
    OutputVert.WorldPos = mul(b, World);
    OutputVert.Position = mul(OutputVert.WorldPos, ViewProj);
    OutputVert.Color = input[1].Color;
    OutputVert.TexCoord = input[1].TexCoord;
    OutputStream.Append(OutputVert);

    // top right
    OutputVert.WorldPos = mul(c, World);
    OutputVert.Position = mul(OutputVert.WorldPos, ViewProj);
    OutputVert.Color = input[0].Color;
    OutputVert.TexCoord = cTex;
    OutputStream.Append(OutputVert);
}

PSIn mainvs(VSIn input)
{
    PSIn output;

    output.Position = mul(mul(float4(input.Position, 1.0f), World), ViewProj);
    output.Color = InColor;
    output.TexCoord = (input.TexCoord * TexcoordModifier.xy) + TexcoordModifier.zw;
    output.Normal = input.Normal;//mul((float3x3)test, input.Normal);
    output.WorldPos = mul(float4(input.Position, 1.f), World); // pass pixel world position as opposed to rendering position for lighitng calculations

    return output;
}

float4 mainps(PSIn input) : SV_TARGET
{
    float4 FinalColor = (0.f, 0.f, 0.f, 1.f);

    float4 SampleColor;
    if (HasTexture == 1)
    {
        SampleColor = ModelTex.Sample(Samp, input.TexCoord);  // Sample the color from the texture
        if (SampleColor.w <= 0.01) // dont draw the pixel if the transparency is low enough
        {
            discard;
        }
    }
    else
    {
        SampleColor = input.Color; 
    }

    FinalColor = SampleColor * (CalcLighting(input.WorldPos) + AmbientLighting);

    return FinalColor;
}

PSIn geovs(VSIn input) // vertex shader to use w the geo shader
{
    PSIn output;

    output.Position = float4(input.Position, 1.0f);
    output.WorldPos = float4(input.Position, 1.f);
    output.Color = InColor;
    output.TexCoord = (input.TexCoord * TexcoordModifier.xy) + TexcoordModifier.zw;
    output.Normal = input.Normal;

    return output;
}

float4 circleps(PSIn input) : SV_TARGET // ps to render circles
{
    // normal.x & y are used as the center position and z is the radius
    float4 FinalColor = mainps(input);
    if (distance(input.WorldPos.xy, input.Normal.xy) <= input.Normal.z)
        return FinalColor;
    else
        discard;

    return float4(0, 0, 0, 0);
}
