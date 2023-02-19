

Texture2D tex : register(t0);
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
    int pad2;
    int pad3;
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
    float4 vColor = tex.Sample(PointSampler, Input.Tex);

    return vColor;
}
