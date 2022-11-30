// Constant Buffers
cbuffer ConstantBuffer : register( b0 )
{
    matrix World;
    matrix View;
    matrix Projection;
};

// Vertex Shader
struct VS_INPUT
{
    float4 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 PositionW : POS_WORLD;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
};

VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output;
    input.Position.w = 1.0f;
    output.Position = mul( input.Position, World );
    output.PositionW = output.Position;
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
    output.Normal = mul( float4( input.Normal, 1.0f ), World ).xyz;
    output.Tangent = mul( float4( input.Tangent, 1.0f ), World ).xyz;
    output.TexCoord = input.TexCoord;
    return output;
}