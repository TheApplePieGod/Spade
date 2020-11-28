#define MAX_INSTANCES 1024
#define NUM_CASCADES 4
#define SHADOWMAP_SIZE 2048.f
static const float CascadeDepths[NUM_CASCADES + 1] = { 0.001f, 3.f, 7.f, 15.f, 30.f };

sampler Samp : register(s0);
SamplerComparisonState ClampSamp : register(s1);
sampler PointSamp : register(s2);
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

	matrix CameraViewMatrix;

	float4x4 SunViewProjectionMatrix[NUM_CASCADES];

	float3 CameraPosition;
	uint CurrentCascade;
};

cbuffer MaterialConstantBuffer : register(b0) // p0
{
	uint TextureDiffuse;
	uint TextureNormal;
	uint TextureReflective;
	float Reflectivity;

	float4 DiffuseColor;
};