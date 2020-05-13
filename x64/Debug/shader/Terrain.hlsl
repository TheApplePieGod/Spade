#include "Shared.hlsl"

Texture2D DiffuseTex : register(t1);

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
	float4 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	float3 CameraPos : TEXCOORD1;
};

struct HSIn
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	uint   InstanceID : INSTANCEID;
};

struct DSIn
{
	float3 Position: POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	uint   InstanceID : INSTANCEID;
};

// Output patch constant data.
struct ConstantOutputType
{
	float Edges[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float Inside			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

#define NUM_CONTROL_POINTS 3

HSIn TerrainVS(VSIn input)
{
	HSIn output;

	output.Position = mul(float4(input.Position, 1.0f), Instances[0].WorldMatrix);
	output.TexCoord = input.TexCoord;
	output.Normal = input.Normal;
	output.InstanceID = input.InstanceID;

	return output;
}

float CalcTessFactor(float3 p, float3 eyePos)
{
	float d = distance(p, eyePos);

	// max norm in xz plane (useful to see detail levels from a bird's eye).
	//float d = max( abs(p.x- eyePos.x), abs(p.z- eyePos.z) );

	float gMinDist = 1.f;
	float gMaxDist = 50.f;
	float gMinTess = 1.f;
	float gMaxTess = 5.f;
	//float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));

	//return pow(2, (lerp(gMaxTess, gMinTess, s)));

	float TessellationScale = round(gMaxTess - ((d - gMinDist) / (gMaxDist - gMinDist)));
	return TessellationScale;
	//return  clamp(TessellationScale * gMaxTess - 1 + gMinTess, gMinTess, gMaxTess);
}

ConstantOutputType PatchConstantFunction(InputPatch<HSIn, NUM_CONTROL_POINTS> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType output;

	float3 m1 = (inputPatch[0].Position + inputPatch[1].Position) * 0.5f;
	float3 mid = (m1 + inputPatch[2].Position) * 0.5f;

	float tessellationAmount = 1.f;

	// Set the tessellation factors for the three edges of the triangle.
	output.Edges[0] = CalcTessFactor(0.5f * (inputPatch[0].Position + inputPatch[1].Position), CameraPosition);
	output.Edges[1] = CalcTessFactor(0.5f * (inputPatch[1].Position + inputPatch[2].Position), CameraPosition);
	output.Edges[2] = CalcTessFactor(0.5f * (inputPatch[2].Position + inputPatch[0].Position), CameraPosition);

	// Set the tessellation factor for tessallating inside the triangle.
	output.Inside = CalcTessFactor(mid, CameraPosition);

	return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("PatchConstantFunction")]
[maxtessfactor(64.0f)]
DSIn TerrainHullShader(InputPatch<HSIn, NUM_CONTROL_POINTS> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	DSIn output;

	// Set the position for this control point as the output position.
	output.Position = patch[pointId].Position;
	output.TexCoord = patch[pointId].TexCoord;
	output.Normal = patch[pointId].Normal;
	output.InstanceID = patch[pointId].InstanceID;

	return output;
}

[domain("tri")]
PSIn TerrainDomainShader(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DSIn, NUM_CONTROL_POINTS> patch)
{
	PSIn output;

	// Determine the position of the new vertex.
	float3 VertPos = uvwCoord.x * patch[0].Position + uvwCoord.y * patch[1].Position + uvwCoord.z * patch[2].Position;
	float2 TexCoord = uvwCoord.x * patch[0].TexCoord + uvwCoord.y * patch[1].TexCoord + uvwCoord.z * patch[2].TexCoord;
	float3 Normal = normalize(VertPos);//uvwCoord.x * patch[0].Normal + uvwCoord.y * patch[1].Normal + uvwCoord.z * patch[2].Normal;

	float NoiseVal = DiffuseTex.SampleLevel(Samp, TexCoord * 1000, 0).r;

	// Calculate the position of the new vertex against the world, view, and projection matrices.
	output.WorldPos = float4(VertPos, 1.f);// mul(float4(VertPos, 1.0f), Instances[patch[0].InstanceID].WorldMatrix);
    output.WorldPos += float4((NoiseVal * 5) * Normal, 0);
	output.Position = mul(output.WorldPos, CameraViewProjectionMatrix);
	output.TexCoord = TexCoord;
	output.CameraPos = CameraPosition;
	output.Normal = Normal;
	output.Tangent = float3(0.f, 0.f, 0.f);
	output.Bitangent = float3(0.f, 0.f, 0.f);

	return output;
}
