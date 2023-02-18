// Resources
Texture2D textureObj : register( t0 );
Texture2D textureNrm : register( t1 );
Texture2D textureDisp : register( t2 );
SamplerState samplerState : register( s0 );

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 PositionW : POS_WORLD;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
};

struct PS_OUTPUT
{
    float4 Position : SV_TARGET0;
    float4 Albedo : SV_TARGET1;
    float4 Normal : SV_TARGET2;
};

PS_OUTPUT PS( PS_INPUT input )
{
    PS_OUTPUT output;

    output.Position = input.PositionW;
    output.Albedo = textureObj.Sample( samplerState, input.TexCoord );

    float3 N = input.Normal;
    float3 T = normalize( input.Tangent - N  * dot( input.Tangent, N ) );
    float3 B = cross( T, N );
    float3x3 TBN = float3x3( T, B, N );

    float3 normalFromMap = textureNrm.Sample( samplerState, input.TexCoord ) * 2.0f - 1.0f;
    normalFromMap.y = -normalFromMap.y;
    input.Normal = normalize( mul( normalFromMap, TBN ) );
    output.Normal = float4( input.Normal, 1.0f );

    return output;
}