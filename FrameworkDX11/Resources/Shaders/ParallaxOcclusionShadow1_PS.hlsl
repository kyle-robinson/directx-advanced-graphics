// Definitions
#define PCF_RANGE 2
#define MAX_LIGHTS 2
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// Resources
Texture2D txDiffuse : register( t0 );
Texture2D txNormal : register( t1 );
Texture2D txParallax : register( t2 );
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
float2 ParallaxOcclusionMapping( float2 texCoord, float3 toEye, float3 Normal )
{
	// Calculate the max of the amount of movement
	float parallaxLimit = -length( toEye.xy ) / toEye.z;
    parallaxLimit *= Material.HeightScale;

	float2 vOffsetDir = normalize( toEye.xy );
    float2 vMaxOffset = vOffsetDir * parallaxLimit;

	float numLayers = lerp( Material.MaxLayers, Material.MinLayers, abs( dot( toEye, Normal ) ) );
	float step = 1.0f / numLayers;

	float2 dx = ddx( texCoord );
	float2 dy = ddy( texCoord );

	float currentRayHeight = 1.0f;
    float2 vCurrOffset = { 0.0f, 0.0f };
	float2 vLastOffset = { 0.0f, 0.0f };
	float currentSampledHeight = 1.0f;
	float lastSampledHeight = 1.0f;
	int currentSample = 0;

	while ( currentSample < numLayers )
	{
		currentSampledHeight = txParallax.SampleGrad( smpLinear, texCoord + vCurrOffset, dx, dy ).x;
		if ( currentSampledHeight > currentRayHeight )
		{
			// Find an intersection
			float delta1 = currentSampledHeight - currentRayHeight;
			float delta2 = ( currentRayHeight + step ) - lastSampledHeight;
			float ratio = delta1 / ( delta1 + delta2 );
			vCurrOffset = ( ratio ) * vLastOffset + ( 1.0f - ratio ) * vCurrOffset;
			currentSample = numLayers + 1;
		}
		else
		{
			// Move to the next layer
			currentSample++;
			currentRayHeight -= step;
			vLastOffset = vCurrOffset;
			vCurrOffset += step * vMaxOffset;
			lastSampledHeight = currentSampledHeight;
		}
	}

	float2 finalTexCoords = texCoord + vCurrOffset;
	return finalTexCoords;
}

float ParallaxSelfShadowing( float3 toLight, float2 texCoord, float3 Normal )
{
	float shadowFactor = 1.0f;
	float2 dx = ddx( texCoord );
	float2 dy = ddy( texCoord );
	float height = 1.0f - txParallax.SampleGrad( smpLinear, texCoord, dx, dy ).x;
	float parallaxScale = Material.HeightScale * ( 1.0f - height );

	// Calculate light for surface oriented to light
	if ( dot( Normal, toLight ) > 0 )
	{
		shadowFactor = 0.0f;
		float numSamplesUnderSurface = 0.0f;
		float numLayers = lerp( Material.MaxLayers, Material.MinLayers, dot( Normal, toLight ) );

		float layerHeight = height / numLayers;
		float2 texStep = parallaxScale * toLight.xy / numLayers;
		float currentLayerHeight = height - layerHeight;
		float2 currentTexCoord = texCoord + texStep;
		float heightFromTex = 1.0f - txParallax.SampleGrad( smpLinear, currentTexCoord, dx, dy ).r;
		int step = 1;

		// Find the shawdow factor
		while ( currentLayerHeight > 0 )
		{
			if ( heightFromTex < currentLayerHeight )
			{
				// Calculate a partial shadowing factor
				numSamplesUnderSurface += 1;
				float newShadowFactor = ( currentLayerHeight - heightFromTex ) * ( 1.0f - step / numLayers );
				shadowFactor = max( shadowFactor, newShadowFactor );
			}

			// Go to the next layer
			step += 1;
			currentLayerHeight -= layerHeight;
			currentTexCoord += texStep;
			heightFromTex = txParallax.SampleGrad( smpLinear, currentTexCoord, dx, dy ).r;
		}

		//set shadow factor
		if ( numSamplesUnderSurface < 1 )
		{
			shadowFactor = 1.0f;
		}
		else
		{
			shadowFactor = 0.9f - shadowFactor;
		}
	}

	return shadowFactor;
}

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

LightingResult DoPointLight( Light light, float3 vertexToEye, float4 vertexPos, float3 N, float3 lightVectorTS )
{
	LightingResult result;

	float3 lightDirectionToVertex = ( vertexPos - light.Position ).xyz;
    float distance = length( lightDirectionToVertex );
    lightDirectionToVertex = lightDirectionToVertex / distance;

	float3 vertexToLight = ( light.Position - vertexPos ).xyz;
	distance = length( vertexToLight );

	float attenuation = DoAttenuation( light, distance );
	result.Diffuse = DoDiffuse( light, lightVectorTS, N ) * attenuation;
    result.Specular = DoSpecular( light, vertexToEye, -lightVectorTS, N ) * attenuation;

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

LightingResult DoSpotLight( Light light, float3 vertexToEye, float4 vertexPos, float3 N, float3 lightVectorTS )
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
	result.Diffuse = DoDiffuse( light, lightVectorTS, N ) * attenuation * spotIntensity;
    result.Specular = DoSpecular( light, vertexToEye, lightVectorTS, N ) * attenuation * spotIntensity;

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

LightingResult ComputeLighting( float4 vertexPos, float3 N, float3 vertexToEye, float3 lightVectorTS[MAX_LIGHTS], float4 shadowPos[MAX_LIGHTS] )
{
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
				result = DoPointLight( Lights[i], vertexToEye, vertexPos, N, lightVectorTS[i] );

			if ( Lights[i].LightType == DIRECTIONAL_LIGHT )
				result = DoDirectionalLight( Lights[i], vertexToEye, vertexPos, N );

			if ( Lights[i].LightType == SPOT_LIGHT )
				result = DoSpotLight( Lights[i], vertexToEye, vertexPos, N, lightVectorTS[i] );

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
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 EyeVectorTS : EYE;
    float3 LightVectorTS[MAX_LIGHTS] : LIGHTVEC;
    float3 PosTS : POSITION1;
    float3 EyePosTS : POSITION2;
    float3 NormTS : NORMAL1;
    float4 LightViewPosition[MAX_LIGHTS] : TEX;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    float2 parallaxTexCoords = ParallaxOcclusionMapping( input.Tex, input.EyeVectorTS, input.NormTS );
    if ( parallaxTexCoords.x > 1.0 || parallaxTexCoords.y > 1.0 || parallaxTexCoords.x < 0.0 || parallaxTexCoords.y < 0.0 )
        discard;

	float4 bumpMap = txNormal.Sample( smpLinear, parallaxTexCoords );
	bumpMap = ( bumpMap * 2.0f ) - 1.0f;
    bumpMap = float4( normalize( bumpMap.xyz ), 1 );
	LightingResult lit = ComputeLighting( input.WorldPos, (float3)bumpMap, input.EyeVectorTS, input.LightVectorTS, input.LightViewPosition );

	float4 texColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float4 emissive = Material.Emissive;
	float4 ambient = Material.Ambient * GlobalAmbient;
	float4 diffuse = Material.Diffuse * lit.Diffuse;
	float4 specular = Material.Specular * lit.Specular;

	if ( Material.UseTexture )
		texColor = txDiffuse.Sample( smpLinear, parallaxTexCoords );

    for ( int i = 0; i < MAX_LIGHTS; ++i )
    {
        float shadow = ParallaxSelfShadowing( input.LightVectorTS[i], parallaxTexCoords, -input.NormTS );
        diffuse = diffuse * shadow;
        specular = specular * shadow;
    }

	float4 finalColor = ( emissive + ambient + diffuse + specular ) * texColor;
	return finalColor;
}