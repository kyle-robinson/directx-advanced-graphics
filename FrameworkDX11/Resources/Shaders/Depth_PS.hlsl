// Pixel Shader
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 DepthPos : TEXCOORD1;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
	float4 texColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    float depth = input.DepthPos.z / input.DepthPos.w;
	texColor = float4( depth, depth, depth, 1.0f );
	return texColor;
}