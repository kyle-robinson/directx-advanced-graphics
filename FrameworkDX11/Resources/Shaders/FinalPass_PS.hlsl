// Resources
Texture2D texDiffuse1 : register( t0 );
Texture2D texDiffuse2 : register( t1 );
SamplerState smpPoint : register( s0 );

// Constant Buffers
cbuffer PostProcessingCB : register( b0 )
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
    float4 vColor = texDiffuse1.Sample( smpPoint, input.Tex );

    if ( UseBloom == 1 )
    {
        float bloomscale = 0.3;
        float4 vColor2 = texDiffuse2.Sample( smpPoint, input.Tex );
        vColor = vColor + ( bloomscale * vColor2 );
    }

    if ( UseColour == 1 )
        vColor = vColor * Color;

    return vColor;
}