struct VS_Input
{
	float2 pos : POS;
	float4 color : COL;
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float4 color : COL;
};
 
VS_Output main(VS_Input input)
{
	VS_Output output;
	output.position = float4(input.pos, 0.0f, 1.0f);
	output.color = input.color;
 
	return output;
}