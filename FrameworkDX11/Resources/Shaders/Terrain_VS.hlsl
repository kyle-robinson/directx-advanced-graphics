// Resources
Texture2D texHeightMap : register( t1 );

SamplerState smpHeightMap
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

// Vertex Shader
struct VERTEX_IN
{
	float4 PosL : POSITION;
	float2 Tex : TEXCOORD0;
    float2 BoundsY : BOUNDS_Y;
};

struct VERTEX_OUT
{
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float2 BoundsY : BOUNDS_Y;
};

VERTEX_OUT VS( VERTEX_IN input )
{
    VERTEX_OUT vout;

	// Specify the terrain in world space
	vout.PosW = input.PosL;

	// Make the eye -> patch dist calculation more accurate by displacing patch corners to world space
    vout.PosW.y = texHeightMap.SampleLevel( smpHeightMap, input.Tex, 0 ).r;

	vout.Tex = input.Tex;
	vout.BoundsY = input.BoundsY;
	return vout;
}