// Constant Buffers
cbuffer MatrixBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
    float4 CameraPosition;
}

// Geometry Shader
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 WorldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};

[maxvertexcount( 4 )]
void GS( point PS_INPUT input[1], inout TriangleStream<PS_INPUT> outputStream )
{
    float3 planeNormal = input[0].WorldPos - CameraPosition;
	planeNormal.y = 0.0f;
	planeNormal = normalize( planeNormal );

	float3 upVector = float3( 0.0f, 1.0f, 0.0f );
	float3 rightVector = normalize( cross( planeNormal, upVector ) );

	// Create the billboard quad
	float3 vert[4];
	vert[0] = (float3)input[0].WorldPos - rightVector;
	vert[1] = (float3)input[0].WorldPos + rightVector;
	vert[2] = (float3)input[0].WorldPos - rightVector + upVector;
	vert[3] = (float3)input[0].WorldPos + rightVector + upVector;

	// Get the billboard texture coordinates
	float2 texCoord[4];
	texCoord[0] = float2( 0.0f, 1.0f );
	texCoord[1] = float2( 1.0f, 1.0f );
	texCoord[2] = float2( 0.0f, 0.0f );
	texCoord[3] = float2( 1.0f, 0.0f );

	PS_INPUT outputvert;
	for ( int i = 0; i < 4; i++ )
	{
		outputvert.Pos = mul( float4(vert[i], 1.0f), View );
		outputvert.Pos = mul( outputvert.Pos, Projection );
		outputvert.WorldPos = float4( vert[i], 0.0f );
		outputvert.Tex = texCoord[i];
		outputvert.Norm = float3( 0.0f, 0.0f, 0.0f );
		outputStream.Append( outputvert );
	}
}