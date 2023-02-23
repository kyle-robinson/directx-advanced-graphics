#pragma once
#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <DirectXMath.h>
using namespace DirectX;

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vOutputColor;
	XMFLOAT4 camPos;
};

struct _Material
{
	_Material()
		: Emissive( 0.0f, 0.0f, 0.0f, 1.0f )
		, Ambient( 0.1f, 0.1f, 0.1f, 1.0f )
		, Diffuse( 1.0f, 1.0f, 1.0f, 1.0f )
		, Specular( 1.0f, 1.0f, 1.0f, 1.0f )
		, SpecularPower( 128.0f )
		, UseTexture( false )
	{}

	XMFLOAT4 Emissive;
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	float SpecularPower;
	int UseTexture;
	float Padding[2];

	float HeightScale;
	float MaxLayers;
	float MinLayers;
	float Padding1;
};

struct MaterialPropertiesConstantBuffer
{
	_Material Material;
};

enum LightType
{
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};

#define MAX_LIGHTS 2

struct Light
{
	Light()
		: Position( 0.0f, 0.0f, 0.0f, 1.0f )
		, Direction( 0.0f, 0.0f, 1.0f, 0.0f )
		, Color( 1.0f, 1.0f, 1.0f, 1.0f )
		, SpotAngle( XM_PIDIV2 )
		, ConstantAttenuation( 1.0f )
		, LinearAttenuation( 0.0f )
		, QuadraticAttenuation( 0.0f )
		, LightType( DirectionalLight )
		, Enabled( 0 )
	{}

	XMFLOAT4 Position;
	XMFLOAT4 Direction;
	XMFLOAT4 Color;

	float SpotAngle;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;

	int LightType;
	int Enabled;
	int Padding[2];

	XMMATRIX mView;
	XMMATRIX mProjection;
};

struct LightPropertiesConstantBuffer
{
	LightPropertiesConstantBuffer()
		: EyePosition( 0.0f, 0.0f, 0.0f, 1.0f )
		, GlobalAmbient( 0.2f, 0.2f, 0.8f, 1.0f )
	{}

	XMFLOAT4 EyePosition;
	XMFLOAT4 GlobalAmbient;
	Light Lights[MAX_LIGHTS];
};

struct PostProcessingCB
{
	XMFLOAT4 Color;

	int UseHDR;
	int UseBloom;
	int UseDepthOfF;
	int UseColour;

	int UseBlur;
	float FadeAmount;
	float FarPlane;
	float FocalWidth;

	float FocalDistance;
	float BlurAttenuation;
	int Padding1;
	int Padding2;
};


struct TerrainCB
{
	XMFLOAT4 gEyePosition;

	float MinDist;
	float MaxDist;
	float MinTess;
	float MaxTess;

	float Layer1MaxHeight;
	float Layer2MaxHeight;
	float Layer3MaxHeight;
	float Layer4MaxHeight;

	float Layer5MaxHeight;
	float Padding1;
	float Padding2;
	float Padding3;

	XMFLOAT4 WorldFrustumPlanes[6];
};

#endif