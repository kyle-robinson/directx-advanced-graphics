// Resources
Texture2D texDiffuse : register( t0 );
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

float4 PS( VERTEX_OUT input ) : SV_TARGET
{
    float4 vColor = texDiffuse.Sample( smpPoint, input.Tex );
    // Reduce the color brightness to the current fade percentage
    vColor = vColor * FadeAmount;
    return vColor;
}