// Resources
Texture2D texDiffuse : register( t0 );
SamplerState smpPoint : register( s0 );

// Pixel Shader
struct VERTEX_OUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

//Pixel shader
float4 PS( VERTEX_OUT input ) : SV_TARGET
{
    float4 vColor = texDiffuse.Sample( smpPoint, input.Tex );
    return vColor;
}