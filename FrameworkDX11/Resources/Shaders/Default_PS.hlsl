//--------------------------------------------------------------------------------------
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// the lighting equations in this code have been taken from https://www.3dgep.com/texturing-lighting-directx-11/
// with some modifications by David White

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
}


Texture2D txDiffuse : register(t0);
Texture2D txNormal: register(t1);
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
	float3x3 TBN_inv:TBNINV;
	float3x3 TBN:TBN;
	float4 lightViewPosition[MAX_LIGHTS] : TEX;
};


float4 DoDiffuse(Light light, float3 L, float3 N)
{
	float NdotL = max(0, dot(N, L));
	return light.Color * NdotL;
}

float4 DoSpecular(Light lightObject, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal)
{
	float4 lightDir = float4(normalize(-lightDirectionToVertex),1);
	vertexToEye = normalize(vertexToEye);

	float lightIntensity = saturate(dot(Normal, lightDir));
	float4 specular = float4(0, 0, 0, 0);
	if (lightIntensity > 0.0f)
	{
		float3  reflection = normalize(2 * lightIntensity * Normal - lightDir);
		specular = pow(saturate(dot(reflection, vertexToEye)), Material.SpecularPower); // 32 = specular power
	}

	return specular;
}

float DoAttenuation(Light light, float d)
{
	return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d);
}

struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};

LightingResult DoPointLight(Light light, float3 vertexToEye, float4 vertexPos, float3 N)
{
	LightingResult result;

	float3 LightDirectionToVertex = (vertexPos - light.Position).xyz;
	float distance = length(LightDirectionToVertex);
	LightDirectionToVertex = LightDirectionToVertex  / distance;

	float3 vertexToLight = (light.Position - vertexPos).xyz;
	distance = length(vertexToLight);
	vertexToLight = vertexToLight / distance;

	float attenuation = DoAttenuation(light, distance);

	result.Diffuse = DoDiffuse(light, vertexToLight, N) * attenuation;
	result.Specular = DoSpecular(light, vertexToEye, LightDirectionToVertex, N) * attenuation;

	return result;
}

LightingResult DoDirectionalLight(Light light, float3 V, float4 P, float3 N)
{
	LightingResult result;

	float3 L = -light.Direction.xyz;

	result.Diffuse = DoDiffuse(light, L, N);
	result.Specular = DoSpecular(light, V, L, N);

	return result;
}


float DoSpotCone(Light light, float3 L)
{
	float minCos = cos(light.SpotAngle);
	float maxCos = (minCos + 1.0f) / 2.0f;
	float cosAngle = dot(light.Direction.xyz, -L);
	return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult DoSpotLight(Light light, float3 vertexToEye, float4 vertexPos, float3 N)
{
	LightingResult result;


	float3 LightDirectionToVertex = (vertexPos - light.Position).xyz;
	float distance = length(LightDirectionToVertex);
	LightDirectionToVertex = LightDirectionToVertex / distance;

	float3 L = (light.Position - vertexPos).xyz;
	distance = length(L);
	L = L / distance;
	

	float attenuation = DoAttenuation(light, distance);
	float spotIntensity = DoSpotCone(light, L);

	result.Diffuse = DoDiffuse(light, L, N) * attenuation * spotIntensity;
	result.Specular = DoSpecular(light, vertexToEye, LightDirectionToVertex, N) * attenuation * spotIntensity;

	return result;
}
#define PCF_RANGE 2
float Shadow(float4 VertLightPos, int Number) {
	float ShadowLevel = 0.0f;
	float3 depthpos = VertLightPos.xyz / VertLightPos.w;


	if (depthpos.z > 1.0f || depthpos.z < 0.0f) {
		ShadowLevel = 1.0f;
	}
	else {
		//bias to make correction to depth 
		float zBiais = depthpos.z - 0.00005f;
		uint width, hight;
		//PCF
		//light number
		if (Number == 1) {

			//PCF
			[unroll]
			for (int x = -PCF_RANGE; x <= PCF_RANGE; x++) {
				[unroll]
				for (int y = -PCF_RANGE; y <= PCF_RANGE; y++) {
					ShadowLevel += txDepth[1].Sample(sampleStateBorder, depthpos.xy, int2(x, y)).r >= zBiais ? 1.0f : 0.0f;
				}
			}
		}
		else {
			//hardware PCF
			[unroll]
			for (int x = -PCF_RANGE; x <= PCF_RANGE; x++) {
				[unroll]
				for (int y = -PCF_RANGE; y <= PCF_RANGE; y++) {
					ShadowLevel += txDepth[0].SampleCmpLevelZero(sampleStateClamp, depthpos.xy, depthpos.z - 0.00005f, int2(x, y));
				}
			}



		}
		ShadowLevel /= ((PCF_RANGE * 2 + 1) * (PCF_RANGE * 2 + 1));
	}


	return ShadowLevel;


}
LightingResult ComputeLighting(float4 vertexPos, float3 N, float4 ShadowPos[MAX_LIGHTS])
{
	float3 vertexToEye = (EyePosition - vertexPos).xyz;
	
	LightingResult totalResult = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		LightingResult result = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };

		if (!Lights[i].Enabled) 
			continue;
		
		float ShadowLevel = Shadow(ShadowPos[i], i);
		if (ShadowLevel != 0.0f) {
			if (Lights[i].LightType == POINT_LIGHT)
				result = DoPointLight(Lights[i], vertexToEye, vertexPos, N);
			if (Lights[i].LightType == DIRECTIONAL_LIGHT)
				result = DoDirectionalLight(Lights[i], vertexToEye, vertexPos, N);
			if (Lights[i].LightType == SPOT_LIGHT)
				result = DoSpotLight(Lights[i], vertexToEye, vertexPos, N);
			result.Diffuse *= ShadowLevel;
			result.Specular *= ShadowLevel;
		}
		else {
			//no light
			result.Diffuse = 0;
			result.Specular = 0;
		}
		totalResult.Diffuse += result.Diffuse;
		totalResult.Specular += result.Specular;
	}

	totalResult.Diffuse = saturate(totalResult.Diffuse);
	totalResult.Specular = saturate(totalResult.Specular);

	return totalResult;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------


//can change this to calualte vertextoeye and vertextolight in vertex shader to improve efficeny as then they will be caluated 3 times foe each vertex insead
//of for each pixle. To do this another function would have to be created to caluate the data for each light and that data would have to be passed to the pixle shader.
// this has not be done here as for the time bing is is esayer to keep the lighting moddle the 


PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
	output.worldPos = output.Pos;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

	// multiply the normal by the world transform (to go from model space to world space)
	output.Norm = mul(float4(input.Norm, 0), World).xyz;

	output.Tex = input.Tex;
  
	// Build TBN matrix
	float3 T = normalize(mul(input.Tangent, World));
	float3 B = normalize(mul(input.BiTangent, World));
	float3 N = normalize(mul(input.Norm, World));
	float3x3 TBN = float3x3(T, B, N);
	output.TBN_inv = transpose(TBN);
	output.TBN = TBN;


	float4 LightView;
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		//Calculate the position of the vertice as viewed by the light source.
		LightView = mul(input.Pos, World);
		LightView = mul(LightView, Lights[i].mView);
		LightView = mul(LightView, Lights[i].mProjection);
		output.lightViewPosition[i] = LightView * float4(0.5f, -0.5f, 1.0f, 1.0f) + float4(0.5f, 0.5f, 0.0f, 0.0f) * LightView.w;
	}


    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(PS_INPUT IN) : SV_TARGET
{
	
	float4 bumpMap = txNormal.Sample(samLinear, IN.Tex);
	// Expand the range of the normal value from (0, +1) to (-1, +1).
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	//transform from tangent space to world space
	float3 bumpedNorm = mul(bumpMap, IN.TBN);


	LightingResult lit = ComputeLighting(IN.worldPos, bumpedNorm, IN.lightViewPosition);

	float4 texColor = { 1, 1, 1, 1 };

	float4 emissive = Material.Emissive;
	float4 ambient = Material.Ambient * GlobalAmbient;
	float4 diffuse = Material.Diffuse * lit.Diffuse;
	float4 specular = Material.Specular * lit.Specular;

	if (Material.UseTexture)
	{
		texColor = txDiffuse.Sample(samLinear, IN.Tex);

	}

	float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;

	return finalColor;
}

//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}
