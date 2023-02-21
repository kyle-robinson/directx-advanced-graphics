// Resources
Texture2D txDiffuse : register( t0 );
SamplerState smpLinear : register( s0 );

// Pixel Shader
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
	float4 texColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	texColor = txDiffuse.Sample( smpLinear, input.Tex );
	return texColor;
}