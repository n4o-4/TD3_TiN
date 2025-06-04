#include "PrimitiveDrawer.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    output.color.rgb = input.color.rgb;
    
    output.color.a = 1.0f;
    
    return output;
}