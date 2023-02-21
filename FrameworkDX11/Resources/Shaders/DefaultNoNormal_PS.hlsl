// Definitions
#define PCF_RANGE 2
#define MAX_LIGHTS 2
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// Resources
Texture2D txDiffuse : register( t0 );
Texture2D txNormal: register( t1 );
Texture2D txDepth[2] : register( t3 );
SamplerState smpLinear : register( s0 );
SamplerComparisonState smpClamp : register( s1 );
SamplerState smpBorder : register( s2 );

// Structs
struct Light
{
	float4 Position;
	float4 Direction;
	float4 Color;

	float SpotAngle;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;

	int LightType;
	bool Enabled;
	int2 Padding;

	matrix mView;
	matrix mProjection;
};

struct _Material
{
	float4 Emissive;
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float SpecularPower;
	bool UseTexture;
	float2 Padding;

	float HeightScale;
	float MaxLayers;
	float MinLayers;
	float Padding1;
};

struct LightingResult
{
    float4 Diffuse;
    float4 Specular;
};

// Constant Buffers
cbuffer MatrixBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
};

cbuffer MaterialProperties : register( b1 )
{
	_Material Material;
};

cbuffer LightProperties : register( b2 )
{
	float4 EyePosition;
	float4 GlobalAmbient;
	Light Lights[MAX_LIGHTS];
};

// Lighting Functions
float4 DoDiffuse( Light light, float3 L, float3 N )
{
	float NdotL = max( 0, dot( N, L ) );
	return light.Color * NdotL;
}

float4 DoSpecular( Light lightObject, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal )
{
	vertexToEye = normalize( vertexToEye );
	float4 lightDir = float4( normalize( -lightDirectionToVertex ), 1 );
	float lightIntensity = saturate( dot( Normal, (float3)lightDir ) );
	float4 specular = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	
	if ( lightIntensity > 0.0f )
	{
		float3  reflection = normalize( 2 * lightIntensity * Normal - (float3)lightDir );
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

	float3 lightDirectionToVertex = ( vertexPos - light.Position ).xyz;
    float distance = length( lightDirectionToVertex );
    lightDirectionToVertex = lightDirectionToVertex / distance;

	float3 vertexToLight = ( light.Position - vertexPos ).xyz;
	distance = length( vertexToLight );
	vertexToLight = vertexToLight / distance;

	float attenuation = DoAttenuation( light, distance );
	result.Diffuse = DoDiffuse( light, vertexToLight, N ) * attenuation;
    result.Specular = DoSpecular( light, vertexToEye, lightDirectionToVertex, N ) * attenuation;
	
	return result;
}

LightingResult DoDirectionalLight( Light light, float3 V, float4 P, float3 N )
{
	LightingResult result;
	float3 L = -light.Direction.xyz;
	result.Diffuse = DoDiffuse( light, L, N );
	result.Specular = DoSpecular( light, V, L, N );
	return result;
}

float DoSpotCone( Light light, float3 L )
{
	float minCos = cos( light.SpotAngle );
	float maxCos = ( minCos + 1.0f ) / 2.0f;
	float cosAngle = dot( light.Direction.xyz, -L );
	return smoothstep( minCos, maxCos, cosAngle );
}

LightingResult DoSpotLight( Light light, float3 vertexToEye, float4 vertexPos, float3 N )
{
	LightingResult result;
	float3 lightDirectionToVertex = ( vertexPos - light.Position ).xyz;
    float distance = length( lightDirectionToVertex );
    lightDirectionToVertex = lightDirectionToVertex / distance;

	float3 L = ( light.Position - vertexPos ).xyz;
	distance = length( L );
	L = L / distance;

	float attenuation = DoAttenuation( light, distance );
	float spotIntensity = DoSpotCone( light, L );
	result.Diffuse = DoDiffuse( light, L, N ) * attenuation * spotIntensity;
    result.Specular = DoSpecular( light, vertexToEye, lightDirectionToVertex, N ) * attenuation * spotIntensity;
	
	return result;
}

float Shadow( float4 vertLightPos, int num )
{
	float shadowLevel = 0.0f;
	float3 depthPos = vertLightPos.xyz / vertLightPos.w;

	if ( depthPos.z > 1.0f || depthPos.z < 0.0f )
	{
		shadowLevel = 1.0f;
	}
	else
	{
		// Bias to correct depth
        float zBias = depthPos.z - 0.00005f;
		uint width, hight;
		
		if ( num == 1 )
		{
			// PCF
			[unroll]
			for ( int x = -PCF_RANGE; x <= PCF_RANGE; x++ )
			{
				[unroll]
				for ( int y = -PCF_RANGE; y <= PCF_RANGE; y++ )
				{
					shadowLevel += txDepth[1].Sample( smpBorder, depthPos.xy, int2( x, y ) ).r >= zBias ? 1.0f : 0.0f;
				}
			}
		}
		else
		{
			// Hardware PCF
			[unroll]
			for ( int x = -PCF_RANGE; x <= PCF_RANGE; x++ )
			{
				[unroll]
				for ( int y = -PCF_RANGE; y <= PCF_RANGE; y++ )
				{
					shadowLevel += txDepth[0].SampleCmpLevelZero( smpClamp, depthPos.xy, depthPos.z - 0.00005f, int2( x, y ) );
				}
			}
		}
		shadowLevel /= ( ( PCF_RANGE * 2 + 1 ) * ( PCF_RANGE * 2 + 1 ) );
	}

	return shadowLevel;
}

LightingResult ComputeLighting( float4 vertexPos, float3 N, float4 shadowPos[MAX_LIGHTS] )
{
	float3 vertexToEye = ( EyePosition - vertexPos ).xyz;
	LightingResult totalResult = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

	[unroll]
	for ( int i = 0; i < MAX_LIGHTS; ++i )
	{
		LightingResult result = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

		if ( !Lights[i].Enabled ) 
			continue;
		
		float shadowLevel = Shadow( shadowPos[i], i );
		if ( shadowLevel != 0.0f )
		{
			if ( Lights[i].LightType == POINT_LIGHT )
				result = DoPointLight( Lights[i], vertexToEye, vertexPos, N );
			
			if ( Lights[i].LightType == DIRECTIONAL_LIGHT )
				result = DoDirectionalLight( Lights[i], vertexToEye, vertexPos, N );
			
			if ( Lights[i].LightType == SPOT_LIGHT )
				result = DoSpotLight( Lights[i], vertexToEye, vertexPos, N );
			
			result.Diffuse *= shadowLevel;
			result.Specular *= shadowLevel;
		}
		else
		{
			result.Diffuse = 0;
			result.Specular = 0;
		}
		
		totalResult.Diffuse += result.Diffuse;
		totalResult.Specular += result.Specular;
	}

	totalResult.Diffuse = saturate( totalResult.Diffuse );
	totalResult.Specular = saturate( totalResult.Specular );
	return totalResult;
}

// Pixel Shader
struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Tangent : tangent;
	float3 BiTangent : biTangent;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 WorldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3x3 TBN_inv : TBNINV;
	float3x3 TBN : TBN;
	float4 LightViewPosition[MAX_LIGHTS] : TEX;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    LightingResult lit = ComputeLighting( input.WorldPos, normalize( input.Norm ), input.LightViewPosition );

    float4 texColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float4 emissive = Material.Emissive;
	float4 ambient = Material.Ambient * GlobalAmbient;
	float4 diffuse = Material.Diffuse * lit.Diffuse;
	float4 specular = Material.Specular * lit.Specular;

	if (Material.UseTexture)
        texColor = txDiffuse.Sample( smpLinear, input.Tex );
	
	float4 finalColor = ( emissive + ambient + diffuse + specular ) * texColor;
	return finalColor;
}