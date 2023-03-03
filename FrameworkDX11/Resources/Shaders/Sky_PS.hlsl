#pragma pack_matrix( row_major )

// Resources
Texture2D txDiffuse : register( t0 );
SamplerState smpClamp : register( s0 );

// Pixel Shader
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm : NORMAL;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    float3 sampleColor = txDiffuse.Sample( smpClamp, input.Tex );
    return float4( sampleColor, 1.0f );
}