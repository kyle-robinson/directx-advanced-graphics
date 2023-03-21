// Resources
Texture2D txDiffuse[5] : register( t0 );
SamplerState smpLinear : register( s0 );

// Constant Buffers
cbuffer VoxelCube : register( b3 )
{
	int CubeType;
	float3 Padding1;
};

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
    float4 texColor = { 1.0f, 0.0f, 0.0f, 1.0f };

	if ( CubeType == 1 )
		texColor = txDiffuse[0].Sample( smpLinear, input.Tex );
	else if ( CubeType == 2 )
		texColor = txDiffuse[1].Sample( smpLinear, input.Tex );
	else if ( CubeType == 3 )
		texColor = txDiffuse[2].Sample( smpLinear, input.Tex );
	else if ( CubeType == 4 )
		texColor = txDiffuse[3].Sample( smpLinear, input.Tex );
	else if ( CubeType == 5 )
		texColor = txDiffuse[4].Sample( smpLinear, input.Tex );

	return texColor;
}