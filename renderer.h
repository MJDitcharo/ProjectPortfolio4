// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
#include "Gateware.h"
#include "FSLogo.h"
#include <chrono>
// TODO: Part 1b

constexpr auto PI = 3.14;

//struct LIGHT_VARS
//{
//	float lightColor[4];
//	float totalAndDeltaTime[2];
//	float lightFallOff[2];
//	float lightDirection[3];
//	float coneRatio;
//	float lightPosition[3];
//	float padding;
//};
//
//struct SHADER_VARS
//{
//	GW::MATH::GMATRIXF worldMatrix; // 16 32bit values
//	GW::MATH::GMATRIXF viewMatrix;
//	GW::MATH::GMATRIXF projectionMatrix;
//};

struct SCENE_DATA
{
	GW::MATH::GVECTORF sunDirection, sunColor; // lighting info
	GW::MATH::GMATRIXF viewMatrix, projectionMatrix; // viewing info
	GW::MATH::GVECTORF padding[6]; // D3D12 requires 256 byte aligned constant buffers
};

struct MESH_DATA
{
	// per sub-mesh transform and material data
	GW::MATH::GMATRIXF world; // final world space transform
	OBJ_ATTRIBUTES material; // color/texture of surface
	unsigned padding[28];
};


float angleToRadian(float input)
{
	return (input * PI) / 180;
}

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX12Surface d3d;


	// Math Proxy
	GW::MATH::GMatrix mat;

	// Camera Proxy
	GW::INPUT::GInput gInput;
	GW::INPUT::GController gController;

	// World, View, and Projection
	GW::MATH::GMATRIXF world;
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection;

	// what we need at a minimum to draw a triangle
	D3D12_VERTEX_BUFFER_VIEW					vertexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>		vertexBuffer;
	// TODO: Part 1g
	D3D12_INDEX_BUFFER_VIEW						indexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>		indexBuffer;

	 
	// TODO: Part 2c
	// TODO: Part 2e
	Microsoft::WRL::ComPtr<ID3D12RootSignature>	rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	pipeline;
	// TODO: Part 2a
	// TODO: Part 2b
	// TODO: Part 4f




	// Load a shader file as a string of characters.
	std::string ShaderAsString(const char* shaderFilePath) {
		std::string output;
		unsigned int stringLength = 0;
		GW::SYSTEM::GFile file; file.Create();
		file.GetFileSize(shaderFilePath, stringLength);
		if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
			output.resize(stringLength);
			file.Read(&output[0], stringLength);
		}
		else
			std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
		return output;
	}

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d)
	{
		using namespace GW::MATH;


		win = _win;
		d3d = _d3d;
		ID3D12Device* creator;
		d3d.GetDevice((void**)&creator);
		mat.Create();
		gController.Create();
		gInput.Create(_win);


		// TODO: part 2a
		mat.IdentityF(world);

		mat.IdentityF(view);
		GVECTORF eye = { 0.75f, .25f, -1.5, 0 };
		GVECTORF at = { 0.15f, 0.75f, 0, 0 };
		GVECTORF up = { 0,1,0,0 };
		mat.LookAtLHF(eye, at, up, view);

		float fov = angleToRadian(65);
		float nPlane = 0.1f;
		float fPlane = 100.0f;
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		mat.IdentityF(projection);
		mat.ProjectionDirectXLHF(fov, aspectRatio, nPlane, fPlane, projection);

		// TODO: part 2b
		// TODO: Part 4f
		// TODO: Part 1c

		// vertex buffer stuff
		{
			creator->CreateCommittedResource( // using UPLOAD heap for simplicity
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(FSLogo_vertices)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
			// Transfer triangle data to the vertex buffer.
			UINT8* transferMemoryLocation;
			vertexBuffer->Map(0, &CD3DX12_RANGE(0, 0),
				reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, FSLogo_vertices, sizeof(FSLogo_vertices));
			vertexBuffer->Unmap(0, nullptr);
			// Create a vertex View to send to a Draw() call.
			vertexView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
			vertexView.StrideInBytes = sizeof(_OBJ_VERT_); // TODO: Part 1e
			vertexView.SizeInBytes = sizeof(FSLogo_vertices); // TODO: Part 1d
		}
		// TODO: Part 1g
		{
			creator->CreateCommittedResource( // using UPLOAD heap for simplicity
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(FSLogo_indices)),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer));


			UINT8* transferMemoryLocation;
			indexBuffer->Map(0, &CD3DX12_RANGE(0, 0),
				reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, FSLogo_indices, sizeof(FSLogo_indices));
			indexBuffer->Unmap(0, nullptr);

			indexView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			indexView.SizeInBytes = sizeof(FSLogo_indices);
			indexView.Format = DXGI_FORMAT_R32_UINT;
		}

		// TODO: Part 2d
		// TODO: Part 2e
		// TODO: Part 2f

		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif

		std::string VS = ShaderAsString("../Vertex_Shader.hlsl");
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;
		if (FAILED(D3DCompile(VS.c_str(), strlen(VS.c_str()),
			nullptr, nullptr, nullptr, "main", "vs_5_0", compilerFlags, 0,
			vsBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			abort();
		}
		// Create Pixel Shader

		std::string PS = ShaderAsString("../Pixel_Shader.hlsl");
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (FAILED(D3DCompile(PS.c_str(), strlen(PS.c_str()),
			nullptr, nullptr, nullptr, "main", "ps_5_0", compilerFlags, 0,
			psBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			abort();
		}
		// TODO: Part 1e
		// Create Input Layout
		D3D12_INPUT_ELEMENT_DESC format[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXTCOORD",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		// TODO: Part 2g
		// create root signature

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		D3D12SerializeRootSignature(&rootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);
		creator->CreateRootSignature(0, signature->GetBufferPointer(),
			signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
		// create pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc;
		ZeroMemory(&psDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psDesc.InputLayout = { format, ARRAYSIZE(format) };
		psDesc.pRootSignature = rootSignature.Get();
		psDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
		psDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
		psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psDesc.SampleMask = UINT_MAX;
		psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psDesc.NumRenderTargets = 1;
		psDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psDesc.SampleDesc.Count = 1;
		creator->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&pipeline));
		// free temporary handle
		creator->Release();
	}
	void Render()
	{
		// TODO: Part 2a
		// TODO: Part 4d
		// grab the context & render target
		ID3D12GraphicsCommandList* cmd;
		D3D12_CPU_DESCRIPTOR_HANDLE rtv;
		D3D12_CPU_DESCRIPTOR_HANDLE dsv;
		d3d.GetCommandList((void**)&cmd);
		d3d.GetCurrentRenderTargetView((void**)&rtv);
		d3d.GetDepthStencilView((void**)&dsv);
		// setup the pipeline
		cmd->SetGraphicsRootSignature(rootSignature.Get());
		// TODO: Part 2h
		// TODO: Part 4e
		cmd->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		cmd->SetPipelineState(pipeline.Get());



		//mat.RotateYLocalF(world, angleToRadian(1), world);
		//SHADER_VARS shaderVars = { world, view, projection };



		// now we can draw
		cmd->IASetVertexBuffers(0, 1, &vertexView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// TODO: Part 1h
		cmd->IASetIndexBuffer(&indexView);
		cmd->DrawIndexedInstanced(FSLogo_indexcount, 1, 0, 0, 0);

		// TODO: Part 3b
			// TODO: Part 3c
			// TODO: Part 4e
		//cmd->DrawInstanced(3885, 1, 0, 0); // TODO: Part 1c
		// release temp handles
		cmd->Release();
	}
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};