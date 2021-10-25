Texture2D shaderTexture : register(t0);
SamplerState smplr : register(s0);

struct PS_IN
{
    float4 inPositionSS : SV_Position;
    float3 inNormalLS   : NORMAL;
    float2 inTexCoords  : TEXCOORD;
};

float4 ps_main(PS_IN psIn) : SV_TARGET
{
    float4 textureColor = shaderTexture.Sample(smplr, psIn.inTexCoords);
    return textureColor;
}