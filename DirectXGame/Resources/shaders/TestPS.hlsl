#include"Test.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t2 uv = input.texcoord;
    float32_t4 textureColor = gTexture.Sample(gSampler, uv);
    float32_t value = dot(textureColor.rgb, float32_t3(0.2125f, 0.7154f, 0.0721f));

    output.color = float32_t4(value, value, value, textureColor.a);
    return output;
}