// Definitions
#define MAX_LIGHTS 1
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// Resources
Texture2D textureDiffuse : register( t0 );
Texture2D textureNormal : register( t1 );
Texture2D textureDisplacement : register( t2 );
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
    float MinLayers;
    float MaxLayers;
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

// Lighting Functions
float4 DoDiffuse( Light light, float3 L, float3 N )
{
	float NdotL = max( 0.0f, dot( N, L ) );
	return light.Color * NdotL;
}

float4 DoSpecular( Light light, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal )
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

LightingResult DoDirectionalLight( Light light, float3 V, float4 P, float3 N, float3 lightVectorTS )
{
    LightingResult result;

    float3 L = -light.Direction.xyz;
    result.Diffuse = DoDiffuse( light, L, N );
    result.Specular = DoSpecular( light, V, L, N );

    return result;
}

LightingResult DoPointLight( Light light, float3 vertexToEye, float4 vertexPos, float3 N, float3 lightVectorTS )
{
	LightingResult result;

	float3 LightDirectionToVertex = ( vertexPos - light.Position ).xyz;
	float distance = length( LightDirectionToVertex );
	LightDirectionToVertex = LightDirectionToVertex  / distance;

	float3 vertexToLight = ( light.Position - vertexPos ).xyz;
	distance = length( vertexToLight );

	float attenuation = DoAttenuation( light, distance );

	result.Diffuse = DoDiffuse( light, lightVectorTS, N ) * attenuation;
	result.Specular = DoSpecular( light, vertexToEye, -lightVectorTS, N ) * attenuation;

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

    float3 LightDirectionToVertex = ( vertexPos - light.Position ).xyz;
    float distance = length( LightDirectionToVertex );
    LightDirectionToVertex = LightDirectionToVertex / distance;

    float3 L = ( light.Position - vertexPos ).xyz;
    distance = length( L );
    L = L / distance;

    float attenuation = DoAttenuation( light, distance );
    float spotIntensity = DoSpotCone( light, L );

    result.Diffuse = DoDiffuse( light, lightVectorTS, N ) * attenuation * spotIntensity;
    result.Specular = DoSpecular( light, vertexToEye, lightVectorTS, N ) * attenuation * spotIntensity;

    return result;
}

LightingResult ComputeLighting( float4 vertexPos, float3 N, float3 vertexToEye, float3 lightVectorTS[MAX_LIGHTS] )
{
	LightingResult totalResult = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

	[unroll]
	for ( int i = 0; i < MAX_LIGHTS; ++i )
	{
		LightingResult result = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

		if ( !Lights[i].Enabled )
			continue;

        if ( Lights[i].LightType == DIRECTIONAL_LIGHT )
            result = DoDirectionalLight( Lights[i], vertexToEye, vertexPos, N, lightVectorTS[i] );
        else if ( Lights[i].LightType == POINT_LIGHT )
            result = DoPointLight( Lights[i], vertexToEye, vertexPos, N, lightVectorTS[i] );
        else if ( Lights[i].LightType == SPOT_LIGHT )
            result = DoSpotLight( Lights[i], vertexToEye, vertexPos, N, lightVectorTS[i] );

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }

	totalResult.Diffuse = saturate( totalResult.Diffuse );
	totalResult.Specular = saturate( totalResult.Specular );

	return totalResult;
}

// Tangent Functions
float3x3 computeTBNMatrix( float3 unitNormal, float3 tangent )
{
    float3 N = unitNormal;
    float3 T = normalize( tangent - dot( tangent, N ) * N );
    float3 B = cross( T, N );
    return float3x3( T, B, N );
}

float3x3 computeTBNMatrixB( float3 unitNormal, float3 tangent, float3 binorm )
{
    float3 N = unitNormal;
    float3 T = normalize( tangent - dot( tangent, N ) * N );
    float3 B = normalize( binorm - dot( binorm, tangent ) * tangent );
    return float3x3( T, B, N );
}

float3 NormalMapping( float2 texCoord )
{
    float3 texNormal = textureNormal.Sample( samplerState, texCoord ).rgb;
    float3 texNorm = ( 2.0f * texNormal ) - 1.0f;
    return normalize( texNorm );
}

float2 SimpleParallax( float2 texCoord, float3 toEye )
{
    float height = textureDisplacement.Sample( samplerState, texCoord ).r;
    float heightSB = Mapping.HeightScale * ( height - 1.0f );
    float2 parallax = toEye.xy * heightSB;
    return ( texCoord + parallax );
}

float2 ParallaxOcclusion( float2 texCoord, float3 normal, float3 toEye )
{
    int nMinSamples = 8;
    int nMaxSamples = 32;
    float3 toEyeTS = -toEye;
    float2 parallaxLimit = Mapping.HeightScale * toEyeTS.xy; // parallax shift

    int numSamples = (int) lerp( nMaxSamples, nMinSamples, abs( dot( toEyeTS, normal ) ) );
    float zStep = 1.0f / (float) numSamples;
    float2 heightStep = zStep * parallaxLimit;

    float2 dx = ddx( texCoord );
    float2 dy = ddy( texCoord );

	// loop variables
    int currSample = 0;
    float2 currParallax = float2( 0.0f, 0.0f );
    float2 prevParallax = float2( 0.0f, 0.0f );
    float2 finalParallax = float2( 0.0f, 0.0f );
    float currZ = 1.0f - heightStep;
    float prevZ = 1.0f;
    float currHeight = 0.0f;
    float prevHeight = 0.0f;

    while ( currSample < numSamples + 1 )
    {
        currHeight = textureDisplacement.SampleGrad( samplerState, texCoord + currParallax, dx, dy ).r;
        if ( currHeight > currZ )
        {
            float n = prevHeight - prevZ;
            float d = prevHeight - currHeight - prevZ + currZ;
            float ratio = n / d;
            finalParallax = prevParallax + ratio * heightStep;
            currSample = numSamples + 1;
        }
        else
        {
            ++currSample;
            prevParallax = currParallax;
            prevZ = currZ;
            prevHeight = currHeight;
            currParallax += heightStep;
            currZ -= zStep;
        }
    }

    return ( texCoord + finalParallax );
}

float ParallaxSelfShadowing( float3 toLight, float2 texCoord, float3 normal, bool softShadow )
{
    float shadowFactor = 1.0f;
    float2 dx = ddx( texCoord );
    float2 dy = ddy( texCoord );
    float height = 1.0f - textureDisplacement.SampleGrad( samplerState, texCoord, dx, dy ).r;
    float parallaxScale = Mapping.HeightScale * ( 1.0f - height );

    if ( dot( normal, toLight ) > 0.0f )
    {
        shadowFactor = 0.0f;
        float numSamplesUnderSurface = 0.0f;
        float numLayers = lerp( Mapping.MaxLayers, Mapping.MinLayers, dot( normal, toLight ) );

        float layerHeight = height / numLayers;
        float2 texStep = parallaxScale * toLight.xy / numLayers;

        float currLayerHeight = height - layerHeight;
        float2 currTexCoord = texCoord + texStep;
        float heightFromTex = 1.0f - textureDisplacement.SampleGrad( samplerState, currTexCoord, dx, dy ).r;
        int stepIndex = 1;

        while ( currLayerHeight > 0.0f )
        {
            if ( heightFromTex < currLayerHeight )
            {
                numSamplesUnderSurface += 1.0f;
                float newShadowFactor = ( currLayerHeight - heightFromTex ) * ( 1.0f - stepIndex / numLayers );
                shadowFactor = max( shadowFactor, newShadowFactor );
            }

            stepIndex += 1;
            currLayerHeight -= layerHeight;
            currTexCoord += texStep;
            heightFromTex = textureDisplacement.SampleGrad( samplerState, currTexCoord, dx, dy ).r;
        }

        if ( numSamplesUnderSurface < 1.0f )
        {
            shadowFactor = 1.0f;
        }
        else
        {
            if ( softShadow )
            {
                shadowFactor = 0.9f - shadowFactor;
            }
            else
            {
                shadowFactor = 0.6f;
            }
        }
    }

    return shadowFactor;
}

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;

    float3 PositionTS : POSITION_TS;
    float4 WorldPosition : WORLD_POSITION;

    float3 EyePosTS : EYE_POSITION_TS;
    float3 EyeVectorsTS : EYE_VECTOR_TS;

    float3 LightVectorTS[MAX_LIGHTS] : LIGHT_VECTORS_TS;
    //float4 LightViewPosition[MAX_LIGHTS] : LIGHT_VIEW_POSITIONS;

    float3 Normal : NORMAL;
    float3 NormalTS : NORMAL_TS;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
	// parallax
    if ( Mapping.UseParallaxMap )
    {
        if ( Mapping.UseParallaxOcclusion )
            input.TexCoord = ParallaxOcclusion( input.TexCoord, input.NormalTS, input.EyeVectorsTS );
        else
            input.TexCoord = SimpleParallax( input.TexCoord, input.EyeVectorsTS );

        if ( input.TexCoord.x > 1.0f || input.TexCoord.y > 1.0f || input.TexCoord.x < 0.0f || input.TexCoord.y < 0.0f )
            discard;
    }

	// normal
    if ( Mapping.UseNormalMap )
        input.Normal = NormalMapping( input.TexCoord );

	// lighting
    LightingResult lit = ComputeLighting( input.WorldPosition, normalize( input.Normal ), input.EyeVectorsTS, input.LightVectorTS );

	// texture/material
    float4 textureColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    float4 emissive = Material.Emissive;
    float4 ambient = Material.Ambient * GlobalAmbient;
    float4 diffuse = Material.Diffuse * lit.Diffuse;
    float4 specular = Material.Specular * lit.Specular;

    // update intensity
    for ( int i = 0; i < MAX_LIGHTS; ++i )
    {
        emissive *= Lights[i].Intensity;
        ambient *= Lights[i].Intensity;
        diffuse *= Lights[i].Intensity;
        specular *= Lights[i].Intensity;
    }

    // update texture
    if ( Material.UseTexture )
        textureColor = textureDiffuse.Sample( samplerState, input.TexCoord );
    else
        textureColor = float4( 1.0f, 1.0f, 1.0f, 1.0f );

    // self-shadowing
    float shadowFactor = 1.0f;
    if ( Mapping.UseParallaxSelfShadowing )
    {
        for ( int i = 0; i < MAX_LIGHTS; ++i )
        {
            shadowFactor = ParallaxSelfShadowing( input.LightVectorTS[i], input.TexCoord, -input.NormalTS, Mapping.UseSoftShadow );
            diffuse = diffuse * shadowFactor;
            specular = specular * shadowFactor;
        }
    }

    // final colour
    float4 finalColor = ( emissive + ambient + diffuse + specular ) * textureColor;
    return finalColor;
}