//--------------------------------------------------------------------------------------
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// the lighting equations in this code have been taken from https://www.3dgep.com/texturing-lighting-directx-11/
// with some modifications by David White

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
}

#define MAX_LIGHTS 2
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct _Material
{
	float4  Emissive;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Ambient;        // 16 bytes
							//------------------------------------(16 byte boundary)
	float4  Diffuse;        // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Specular;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float   SpecularPower;  // 4 bytes
	bool    UseTexture;     // 4 bytes
	float2  Padding;        // 8 bytes
							//----------------------------------- (16 byte boundary)
	float               HightScale;
	float               MaxLayers;
	float               MinLayers;
	float               Padding1;
};  // Total:               // 80 bytes ( 5 * 16 )

cbuffer MaterialProperties : register(b1)
{
	_Material Material;
};

struct Light
{
	float4      Position;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Direction;              // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Color;                  // 16 bytes
										//----------------------------------- (16 byte boundary)
	float       SpotAngle;              // 4 bytes
	float       ConstantAttenuation;    // 4 bytes
	float       LinearAttenuation;      // 4 bytes
	float       QuadraticAttenuation;   // 4 bytes
										//----------------------------------- (16 byte boundary)
	int         LightType;              // 4 bytes
	bool        Enabled;                // 4 bytes
	int2        Padding;                // 8 bytes
										//----------------------------------- (16 byte boundary)
	matrix mView;
	matrix mProjection;
};  // Total:                           // 80 bytes (5 * 16)

cbuffer LightProperties : register(b2)
{
	float4 EyePosition;                 // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4 GlobalAmbient;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS];           // 80 * 8 = 640 bytes
};


cbuffer TerrainCB : register(b4)
{
	float4 gEyePosition;                 // 16 bytes
										//----------------------------------- (16 byte boundary)
	float MinDist;
	float MaxDist;
	float MinTess;
	float MaxTess;
	//----------------------------------- (16 byte boundary)
	float Layer1MaxHight;
	float Layer2MaxHight;
	float Layer3MaxHight;
	float Layer4MaxHight;
	//----------------------------------- (16 byte boundary)
	float Layer5MaxHight;
	float pad1;
	float pad2;
	float pad3;
	//----------------------------------- (16 byte boundary)
	float4 WorldFrustumPlanes[6];
};

//--------------------------------------------------------------------------------------
struct VertexIn
{
	float4 PosL : POSITION;
	float2 Tex : TEXCOORD0;
	float2 BoundsY : BoundsY;
};

struct VertexOut
{
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float2 BoundsY : BoundsY;
};


Texture2D gBlendMap: register(t0);
Texture2D gHeightMap: register(t1);
Texture2D gLayerMapArray[5]: register(t2);
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};
SamplerState samHeightmap
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VertexOut VS(VertexIn input)
{
	VertexOut vout;
	
	// Terrain specified directly in world space.
	vout.PosW = input.PosL;
	
	// Displace the patch corners to world space.  This is to make 
	// the eye to patch distance calculation more accurate.
	vout.PosW.y = gHeightMap.SampleLevel(samHeightmap, input.Tex, 0).r;

	// Output vertex attributes to next stage.
	vout.Tex = input.Tex;
	vout.BoundsY = input.BoundsY;
	
	
	return vout;
}


//code supported by https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/tree/master/Chapter%2019%20Terrain%20Rendering/Terrain

//get the distace from cam and get tess factor.
float CalcTessFactor(float3 p)
{
	
	float d = distance(p, gEyePosition);

	float s = saturate((d - MinDist) / (MaxDist - MinDist));

	return pow(2, (lerp(MaxTess, MinTess, s)));
	
}

// Returns true if the box is completely behind (in negative half space) of plane.
bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane)
{
	float3 n = abs(plane.xyz);

	// This is always positive.
	float r = dot(extents, n);

	// signed distance from center point to plane.
	float s = dot(float4(center, 1.0f), plane);

	// If the center point of the box is a distance of e or more behind the
	// plane (in which case s is negative since it is behind the plane),
	// then the box is completely in the negative half space of the plane.
	return (s + r) < 0.0f;
}

// Returns true if the box is completely outside the frustum.
bool AabbOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6])
{
	for (int i = 0; i < 6; ++i)
	{
		// If the box is completely behind any of the frustum planes
		// then it is outside the frustum.
		if (AabbBehindPlaneTest(center, extents, frustumPlanes[i]))
		{
			return true;
		}
	}

	return false;
}

struct PatchTess
{
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	//
	// Frustum cull
	//

	//get y bounds
	float minY = patch[0].BoundsY.x;
	float maxY = patch[0].BoundsY.y;

	// Build axis-aligned bounding box.  
	float3 vMin = float3(patch[2].PosW.x, minY, patch[2].PosW.z);
	float3 vMax = float3(patch[1].PosW.x, maxY, patch[1].PosW.z);

	//put into world space 
	vMin = mul(float4(vMin,1), World);
	vMax = mul(float4(vMax,1), World);

	float3 boxCenter = 0.5f * (vMin + vMax);
	float3 boxExtents = 0.5f * (vMax - vMin);
	if (AabbOutsideFrustumTest(boxCenter, boxExtents, WorldFrustumPlanes))
	{
		//null patch out
		pt.EdgeTess[0] = 0.0f;
		pt.EdgeTess[1] = 0.0f;
		pt.EdgeTess[2] = 0.0f;
		pt.EdgeTess[3] = 0.0f;

		pt.InsideTess[0] = 0.0f;
		pt.InsideTess[1] = 0.0f;

		return pt;
	}
	//tesslation
	else
	{
		
		//put into worl space
		float3 pos0 = mul(float4(patch[0].PosW, 1), World);
		float3 pos1 = mul(float4(patch[1].PosW, 1), World);
		float3 pos2 = mul(float4(patch[2].PosW, 1), World);
		float3 pos3 = mul(float4(patch[3].PosW, 1), World);

		// Compute midpoint on edges, and patch center. so not brakes in the sauface are displayed
		float3 e0 = 0.5f * (pos0 + pos2);
		float3 e1 = 0.5f * (pos0 + pos1);
		float3 e2 = 0.5f * (pos1 + pos3);
		float3 e3 = 0.5f * (pos2 + pos3);
		float3  c = 0.25f * (pos0 + pos1 + pos2 + pos3);

		//get the tess factor
		pt.EdgeTess[0] = CalcTessFactor(e0);
		pt.EdgeTess[1] = CalcTessFactor(e1);
		pt.EdgeTess[2] = CalcTessFactor(e2);
		pt.EdgeTess[3] = CalcTessFactor(e3);

		pt.InsideTess[0] = CalcTessFactor(c);
		pt.InsideTess[1] = pt.InsideTess[0];

		return pt;
	}
}

struct HullOut
{
	float3 PosW     : POSITION;
	float2 Tex      : TEXCOORD0;
};

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 4> p,
	uint i : SV_OutputControlPointID,
	uint patchId : SV_PrimitiveID)
{
	HullOut hout;

	//// Pass through shader.
	hout.PosW = p[i].PosW;
	hout.Tex = p[i].Tex;

	return hout;
}

struct DomainOut
{
	float4 PosH     : SV_POSITION;
	float3 PosW     : POSITION;
	float2 Tex      : TEXCOORD0;
	float2 TiledTex : TiledTex;
};

// The domain shader is called for every vertex created by the tessellator.  
[domain("quad")]
DomainOut DS(PatchTess patchTess,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HullOut, 4> quad)
{
	DomainOut dout;
	
	// Bilinear interpolation.
	dout.PosW = lerp(
		lerp(quad[0].PosW, quad[1].PosW, uv.x),
		lerp(quad[2].PosW, quad[3].PosW, uv.x),
		uv.y);

	dout.Tex = lerp(
		lerp(quad[0].Tex, quad[1].Tex, uv.x),
		lerp(quad[2].Tex, quad[3].Tex, uv.x),
		uv.y);

	// Tile layer textures over terrain.
	dout.TiledTex = dout.Tex * 50.0f;

	// Displacement mapping
	
	dout.PosW.y = gHeightMap.SampleLevel(samHeightmap, dout.Tex, 0).r;
	
	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), World);
	dout.PosH = mul(dout.PosH, View);
	dout.PosH = mul(dout.PosH, Projection);
	return dout;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(DomainOut IN) : SV_TARGET
{

	
	float4 texColor = { 1, 0, 0, 1 };

	// Sample layers in texture array.
	float4 water = { 0,0,1,1 };
	float4 c0 = gLayerMapArray[0].Sample(samLinear, IN.TiledTex);
	float4 c1 = gLayerMapArray[1].Sample(samLinear, IN.TiledTex);
	float4 c2 = gLayerMapArray[2].Sample(samLinear, IN.TiledTex);
	float4 c3 = gLayerMapArray[3].Sample(samLinear, IN.TiledTex);
	float4 c4 = gLayerMapArray[4].Sample(samLinear, IN.TiledTex);

	bool useBlendMap = false;
	if (useBlendMap==true) {
		// Sample the blend map.
		float4 t = gBlendMap.Sample(samLinear, IN.Tex);

		// Blend the layers on top of each other.
		texColor = c0;
		texColor = lerp(texColor, c1, t.r);
		texColor = lerp(texColor, c2, t.g);
		texColor = lerp(texColor, c3, t.b);
		texColor = lerp(texColor, c4, t.a);
	}
	else {
		//use hight to genrate data
		float BlendFactor = 0;
		if (IN.PosW.y < 0) {
			texColor = water;
		}
		else if (IN.PosW.y < Layer1MaxHight) {
			BlendFactor = smoothstep(0, Layer1MaxHight, IN.PosW.y);
			texColor = lerp(water, c0, BlendFactor);
		}
		else if (IN.PosW.y < Layer2MaxHight) {
			BlendFactor = smoothstep(Layer1MaxHight, Layer2MaxHight, IN.PosW.y);
			texColor = lerp(c0, c1, BlendFactor);
		}
		else if (IN.PosW.y < Layer3MaxHight) {
			BlendFactor = smoothstep(Layer2MaxHight, Layer3MaxHight, IN.PosW.y);
			texColor = lerp(c1, c2, BlendFactor);
		}
		else if (IN.PosW.y < Layer4MaxHight) {
			BlendFactor = smoothstep(Layer3MaxHight, Layer4MaxHight, IN.PosW.y);
			texColor = lerp(c2, c3, BlendFactor);;
		}
		else {
			BlendFactor = smoothstep(Layer4MaxHight, Layer5MaxHight, IN.PosW.y);
			texColor = lerp(c3, c4, BlendFactor);;
		}
	}

	float4 finalColor = texColor;
	return finalColor;
}

//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(VertexOut input) : SV_Target
{
	return vOutputColor;
}



