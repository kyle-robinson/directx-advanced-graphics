// Resources
Texture2D textureQuad : register( t0 );
Texture2D textureDepth : register( t1 );
Texture2D textureNormalDepth : register( t2 );
Texture2D textureRandom : register( t3 );

SamplerState samplerWrap : register( s0 );
SamplerState samplerClamp : register( s1 );

// Structs
struct _MotionBlur
{
    matrix ViewProjectionInverseMatrix;
    matrix PreviousViewProjectionMatrix;

    bool UseMotionBlur;
    int NumSamples;
    float2 Padding;
};

struct _FXAA
{
    float2 TextureSizeInverse;
    float SpanMax;
    float ReduceMin;

    float ReduceMul;
    bool UseFXAA;
    float2 Padding;
};

struct _SSAO
{
    matrix ProjectionMatrix;

    float2 ScreenSize;
    float2 NoiseScale;

    float Radius;
    float Power;
    int KernelSize;
    bool UseSSAO;

    float3 KernelOffsets[16];
    float Padding;
};

// Constant Buffers
cbuffer MotionBlurData : register( b0 )
{
    _MotionBlur MotionBlur;
}

cbuffer FXAAData : register( b1 )
{
    _FXAA FXAA;
}

cbuffer SSAOData : register( b2 )
{
    _SSAO SSAO;
}

// Math Functions
float3 DecodeSphereMap( float2 e )
{
    float2 tmp = e - e * e;
    float f = tmp.x + tmp.y;
    float m = sqrt( 4.0f * f - 1.0f );

    float3 n;
    n.xy = m * ( e * 4.0f - 2.0f );
    n.z = 3.0f - 8.0f * f;
    return n;
}

float3 ComputePositionViewFromZ( uint2 coords, float zbuffer )
{
    float2 screenPixelOffset = float2( 2.0f, -2.0f ) / float2( SSAO.ScreenSize.x, SSAO.ScreenSize.y );
    float2 positionScreen = ( float2( coords.xy ) + 0.5f ) * screenPixelOffset.xy + float2( -1.0f, 1.0f );
    float viewSpaceZ = SSAO.ProjectionMatrix._43 / ( zbuffer - SSAO.ProjectionMatrix._33 );

    float2 screenSpaceRay = float2( positionScreen.x / SSAO.ProjectionMatrix._11, positionScreen.y / SSAO.ProjectionMatrix._22 );
    float3 positionView;
    positionView.z = viewSpaceZ;
    positionView.xy = screenSpaceRay.xy * positionView.z;

    return positionView;
}

// Effects Functions
float4 DoMotionBlur( float2 texCoord )
{
    // Get the depth buffer value at this pixel.
    float zOverW = textureDepth.Sample( samplerWrap, texCoord );
    // H is the viewport position at this pixel in the range -1 to 1.
    float4 H = float4( texCoord.x * 2 - 1, ( 1 - texCoord.y ) * 2 - 1, zOverW, 1 );
    // Transform by the view-projection inverse.
    float4 D = mul( H, MotionBlur.ViewProjectionInverseMatrix );
    // Divide by w to get the world position.
    float4 worldPos = D / D.w;

    // Current viewport position
    float4 currentPos = H;
    // Use the world position, and transform by the previous view-    // projection matrix.
    float4 previousPos = mul( worldPos, MotionBlur.PreviousViewProjectionMatrix );
    // Convert to nonhomogeneous points [-1,1] by dividing by w.
    previousPos /= previousPos.w;
    // Use this frame's position and last frame's to compute the pixel    // velocity.
    float2 velocity = ( currentPos - previousPos ) / 8.f;

    // Get the initial color at this pixel.
    float4 color = textureQuad.Sample( samplerWrap, texCoord );
    texCoord += velocity;

    for ( int i = 1; i < MotionBlur.NumSamples; ++i )
    {
        texCoord += velocity;

        // Sample the color buffer along the velocity vector.
        float4 currentColor = textureQuad.Sample( samplerWrap, texCoord );

        // Add the current color to our color sum.
        color += currentColor;
    }

    // Average all of the samples to get the final blur color.
    return color / MotionBlur.NumSamples;
}

float4 DoFXAA( float2 texCoord )
{
    float3 luma = { 0.299f, 0.587f, 0.114f };
    float lumaTL = dot( luma, (float3)textureQuad.Sample( samplerWrap, texCoord + ( float2( -1.0f,  1.0f ) * FXAA.TextureSizeInverse ) ) );
    float lumaTR = dot( luma, (float3)textureQuad.Sample( samplerWrap, texCoord + ( float2(  1.0f,  1.0f ) * FXAA.TextureSizeInverse ) ) );
    float lumaBL = dot( luma, (float3)textureQuad.Sample( samplerWrap, texCoord + ( float2( -1.0f, -1.0f ) * FXAA.TextureSizeInverse ) ) );
    float lumaBR = dot( luma, (float3)textureQuad.Sample( samplerWrap, texCoord + ( float2(  1.0f, -1.0f ) * FXAA.TextureSizeInverse ) ) );
    float lumaM = dot( luma, (float3)textureQuad.Sample( samplerWrap, texCoord ) );

    float2 dir;
	dir.x = ( ( lumaTL + lumaTR ) - ( lumaBL + lumaBR ) );
	dir.y = ( ( lumaTL + lumaBL ) - ( lumaTR + lumaBR ) );

    float dirReduce = max( ( lumaTL + lumaTR + lumaBL + lumaBR ) * ( FXAA.ReduceMul * 0.25f ), FXAA.ReduceMin );
	float inverseDirAdjustment = 1.0f / ( min( abs( dir.x ), abs( dir.y ) ) + dirReduce );

	dir.x = min( float2( FXAA.SpanMax, FXAA.SpanMax ), max( float2( -FXAA.SpanMax, -FXAA.SpanMax ), dir * inverseDirAdjustment ) ) * FXAA.TextureSizeInverse;
	dir.y = min( float2( FXAA.SpanMax, FXAA.SpanMax ), max( float2( -FXAA.SpanMax, -FXAA.SpanMax ), dir * inverseDirAdjustment ) ) * FXAA.TextureSizeInverse;

	float3 result1 = ( 1.0f / 2.0f ) * (
		textureQuad.Sample( samplerWrap, texCoord + ( dir * float2( 1.0f / 3.0f - 0.5f, 1.0f / 3.0f - 0.5f ) ) ) +
		textureQuad.Sample( samplerWrap, texCoord + ( dir * float2( 2.0f / 3.0f - 0.5f, 2.0f / 3.0f - 0.5f ) ) ) );

	float3 result2 = result1 * ( 1.0f / 2.0f ) + ( 1.0f / 4.0f ) * (
		(float3)textureQuad.Sample( samplerWrap, texCoord + ( dir * float2( 0.0f / 3.0f - 0.5f, 0.0f / 3.0f - 0.5f ) ) ) +
		(float3)textureQuad.Sample( samplerWrap, texCoord + ( dir * float2( 3.0f / 3.0f - 0.5f, 3.0f / 3.0f - 0.5f ) ) ) );

	float lumaMin = min( lumaM, min( min( lumaTL, lumaTR ), min( lumaBL, lumaBR ) ) );
	float lumaMax = max( lumaM, max( max( lumaTL, lumaTR ), max( lumaBL, lumaBR ) ) );
	float lumaResult2 = dot( luma, result2 );
	
	if ( lumaResult2 < lumaMin || lumaResult2 > lumaMax )
		return float4( result1.x, result1.y, result1.z, 1.0f );
	else
		return float4( result2.x, result2.y, result2.z, 1.0f );
}

float4 DoSSAO( float2 texCoord, float4 posView )
{
    float centerZBuffer = textureNormalDepth.Sample( samplerClamp, texCoord ).w;
    float3 centerDepthPos = ComputePositionViewFromZ( uint2( posView.xy ), centerZBuffer );
    float3 normal = DecodeSphereMap( textureNormalDepth.Sample( samplerClamp, texCoord ).xy );

    float3 randomVector = textureRandom.Sample( samplerWrap, texCoord * SSAO.NoiseScale ).xyz;
    float3 tangent = normalize( randomVector - normal * dot( randomVector, normal ) );
    float3 bitangent = cross( normal, tangent );
    float3x3 transformMat = float3x3( tangent, bitangent, normal );

    float occlusion = 0.0f;
    for ( int i = 0; i < SSAO.KernelSize; ++i )
    {
        float3 samplePos = mul( SSAO.KernelOffsets[i], transformMat );
        samplePos = samplePos * SSAO.Radius + centerDepthPos;

        float3 sampleDir = normalize( samplePos - centerDepthPos );

        float nDotS = max( dot( normal, sampleDir ), 0.0f );

        float4 offset = mul( float4( samplePos, 1.0f ), SSAO.ProjectionMatrix );
        offset.xy /= offset.w;

        float sampleDepth = textureNormalDepth.Sample( samplerClamp, float2( offset.x * 0.5f + 0.5f, -offset.y * 0.5f + 0.5f ) ).w;
        sampleDepth = ComputePositionViewFromZ( offset.xy, sampleDepth ).z;

        float rangeCheck = smoothstep( 0.0f, 1.0f, SSAO.Radius / abs( centerDepthPos.z - sampleDepth ) );
        occlusion += rangeCheck * step( sampleDepth, samplePos.z ) * nDotS;
    }

    occlusion = 1.0f - ( occlusion / SSAO.KernelSize );
    float finalOcclusion = pow( occlusion, SSAO.Power );
    return float4( finalOcclusion, finalOcclusion, finalOcclusion, 1.0f );
}

// Pixel Shader
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 PositionV : POS_VIEW;
    float2 TexCoord : TEXCOORD;
};

float4 PS( PS_INPUT input ) : SV_TARGET
{
    if ( MotionBlur.UseMotionBlur )
        return DoMotionBlur( input.TexCoord );

    if ( FXAA.UseFXAA )
        return DoFXAA( input.TexCoord );

    float4 ssaoAmount = { 1.0f, 1.0f, 1.0f, 1.0f };
    if ( SSAO.UseSSAO )
        ssaoAmount = DoSSAO( input.TexCoord, input.PositionV );
    
    // Sample from render target texture
    return saturate( textureQuad.Sample( samplerWrap, input.TexCoord ).rgba * ssaoAmount );
}