
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
    matrix World : WORLD;
    matrix ViewProj : VIEWPROJ;
};

PSIn quadvs(VSIn input)
{
    PSIn output;

    //output.Position = mul(mul(float4(input.Position, 1.f), World), ViewProj);
    output.Position = float4(input.Position, 1.f);
    output.Color = InColor;
    output.TexCoord = input.TexCoord;
    output.HasTexture = HasTexture;
    output.IsFont = IsFont;
    output.World = World;
    output.ViewProj = ViewProj;

    return output;
}

//input : top left vert then bottom right vert
[maxvertexcount(4)]
void quadgs(line PSIn input[2], inout TriangleStream<PSIn> OutputStream)
{
    float4 a = input[0].Position; // top left
    float4 b = input[1].Position; // bottom right
    float4 c = float4(b.x, a.y, a.z, a.w); // top right
    float4 d = float4(a.x, b.y, a.z, a.w); // bottom left

    PSIn OutputVert;
    OutputVert.HasTexture = input[0].HasTexture;
    OutputVert.IsFont = input[0].IsFont;
    OutputVert.World = input[0].World;
    OutputVert.ViewProj = input[0].ViewProj;
    
    // bottom left
    OutputVert.Position = mul(mul(d, input[0].World), input[0].ViewProj);
    OutputVert.Color = input[1].Color;
    OutputVert.TexCoord = float2(input[0].TexCoord.x, input[1].TexCoord.y);
    OutputStream.Append(OutputVert);

    // top left
    OutputVert.Position = mul(mul(a, input[0].World), input[0].ViewProj);
    OutputVert.Color = input[0].Color;
    OutputVert.TexCoord = input[0].TexCoord;
    OutputStream.Append(OutputVert);

    // bottom right
    OutputVert.Position = mul(mul(b, input[0].World), input[0].ViewProj);
    OutputVert.Color = input[1].Color;
    OutputVert.TexCoord = input[1].TexCoord;
    OutputStream.Append(OutputVert);

    // top right
    OutputVert.Position = mul(mul(c, input[0].World), input[0].ViewProj);
    OutputVert.Color = input[0].Color;
    OutputVert.TexCoord = float2(input[1].TexCoord.x, input[0].TexCoord.y);
    OutputStream.Append(OutputVert);
}

// float4 quadps(PSIn input) : SV_Target
// {
//     float4 SampleColor;
//     if (input.HasTexture)
//     {
//         SampleColor = FontAtlas.Sample(Samp, input.TexCoord);
//         if (SampleColor.w == 0)
//             discard;
//         if (input.IsFont)
//             SampleColor.xyz = 1;
//     }
//     else
//         SampleColor.xyzw = 1;

//     return SampleColor * input.Color;// * InColor;
// }