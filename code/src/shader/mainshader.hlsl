#define MAX_LIGHTS 8
#define MAX_POINTS 50

// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

sampler Samp : register(s0);
sampler ShadowClamp : register(s1);
Texture2D <float4> ModelTex : register(t0);// register more textures based on if the material is texture based or not
Texture2DArray <float4> DepthMapArray : register(t1);
Texture2DArray <float4> PointLightMaps : register(t2);// support for MAX_POINTS amount of point light shadow casters on the screen at once

struct Light
{
    float3      Position;               // 16 bytes
    float Intensity;
    //----------------------------------- (16 byte boundary)
    float3      Direction;              // 16 bytes
    int Padding2;
    //----------------------------------- (16 byte boundary)
    float4      Color;                  // 16 bytes
    //----------------------------------- (16 byte boundary)
    float       SpotAngle;              // 4 bytes
    float       ConstantAttenuation;    // 4 bytes
    float       LinearAttenuation;      // 4 bytes
    float       QuadraticAttenuation;   // 4 bytes
    //----------------------------------- (16 byte boundary)
    int         LightType;              // 4 bytes
    bool        Enabled;               // 4 bytes
    bool        CastShadow;
    int        Padding3;                // 12 bytes
    //----------------------------------- (16 byte boundary)
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};  

// per object
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix World;
    matrix ViewProj;
    //matrix test;
    float3 RenderLocation; // Location of any actors/components
    uint padding;
    float3 Scale;
    uint   HasTexture;
    float4 InColor;     // Solid Color
    //float padding;
    //uint   IsInstanced;
};

cbuffer MaterialBuffer : register(b0)
{
    // material properties
    //float3 Normal; // implement normals and texture normals
    float4 DiffuseColor;
    uint UsesTextureDiffuse;
    float SpecularPower;
    uint ShaderID;
};

cbuffer LightingBuffer : register(b1)
{
    float4 EyePosition; // general camera location for specular
    // lights properties
    Light Lights[MAX_LIGHTS];
    float4 GlobalAmbient;
    matrix CascadeMatrices[3];
    uint UsesDirectional;
    float3 Padding;
};

struct LightingResult
{
    float4 Diffuse;
    float4 Specular;
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
    float3 WorldPos : POSITION;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    //float4 LightPos :TEXCOORD2;
    uint   HasTexture : INCLUDETEX;
};

float4 CalcDiffuse( Light light, float3 LV, float3 NV )
{
    float NdotL = max( 0, dot( NV, LV ) );
    return light.Color * NdotL;
}

float4 CalcSpecular( Light light, float3 VV, float3 LV, float3 NV )
{
    // Phong lighting.
    //float3 R = normalize( reflect( -LV, NV ) );
    //float RdotV = max( 0, dot( R, VV ) );
 
    // Blinn-Phong lighting
    float3 H = normalize( LV + VV );
    float NdotH = max( 0, dot( NV, H ) );
 
    return light.Color * pow( NdotH, SpecularPower );
}

float CalcAttenutation( Light light, float d )
{
    return 1.0f / ( light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d );
}

LightingResult ComputeLighting( float4 P, float3 N, float4 ViewPos )
{
    //N = float3(-1, 0, 0);
    float3 V = normalize( EyePosition - P ).xyz;
 
    LightingResult totalResult = { {0, 0, 0, 0}, {0, 0, 0, 0} };
    float bias = 0.00001f; // shadow float precision bias
    float2 ShadowTexCoord;
    float DepthValue = 0;
    float LightDepthValue;
    int NumActiveLights = 0;

    [unroll]
    for( int i = 0; i < MAX_LIGHTS; i++ )
    {
        LightingResult result = { {0, 0, 0, 0}, {0, 0, 0, 0} };
 
        if ( !Lights[i].Enabled ) continue;
        
        NumActiveLights++;
        switch( Lights[i].LightType )
        {
        case DIRECTIONAL_LIGHT: // calculate directional light lighting / shadow
            {
                float3 L = -Lights[i].Direction.xyz;
                if (Lights[i].CastShadow) // 3 cascades max
                {
                    int Cascade = min((ViewPos.z / ViewPos.w) * 60, 2);
                    Cascade = 0;
                    float4 LightPos = mul(mul(P, Lights[i].ViewMatrix), CascadeMatrices[Cascade]); // asumes directional light is in the first slot
                    LightPos.xyz /= LightPos.w;

                    if( LightPos.x < -1.0f || LightPos.x > 1.0f || // if outside of depth texture
                        LightPos.y < -1.0f || LightPos.y > 1.0f ||
                        LightPos.z < 0.0f  || LightPos.z > 1.0f )
                    {}
                    else
                    {
                        ShadowTexCoord.x = LightPos.x / 2.0f + 0.5f;
                        ShadowTexCoord.y = LightPos.y / -2.0f + 0.5f;

                        DepthValue = DepthMapArray.Sample(ShadowClamp, float3(ShadowTexCoord, Cascade)).r;

                        // If pixel is shadowed
                        if (DepthValue < LightPos.z - bias)
                            break;
                    }
                }

                result.Diffuse = CalcDiffuse( Lights[i], L, N);
                result.Specular = CalcSpecular( Lights[i], V, L, N );
            }
            break;
        case POINT_LIGHT: // calculate point light lighting / shadow
            {
                float3 L = ( Lights[i].Position - P.xyz  );
                float distance = length(L);
                L = L / distance;

                float4 LightPos = mul(P, Lights[i].ViewMatrix);
                float attenuation = CalcAttenutation( Lights[i], distance );
            
                if (attenuation > 0)
                {
                    // if (Lights[i].CastShadow)
                    // {
                    //     float L = length(LightPos);
                    //     float3 P0 = (LightPos).xyz / L;

                    //     float alpha = .5f + LightPos.z / 100;
                    //     //generate texture coords for the front hemisphere
                    //     P0.z = P0.z + 1;
                    //     P0.x = P0.x / P0.z;
                    //     P0.y = P0.y / P0.z;
                    //     P0.z = L / 100;

                    //     P0.x = .5f * P0.x + .5f;
                    //     P0.y = -.5f * P0.y + .5f;

                    //     float3 P1 = (LightPos).xyz / L;
                    //     //generate texture coords for the rear hemisphere
                    //     P1.z = 1 - P1.z;
                    //     P1.x = P1.x / P1.z;
                    //     P1.y = P1.y / P1.z;
                    //     P1.z = L / 100;

                    //     P1.x = .5f * P1.x + .5f;
                    //     P1.y = -.5f * P1.y + .5f;

                    //     float depth;
                    //     float mydepth;
                    //     if(alpha >= 0.5f)
                    //     {		          
                    //         mydepth = P0.z;
                    //         depth = PointLightMaps.Sample(ShadowClamp, float3(P0.xy, 0)).r;
                    //     }
                    //     else
                    //     {
                    //         mydepth = P1.z;                  
                    //         depth = PointLightMaps.Sample(ShadowClamp, float3(P1.xy, 1)).r;
                    //     }
                    //     if (depth < mydepth - 0)
                    //         break;
                    // }
                    result.Diffuse = CalcDiffuse( Lights[i], L, N) * attenuation;
                    result.Specular = CalcSpecular( Lights[i], V, L, N ) * attenuation;
                }
            }
            break;
        // case POINT_LIGHT: 
        //     {
        //         result = DoPointLight( Lights[i], V, P, N );
        //     }
        //     break;
        // case SPOT_LIGHT:
        //     {
        //         result = DoSpotLight( Lights[i], V, P, N );
        //     }
        //     break;
        }
        totalResult.Diffuse += result.Diffuse * Lights[i].Intensity;
        totalResult.Specular += result.Specular;
    }
 
    if (NumActiveLights != 0) // if there are no lights in the scene, assume no lighting is wanted
    {
        totalResult.Diffuse = totalResult.Diffuse;
        totalResult.Specular = saturate(totalResult.Specular);
    }
    else
    {
        totalResult.Diffuse = float4(1.f, 1.f, 1.f, 1.f);
        totalResult.Specular = float4(0.f, 0.f, 0.f, 0.f);
    }

    return totalResult;
}

PSIn mainvs(VSIn input)
{
    PSIn output;

    output.Position = mul(mul(float4(input.Position, 1.0f), World), ViewProj);
    output.Color = InColor;
    output.TexCoord = input.TexCoord;
    output.HasTexture = HasTexture;
    output.Normal = input.Normal;//mul((float3x3)test, input.Normal);
    output.WorldPos = mul(float4(input.Position, 1.f), World); // pass pixel world position as opposed to rendering position for lighitng calculations

    return output;
}

float4 mainps(PSIn input) : SV_TARGET
{
    float4 FinalColor = (0.f, 0.f, 1.f, 1.f);

    float4 SampleColor;
    if (input.HasTexture == 1)
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

    LightingResult lit;
    float4 ambient = GlobalAmbient; // * Material.Ambient;

    lit = ComputeLighting( float4(input.WorldPos, 1), normalize(input.Normal), input.Position);

    //float4 emissive = Material.Emissive;
    float4 diffuse = DiffuseColor * lit.Diffuse; // diffuse tex
    float4 specular = float4(1.f, 1.f, 1.f, 1.f) * lit.Specular * SpecularPower * 0;

    FinalColor = ( ambient + diffuse + specular) * SampleColor;

    FinalColor.w = 1.f;
    return FinalColor;

    // int Cascade = min((input.Position.z / input.Position.w) * 60, 2);
    // if (Cascade == 0)
    //     return float4(1, 0, 0, 1);
    // else if (Cascade == 1)
    //     return float4(0, 1, 0, 1);
    // else
    //     return float4(0, 0, 1, 1);
}