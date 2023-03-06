// Resources
Texture2D texDiffuse : register( t0 );
SamplerState smpPoint : register( s0 );

// Pixel Shader
struct VERTEX_OUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float2 texCoord1 : TEXCOORD1;
    float2 texCoord2 : TEXCOORD2;
    float2 texCoord3 : TEXCOORD3;
    float2 texCoord4 : TEXCOORD4;
    float2 texCoord5 : TEXCOORD5;
    float2 texCoord6 : TEXCOORD6;
    float2 texCoord7 : TEXCOORD7;
    float2 texCoord8 : TEXCOORD8;
    float2 texCoord9 : TEXCOORD9;
};

float4 PS( VERTEX_OUT input ) : SV_TARGET
{
    float4 vColor = texDiffuse.Sample( smpPoint, input.Tex );

    // Create the weights that each neighbor pixel will contribute to the blur
    float weight0 = 1.0f;
    float weight1 = 0.9f;
    float weight2 = 0.55f;
    float weight3 = 0.18f;
    float weight4 = 0.1f;

    // Create a normalized value to average the weights out a bit
    float normalization = ( weight0 + 2.0f * ( weight1 + weight2 + weight3 + weight4 ) );

    // Normalize the weights.
    weight0 = weight0 / normalization;
    weight1 = weight1 / normalization;
    weight2 = weight2 / normalization;
    weight3 = weight3 / normalization;
    weight4 = weight4 / normalization;

    // Add the nine horizontal pixels to the color by the specific weight of each
    vColor = float4( 0.0f, 0.0f, 0.0f, 0.0f );
    vColor += texDiffuse.Sample( smpPoint, input.texCoord1 ) * weight4;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord2 ) * weight3;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord3 ) * weight2;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord4 ) * weight1;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord5 ) * weight0;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord6 ) * weight1;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord7 ) * weight2;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord8 ) * weight3;
    vColor += texDiffuse.Sample( smpPoint, input.texCoord9 ) * weight4;
    
    return vColor;
}