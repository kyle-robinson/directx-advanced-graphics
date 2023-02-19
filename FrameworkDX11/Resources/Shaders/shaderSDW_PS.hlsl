// Resources
Texture2D textureDiffuse : register( t0 );
Texture2D textureNormal : register( t1 );
Texture2D textureDisplacement : register( t2 );

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPos : WORLD_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 DepthPos : TEXTURE0;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    float4 texColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    float depth = input.DepthPos.z / input.DepthPos.w;
    texColor = float4( depth, depth, depth, 1.0f );
    return texColor;
}