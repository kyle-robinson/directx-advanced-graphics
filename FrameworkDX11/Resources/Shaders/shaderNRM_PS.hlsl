// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    // Interpolating normal can unnormalize it, so normalize it. 
    input.Normal = normalize( input.Normal );
    return float4( input.Normal, 1.0f );
}