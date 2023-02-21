// Resources
Texture2D texLayerMapArray[5] : register( t0 );
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
    float4 texColor = { 0.0f, 1.0f, 0.0f, 1.0f };
    float4 water = { 0.0f, 0.0f, 1.0f, 1.0f };
    float4 stone = { 0.69f, 0.70f, 0.70f, 1.0f };
    float4 snow = { 0.0f, 0.0f, 0.0f, 1.0f };
	float4 c0 = texLayerMapArray[0].Sample( smpLinear, input.Tex );
	float4 c1 = texLayerMapArray[1].Sample( smpLinear, input.Tex );
	float4 c2 = texLayerMapArray[2].Sample( smpLinear, input.Tex );
	float4 c3 = texLayerMapArray[3].Sample( smpLinear, input.Tex );
	float4 c4 = texLayerMapArray[4].Sample( smpLinear, input.Tex );

	float blendFactor = 0.0f;
	if ( CubeType == 1 )
		texColor = water;
	else if ( CubeType == 2 )
		texColor = c0;
	else if ( CubeType == 3 )
		texColor = snow;
	else if ( CubeType == 4 )
        texColor = stone;
	else if ( CubeType == 5 )
		texColor = c3;

	return texColor;
}