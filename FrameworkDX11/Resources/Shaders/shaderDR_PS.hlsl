// Definitions
#define MAX_LIGHTS 1

// Resources
Texture2D texturePosition : register( t0 );
Texture2D textureAlbedo : register( t1 );
Texture2D textureNormal : register( t2 );
SamplerState samplerState : register( s0 );

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

struct _Deferred
{
	bool UseDeferredShading;
	bool OnlyPositions;
	bool OnlyAlbedo;
	bool OnlyNormals;
};

// Constant Buffers
cbuffer MaterialProperties : register( b0 )
{
	_Material Material;
};

cbuffer LightProperties : register( b1 )
{
    float4 CameraPosition;
    float4 GlobalAmbient;
    Light Lights[MAX_LIGHTS];
};

cbuffer MappingProperties : register( b2 )
{
    _Mapping Mapping;
}

cbuffer DeferredProperties : register( b3 )
{
    _Deferred Deferred;
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

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    int3 sampleIndices = int3( input.Position.xy, 0 );
    float4 position = texturePosition.Load( sampleIndices );
    if ( Deferred.OnlyPositions )
        return position;
        
    float3 normal = textureNormal.Load( sampleIndices ).rgb;
    if ( !Mapping.UseNormalMap )
        normal = input.Normal;
    if ( Deferred.OnlyNormals )
        return float4( normal, 1.0f );
        
    float4 albedo = textureAlbedo.Load( sampleIndices );
    if ( !Material.UseTexture )
        albedo = float4( 1.0f, 1.0f, 1.0f, 1.0f );
    if ( Deferred.OnlyAlbedo )
        return albedo;

    // lighting
    float3 vertexToLight = normalize( Lights[0].Position - position ).xyz;
    LightingResult lit = ComputeLighting( position, normal, vertexToLight );

	// texture/material
	float4 emissive = Material.Emissive * Lights[0].Intensity;
	float4 ambient = Material.Ambient * GlobalAmbient * Lights[0].Intensity;
    float4 diffuse = Material.Diffuse * lit.Diffuse * Lights[0].Intensity;
	float4 specular = Material.Specular * lit.Specular * Lights[0].Intensity;
	
    // final colour
	float4 finalColor = ( emissive + ambient + diffuse + specular ) * albedo;
	return finalColor;
}