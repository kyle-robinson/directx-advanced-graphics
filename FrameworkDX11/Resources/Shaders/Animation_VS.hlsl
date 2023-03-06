// Definitions
#define MAX_LIGHTS 2

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

// Constant Buffers
cbuffer MatrixBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
};

cbuffer LightProperties : register( b2 )
{
	float4 EyePosition;
	float4 GlobalAmbient;
	Light Lights[MAX_LIGHTS];
};

cbuffer SkinnedBones: register( b5 )
{
	matrix BoneTransforms[96];
};

// Vertex Shader
struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Tangent : tangent;
	float3 Weights : WEIGHTS;
	float4 BoneIndices : BONEINDICES;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 WorldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 LightViewPosition[MAX_LIGHTS] : TEX;
};

/* Original method by https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/tree/master/Chapter%2025%20Character%20Animation/SkinnedMesh */

PS_INPUT VS( VS_INPUT input )
{
	// Get data on the bones weights
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = input.Weights.x;
	weights[1] = input.Weights.y;
	weights[2] = input.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	// Get vertex position based on the bone
	float3 posL = float3( 0.0f, 0.0f, 0.0f );
	float3 normalL = float3( 0.0f, 0.0f, 0.0f );
	float3 tangentL = float3( 0.0f, 0.0f, 0.0f );
	for ( int i = 0; i < 4; ++i )
	{
		posL += weights[i] *  mul( float4( input.Pos.xyz, 1.0f ), BoneTransforms[input.BoneIndices[i]] ).xyz;
		normalL += weights[i] * mul( input.Norm, (float3x3)BoneTransforms[input.BoneIndices[i]] );
		tangentL += weights[i] * mul( input.Tangent.xyz, (float3x3)BoneTransforms[input.BoneIndices[i]] );
	}

	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul( float4( posL.xyz, 1.0f ), World );
	output.WorldPos = output.Pos;
	output.Pos = mul( output.Pos, View );
	output.Pos = mul( output.Pos, Projection );

	// Convert from model to world space
	output.Norm = mul( float4( input.Norm, 0 ), World ).xyz;
	output.Tex = input.Tex;

	float4 lightView;
	for ( int i = 0; i < MAX_LIGHTS; ++i )
	{
		// Calculate vertex position from light
		lightView = mul( input.Pos, World );
		lightView = mul( lightView, Lights[i].mView );
		lightView = mul( lightView, Lights[i].mProjection );
		output.LightViewPosition[i] = lightView * float4( 0.5f, -0.5f, 1.0f, 1.0f ) + float4( 0.5f, 0.5f, 0.0f, 0.0f ) * lightView.w;
	}

	return output;
}