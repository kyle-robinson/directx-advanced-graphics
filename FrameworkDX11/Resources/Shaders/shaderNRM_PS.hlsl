// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 PositionV : POS_VIEW;
    float3 Normal : NORMAL;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    // Interpolating normal can unnormalize it, so normalize it. 
    input.Normal = normalize( input.Normal );
    return float4( input.Normal, input.PositionV.z );
}