cbuffer ConstantBuffer : register(b0)
{
	matrix WorldViewProjection;
}

struct VS_Input
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output = (VS_Output)0;
    output.position = mul(float4(input.position,1.0f), WorldViewProjection);
	output.color = input.color;

	return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return input.color;
}
