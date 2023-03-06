#pragma pack_matrix( row_major )

// Constant Buffers
cbuffer MatrixBuffer : register( b0 )
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vOutputColor;
};

// Vertex Shader
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
    float3 Tangent : tangent;
    float3 BiTangent : biTangent;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm : NORMAL;
};

VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output;

    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

    output.Norm = normalize( mul( float4( input.Norm, 1.0f ), World ) );
    //input.Tex /= 4.0f;
    output.Tex = input.Tex;

    return output;
}