#include "Lighting.hlsl"
#include "AtmosphericScattering.hlsl"

Texture2D DiffuseTex : register(t2);
Texture2D NormalTex : register(t3);
Texture2D ShadowMap : register(t4);

static const float TexcoordScalar = 5000;

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
	float4 LightPos : POSITION1; // sun space
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
	/*float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;*/
	float3x3 TBN : TBN;
	float3 TerrainInfo : TERRAININFO;
	float3 CameraPos : TEXCOORD1;
};

float3 FetchNormalVector(float2 TexCoord)
{
	float3 Color = NormalTex.Sample(Samp, TexCoord * TexcoordScalar);
	Color *= 2.f;
	return float3(Color.x - 1.f, Color.y - 1.f, Color.z - 1.f);
}

PSIn mainvs(VSIn input)
{
    PSIn output;

	output.WorldPos = mul(float4(input.Position, 1.f), Instances[input.InstanceID].WorldMatrix); // pass pixel world position as opposed to screen space position for lighitng calculations
	output.Position = mul(output.WorldPos, CameraViewProjectionMatrix);
	output.TexCoord = input.TexCoord;
	output.CameraPos = CameraPosition;

	output.Normal = normalize(mul(float4(input.Normal, 0.0), Instances[input.InstanceID].WorldMatrix).xyz);//normalize(mul(input.Normal, (float3x3)Instances[input.InstanceID].InverseTransposeWorldMatrix));
	output.LightPos = mul(output.WorldPos + float4(output.Normal.xyz * 0.4f, 0), SunViewProjectionMatrix);
	float3 Tangent = normalize(mul(float4(input.Tangent, 0.0), Instances[input.InstanceID].WorldMatrix).xyz);//normalize(mul(input.Tangent, (float3x3)Instances[input.InstanceID].InverseTransposeWorldMatrix));
	//output.Bitangent = //normalize(mul(float4(cross(input.Normal, input.Tangent), 0.0), Instances[input.InstanceID].WorldMatrix).xyz);//normalize(mul(input.Bitangent, (float3x3)Instances[input.InstanceID].WorldMatrix));
	//output.Tangent = normalize(output.Tangent - dot(output.Tangent, output.Normal) * output.Normal);
	float3 Bitangent = normalize(cross(output.Normal, Tangent));
	//float3 Bitangent = normalize(mul(cross(output.Normal, Tangent), (float3x3)Instances[input.InstanceID].WorldMatrix));
	//if (dot(cross(output.Normal, Tangent), Bitangent) < 0.0f)
	//	Tangent *= -1.f;
	output.TerrainInfo = input.Bitangent;

	output.TBN = transpose(float3x3(Tangent, Bitangent, output.Normal));

    return output;
}

float4 GroundFromAtmospherePS(PSIn input) : SV_TARGET
{
	float fCameraHeight = max(length(input.CameraPos), fInnerRadius);

	// Get the ray from the camera to the vertex and its length (which is the far point of the ray passing through the atmosphere)
	float3 v3Pos = input.WorldPos.xyz;
	float3 v3Ray = v3Pos - input.CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	//if (fCameraHeight < length(v3Pos)) { // ?
	//	float3 t = v3Pos;
	//	v3Pos = input.CameraPos;
	//	input.CameraPos = t;
	//	v3Ray = -v3Ray;
	//}

	v3Pos = normalize(v3Pos);

	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = input.CameraPos;
	float fDepth = exp((fInnerRadius - fCameraHeight) * (1.0 / fScaleDepth));
	float fCameraAngle = 1.f;// dot(-v3Ray, v3Pos);
	//if (fCameraAngle == 0)
	//	fCameraAngle = 0.1;
	float fLightAngle = dot(-SunDirection, v3Pos);
	float fCameraScale = scale(fCameraAngle, fScaleDepth);
	float fLightScale = scale(fLightAngle, fScaleDepth);
	float fCameraOffset = fDepth * fCameraScale;
	float fTemp = (fLightScale + fCameraScale);

	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
	// Now loop through the sample rays
	float3 v3FrontColor = float3(0.0, 0.0, 0.0);
	float3 v3Attenuate;
	for (int i = 0; i < nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fScatter = fDepth * fTemp - fCameraOffset;
		v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
	float3 c0 = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);
	float3 c1 = v3Attenuate;

	float3 LightVector = -SunDirection;
	float3 TexNormal = FetchNormalVector(input.TexCoord);
	float3 WorldNormal = normalize(mul(input.TBN, TexNormal));
	float nDotL2 = max(0.0, dot(input.Normal, LightVector));
	float nDotL = max(0.0, dot(WorldNormal, LightVector)) * nDotL2;
	float4 SampleColor = float4(1.f, 1.f, 1.f, 1.f);
	//if (TextureDiffuse)
	//{
	//	SampleColor = DiffuseTex.Sample(Samp, input.TexCoord * 1000);  // Sample the color from the texture
	//	SampleColor *= DiffuseColor;
	//}
	//else
	//	SampleColor = DiffuseColor;

	float4 FirstTerrainColor = LandscapeTextures.Sample(Samp, float3(input.TexCoord * TexcoordScalar, round(input.TerrainInfo.x)));
	SampleColor.xyz = FirstTerrainColor.xyz;
	if (input.TerrainInfo.x != input.TerrainInfo.y)
	{
		float4 SecondTerrainColor = LandscapeTextures.Sample(Samp, float3(input.TexCoord * TexcoordScalar, round(input.TerrainInfo.y)));
		SampleColor.xyz = lerp(FirstTerrainColor.xyz, SecondTerrainColor.xyz, 1.0 - input.TerrainInfo.z);
	}

	SampleColor = LandscapeTextures.Sample(Samp, float3(input.TexCoord * 5000, input.TerrainInfo.x));
	float angle = acos(dot(normalize(input.WorldPos.xyz), input.Normal));
	if (abs(angle) > 3.14159 * 0.2)
		SampleColor = float4(0.4f, 0.26f, 0.13f, 1.f);

	float3 Ambient = SampleColor * AmbientColor;

	// Shadowing

	float bias = 0.001f;
	float2 shadowTexCoord;
	shadowTexCoord.x = input.LightPos.x / input.LightPos.w / 2.f + 0.5f;
	shadowTexCoord.y = -input.LightPos.y / input.LightPos.w / 2.f + 0.5f;

	float lightDepthValue = (input.LightPos.z / input.LightPos.w) - bias;

	if (saturate(shadowTexCoord.x) != shadowTexCoord.x || saturate(shadowTexCoord.y) != shadowTexCoord.y || lightDepthValue <= 0)
		return float4(0.f, 0.f, 0.f, 1.f);

	//float2 shadowDepth = ShadowMap.SampleCmpLevelZero(ClampSamp, shadowTexCoord, lightDepthValue);
	float2 shadowDepth = ShadowMap.Sample(Samp, shadowTexCoord).rg;

	//float shadowCoeff = CalcShadowFactor(ClampSamp, ShadowMap, input.LightPos);
	float shadowCoeff = 0.f;

	if (lightDepthValue <= shadowDepth.r)
		shadowCoeff = 1.f;
	else
	{
		float variance = (shadowDepth.g) - (shadowDepth.r * shadowDepth.r);
		variance = min(1.f, max(0.f, variance + 0.00001f));
		float mean = shadowDepth.r;
		float d = lightDepthValue - mean;
		shadowCoeff = variance / (variance + d * d);
	}

	return float4(Ambient + (shadowCoeff * SampleColor * nDotL), 1.f);

	//if (lightDepthValue > shadowDepth)
	//	return float4(Ambient, 1.f);

	//otherwise continue with lighting calculation

	//------------

	SampleColor *= nDotL;

	return float4(SampleColor.xyz + Ambient.xyz, 1.f);

	//float3 color = c0 + SampleColor.xyz * c1;
	//color += Ambient;
	//return float4(color, 1);
}

float4 SkyFromAtmospherePS(PSIn input) : SV_TARGET
{
	float fCameraHeight = length(input.CameraPos);

	// Get the ray from the camera to the vertex and its length (which is the far point of the ray passing through the atmosphere)
	float3 v3Pos = input.WorldPos.xyz;
	float3 v3Ray = v3Pos - input.CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = input.CameraPos;
	float fHeight = length(v3Start);
	float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	float fStartOffset = fDepth * scale(fStartAngle, fScaleDepth);
	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
	// Now loop through the sample rays
	float3 v3FrontColor = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fLightAngle = dot(-SunDirection, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth * (scale(fLightAngle, fScaleDepth) - scale(fCameraAngle, fScaleDepth)));
		float3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
	float3 c0 = v3FrontColor * (v3InvWavelength * fKrESun * 1);
	float3 c1 = v3FrontColor * fKmESun * 1;
	float3 v3Direction = input.CameraPos - v3Pos;
	float fCos = dot(-SunDirection, v3Direction) / length(v3Direction);
	float fCos2 = fCos * fCos;
	float3 color = getRayleighPhase(fCos2) * c0 + getMiePhase(fCos, fCos2) * c1;
	color = pow(color, 1.0 / 2.2);//1.0 - exp(color * -fHdrExposure);
	float4 AtmoColor = float4(color, color.b);
	//return float4(0,0,0,0);
	return AtmoColor;
}

float4 GroundFromSpacePS(PSIn input) : SV_TARGET
{
	float fCameraHeight = length(input.CameraPos);
	float fCameraHeight2 = fCameraHeight * fCameraHeight;

	// Get the ray from the camera to the vertex and its length (which is the far point of the ray passing through the atmosphere)
	float3 v3Pos = input.WorldPos.xyz;
	float3 v3Ray = v3Pos - input.CameraPos;
	v3Pos = normalize(v3Pos);
	float fFar = length(v3Ray);
	v3Ray /= fFar;
	// Calculate the closest intersection of the ray with the outer atmosphere (which is the near point of the ray passing through the atmosphere)
	float fNear = getNearIntersection(input.CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = input.CameraPos + v3Ray * fNear;
	fFar -= fNear;
	float fDepth = exp((fInnerRadius - fOuterRadius) * fInvScaleDepth);
	float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
	float fLightAngle = dot(-SunDirection, v3Pos);
	float fCameraScale = scale(fCameraAngle, fScaleDepth);
	float fLightScale = scale(fLightAngle, fScaleDepth);
	float fCameraOffset = fDepth * fCameraScale;
	float fTemp = (fLightScale + fCameraScale);
	// Initialize the scattering loop variables
	//gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0);
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
	// Now loop through the sample rays
	float3 v3FrontColor = float3(0.0, 0.0, 0.0);
	float3 v3Attenuate;
	for (int i = 0; i < nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fScatter = fDepth * fTemp - fCameraOffset;
		v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
	float3 c0 = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);
	float3 c1 = v3Attenuate;
	
	float3 LightVector = -SunDirection;
	float3 TexNormal = FetchNormalVector(input.TexCoord);
	float3 WorldNormal = normalize(mul(input.TBN, TexNormal));
	float nDotL2 = max(0.0, dot(input.Normal, LightVector));
	float nDotL = max(0.0, dot(WorldNormal, LightVector)) * nDotL2;
	float4 SampleColor = float4(1.f, 1.f, 1.f, 1.f);
	//float angle = acos(dot(normalize(input.WorldPos.xyz), input.Normal) / (length(normalize(input.WorldPos.xyz)) * length(input.Normal)));
	//if (abs(angle) > 30)
	//	SampleColor = float4(1.f, 1.f, 1.f, 1.f);// * nDotL;
	//if (TextureDiffuse)
	//{
	//	SampleColor = DiffuseTex.Sample(Samp, input.TexCoord * 1000);  // Sample the color from the texture
	//	SampleColor *= DiffuseColor;
	//}
	//else
	//	SampleColor = DiffuseColor;
	//SampleColor.xyz = input.TerrainInfo;

	float4 FirstTerrainColor = LandscapeTextures.Sample(Samp, float3(input.TexCoord * TexcoordScalar, round(input.TerrainInfo.x)));
	SampleColor.xyz = FirstTerrainColor.xyz;
	if (input.TerrainInfo.x != input.TerrainInfo.y)
	{
		float4 SecondTerrainColor = LandscapeTextures.Sample(Samp, float3(input.TexCoord * TexcoordScalar, round(input.TerrainInfo.y)));
		SampleColor.xyz = lerp(FirstTerrainColor.xyz, SecondTerrainColor.xyz, 1.0 - input.TerrainInfo.z);
	}
	SampleColor = LandscapeTextures.Sample(Samp, float3(input.TexCoord * 5000, input.TerrainInfo.x));

	float3 Ambient = SampleColor * AmbientColor;

	// Shadowing

	float bias = 0.001f;
	float2 shadowTexCoord;
	shadowTexCoord.x = input.LightPos.x / input.LightPos.w / 2.f + 0.5f;
	shadowTexCoord.y = -input.LightPos.y / input.LightPos.w / 2.f + 0.5f;

	float lightDepthValue = (input.LightPos.z / input.LightPos.w) - bias;

	if (saturate(shadowTexCoord.x) != shadowTexCoord.x || saturate(shadowTexCoord.y) != shadowTexCoord.y || lightDepthValue <= 0)
		return float4(0.f, 0.f, 0.f, 1.f);

	//float shadowDepth = ShadowMap.Sample(ClampSamp, shadowTexCoord).r;
	float shadowDepth = ShadowMap.SampleCmpLevelZero(ClampSamp, shadowTexCoord, lightDepthValue).r;

	if (lightDepthValue > shadowDepth)
		return float4(0.f, 0.f, 1.f, 1.f);

	//otherwise continue with lighting calculation

	//------------

	SampleColor *= nDotL;
	return float4(SampleColor.xyz + Ambient.xyz, 1.f);

	//SampleColor = float4(0.2f,0.6f,1.f,1.f);
	//float3 color = c0 + SampleColor.xyz * c1;
	//color += Ambient;
	//return float4(color, 1);
	//float3 color = c0 + 0.25 * c1;
	//color = 1.0 - exp(color * -fHdrExposure);
	//SampleColor *= color.b;
	//return float4(SampleColor + color, 1);
}

float4 SkyFromSpacePS(PSIn input) : SV_TARGET
{
	float fCameraHeight = length(input.CameraPos);
	float fCameraHeight2 = fCameraHeight * fCameraHeight;

	// Get the ray from the camera to the vertex and its length (which is the far point of the ray passing through the atmosphere)
	float3 v3Pos = input.WorldPos.xyz;
	float3 v3Ray = v3Pos - input.CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;
	// Calculate the closest intersection of the ray with the outer atmosphere (which is the near point of the ray passing through the atmosphere)
	float fNear = getNearIntersection(input.CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);
	// Calculate the ray's start and end positions in the atmosphere, then calculate its scattering offset
	float3 v3Start = input.CameraPos + v3Ray * fNear;
	fFar -= fNear;
	float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
	float fStartDepth = exp(-fInvScaleDepth);
	float fStartOffset = fStartDepth * scale(fStartAngle, fScaleDepth);
	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
	// Now loop through the sample rays
	float3 v3FrontColor = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fLightAngle = dot(-SunDirection, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth * (scale(fLightAngle, fScaleDepth) - scale(fCameraAngle, fScaleDepth)));
		float3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
	float3 c0 = v3FrontColor * (v3InvWavelength * fKrESun);
	float3 c1 = v3FrontColor * fKmESun;
	float3 v3Direction = input.CameraPos - v3Pos;
	float fCos = dot(-SunDirection, v3Direction) / length(v3Direction);
	float fCos2 = fCos * fCos;
	float3 color = getRayleighPhase(fCos2) * c0 + getMiePhase(fCos, fCos2) * c1;
	color = 1.0 - exp(color * -fHdrExposure);
	float4 AtmoColor = float4(color, color.b);
	return AtmoColor;
}

float4 mainps(PSIn input) : SV_TARGET
{
	float4 SampleColor = float4(0.f, 0.f, 0.f, 1.f);
	if (TextureDiffuse)
	{
		SampleColor = DiffuseTex.Sample(Samp, input.TexCoord * 1000);  // Sample the color from the texture
		SampleColor *= DiffuseColor;
	}
	else
		SampleColor = DiffuseColor;

	float3 LightVector = -SunDirection;
	float nDotL = max(0.0, dot(input.Normal, LightVector));

	SampleColor *= nDotL;
	SampleColor.w = 1;
	return SampleColor;
}

//float4 mainps(PSIn input) : SV_TARGET
//{
//    float3 FinalColor = float3(0.f, 0.f, 0.f);
//	float4 SampleColor = float4(0.f, 0.f, 0.f, 1.f);
//
//	if (TextureDiffuse)
//	{
//		SampleColor = DiffuseTex.Sample(Samp, input.TexCoord);  // Sample the color from the texture
//		SampleColor *= DiffuseColor;
//	}
//	else
//		SampleColor = DiffuseColor;
//
//	if (SampleColor.a <= 0.01) // dont draw the pixel if the transparency is low enough
//		discard;
//
//	float3 Normal = input.Normal;
//	if (true)
//	{
//		float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);
//		float3 NormalMap = normalize((2.0 * NormalTex.Sample(Samp, input.TexCoord).xyz) - 1.0);
//		Normal = mul(NormalMap, TBN);
//	}
//
//	float Reflect = Reflectivity;
//	if (TextureReflective)
//	{
//		// sample from null bind?
//	}
//
//	// Constant normal incidence Fresnel factor for all dielectrics.
//	const float3 Fdielectric = 0.04;
//	const float Epsilon = 0.00001;
//
//	float3 LightVector = -SunDirection;
//	float3 ViewVector = normalize(input.CameraPos - input.WorldPos.xyz);
//	
//	float3 Halfway = normalize(LightVector + ViewVector);
//
//	float NdotL = max(0.0, dot(Normal, LightVector));
//	float NdotV = max(0.0, dot(Normal, ViewVector));
//	float NdotH = max(0.0, dot(Normal, Halfway));
//
//	float SpecularPower = 750.f;
//	float3 SpecularColor = float3(1.f, 1.f, 1.f);
//	float Roughness = 1.0;
//	float Metalness = 0.9;
//
//	// Fresnel reflectance at normal incidence (for metals use albedo color).
//	float3 F0 = lerp(Fdielectric, SampleColor.rgb, Metalness);
//	float3 F = FresnelSchlick(F0, max(0.0, dot(Halfway, ViewVector)));
//	float3 D = ndfGGX(NdotH, Roughness);
//	float3 G = gaSchlickGGX(NdotL, NdotV, Roughness);
//
//	// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
//	// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
//	// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
//	float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), Metalness);
//
//	// Lambert diffuse BRDF.
//	// We don't scale by 1/PI for lighting & material units to be more convenient.
//	// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
//	float3 diffuseBRDF = kd * SampleColor.rgb;
//
//	// Cook-Torrance specular microfacet BRDF.
//	float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * NdotL * NdotV);
//
//	// Total contribution for this light.
//	//FinalColor += (diffuseBRDF + specularBRDF) * 50 * NdotL;
//
//	FinalColor.rgb = CalcDiffuseReflection(SampleColor.rgb, Normal, LightVector, SunColor);
//	FinalColor.rgb += CalcSpecularReflection(Normal, Halfway, SpecularPower, NdotL, SunColor, SpecularColor);
//	//FinalColor += SampleEnvironmentMap(Normal, ViewVector, Metalness, 0);
//
//	float3 AmbientLighting;
//	{
//		float3 Irradiance = SampleEnvironmentMap(Normal, ViewVector, 1, Roughness * 9).rgb;
//
//		// Calculate Fresnel term for ambient lighting.
//		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
//		// use NdotV instead of angle with light's half-vector (NdotH above).
//		float3 F = FresnelSchlick(F0, NdotV);
//
//		// Get diffuse contribution factor (as with direct lighting).
//		float3 kd = lerp(1.0 - F, 0.0, Metalness);
//
//		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
//		float3 diffuseIBL = kd * SampleColor.rgb * Irradiance;
//
//		AmbientLighting = diffuseIBL;
//	}
//
//	//FinalColor += AmbientLighting;
//
//	//return float4(input.Tangent, 1);
//
//    return float4(FinalColor, SampleColor.a);
//}