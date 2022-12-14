#pragma once
#ifndef STRUCTURES_H
#define STRUCTURES_H

using namespace DirectX;

// Matrices
struct Matrices
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
};

struct MatricesNormalDepth
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mWorldInvTransposeView;
};

struct MatricesShadow
{
	XMMATRIX mWorld;
	XMMATRIX mWorldInvTranspose;
	XMMATRIX mWorldViewProj;
	XMMATRIX mTexTransform;
	XMMATRIX mShadowTransform;
};

// Motion Blur
struct MotionBlurData
{
	MotionBlurData()
		: UseMotionBlur( FALSE )
		, NumSamples( 2 )
	{}

	XMMATRIX mViewProjectionInverse;
	XMMATRIX mPreviousViewProjection;

	BOOL UseMotionBlur;
	int NumSamples;
	XMFLOAT2 Padding;
};

struct MotionBlur_CB
{
	MotionBlurData MotionBlur;
};

// Fast Approximate Anti-Aliasing
struct FXAAData
{
	FXAAData()
		: SpanMax( 8.0f )
		, ReduceMin( 1.0f / 128.0f )
		, ReduceMul( 1.0f / 8.0f )
		, UseFXAA( FALSE )
	{}

	XMFLOAT2 TextureSizeInverse;
	FLOAT SpanMax;
	FLOAT ReduceMin;

	FLOAT ReduceMul;
	BOOL UseFXAA;
	XMFLOAT2 Padding;
};

struct FXAA_CB
{
	FXAAData FXAA;
};

// Screen-Space Ambient Occlusion
struct SSAOData
{
	SSAOData()
		: Radius( 0.5f )
		, Power( 2.0f )
		, KernelSize( 16 )
		, UseSSAO( FALSE )
	{
		srand( static_cast<unsigned>( time( 0 ) ) );
		for ( uint32_t i = 0u; i < 16u; i++ )
		{
			XMFLOAT3 sample;
			sample.x = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f;
			sample.y = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f;
			sample.z = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX );
			KernelOffsets[i] = sample;
		}
	}

	XMMATRIX ProjectionMatrix;

	XMFLOAT2 ScreenSize;
	XMFLOAT2 NoiseScale;

	FLOAT Radius;
	FLOAT Power;
	int KernelSize;
	BOOL UseSSAO;

	XMFLOAT3 KernelOffsets[16];
	FLOAT Padding;
};

struct SSAO_CB
{
	SSAOData SSAO;
};

// Shadow Mapping
struct ShadowData
{
	ShadowData()
		: UseShadows( FALSE )
	{}

	BOOL UseShadows;
	XMFLOAT3 Padding;
};

struct Shadow_CB
{
	ShadowData Shadows;
};

// Materials
struct MaterialData
{
	MaterialData()
		: Emissive( 0.0f, 0.0f, 0.0f, 1.0f )
		, Ambient( 0.1f, 0.1f, 0.1f, 1.0f )
		, Diffuse( 1.0f, 1.0f, 1.0f, 1.0f )
		, Specular( 1.0f, 1.0f, 1.0f, 1.0f )
		, SpecularPower( 128.0f )
		, UseTexture( TRUE )
	{}

	XMFLOAT4 Emissive;
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	
	FLOAT SpecularPower;
	BOOL UseTexture;
	XMFLOAT2 Padding;
};

struct Material_CB
{
	MaterialData Material;
};

// Lights
enum LightType
{
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};

struct LightData
{
	LightData()
		: Position( 0.0f, 0.0f, 0.0f, 1.0f )
		, Direction( 0.0f, 0.0f, 1.0f, 0.0f )
		, Color( 1.0f, 1.0f, 1.0f, 1.0f )
		, SpotAngle( XM_PIDIV2 )
		, ConstantAttenuation( 1.0f )
		, LinearAttenuation( 0.0f )
		, QuadraticAttenuation( 0.0f )
		, Intensity( 4.0f )
		, LightType( PointLight )
		, Enabled( TRUE )
	{}

	XMFLOAT4 Position;
	XMFLOAT4 Direction;
	XMFLOAT4 Color;

	FLOAT SpotAngle;
	FLOAT ConstantAttenuation;
	FLOAT LinearAttenuation;
	FLOAT QuadraticAttenuation;

	FLOAT Intensity;
	int LightType;
	BOOL Enabled;
	FLOAT Padding;
};

#define MAX_LIGHTS 1
struct Light_CB
{
	Light_CB()
		: EyePosition( 0.0f, 0.0f, 0.0f, 1.0f )
		, GlobalAmbient( 0.2f, 0.2f, 0.8f, 1.0f )
	{}

	XMFLOAT4 EyePosition;
	XMFLOAT4 GlobalAmbient;
	LightData Lights[MAX_LIGHTS];
};

// Texture Mapping
struct MappingData
{
	MappingData()
		: UseNormalMap( TRUE )
		, UseParallaxMap( TRUE )
		, UseParallaxOcclusion( TRUE )
		, UseParallaxSelfShadowing( TRUE )
		, UseSoftShadow( TRUE )
		, HeightScale( 0.1f )
	{}

	BOOL UseNormalMap;
	BOOL UseParallaxMap;
	BOOL UseParallaxOcclusion;
	BOOL UseParallaxSelfShadowing;

	BOOL UseSoftShadow;
	FLOAT HeightScale;
	XMFLOAT2 Padding;
};

struct Mapping_CB
{
	MappingData MapData;
};

// Deferred Rendering
struct DeferredData
{
	DeferredData()
		: UseDeferredShading( FALSE )
		, OnlyPositions( FALSE )
		, OnlyAlbedo( FALSE )
		, OnlyNormals( FALSE )
	{}

	BOOL UseDeferredShading;
	BOOL OnlyPositions;
	BOOL OnlyAlbedo;
	BOOL OnlyNormals;
};

struct Deferred_CB
{
	DeferredData Deferred;
};

#endif