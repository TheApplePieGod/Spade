cbuffer LightInputs : register(b0)
{
    matrix LightViewMatrix;
    float NearPlane;
    float FarPlane;
    float Direction;
    float padding;
    float3 RenderLocation;
    float padding2;
    float3 Scale;
    float padding3;
}

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
    float ClipDepth : TEXCOORD1;
    float Depth : TEXCOORD2;
};

PSIn mainvs(VSIn input)
{
    PSIn output;

    output.Position = mul(float4((input.Position * Scale) + RenderLocation, 1.f), LightViewMatrix);
    output.Position /= output.Position.w;
    //output.Position *= Direction; // front/back map
    
    // to unit vector
    float fLength = length(output.Position.xyz);
    output.Position /= fLength;

    // calc "normal" on intersection, by adding the 
	// reflection-vector(0,0,1) and divide through 
	// his z to get the texture coords
    output.Position.z += 1;
	output.Position.x /= output.Position.z;
	output.Position.y /= output.Position.z;

    // update z for depth buffer
    output.Position.z = (fLength - NearPlane) / (FarPlane - NearPlane);
    output.Position.w = 1.f;
    output.Depth = output.Position.z;
    output.ClipDepth = output.Position.z;

    return output;
}

float4 mainps(PSIn input) : SV_Target
{
    clip(input.ClipDepth);

    return input.Depth;
}