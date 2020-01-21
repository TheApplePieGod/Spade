#define MAX_INSTANCES 1024

sampler Samp : register(s0);
TextureCube EnvironmentMap : register(t0);

struct PerInstanceData
{
	matrix WorldMatrix;
	matrix InverseTransposeWorldMatrix;
};

// per instance
cbuffer ActorInstanceConstantBuffer : register(b1) // v1
{
	PerInstanceData Instances[2]; // ?
};

cbuffer FrameConstantBuffer : register(b0) // v0
{
	matrix CameraViewProjectionMatrix;

	matrix CameraWorldMatrix;

	float3 CameraPosition;
	float padding3;
};

cbuffer MaterialConstantBuffer : register(b0) // p0
{
	bool TextureDiffuse;
	float3 padding;

	float4 DiffuseColor;

	bool TextureNormal;
	float3 padding2;
};