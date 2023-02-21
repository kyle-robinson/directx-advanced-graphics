// Resources
Texture2D texDiffuse : register( t0 );
Texture2D texBlur : register( t1 );
Texture2D texDepth : register( t2 );
SamplerState smpPoint : register( s0 );

// Constant Buffers
cbuffer PostProcessingCB: register( b0 )
{
    float4 Color;
    
    int UseHDR;
    int UseBloom;
    int UseDepthOfField;
    int UseColour;
    
    int UseBlur;
    float FadeAmount;
    float FarPlane;
    float FocalWidth;
    
    float FocalDistance;
    float BlurAttenuation;
    int Padding1;
    int Padding2;
};

// Pixel Shader
struct VERTEX_OUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//Pixel shader
float4 PS( VERTEX_OUT input ) : SV_TARGET
{
    float4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
    float4 vColor = texDiffuse.Sample( smpPoint, input.Tex );
    float4 blurTex = texBlur.Sample( smpPoint, input.Tex );

    float4 depth = { 0, 0, 1, 1 };
    depth = texDepth.Sample( smpPoint, input.Tex );

    // Find if depth is at focal point transition
    float zDepth = smoothstep( 0, FocalWidth, abs( FocalDistance - ( depth.z * FarPlane ) ) );

    // Blend between blurred and non-blurred images
    color = lerp( vColor, blurTex, saturate( zDepth ) * BlurAttenuation );
    
    return color;
}