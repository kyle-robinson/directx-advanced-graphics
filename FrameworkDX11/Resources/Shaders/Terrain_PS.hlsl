// Resources
Texture2D texBlendMap : register( t0 );
Texture2D texLayerMapArray[5] : register( t2 );

SamplerState smpLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

// Constant Buffers
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

// Pixel Shader
struct DOMAINOUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 TiledTex : TILEDTEX;
};

float4 PS( DOMAINOUT input ) : SV_TARGET
{
	float4 texColor = { 1, 0, 0, 1 };

	// Sample the layers in the texture array
	float4 water = { 0, 0, 1, 1 };
	float4 c0 = texLayerMapArray[0].Sample( smpLinear, input.TiledTex );
	float4 c1 = texLayerMapArray[1].Sample( smpLinear, input.TiledTex );
	float4 c2 = texLayerMapArray[2].Sample( smpLinear, input.TiledTex );
	float4 c3 = texLayerMapArray[3].Sample( smpLinear, input.TiledTex );
	float4 c4 = texLayerMapArray[4].Sample( smpLinear, input.TiledTex );

	bool useBlendMap = false;
	if ( useBlendMap == true )
	{
		// Sample the blend map
		float4 t = texBlendMap.Sample( smpLinear, input.Tex );

		// Blend the texture layers
		texColor = c0;
		texColor = lerp( texColor, c1, t.r );
		texColor = lerp( texColor, c2, t.g );
		texColor = lerp( texColor, c3, t.b );
		texColor = lerp( texColor, c4, t.a );
	}
	else
	{
		// Use the height to generate data
		float blendFactor = 0.0f;
		if ( input.PosW.y < 0.0f )
		{
			texColor = water;
		}
		else if ( input.PosW.y < Layer1MaxHeight )
		{
			blendFactor = smoothstep( 0, Layer1MaxHeight, input.PosW.y );
			texColor = lerp( water, c0, blendFactor );
		}
		else if ( input.PosW.y < Layer2MaxHeight )
		{
			blendFactor = smoothstep( Layer1MaxHeight, Layer2MaxHeight, input.PosW.y );
			texColor = lerp( c0, c1, blendFactor );
		}
		else if ( input.PosW.y < Layer3MaxHeight )
		{
			blendFactor = smoothstep( Layer2MaxHeight, Layer3MaxHeight, input.PosW.y );
			texColor = lerp( c1, c2, blendFactor );
		}
		else if ( input.PosW.y < Layer4MaxHeight )
		{
			blendFactor = smoothstep( Layer3MaxHeight, Layer4MaxHeight, input.PosW.y );
			texColor = lerp( c2, c3, blendFactor );
		}
		else
		{
			blendFactor = smoothstep( Layer4MaxHeight, Layer5MaxHeight, input.PosW.y );
			texColor = lerp( c3, c4, blendFactor );
		}
	}

	float4 finalColor = texColor;
	return finalColor;
}