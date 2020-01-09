#define MAX_LIGHTS 10

// per object
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProj;

    float3 RenderLocation; // Location of any actors/components
    uint IsFont;

    float3 Scale;
    uint   HasTexture;

    float4 InColor;     // Solid Color
    
    float4 TexcoordModifier;

    float4 AmbientLighting;

    float SkyEndY;
    uint DisableLighting;
    float2 padding;
};

struct Light
{
    float3      Position;               // 16 bytes
    float       Intensity;
    //----------------------------------- (16 byte boundary)
    float4      Color;                  // 16 bytes
    //----------------------------------- (16 byte boundary)
    float       ConstantAttenuation;    // 4 bytes
    float       LinearAttenuation;      // 4 bytes
    float       QuadraticAttenuation;   // 4 bytes
    bool        Enabled; 
}; 

cbuffer LightingBuffer : register(b1)
{
    Light Lights[MAX_LIGHTS];
};

float CalcAttenutation( Light light, float d )
{
    return 1.0f / ( light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d );
}

float4 CalcLighting(float4 P)
{
    float4 FinalResult = float4(0, 0, 0, 1);

    [unroll]
    for( int i = 0; i < MAX_LIGHTS; i++ )
    {
       if ( !Lights[i].Enabled ) continue;

        float2 L = ( Lights[i].Position.xy - P.xy  );
        float dist = distance(Lights[i].Position.xy, P.xy);
        float radius = Lights[i].ConstantAttenuation;

        float attenuation = clamp(1.0 - dist*dist/(radius*radius), 0.0, 1.0);//CalcAttenutation( Lights[i],  );
        attenuation *= attenuation;

        FinalResult += Lights[i].Color * attenuation * Lights[i].Intensity;
    }

    FinalResult.w = 1;
    return min(FinalResult, float4(1, 1, 1, 1));
    //return FinalResult;
}