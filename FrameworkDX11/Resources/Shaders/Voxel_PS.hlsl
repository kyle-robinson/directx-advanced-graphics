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
    float4 water = { 0.1f, 0.1f, 0.5f, 1.0f };
    float4 grass = { 0.1f, 0.5f, 0.1f, 1.0f };
    float4 lightDirt = { 0.8f, 0.6f, 0.5f, 1.0f };
    float4 darkDirt = { 0.5f, 0.3f, 0.1f, 1.0f };
    float4 stone = { 0.6f, 0.6f, 0.6f, 1.0f };
    float4 snow = { 0.9f, 0.9f, 0.9f, 1.0f };

	if ( CubeType == 1 )
		texColor = water;
	else if ( CubeType == 2 )
		texColor = lightDirt;
	else if ( CubeType == 3 )
		texColor = snow;
	else if ( CubeType == 4 )
        texColor = stone;
	else if ( CubeType == 5 )
		texColor = grass;

	return texColor;
}