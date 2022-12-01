// Definitions
#define MAX_LIGHTS 1
static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

// Textures
Texture2D gShadowMap : register( t0 );
Texture2D gDiffuseMap : register( t1 );

// Sampler States
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

SamplerComparisonState samShadow
{
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    AddressW = BORDER;
    BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    ComparisonFunc = LESS_EQUAL;
};

// Structs
struct _Material
{
    float4 Emissive;
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float SpecularPower;
    bool UseTexture;
    float2 Padding;
};

struct Light
{
    float4 Position;
    float4 Direction;
    float4 Color;
    
    float SpotAngle;
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
    
    float Intensity;
    int LightType;
    bool Enabled;
    float Padding;
};

struct LightingResult
{
    float4 Diffuse;
    float4 Specular;
};

struct _Mapping
{
    bool UseNormalMap;
    bool UseParallaxMap;
    bool UseParallaxOcclusion;
    bool UseParallaxSelfShadowing;

    bool UseSoftShadow;
    float HeightScale;
    float2 Padding;
};

// Constant Buffers
cbuffer ConstantBuffer : register( b0 )
{
    matrix gWorld;
    matrix gWorldInvTranspose;
    matrix gWorldViewProj;
    matrix gTexTransform;
    matrix gShadowTransform;
}

cbuffer MaterialProperties : register( b1 )
{
	_Material Material;
};

cbuffer LightProperties : register( b2 )
{
    float4 CameraPosition;
    float4 GlobalAmbient;
    Light Lights[MAX_LIGHTS];
};

cbuffer MappingProperties : register( b3 )
{
    _Mapping Mapping;
}

// Shadow Function
float CalcShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    // Texel size.
    const float dx = SMAP_DX;
    float percentLit = 0.0f;
    
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(samShadow,
        shadowPosH.xy + offsets[i], depth).r;
    }
    return percentLit /= 9.0f;
}

// Lighting Functions
float4 DoDiffuse( Light light, float3 L, float3 N )
{
	float NdotL = max( 0.0f, dot( N, L ) );
	return light.Color * NdotL;
}

float4 DoSpecular( Light lightObject, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal )
{
	float3 lightDir = normalize( -lightDirectionToVertex );
	vertexToEye = normalize( vertexToEye );

	float lightIntensity = saturate( dot( Normal, lightDir ) );
	float4 specular = float4( 0.0f, 0.0f, 0.0f, 0.0f );
    
	if ( lightIntensity > 0.0f )
	{
		float3  reflection = normalize( 2.0f * lightIntensity * Normal - lightDir );
		specular = pow( saturate( dot( reflection, vertexToEye ) ), Material.SpecularPower );
	}

	return specular;
}

float DoAttenuation( Light light, float d )
{
	return 1.0f / ( light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d );
}

LightingResult DoPointLight( Light light, float3 vertexToEye, float4 vertexPos, float3 N )
{
	LightingResult result;

	float3 LightDirectionToVertex = ( vertexPos - light.Position ).xyz;
	float distance = length( LightDirectionToVertex );
	LightDirectionToVertex = LightDirectionToVertex  / distance;

	float3 vertexToLight = ( light.Position - vertexPos ).xyz;
	distance = length( vertexToLight );
	vertexToLight = vertexToLight / distance;

	float attenuation = DoAttenuation( light, distance );
	//attenuation = 1;

	result.Diffuse = DoDiffuse( light, vertexToLight, N ) * attenuation;
	result.Specular = DoSpecular( light, vertexToEye, LightDirectionToVertex, N ) * attenuation;

	return result;
}

LightingResult ComputeLighting( float4 vertexPos, float3 N, float3 vertexToEye )
{
	LightingResult totalResult = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

	[unroll]
	for ( int i = 0; i < MAX_LIGHTS; ++i )
	{
		LightingResult result = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

		if ( !Lights[i].Enabled ) 
			continue;
		
		result = DoPointLight( Lights[i], vertexToEye, vertexPos, N );
		
		totalResult.Diffuse += result.Diffuse;
		totalResult.Specular += result.Specular;
	}

	totalResult.Diffuse = saturate( totalResult.Diffuse );
	totalResult.Specular = saturate( totalResult.Specular );

	return totalResult;
}

struct PixelIn
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

float4 PS(PixelIn pin, uniform int gLightCount, uniform bool gUseTexure) : SV_Target
{
    // Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

    // The toEye vector is used in lighting.
    float3 toEye = (float3)CameraPosition - pin.PosW;

    // Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);

    // Normalize.
    toEye /= distToEye;

    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if (gUseTexure)
    {
        // Sample texture.
        texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
    }
    
    //
    // Lighting.
    //
    
    float4 litColor = texColor;
    if (gLightCount > 0)
    {
        // Only the first light casts a shadow.
        float3 shadow = float3(1.0f, 1.0f, 1.0f);
        shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

        float3 vertexToEye = normalize((float3)CameraPosition - pin.PosW).xyz;
        LightingResult lit = ComputeLighting( float4( pin.PosW, 1.0f ), pin.NormalW, vertexToEye );
        
        // texture/material
	    float4 emissive = Material.Emissive * Lights[0].Intensity;
	    float4 ambient = Material.Ambient * GlobalAmbient * Lights[0].Intensity;
        float4 diffuse = Material.Diffuse + shadow[0] * lit.Diffuse * Lights[0].Intensity;
	    float4 specular = Material.Specular + shadow[0] * lit.Specular * Lights[0].Intensity;
        
        float4 finalColor = texColor * ( emissive + ambient + diffuse ) + specular;
        return finalColor;
    }

    // Common to take alpha from diffuse material and texture.
    litColor.a = Material.Diffuse.a * texColor.a;
    return litColor;
}