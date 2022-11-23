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
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    //float4 PositionV : POS_VIEW;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output;
    
    input.Position.w = 1.0f;
    output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
    
    //output.PositionV = output.Position;
    
    output.Normal = mul( input.Normal, (float3x3)World );
    //output.Normal = mul( float4( input.Normal, 1.0f ), World );
    output.TexCoord = input.TexCoord;
    
    return output;
}