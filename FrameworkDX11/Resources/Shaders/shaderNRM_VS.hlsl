// Constant Buffers
cbuffer ConstantBuffer : register( b0 )
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4x4 WorldInvTransposeView;
};

// Vertex Shader
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
};

VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output;
    output.Position = float4( mul( float4( input.Position, 1.0f ), World ).xyz, 1.0f );
    output.Position = float4( mul( float4( output.Position.xyz, 1.0f ), View ).xyz, 1.0f );
    output.Position = float4( mul( float4( output.Position.xyz, 1.0f ), Projection ).xyz, 1.0f );
    output.Normal = mul( input.Normal, (float3x3)WorldInvTransposeView );
    return output;
}