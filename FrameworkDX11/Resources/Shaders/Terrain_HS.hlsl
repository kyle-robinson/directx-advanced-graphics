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

cbuffer TerrainCB : register( b4 )
{
	float4 EyePosition;

	float MinDist;
	float MaxDist;
	float MinTess;
	float MaxTess;

	float Layer1MaxHeight;
	float Layer2MaxHeight;
	float Layer3MaxHeight;
	float Layer4MaxHeight;

	float Layer5MaxHeight;
	float Paddin1;
	float Paddin2;
	float Paddin3;

	float4 WorldFrustumPlanes[6];
};

// Terrain Functions

/* Original method by https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/tree/master/Chapter%2019%20Terrain%20Rendering/Terrain */

// Get tess factor and dist from camera
float CalcTessFactor( float3 p )
{
    float d = distance( p, (float3)EyePosition );
    float s = saturate( ( d - MinDist ) / ( MaxDist - MinDist ) );
    return pow( 2, ( lerp( MaxTess, MinTess, s ) ) );
}

// Return true if box is completely behind plane
bool AabbBehindPlaneTest( float3 center, float3 extents, float4 plane )
{
    float3 n = abs( plane.xyz );

	// Always positive
    float r = dot( extents, n );

	// Signed distance from center point to plane.
    float s = dot( float4( center, 1.0f ), plane );

	// If the center point of the box is a distance of e or more behind the
	// plane (in which case s is negative since it is behind the plane),
	// then the box is completely in the negative half space of the plane
    return ( s + r ) < 0.0f;
}

// Returns true if the box is completely outside the frustum
bool AabbOutsideFrustumTest( float3 center, float3 extents, float4 frustumPlanes[6] )
{
    for ( int i = 0; i < 6; ++i )
    {
		// If the box is completely behind any of the frustum planes
		// then it is outside the frustum
        if (AabbBehindPlaneTest( center, extents, frustumPlanes[i] ) )
        {
            return true;
        }
    }
    return false;
}

// Hull Shader
struct VERTEX_OUT
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : BOUNDS_Y;
};

PatchTess ConstantHS( InputPatch<VERTEX_OUT, 4> patch, uint patchID : SV_PrimitiveID )
{
    PatchTess pt;

	// Get y bounds
    float minY = patch[0].BoundsY.x;
    float maxY = patch[0].BoundsY.y;

	// Build axis-aligned bounding box
    float3 vMin = float3( patch[2].PosW.x, minY, patch[2].PosW.z );
    float3 vMax = float3( patch[1].PosW.x, maxY, patch[1].PosW.z );

	// Put into world space
    vMin = mul( float4( vMin, 1 ), World );
    vMax = mul( float4( vMax, 1 ), World );

    float3 boxCenter = 0.5f * ( vMin + vMax );
    float3 boxExtents = 0.5f * ( vMax - vMin );
    if ( AabbOutsideFrustumTest( boxCenter, boxExtents, WorldFrustumPlanes ) )
    {
		// Null patch out
        pt.EdgeTess[0] = 0.0f;
        pt.EdgeTess[1] = 0.0f;
        pt.EdgeTess[2] = 0.0f;
        pt.EdgeTess[3] = 0.0f;

        pt.InsideTess[0] = 0.0f;
        pt.InsideTess[1] = 0.0f;

        return pt;
    }
    else
    {
		// Put into world space
        float3 pos0 = mul( float4( patch[0].PosW, 1 ), World );
        float3 pos1 = mul( float4( patch[1].PosW, 1 ), World );
        float3 pos2 = mul( float4( patch[2].PosW, 1 ), World );
        float3 pos3 = mul( float4( patch[3].PosW, 1 ), World );

		// Compute midpoint on edges, and patch center
        float3 e0 = 0.5f * ( pos0 + pos2 );
        float3 e1 = 0.5f * ( pos0 + pos1 );
        float3 e2 = 0.5f * ( pos1 + pos3 );
        float3 e3 = 0.5f * ( pos2 + pos3 );
        float3 c = 0.25f * ( pos0 + pos1 + pos2 + pos3 );

		//get the tess factor
        pt.EdgeTess[0] = CalcTessFactor( e0 );
        pt.EdgeTess[1] = CalcTessFactor( e1 );
        pt.EdgeTess[2] = CalcTessFactor( e2 );
        pt.EdgeTess[3] = CalcTessFactor( e3 );

        pt.InsideTess[0] = CalcTessFactor( c );
        pt.InsideTess[1] = pt.InsideTess[0];

        return pt;
    }
}

struct HULL_OUT
{
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
};

[domain( "quad" )]
[partitioning( "fractional_even" )]
[outputtopology( "triangle_cw" )]
[outputcontrolpoints( 4 )]
[patchconstantfunc( "ConstantHS" )]
[maxtessfactor( 64.0f )]
HULL_OUT HS( InputPatch<VERTEX_OUT, 4> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID )
{
	// Pass-through
    HULL_OUT output;
    output.PosW = p[i].PosW;
    output.Tex = p[i].Tex;
    return output;
}