// Vertex Shader
struct VS_INPUT
{
    float2 Position : POSITION;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT VS( VS_INPUT input )
{   
    VS_OUTPUT output;
    output.Position = float4( input.Position, 0.0f, 1.0f );
    output.TexCoord = float2( ( input.Position.x + 1 ) / 2.0f, -( input.Position.y - 1 ) / 2.0f );
    return output;
}

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

Texture2DMS<float4, 2> textureQuad : register( t0 );
SamplerState samplerState : register( s0 );

float4 PS( PS_INPUT input ) : SV_TARGET
{
    //return saturate( textureQuad.Sample( samplerState, input.TexCoord ).rgba );
    
    float4 textureColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    [unroll]
    for ( int i = 0; i < 4; i++ )
        textureColor += textureQuad.Load( input.TexCoord, i );
    textureColor = textureColor / 4;
    return textureColor;
    
    //return textureQuad.Load( input.TexCoord, 0 );
}