#include "Fullscreen.hlsli"

struct Material
{
    int numSamples;
    float2 center;
    float blurWidth;
};
   
    

Texture2D<float4> gTexture : register(t0);
SamplerState gSamplerLinear : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{ 
    // ���K��������������艓�����T���v�����O����
    float2 direction = input.texcoord - gMaterial.center;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    for (int sampleIndex = 0; sampleIndex < gMaterial.numSamples;++sampleIndex)
    {
        // ���݂�uv�����قǌv�Z���������ɃT���v���_��i�߂Ȃ���T���v�����O���Ă���
        float2 texcoord = input.texcoord + direction * gMaterial.blurWidth * float(sampleIndex);

        outputColor.rgb += gTexture.Sample(gSamplerLinear, texcoord).rgb;
    }
    // ���ω�����
    outputColor.rgb *= rcp(gMaterial.numSamples);
    
    PixelShaderOutput output;
    output.color.rgb = outputColor;
    output.color.a = 1.0f;
    
    return output;
}