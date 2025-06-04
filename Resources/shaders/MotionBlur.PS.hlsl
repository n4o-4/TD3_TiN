//#include "Fullscreen.hlsli"

//struct Material
//{
//    int numSamples;
//    float2 center;
//    float blurWidth;
//    float3 diff;
//};
   
//Texture2D<float4> gTexture : register(t0);
//SamplerState gSamplerLinear : register(s0);
//ConstantBuffer<Material> gMaterial : register(b0);

//struct PixelShaderOutput
//{
//    float4 color : SV_TARGET0;
//};

//PixelShaderOutput main(VertexShaderOutput input)
//{ 
//    // `diff` �𓮂��̕����Ƌ����Ƃ��ė��p
//    // `diff` �͑O�t���[���ƌ��݃t���[���̍����ŁA�J�����╨�̂̓����������x�N�g��
//    float2 direction = gMaterial.diff.xz; // x, y �̍������ړ������Ɏg�p

//    // �F�̏�����
//    float3 outputColor = float3(0.0f, 0.0f, 0.0f);

//    // �����̕����ɉ����ĕ����̃T���v�������
//    for (int sampleIndex = 0; sampleIndex < gMaterial.numSamples; ++sampleIndex)
//    {
//        // ���݂̃e�N�X�`�����W�ɁA�����̕����ɉ����ăT���v���_��i�߂�
//        float2 texcoord = input.texcoord + direction * gMaterial.blurWidth * float(sampleIndex);

//        // �T���v�������
//        outputColor.rgb += gTexture.Sample(gSamplerLinear, texcoord).rgb;
//    }

//    // �T���v���̕��ω�
//    outputColor.rgb *= rcp(gMaterial.numSamples);

//    // �o�͐F��ݒ�
//    PixelShaderOutput output;
//    output.color.rgb = outputColor;
//    output.color.a = 1.0f; // �s�����x��1�ɐݒ�

//    return output;
//}

#include "Fullscreen.hlsli"

struct Material
{
    int numSamples;
    float2 center; // ���S���W (�ʏ�: float2(0.5, 0.5))
    float blurWidth; // �u���[�̕�
    float3 diff; // �����̕���
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
    float2 uv = input.texcoord;

    // ���S����̋����Ńu���[���x������
    float dist = distance(uv, gMaterial.center);
    
    float innerRadius = 0.3f; // ����������̓u���[�Ȃ�
    float outerRadius = 1.0f; // ������O�͍ő�u���[

    float vignette = saturate((dist - innerRadius) / (outerRadius - innerRadius));
    
    // motion blur �̕���
    float2 direction = gMaterial.diff.xz;

    // motion blur �p�u���[�T���v�����O
    float3 blurredColor = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < gMaterial.numSamples; ++i)
    {
        float2 offset = direction * gMaterial.blurWidth * float(i);
        blurredColor += gTexture.Sample(gSamplerLinear, uv + offset).rgb;
    }
    blurredColor *= rcp(gMaterial.numSamples);

    // ���̉摜�i���S���Ɏg���j
    float3 originalColor = gTexture.Sample(gSamplerLinear, uv).rgb;

    // �u�����h
    float3 finalColor = lerp(originalColor, blurredColor, vignette);

    PixelShaderOutput output;
    output.color = float4(finalColor, 1.0f);
    return output;
}