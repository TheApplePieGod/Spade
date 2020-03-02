
// TODO: Move to buffer
// The number of sample points taken along the ray
static const int nSamples = 4;
static const float fSamples = 4.0f;
// Mie phase assymetry factor
static const float g = -0.95f;
static const float g2 = 0.9025f;

static const float Km = 0.0015f;
static const float Kr = 0.0025f;
static const float ESun = 10.f;
static const float3 v3InvWavelength = float3(
	1.0f / pow(0.650f, 4),
	1.0f / pow(0.570f, 4),
	1.0f / pow(0.475f, 4));
static const float fInnerRadius = 1000.f;
static const float fInnerRadius2 = fInnerRadius * fInnerRadius;
static const float fOuterRadius = fInnerRadius * 1.025;
static const float fOuterRadius2 = fOuterRadius * fOuterRadius;
static const float fKrESun = Kr * ESun;
static const float fKmESun = Km * ESun;
static const float fKr4PI = Kr * 4.0f * Pi;
static const float fKm4PI = Km * 4.0f * Pi;
static const float fScaleDepth = 0.23f;
static const float fInvScaleDepth = 1.0f / fScaleDepth;
static const float fScale = 1.0f / (fOuterRadius - fInnerRadius);
static const float fScaleOverScaleDepth = fScale / fScaleDepth;
//static const float3 v3LightPos = normalize(float3(0.5f, 0.5f, -0.5f));
static const float fHdrExposure = 0.8f;

// The scale equation calculated by Vernier's Graphical Analysis
float scale(float fCos, float fScaleDepth)
{
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}
// Calculates the Mie phase function
float getMiePhase(float fCos, float fCos2)
{
	return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(abs(1.0 + g2 - 2.0 * g * fCos), 1.5);
}
// Calculates the Rayleigh phase function
float getRayleighPhase(float fCos2)
{
	//return 0.75 * (1.0 + fCos2);
	return 0.75 + 0.75 * fCos2;
}
// Returns the near intersection point of a line and a sphere
float getNearIntersection(float3 v3Pos, float3 v3Ray, float fDistance2, float fRadius2)
{
	float B = 2.0 * dot(v3Pos, v3Ray);
	float C = fDistance2 - fRadius2;
	float fDet = max(0.0, B * B - 4.0 * C);
	return 0.5 * (-B - sqrt(fDet));
}
// Returns the far intersection point of a line and a sphere
float getFarIntersection(float3 v3Pos, float3 v3Ray, float fDistance2, float fRadius2)
{
	float B = 2.0 * dot(v3Pos, v3Ray);
	float C = fDistance2 - fRadius2;
	float fDet = max(0.0, B * B - 4.0 * C);
	return 0.5 * (-B + sqrt(fDet));
}