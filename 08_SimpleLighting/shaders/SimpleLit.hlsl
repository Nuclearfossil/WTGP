cbuffer ConstantBuffer : register(b0)
{
    matrix ModelView;
    matrix ModelViewProjection;
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

    output.worldpos = float3(input.position);
    output.position = mul(float4(input.position, 1.0f), ModelViewProjection);
    output.color = input.color;
    output.normal = normalize(mul(input.normal, (float3x3) ModelView));

    return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return input.color;
}