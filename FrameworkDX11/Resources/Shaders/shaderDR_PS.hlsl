// Resources
Texture2D textureObj : register( t0 );
Texture2D textureNrm : register( t1 );
SamplerState samplerState : register( s0 );

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 PositionW : POS_WORLD;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct PS_OUTPUT
{
    float4 Position : SV_TARGET0;
    float4 Albedo : SV_TARGET1;
    float4 Normal : SV_TARGET2;
    float4 Tangent : SV_TARGET3;
    float4 Binormal : SV_TARGET4;
    float4 NormalMap : SV_TARGET5;
};

PS_OUTPUT PS( PS_INPUT input )
{
    PS_OUTPUT output;
    output.Position = input.PositionW;
    output.Albedo = textureObj.Sample( samplerState, input.TexCoord ); // TODO : Maybe try sending back texcoords instead of texture, and do the texture later in the other shader
    output.Normal = float4( normalize( input.Normal ), 1.0f );
    output.Tangent = float4( normalize( input.Tangent ), 1.0f );
    output.Binormal = float4( normalize( input.Binormal ), 1.0f );
    output.NormalMap = textureNrm.Sample( samplerState, input.TexCoord );
    return output;
}