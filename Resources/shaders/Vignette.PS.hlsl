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
    
    // üˆÍ‚ğ0‚ÉA’†S‚É‚È‚é‚Ù‚Ç–¾‚é‚­‚È‚é‚æ‚¤‚ÉŒvZ‚Å’²®
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    
    // correct‚¾‚¯‚¾‚ÆˆÃ‚·‚¬‚é‚Ì‚Å’²®
    float vignette = correct.x * correct.y * 16.0f;
    
    vignette = saturate(pow(vignette, 0.8f));
    
    // ŒW”‚Æ‚µ‚ÄæZ
    output.color.rgb *= vignette;
    
    return output;
}