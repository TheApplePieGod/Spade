#define MAX_LIGHTS 10
#define MAX_INSTANCES 1024

struct PerInstanceData
{
	matrix WorldMatrix;
};

// per instance
cbuffer ActorInstanceConstantBuffer : register(b1)
{
	PerInstanceData Instances[2]; // ?
};

cbuffer FrameConstantBuffer : register(b0)
{
	matrix ViewProjectionMatrix;
};

cbuffer MaterialConstantBuffer : register(b0)
{
	bool TextureDiffuse;
	float3 padding;

	float4 DiffuseColor;

	bool TextureNormal;
	float3 padding2;
};