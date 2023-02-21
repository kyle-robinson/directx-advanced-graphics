// Resources
Texture2D texHeightMap : register( t1 );

SamplerState smpHeightMap
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

// Structs
struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

// Constant Buffers
cbuffer MatrixBuffer : register( b0 )
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vOutputColor;
}

// Domain Shader
struct HULL_OUT
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
};

struct DOMAIN_OUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 TiledTex : TILEDTEX;
};

[domain( "quad" )]
DOMAIN_OUT DS( PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HULL_OUT, 4> quad )
{
    DOMAIN_OUT output;

	// Bilinear interpolation
    output.PosW = lerp(
		lerp( quad[0].PosW, quad[1].PosW, uv.x ),
		lerp( quad[2].PosW, quad[3].PosW, uv.x ), uv.y );

    output.Tex = lerp(
		lerp( quad[0].Tex, quad[1].Tex, uv.x ),
		lerp( quad[2].Tex, quad[3].Tex, uv.x ), uv.y );

	// Tile layer textures over terrain
    output.TiledTex = output.Tex * 50.0f;

	// Displacement mapping
    output.PosW.y = texHeightMap.SampleLevel( smpHeightMap, output.Tex, 0 ).r;

	// Project to homogeneous clip space
    output.PosH = mul( float4( output.PosW, 1.0f ), World );
    output.PosH = mul( output.PosH, View );
    output.PosH = mul( output.PosH, Projection );

    return output;
}