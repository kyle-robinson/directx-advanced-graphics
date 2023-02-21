// Vertex Shader
struct VERTEX_IN 
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct VERTEX_OUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VERTEX_OUT VS( VERTEX_IN input )
{
    VERTEX_OUT output;
    output.Pos = input.Pos;
    output.Tex = input.Tex;

    // Get texel size for screen of this width
    float texelSize = 1.0f / 1280.0f;
    return output;
}