#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    // ���͂�0�ɁA���S�ɂȂ�قǖ��邭�Ȃ�悤�Ɍv�Z�Œ���
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    
    // correct�������ƈÂ�����̂Œ���
    float vignette = correct.x * correct.y * 16.0f;
    
    vignette = saturate(pow(vignette, 0.8f));
    
    // �W���Ƃ��ď�Z
    output.color.rgb *= vignette;
    
    return output;
}