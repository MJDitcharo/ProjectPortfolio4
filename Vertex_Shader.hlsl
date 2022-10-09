// an ultra simple hlsl vertex shader
// TODO: Part 2i
// TODO: Part 2b
// TODO: Part 4f
// TODO: Part 4a
// TODO: Part 1f
// TODO: Part 4b
#pragma pack_matrix(row_major)


struct VS_IN {
	float3 pos : POSITION;
	float3 tex : TEXTCOORD;
	float3 norm : NORMAL;
};
struct VS_OUT { 
	float4 pos : SV_POSITION; 
	float3 tex : TEXTCOORD;
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

ConstantBuffer<SCENE_DATA> cameraAndLights   : register(b0, Space0);
ConstantBuffer<MESH_DATA>  meshInfo			 : register(b1, Space0);

// TODO: Part 4f
// TODO: Part 4a
// TODO: Part 1f
// TODO: Part 4b
VS_OUT main(VS_IN input)
{

    VS_OUT output = (VS_OUT) 0;
	output.pos = float4(input.pos, 1);
	
    output.pos = mul(output.pos, meshInfo.world);
    output.pos = mul(output.pos, cameraAndLights.viewMatrix);
    output.pos = mul(output.pos, cameraAndLights.projectionMatrix);
	
	output.tex = input.tex;
	output.norm = input.norm;
	
	

	// TODO: Part 1h
	return output;
	// TODO: Part 2i
	// TODO: Part 4b
}