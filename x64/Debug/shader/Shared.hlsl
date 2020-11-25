#define MAX_INSTANCES 1024

sampler Samp : register(s0);
TextureCube EnvironmentMap : register(t0);
Texture2DArray LandscapeTextures : register(t1);

static const float Pi = 3.1415926535;

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

	matrix SunViewProjectionMatrix;

	float3 CameraPosition;
	float padding3;
};

cbuffer MaterialConstantBuffer : register(b0) // p0
{
	uint TextureDiffuse;
	uint TextureNormal;
	uint TextureReflective;
	float Reflectivity;

	float4 DiffuseColor;
};