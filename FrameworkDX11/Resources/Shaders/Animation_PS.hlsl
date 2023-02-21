// Resources
Texture2D txDiffuse : register( t0 );
SamplerState smpLinear : register( s0 );

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

	float HeightScale;
	float MaxLayers;
	float MinLayers;
	float Padding1;
};

// Constant Buffers
cbuffer MaterialProperties : register( b1 )
{
	_Material Material;
};

// Pixel Shader
struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Tangent : tangent;
	float3 Weights    : WEIGHTS;
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

float4 PS( PS_INPUT input ) : SV_TARGET
{
	float4 texColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	if ( Material.UseTexture )
		texColor = txDiffuse.Sample( smpLinear, input.Tex );
	return texColor;
}