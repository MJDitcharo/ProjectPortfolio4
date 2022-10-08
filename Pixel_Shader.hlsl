
struct PS_IN { 
	float4 pos : SV_POSITION; 
	float3 textcoord : TEXTCOORD;
	float3 norm : NORMAL;
};


// TODO: Part 2b
// TODO: Part 4f
// TODO: Part 4b

float4 main(PS_IN input) : SV_TARGET
{
	// TODO: Part 3a
	return float4 (input.norm, 1.0f); // TODO: Part 1a
// TODO: Part 4c
// TODO: Part 4g
}