

Texture2D tex : register(t0);
Texture2D Bluertex : register(t1);
Texture2D txDepth : register(t2);

SamplerState PointSampler : register(s0);


cbuffer PostProcessingCB: register(b0)
{
    float4  Color;
    //----------
    int UseHDR;
    int UseBloom;
    int UseDepthOfF;
    int UseColour;
    //------------
    int UseBlur;
    float fadeAmount;
    float FarPlane;
    float focalwidth;
    //--------------
    float focalDistance;
    float blerAttenuation;
    int pad1;
    int pad2;
};

struct QuadVS_Input
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
 
};

QuadVS_Output QuadVS(QuadVS_Input Input)
{
    // no mvp transform - model coordinates already in projection space (-1 to 1)
    QuadVS_Output Output;
    Output.Pos = Input.Pos;
    Output.Tex = Input.Tex;

    return Output;
}

//Pixel shader
float4 QuadPS(QuadVS_Output Input) : SV_TARGET
{
    float4 Colour = { 0, 0, 0, 1 };
    float4 vColor = tex.Sample(PointSampler, Input.Tex);
    float4 blurTex = Bluertex.Sample(PointSampler, Input.Tex);

    float4 depth = { 0, 0, 1, 1 };
    depth = txDepth.Sample(PointSampler, Input.Tex);

    //find if depth is at focal point transition
    float zDepth = smoothstep(0, focalwidth, abs(focalDistance - (depth.z * FarPlane)));
    
  

   //blend between bluer immage and non bluered immage
    Colour= lerp(vColor, blurTex, saturate(zDepth) * blerAttenuation);
    
    return Colour;
}
