#include "Fullscreen.hlsli"

struct Material
{
    float3 edgeColor;
    float Threshold;
    float ThresholdWidth;
};

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gMaskTexture : register(t1);  
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    
    // maskの値が閾値以下の場合はdiscard
    if (mask < gMaterial.Threshold)
    {
        discard;
    }
    
    float edge = 1.0f - smoothstep(gMaterial.Threshold, gMaterial.Threshold + gMaterial.ThresholdWidth, mask);
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    // Edgeっぽい指定した色を加算
    output.color.rgb += edge * gMaterial.edgeColor;
    
    return output;
}