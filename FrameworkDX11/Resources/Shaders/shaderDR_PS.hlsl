// Resources
Texture2D textureObj : register( t0 );
SamplerState samplerState : register( s0 );

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    //float4 PositionV : POS_VIEW;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    //float4 Position : SV_TARGET0;
    float4 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
};

PS_OUTPUT PS( PS_INPUT input )
{
    PS_OUTPUT output;
    
    //output.Position = input.Position;
    //output.NormalD.w = input.PositionV.z / input.PositionV.w;
    
    output.Albedo = textureObj.Sample( samplerState, input.TexCoord );
    //output.Albedo = float4( 1.0f, 0.5f, 0.2, 1.0f );
    output.Normal = float4( normalize( input.Normal ), 1.0f );
    //output.Normal = normalize( input.Normal );
    
    return output;
}