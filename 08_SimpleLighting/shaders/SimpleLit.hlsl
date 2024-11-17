cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ModelViewProjection;
}
cbuffer LightBuffer : register(b0)
{
    float3 position;
    float4 color;
}

struct VS_Input
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float3 worldpos : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output = (VS_Output) 0;

    output.worldpos = mul(float4(input.position, 1.0f), World);
    output.position = mul(float4(input.position, 1.0f), ModelViewProjection);
    output.color = input.color;
    output.normal = normalize(mul(input.normal, (float3x3) World));

    return output;
}

// Simple Ambient + diffuse lighting model
float4 ps_main(VS_Output input) : SV_TARGET
{
    float4 minColor = (0.0, 0.0, 0.0, 0.0);
    float4 maxColor = (1.0, 1.0, 1.0, 1.0);
    float4 ambient = input.color * .1;
    float3 lightPosition = position;
    float3 l = normalize(lightPosition - input.worldpos);
    float ndotl = max(dot(l, input.normal), 0.0f);
    float4 lambertCont = input.color * ndotl;
    return clamp(lambertCont + ambient, minColor, maxColor);
}