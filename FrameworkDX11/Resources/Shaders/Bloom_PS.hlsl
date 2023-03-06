// Resources
Texture2D texDiffuse : register( t0 );
SamplerState smpPoint : register( s0 );

// Pixel Shader
struct VERTEX_OUT 
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 PS( VERTEX_OUT input ) : SV_TARGET
{
    float4 vColor = texDiffuse.Sample( smpPoint, input.Tex );
    if ( vColor.x <= 1 || vColor.y <= 1|| vColor.z <= 1|| vColor.a <= 1 )
        vColor = 0;
    return vColor;
}