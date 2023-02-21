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

Texture2D txDiffuse : register(t0);
Texture2D txNormal: register(t1);
Texture2D txParallax: register(t2);
Texture2D txDepth[2] : register(t3);


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
	output.Pos = input.Pos;
	output.worldPos = mul(input.Pos, World);

	

	return output;
}

[maxvertexcount(4)]
void GS(point PS_INPUT input[1], inout TriangleStream<PS_INPUT> OutputStream) {
	
	float3 planeNormal = input[0].worldPos - camPos;
	planeNormal.y = 0.0f;
	planeNormal = normalize(planeNormal);

	float3 upVector = float3(0.0f, 1.0f, 0.0f);
	float3 rightVector = normalize(cross(planeNormal, upVector));

	// Create the billboards quad	
	float3 vert[4];
	vert[0] = input[0].worldPos - rightVector; 
	vert[1] = input[0].worldPos + rightVector; 
	vert[2] = input[0].worldPos - rightVector + upVector; 
	vert[3] = input[0].worldPos + rightVector + upVector; 

	// Get billboards texture coordinates	
	float2 texCoord[4];
	texCoord[0] = float2(0, 1);	
	texCoord[1] = float2(1, 1);
	texCoord[2] = float2(0, 0);
	texCoord[3] = float2(1, 0);


	PS_INPUT outputvert;

	for (int i = 0; i < 4; i++) {	    
		// VP = view / projection transform matrix, coordinate already has world transform    
		outputvert.Pos = mul(float4(vert[i], 1.0f), View);
		outputvert.Pos = mul(outputvert.Pos, Projection);
		outputvert.worldPos = float4(vert[i], 0.0f);
		outputvert.Tex = texCoord[i];
		outputvert.Norm = float3(0, 0, 0);
		OutputStream.Append(outputvert);
	}

}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(PS_INPUT IN) : SV_TARGET
{


	float4 texColor = { 1, 1, 1, 1 };


	
	texColor = txDiffuse.Sample(samLinear, IN.Tex);
	


	


	return texColor;
}




