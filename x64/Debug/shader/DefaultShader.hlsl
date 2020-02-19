#include "Lighting.hlsl"

Texture2D DiffuseTex : register(t1);
Texture2D NormalTex : register(t2);

struct VSIn
{
    float3 Position : POSITION;
    float2 TexCoord   : TEXCOORD;
    float3 Normal : NORMAL;
	float3 Tangent: TANGENT;
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

float3 FetchNormalVector(float2 TexCoord)
{
	float2 m = NormalTex.Sample(Samp, TexCoord).xy;
	return (float3(m, sqrt(1.0 - m.x * m.x - m.y * m.y)));
}

PSIn mainvs(VSIn input)
{
    PSIn output;

	output.WorldPos = mul(float4(input.Position, 1.f), Instances[input.InstanceID].WorldMatrix); // pass pixel world position as opposed to screen space position for lighitng calculations
	output.Position = mul(output.WorldPos, CameraViewProjectionMatrix);
	output.TexCoord = input.TexCoord;
	output.CameraPos = CameraPosition;

	output.Normal = normalize(mul(float4(input.Normal, 0.f), Instances[input.InstanceID].InverseTransposeWorldMatrix).xyz);
	output.Tangent = normalize(mul(float4(input.Tangent, 0.f), Instances[input.InstanceID].InverseTransposeWorldMatrix).xyz);
	output.Tangent = normalize(output.Tangent - dot(output.Tangent, output.Normal) * output.Normal);
	output.Bitangent = normalize(mul(float4(cross(output.Normal, output.Tangent), 0.f), Instances[input.InstanceID].InverseTransposeWorldMatrix).xyz);

    return output;
}

float4 mainps(PSIn input) : SV_TARGET
{
    float3 FinalColor = float3(0.f, 0.f, 0.f);
	float4 SampleColor = float4(0.f, 0.f, 0.f, 1.f);

	if (TextureDiffuse)
	{
		SampleColor = DiffuseTex.Sample(Samp, input.TexCoord);  // Sample the color from the texture
		SampleColor *= DiffuseColor;
	}
	else
		SampleColor = DiffuseColor;

	if (SampleColor.a <= 0.01) // dont draw the pixel if the transparency is low enough
		discard;

	float3 Normal = input.Normal;
	if (false)
	{
		float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);
		float3 NormalMap = (2.0 * NormalTex.Sample(Samp, input.TexCoord).xyz) - 1.0;
		Normal = normalize(mul(NormalMap, TBN));
	}

	float Reflect = Reflectivity;
	if (TextureReflective)
	{
		// sample from null bind?
	}

	// Constant normal incidence Fresnel factor for all dielectrics.
	const float3 Fdielectric = 0.04;
	const float Epsilon = 0.00001;

	float3 ViewVector = normalize(input.CameraPos - input.WorldPos.xyz);
	float3 LightVector = -SunDirection;
	float3 Halfway = normalize(LightVector + ViewVector);

	float NdotL = max(0.0, dot(Normal, LightVector));
	float NdotV = max(0.0, dot(Normal, ViewVector));
	float NdotH = max(0.0, dot(Normal, Halfway));

	float SpecularPower = 750.f;
	float3 SpecularColor = float3(1.f, 1.f, 1.f);
	float Roughness = 1.0;
	float Metalness = 0.9;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	float3 F0 = lerp(Fdielectric, SampleColor.rgb, Metalness);
	float3 F = FresnelSchlick(F0, max(0.0, dot(Halfway, ViewVector)));
	float3 D = ndfGGX(NdotH, Roughness);
	float3 G = gaSchlickGGX(NdotL, NdotV, Roughness);

	// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
	// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
	// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
	float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), Metalness);

	// Lambert diffuse BRDF.
	// We don't scale by 1/PI for lighting & material units to be more convenient.
	// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
	float3 diffuseBRDF = kd * SampleColor.rgb;

	// Cook-Torrance specular microfacet BRDF.
	float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * NdotL * NdotV);

	// Total contribution for this light.
	//FinalColor += (diffuseBRDF + specularBRDF) * 50 * NdotL;

	FinalColor.rgb = CalcDiffuseReflection(SampleColor.rgb, Normal, LightVector, SunColor);
	FinalColor.rgb += CalcSpecularReflection(Normal, Halfway, SpecularPower, NdotL, SunColor, SpecularColor);
	//FinalColor += SampleEnvironmentMap(Normal, ViewVector, Metalness, 0);

	float3 AmbientLighting;
	{
		float3 Irradiance = SampleEnvironmentMap(Normal, ViewVector, 1, Roughness * 9).rgb;

		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use NdotV instead of angle with light's half-vector (NdotH above).
		float3 F = FresnelSchlick(F0, NdotV);

		// Get diffuse contribution factor (as with direct lighting).
		float3 kd = lerp(1.0 - F, 0.0, Metalness);

		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		float3 diffuseIBL = kd * SampleColor.rgb * Irradiance;

		AmbientLighting = diffuseIBL;
	}

	//FinalColor += AmbientLighting;

    return float4(FinalColor, SampleColor.a);
}