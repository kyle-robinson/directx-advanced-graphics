// Constant Buffers
cbuffer ConstantBuffer : register( b0 )
{
    matrix gWorld;
    matrix gWorldInvTranspose;
    matrix gWorldViewProj;
    matrix gTexTransform;
    matrix gShadowTransform;
}

// Vertex Shader
struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to world space space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);

    // Transform to homogeneous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

    // Output vertex attributes for interpolation across triangle.
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);
    return vout;
}