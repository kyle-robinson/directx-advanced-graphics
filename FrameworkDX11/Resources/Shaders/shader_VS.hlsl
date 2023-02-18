// Definitions
#define MAX_LIGHTS 1

// Structs
struct Light
{
    float4 Position;
    float4 Direction;
    float4 Color;

    float SpotAngle;
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;

    float Intensity;
    int LightType;
    bool Enabled;
    float Padding;

    matrix View;
    matrix Projection;
};

// Constant Buffers
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
}

cbuffer LightProperties : register( b1 )
{
    float4 CameraPosition;
    float4 GlobalAmbient;
    Light Lights[MAX_LIGHTS];
};

// Vertex Shader
struct VS_INPUT
{
    float4 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;

    float3 PositionTS : POSITION_TS;
    float4 WorldPosition : WORLD_POSITION;

    float3 EyePosTS : EYE_POSITION_TS;
    float3 EyeVectorsTS : EYE_VECTOR_TS;

    float3 LightVectorTS[MAX_LIGHTS] : LIGHT_VECTORS_TS;
    //float4 LightViewPosition[MAX_LIGHTS] : LIGHT_VIEW_POSITIONS;

    float3 Normal : NORMAL;
    float3 NormalTS : NORMAL_TS;
    //float3 Tangent : TANGENT;
    //float3 Binormal : BINORMAL;
};

float3 VectorToTangentSpace( float3 vectorV, float3x3 TBN_inv )
{
	// Transform from tangent space to world space.
    float3 tangentSpaceNormal = normalize( mul( vectorV, TBN_inv ) );
    return tangentSpaceNormal;
}

VS_OUTPUT VS( VS_INPUT input )
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    output.Position = mul( input.Position, World );
	output.WorldPosition = output.Position;
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );

	// convert from model to world space
	output.Normal = mul( float4( input.Normal, 0.0f ), World ).xyz;
	//output.Tangent = mul( float4( input.Tangent, 1.0f ), World ).xyz;
	//output.Binormal = mul( float4( input.Binormal, 1.0f ), World ).xyz;

    // Build TBN matrix
    float3 T = normalize( mul( input.Tangent, (float3x4)World ) );
    float3 B = normalize( mul( input.Binormal, (float3x4)World ) );
    float3 N = normalize( mul( input.Normal, (float3x4)World ) );
    float3x3 TBN = float3x3( T, B, N );
    float3x3 TBN_inv = transpose( TBN );

    float3 vertexToEye = CameraPosition.xyz - output.WorldPosition.xyz;
    output.EyeVectorsTS = VectorToTangentSpace( vertexToEye.xyz, TBN_inv );

    //float4 LightView;
    for ( int i = 0; i < MAX_LIGHTS; ++i )
    {
        float3 vertexToLight = (float3)Lights[i].Position - output.WorldPosition.xyz;
        output.LightVectorTS[i] = VectorToTangentSpace( vertexToLight.xyz, TBN_inv );

		// Calculate the position of the vertex as viewed by the light source
        //LightView = mul( input.Position, World );
        //LightView = mul( LightView, Lights[i].View );
        //LightView = mul( LightView, Lights[i].Projection );
        //output.LightViewPosition[i] = LightView * float4( 0.5f, -0.5f, 1.0f, 1.0f ) + float4( 0.5f, 0.5f, 0.0f, 0.0f ) * LightView.w;
    }

    output.PositionTS = VectorToTangentSpace( output.WorldPosition.xyz, TBN_inv );
    output.EyePosTS = VectorToTangentSpace( CameraPosition.xyz, TBN_inv );
    output.NormalTS = VectorToTangentSpace( output.Normal.xyz, TBN_inv );

	output.TexCoord = input.TexCoord;
    return output;
}