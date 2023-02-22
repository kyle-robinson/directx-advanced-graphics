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

// Vertex Shader
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

PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
	output.WorldPos = output.Pos;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

	// Convert from model to world space
	output.Norm = mul( float4( input.Norm, 0 ), World ).xyz;
	output.Tex = input.Tex;

	// Build TBN matrix
	float3 T = normalize( mul( input.Tangent, (float3x4)World ) );
	float3 B = normalize( mul( input.BiTangent, (float3x4)World ) );
	float3 N = normalize( mul( input.Norm, (float3x4)World ) );
	float3x3 TBN = float3x3( T, B, N );
	output.TBN_inv = transpose( TBN );
	output.TBN = TBN;

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