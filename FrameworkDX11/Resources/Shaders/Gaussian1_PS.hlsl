

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

QuadVS_Output QuadVS(QuadVS_Input Input)
{
    // no mvp transform - model coordinates already in projection space (-1 to 1)
    QuadVS_Output Output;
    Output.Pos = Input.Pos;
    Output.Tex = Input.Tex;

    // Determine the floating point size of a texel for a screen with this specific width.
   float texelSize = 1.0f / 1280;

    // Create UV coordinates for the pixel and its four horizontal neighbors on either side.
    Output.texCoord1 = Input.Tex + float2(texelSize * -4.0f, 0.0f);
    Output.texCoord2 = Input.Tex + float2(texelSize * -3.0f, 0.0f);
    Output.texCoord3 = Input.Tex + float2(texelSize * -2.0f, 0.0f);
    Output.texCoord4 = Input.Tex + float2(texelSize * -1.0f, 0.0f);
    Output.texCoord5 = Input.Tex + float2(texelSize * 0.0f, 0.0f);
    Output.texCoord6 = Input.Tex + float2(texelSize * 1.0f, 0.0f);
    Output.texCoord7 = Input.Tex + float2(texelSize * 2.0f, 0.0f);
    Output.texCoord8 = Input.Tex + float2(texelSize * 3.0f, 0.0f);
    Output.texCoord9 = Input.Tex + float2(texelSize * 4.0f, 0.0f);


    return Output;
}

//Pixel shader
float4 QuadPS(QuadVS_Output Input) : SV_TARGET
{
    float4 vColor = tex.Sample(PointSampler, Input.Tex);

   
          //pass 1
          float weight0, weight1, weight2, weight3, weight4;
          float normalization;


          // Create the weights that each neighbor pixel will contribute to the blur.
          weight0 = 1.0f;
          weight1 = 0.9f;
          weight2 = 0.55f;
          weight3 = 0.18f;
          weight4 = 0.1f;

          // Create a normalized value to average the weights out a bit.
          normalization = (weight0 + 2.0f * (weight1 + weight2 + weight3 + weight4));

          // Normalize the weights.
          weight0 = weight0 / normalization;
          weight1 = weight1 / normalization;
          weight2 = weight2 / normalization;
          weight3 = weight3 / normalization;
          weight4 = weight4 / normalization;

          vColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

          // Add the nine horizontal pixels to the color by the specific weight of each.
          vColor += tex.Sample(PointSampler, Input.texCoord1) * weight4;
          vColor += tex.Sample(PointSampler, Input.texCoord2) * weight3;
          vColor += tex.Sample(PointSampler, Input.texCoord3) * weight2;
          vColor += tex.Sample(PointSampler, Input.texCoord4) * weight1;
          vColor += tex.Sample(PointSampler, Input.texCoord5) * weight0;
          vColor += tex.Sample(PointSampler, Input.texCoord6) * weight1;
          vColor += tex.Sample(PointSampler, Input.texCoord7) * weight2;
          vColor += tex.Sample(PointSampler, Input.texCoord8) * weight3;
          vColor += tex.Sample(PointSampler, Input.texCoord9) * weight4;

          
      
     

  
    return vColor;
}
