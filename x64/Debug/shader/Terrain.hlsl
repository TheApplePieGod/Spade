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

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;
	output.Normal = input.Normal;
	output.InstanceID = input.InstanceID;

	return output;
}

ConstantOutputType PatchConstantFunction(InputPatch<HSIn, NUM_CONTROL_POINTS> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType output;

	float tessellationAmount = 1;

	// Set the tessellation factors for the three edges of the triangle.
	output.Edges[0] = tessellationAmount;
	output.Edges[1] = tessellationAmount;
	output.Edges[2] = tessellationAmount;

	// Set the tessellation factor for tessallating inside the triangle.
	output.Inside = tessellationAmount;

	return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(NUM_CONTROL_POINTS)]
[patchconstantfunc("PatchConstantFunction")]
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

	float NoiseVal = DiffuseTex.SampleLevel(Samp, TexCoord * 200, 0).r;

	// Calculate the position of the new vertex against the world, view, and projection matrices.
	output.WorldPos = mul(float4(VertPos, 1.0f), Instances[patch[0].InstanceID].WorldMatrix);
	output.WorldPos += float4((NoiseVal * 10) * Normal, 0);
	output.Position = mul(output.WorldPos, CameraViewProjectionMatrix);
	output.TexCoord = TexCoord;
	output.CameraPos = CameraPosition;
	output.Normal = Normal;
	output.Tangent = float3(0.f, 0.f, 0.f);
	output.Bitangent = float3(0.f, 0.f, 0.f);

	return output;
}
