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
	float4 camPos;
}

Texture2D gLayerMapArray[5]: register(t0);

SamplerState samLinear : register(s0);
SamplerComparisonState sampleStateClamp : register(s1);
SamplerState sampleStateBorder : register(s2);
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

cbuffer VoxleCube : register(b3)
{
	int CubeType;   
	float3 Pad1;

};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Tangent : tangent;
	float3 BiTangent : biTangent;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;

};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{

	input.Pos.w = 1.0f;
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, World);
	output.worldPos = output.Pos;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);



	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(PS_INPUT IN) : SV_TARGET
{


	float4 texColor = { 0, 1, 0, 1 };

	float4 water = { 0,0,1,1 };
	float4 Stone = { 0.69 ,0.70 ,0.70 ,1 };
	float4 snow = { 0,0,0,1 };
	float4 c0 = gLayerMapArray[0].Sample(samLinear, IN.Tex);
	float4 c1 = gLayerMapArray[1].Sample(samLinear, IN.Tex);
	float4 c2 = gLayerMapArray[2].Sample(samLinear, IN.Tex);
	float4 c3 = gLayerMapArray[3].Sample(samLinear, IN.Tex);
	float4 c4 = gLayerMapArray[4].Sample(samLinear, IN.Tex);

	float BlendFactor = 0;
	//set cube textuer
	if (CubeType==1) {
		texColor = water;
	}
	else if (CubeType==2) {
		texColor = c0;
	}
	else if (CubeType==3) {
		
		texColor = snow;
	}
	else if (CubeType==4) {
		texColor = Stone;
	}
	else if (CubeType==5) {
		texColor = c3;
	}
	

	

	return texColor;
}




