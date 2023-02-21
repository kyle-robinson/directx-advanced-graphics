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
    float2 texCoord1 : TEXCOORD1;
    float2 texCoord2 : TEXCOORD2;
    float2 texCoord3 : TEXCOORD3;
    float2 texCoord4 : TEXCOORD4;
    float2 texCoord5 : TEXCOORD5;
    float2 texCoord6 : TEXCOORD6;
    float2 texCoord7 : TEXCOORD7;
    float2 texCoord8 : TEXCOORD8;
    float2 texCoord9 : TEXCOORD9;
};

VERTEX_OUT VS( VERTEX_IN input )
{
    VERTEX_OUT output;
    output.Pos = input.Pos;
    output.Tex = input.Tex;

    // Get texel size for screen of this width
    float texelSize = 1.0f / 1280;

    // Create UV coordinates for the pixel and its four horizontal neighbors on either side
    output.texCoord1 = input.Tex + float2( texelSize * -4.0f, 0.0f );
    output.texCoord2 = input.Tex + float2( texelSize * -3.0f, 0.0f );
    output.texCoord3 = input.Tex + float2( texelSize * -2.0f, 0.0f );
    output.texCoord4 = input.Tex + float2( texelSize * -1.0f, 0.0f );
    output.texCoord5 = input.Tex + float2( texelSize * 0.0f, 0.0f );
    output.texCoord6 = input.Tex + float2( texelSize * 1.0f, 0.0f );
    output.texCoord7 = input.Tex + float2( texelSize * 2.0f, 0.0f );
    output.texCoord8 = input.Tex + float2( texelSize * 3.0f, 0.0f );
    output.texCoord9 = input.Tex + float2( texelSize * 4.0f, 0.0f );

    return output;
}