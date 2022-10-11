// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft
#include "Gateware.h"
#include "FSLogo.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

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
	GW::MATH::GVECTORF sunDirection, sunColor, sunAmbience, cameraPos; // lighting info
	GW::MATH::GMATRIXF viewMatrix, projectionMatrix; // viewing info
	GW::MATH::GVECTORF padding[4]; // D3D12 requires 256 byte aligned constant buffers
};

struct MESH_DATA
{
	// per sub-mesh transform and material data
	GW::MATH::GMATRIXF world; // final world space transform
	OBJ_ATTRIBUTES material; // color/texture of surface
	unsigned padding[28];
};

#pragma region HelperShit

std::vector<GW::MATH::GMATRIXF> levelParse(const char* fileName)
{
	std::fstream f;
	std::vector<float> values;
	std::vector<GW::MATH::GMATRIXF> output;
	f.open(fileName, std::ios::in);

	while (!f.eof())
	{
		std::string str, h2b, strTemp;
		std::getline(f, str, '\n');
		if (std::strcmp(str.c_str(), "MESH") == 0 || std::strcmp(str.c_str(), "LIGHT") == 0 || std::strcmp(str.c_str(), "CAMERA") == 0)
		{
			std::cout << str << std::endl;

			// Get and Print .h2b
			std::getline(f, h2b, '\n');
			std::cout << h2b << std::endl;

			// First Matrix Row
			{
				std::getline(f, strTemp, '(');
				std::cout << strTemp;
				std::getline(f, strTemp, ')');
				std::cout << '(' << strTemp << ')';
				std::string numString = "";
				strTemp.append(")");
				for (std::string::iterator it = strTemp.begin(); it != strTemp.end(); ++it)
				{
					if (*it != ',' && *it != ')')
						numString += *it;
					else
					{
						float numOutput = std::stof(numString);
						values.push_back(numOutput);
						numString = "";
					}
				}
			}

			// Second Matrix Row
			{
				std::getline(f, strTemp, '(');
				std::cout << strTemp;
				std::getline(f, strTemp, ')');
				std::cout << '(' << strTemp << ')';
				std::string numString = "";
				strTemp.append(")");
				for (std::string::iterator it = strTemp.begin(); it != strTemp.end(); ++it)
				{
					if (*it != ',' && *it != ')')
						numString += *it;
					else
					{
						float numOutput = std::stof(numString);
						values.push_back(numOutput);
						numString = "";
					}
				}
			}

			// Third Matrix Row
			{
				std::getline(f, strTemp, '(');
				std::cout << strTemp;
				std::getline(f, strTemp, ')');
				std::cout << '(' << strTemp << ')';
				std::string numString = "";
				strTemp.append(")");
				for (std::string::iterator it = strTemp.begin(); it != strTemp.end(); ++it)
				{
					if (*it != ',' && *it != ')')
						numString += *it;
					else
					{
						float numOutput = std::stof(numString);
						values.push_back(numOutput);
						numString = "";
					}
				}
			}

			// Last Matrix Row
			{
				std::getline(f, strTemp, '(');
				std::cout << strTemp;
				std::getline(f, strTemp, ')');
				std::cout << '(' << strTemp << ')';
				std::string numString = "";
				strTemp.append(")");
				for (std::string::iterator it = strTemp.begin(); it != strTemp.end(); ++it)
				{
					if (*it != ',' && *it != ')')
						numString += *it;
					else
					{
						float numOutput = std::stof(numString);
						values.push_back(numOutput);
						numString = "";
					}
				}
			}
			std::cout << '\n';
		}
	}
	f.close();


	GW::MATH::GMATRIXF matTemp;
	for (size_t j = 0; j < values.size(); j += 16)
	{
		for (size_t i = 0; i < 16; i += 4)
		{

			if (i < 4)
			{
				matTemp.row1.x = values[i];
				matTemp.row1.y = values[i + 1];
				matTemp.row1.z = values[i + 2];
				matTemp.row1.w = values[i + 3];
			}
			else if (i < 8)
			{
				matTemp.row2.x = values[i];
				matTemp.row2.y = values[i + 1];
				matTemp.row2.z = values[i + 2];
				matTemp.row2.w = values[i + 3];
			}
			else if (i < 12)
			{
				matTemp.row3.x = values[i];
				matTemp.row3.y = values[i + 1];
				matTemp.row3.z = values[i + 2];
				matTemp.row3.w = values[i + 3];
			}
			else if (i < 16)
			{
				matTemp.row4.x = values[i];
				matTemp.row4.y = values[i + 1];
				matTemp.row4.z = values[i + 2];
				matTemp.row4.w = values[i + 3];
				output.push_back(matTemp);
			}
		}
	}



	return output;
}

float angleToRadian(float input)
{
	return (input * PI) / 180;
}
#pragma endregion


// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX12Surface d3d;


	// Math Proxy
	GW::MATH::GMatrix mat;
	GW::MATH::GVector vecProxy;

	// Camera Proxy
	GW::INPUT::GInput gInput;
	GW::INPUT::GController gController;


	// what we need at a minimum to draw a triangle
	D3D12_VERTEX_BUFFER_VIEW					vertexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>		vertexBuffer;
	// TODO: Part 1g
	D3D12_INDEX_BUFFER_VIEW						indexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>		indexBuffer;


	// TODO: Part 2c
	Microsoft::WRL::ComPtr<ID3D12Resource>		constantBuffer;

	// TODO: Part 2e
	Microsoft::WRL::ComPtr<ID3D12RootSignature>	rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	pipeline;
	ID3D12DescriptorHeap* descHeap[1];


	
	MESH_DATA logoMesh;
	MESH_DATA titleMesh;
	SCENE_DATA sceneData;

	// World, View, and Projection
	GW::MATH::GMATRIXF worldTitle;
	GW::MATH::GMATRIXF worldLogo;
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection;



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
		mat.IdentityF(worldTitle);
		mat.IdentityF(worldLogo);

		mat.IdentityF(view);
		GVECTORF eye = { 0.75f, .25f, -1.5, 0 };
		GVECTORF at = { 0.15f, 0.75f, 0, 0 };
		GVECTORF up = { 0,1,0,0 };
		mat.LookAtLHF(eye, at, up, view);

		sceneData.cameraPos = eye;
		GVECTORF ambientVec = { 0.25f, 0.25f, 0.35f, 1 };
		sceneData.sunAmbience = ambientVec;

		float fov = angleToRadian(65);
		float nPlane = 0.1f;
		float fPlane = 100.0f;
		float aspectRatio;
		d3d.GetAspectRatio(aspectRatio);
		mat.IdentityF(projection);
		mat.ProjectionDirectXLHF(fov, aspectRatio, nPlane, fPlane, projection);

		sceneData.viewMatrix = view;
		sceneData.projectionMatrix = projection;

		GW::MATH::GVECTORF sunDirectionVec = { -1,-1, 2 };
		vecProxy.NormalizeF(sunDirectionVec, sunDirectionVec);
		sceneData.sunDirection = sunDirectionVec;

		GW::MATH::GVECTORF sunColorVec = { 0.9f, 0.9f, 1.0f, 1.0f };
		sceneData.sunColor = sunColorVec;

		logoMesh.world = worldLogo;
		logoMesh.material = FSLogo_materials[0].attrib;

		titleMesh.world = worldTitle;
		titleMesh.material = FSLogo_materials[1].attrib;



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
		IDXGISwapChain4* swapChain;
		d3d.GetSwapchain4((void**)&swapChain);
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		swapChain->GetDesc(&swapChainDesc);
		unsigned meshOffset = sizeof(logoMesh) + sizeof(sceneData);
		unsigned sceneOffset = sizeof(sceneData);
		unsigned constBuffMemory = (sizeof(SCENE_DATA) + (FSLogo_meshcount * sizeof(MESH_DATA))) * swapChainDesc.BufferCount;
		{



			HRESULT hr = creator->CreateCommittedResource( // using UPLOAD heap for simplicity
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(constBuffMemory),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constantBuffer));

			if (FAILED(hr))
				throw(std::runtime_error::runtime_error("Error creating a const buffer."));


			UINT8* transferMemoryLocation;
			constantBuffer->Map(0, &CD3DX12_RANGE(0, 0),
				reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, &sceneData, sizeof(SCENE_DATA));
			memcpy(transferMemoryLocation + sceneOffset, &logoMesh, sizeof(MESH_DATA));
			memcpy(transferMemoryLocation + meshOffset, &titleMesh, sizeof(MESH_DATA));
			constantBuffer->Unmap(0, nullptr);

		}


		// TODO: Part 2e
		D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
		descHeapDesc.NumDescriptors = swapChainDesc.BufferCount;
		descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		creator->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap[0]));

		// TODO: Part 2f
		D3D12_CONSTANT_BUFFER_VIEW_DESC bufferDesc;
		bufferDesc.BufferLocation = constantBuffer.Get()->GetGPUVirtualAddress();
		bufferDesc.SizeInBytes = constBuffMemory;

		D3D12_CPU_DESCRIPTOR_HANDLE descHandle = descHeap[0]->GetCPUDescriptorHandleForHeapStart();
		creator->CreateConstantBufferView(&bufferDesc, descHandle);

		CD3DX12_ROOT_PARAMETER rootParameters[2];

		rootParameters[0].InitAsConstantBufferView(0);
		rootParameters[1].InitAsConstantBufferView(1);

		CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
		rootDesc.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


		// Create Vertex Shader
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif

		std::string VS = ShaderAsString("../Vertex_Shader.hlsl");
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;
		if (FAILED(D3DCompile(VS.c_str(), strlen(VS.c_str()),
			nullptr, nullptr, nullptr, "main", "vs_5_1", compilerFlags, 0,
			vsBlob.GetAddressOf(), errors.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			abort();
		}
		// Create Pixel Shader

		std::string PS = ShaderAsString("../Pixel_Shader.hlsl");
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob; errors.Reset();
		if (FAILED(D3DCompile(PS.c_str(), strlen(PS.c_str()),
			nullptr, nullptr, nullptr, "main", "ps_5_1", compilerFlags, 0,
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
		CD3DX12_ROOT_PARAMETER rootParams[2];
		rootParams[0].InitAsConstantBufferView(0);
		rootParams[1].InitAsConstantBufferView(1);


		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(2, rootParams, 0, nullptr,
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
	

		// grab the context & render target
		ID3D12GraphicsCommandList* cmd;
		D3D12_CPU_DESCRIPTOR_HANDLE rtv;
		D3D12_CPU_DESCRIPTOR_HANDLE dsv;
		d3d.GetCommandList((void**)&cmd);
		d3d.GetCurrentRenderTargetView((void**)&rtv);
		d3d.GetDepthStencilView((void**)&dsv);


		// setup the pipeline
		cmd->SetGraphicsRootSignature(rootSignature.Get());
		cmd->SetDescriptorHeaps(1, descHeap);
		cmd->SetGraphicsRootConstantBufferView(0, constantBuffer.Get()->GetGPUVirtualAddress());
		cmd->SetGraphicsRootConstantBufferView(1, constantBuffer.Get()->GetGPUVirtualAddress() + sizeof(sceneData));


		cmd->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		cmd->SetPipelineState(pipeline.Get());



		mat.RotateYGlobalF(titleMesh.world, angleToRadian(0.5), titleMesh.world);

		// Update the Constant Buffer to Rotate Logo
		{
			IDXGISwapChain4* swapChain;
			d3d.GetSwapchain4((void**)&swapChain);
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			swapChain->GetDesc(&swapChainDesc);

			unsigned meshOffset = sizeof(logoMesh) + sizeof(sceneData);
			unsigned sceneOffset = sizeof(sceneData);
			unsigned constBuffMemory = (sizeof(SCENE_DATA) + (FSLogo_meshcount * sizeof(MESH_DATA))) * swapChainDesc.BufferCount;

			UINT8* transferMemoryLocation;
			constantBuffer->Map(0, &CD3DX12_RANGE(0, 0),
				reinterpret_cast<void**>(&transferMemoryLocation));
			memcpy(transferMemoryLocation, &sceneData, sizeof(SCENE_DATA));
			memcpy(transferMemoryLocation + sceneOffset, &logoMesh, sizeof(MESH_DATA));
			memcpy(transferMemoryLocation + meshOffset, &titleMesh, sizeof(MESH_DATA));
			constantBuffer->Unmap(0, nullptr);
		}


		// now we can draw
		cmd->IASetVertexBuffers(0, 1, &vertexView);
		cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// TODO: Part 1h


		cmd->IASetIndexBuffer(&indexView);
		cmd->DrawIndexedInstanced(FSLogo_meshes[0].indexCount, 1, FSLogo_meshes[0].indexOffset, 0, 0);


		cmd->SetGraphicsRootConstantBufferView(1, constantBuffer.Get()->GetGPUVirtualAddress() + sizeof(SCENE_DATA) + sizeof(MESH_DATA));
		cmd->DrawIndexedInstanced(FSLogo_meshes[1].indexCount, 1, FSLogo_meshes[1].indexOffset, 0, 0);



		// release temp handles
		cmd->Release();
	}
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here 
	}
};
