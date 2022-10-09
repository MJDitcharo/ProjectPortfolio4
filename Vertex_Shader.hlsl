// an ultra simple hlsl vertex shader
// TODO: Part 2i
// TODO: Part 2b
// TODO: Part 4f
// TODO: Part 4a
// TODO: Part 1f
// TODO: Part 4b

struct VS_IN {
	float3 pos : POSITION;
	float3 textcoord : TEXTCOORD;
	float3 norm : NORMAL;
};
struct VS_OUT { 
	float4 pos : SV_POSITION; 
	float3 textcoord : TEXTCOORD;
	float3 norm: NORMAL; 
};

struct OBJ_ATTRIBUTES
{
	float3           Kd; // diffuse reflectivity
	float	          d; // dissolve (transparency) 
	float3           Ks; // specular reflectivity
	float            Ns; // specular exponent
	float3           Ka; // ambient reflectivity
	float     sharpness; // local reflection map sharpness
	float3           Tf; // transmission filter
	float            Ni; // optical density (index of refraction)
	float3           Ke; // emissive reflectivity
	unsigned int    illum; // illumination model
};

struct SCENE_DATA
{
	float4 sunDirection, sunColor;
	float4x4 viewMatrix, projectionMatrix;
	float4 padding[6];
};

struct MESH_DATA
{
	float4x4 world;
	OBJ_ATTRIBUTES material;
	unsigned int padding[28];

};

// TODO: Part 4f
// TODO: Part 4a
// TODO: Part 1f
// TODO: Part 4b
VS_OUT main(VS_IN input)
{

	VS_OUT output;
	output.pos = float4(input.pos + float3(0.0f, -0.75f, 0.75f), 1);
	output.textcoord = input.textcoord;
	output.norm = input.norm;
	
	

	// TODO: Part 1h
	return output;
	// TODO: Part 2i
	// TODO: Part 4b
}