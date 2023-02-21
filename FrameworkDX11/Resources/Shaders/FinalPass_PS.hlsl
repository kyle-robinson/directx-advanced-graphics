

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
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

    // Determine the floating point size of a texel for a screen with this specific width.
   float texelSize = 1.0f / 1280;

   


    return Output;
}

//Pixel shader
float4 QuadPS(QuadVS_Output Input) : SV_TARGET
{
    float4 vColor = tex.Sample(PointSampler, Input.Tex);
    
    if (UseBloom == 1) {
        float bloomscale = 0.3;
       float4 vColor2 = tex2.Sample(PointSampler, Input.Tex);
       vColor = vColor+ (bloomscale *vColor2);
       

    }
    if (UseColour == 1) {
        vColor = vColor * Color;

    }
  
    
    return vColor;
}
