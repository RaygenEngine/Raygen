#include "pch.h"

#include "Layer.h"

#include "core/CommandQueue.h"
#include "core/Device.h"
#include "core/SwapChain.h"
#include "editor/imgui/ImguiImpl.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"

// Vertex data for a colored cube.
struct VertexPosColor {
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

constexpr VertexPosColor g_Vertices[8] = {
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },  // 1
	{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },   // 2
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },  // 3
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },  // 4
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },   // 5
	{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },    // 6
	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }    // 7
};

constexpr WORD g_Indicies[36]
	= { 0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7 };

DirectX::XMMATRIX modelMatrix;
DirectX::XMMATRIX viewMatrix;
DirectX::XMMATRIX projectionMatrix;

Layer_::Layer_()
{
	Device = new Device_();
	DeviceQueues.AddQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	DeviceQueues.AddQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

	GpuAssetManager = new GpuAssetManager_();

	m_swapChain = std::make_unique<SwapChain>(false);
	m_scene = std::make_unique<Scene>();

	m_scissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
	m_viewport = CD3DX12_VIEWPORT(static_cast<float>(g_ViewportCoordinates.position.x),
		static_cast<float>(g_ViewportCoordinates.position.y), static_cast<float>(g_ViewportCoordinates.size.x),
		static_cast<float>(g_ViewportCoordinates.size.y));

	Event::OnWindowResize.Bind(this, [&, &vpref = m_viewport, &swapref = *m_swapChain](int32 width, int32 height) {
		Device->Flush();

		swapref.Resize(width, height);

		UpdateDSV(width, height);
	});

	Event::OnViewportUpdated.Bind(this, [&vpref = m_viewport, &scisref = m_scissorRect]() {
		vpref = CD3DX12_VIEWPORT(static_cast<float>(g_ViewportCoordinates.position.x),
			static_cast<float>(g_ViewportCoordinates.position.y), static_cast<float>(g_ViewportCoordinates.size.x),
			static_cast<float>(g_ViewportCoordinates.size.y));
		scisref = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
	});


	DSVDescriptorHeap = Device->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
	UpdateDSV(m_swapChain->GetSize().x, m_swapChain->GetSize().y);

	PreparePipelineStuff();
}

Layer_::~Layer_()
{
	Device->Flush();

	delete GpuAssetManager;

	delete Device;
}

void Layer_::DrawFrame()
{
	GpuAssetManager->ConsumeAssetUpdates();

	m_scene->ConsumeCmdQueue();
	m_scene->UploadDirty(m_currFrame); // NEW:: param

	CommandQueue& dqueue = DeviceQueues[D3D12_COMMAND_LIST_TYPE_DIRECT];
	auto commandList = dqueue.GetCommandList();

	UINT currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	auto backBuffer = m_swapChain->GetCurrentBackBuffer();
	auto rtv = m_swapChain->GetCurrentRenderTargetView();
	auto dsv = DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// Clear the render targets.
	{
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		FLOAT clearColor[]{ 0.f, 0.f, 0.f, 1.0f };
		commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	}

	commandList->SetPipelineState(m_pipelineState.Get());
	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	DrawStaticGeometry(commandList);


	ImguiImpl::RenderDX12(commandList.Get());


	// Present
	{
		TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_frameFenceValues[currentBackBufferIndex] = dqueue.ExecuteCommandList(commandList);

		currentBackBufferIndex = m_swapChain->Present();

		dqueue.WaitForFenceValue(m_frameFenceValues[currentBackBufferIndex]);
	}

	++m_currFrame;
}

#include "rendering/scene/SceneStructs.h"
#include "rendering/assets/GpuMesh.h"

void Layer_::DrawStaticGeometry(WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList)
{
	auto camera = m_scene->GetElement<SceneCamera>(m_scene->activeCamera);
	XMMATRIX viewProj = XMLoadFloat4x4A(&camera->ubo.viewProj);

	commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &viewProj, 0);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto geom : m_scene->Get<SceneGeometry>()) {
		auto& gpuMesh = geom->mesh.Lock();

		for (auto& gg : gpuMesh.geometryGroups) {

			auto vertexBufferView = gg.vertexBuffer->GetBufferView();
			auto indexBufferView = gg.indexBuffer->GetBufferView();

			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
			commandList->IASetIndexBuffer(&indexBufferView);

			commandList->DrawIndexedInstanced(gg.indexBuffer->numIndicies, 1, 0, 0, 0);
		}
	}
}

void Layer_::PreparePipelineStuff()
{
	auto device = Device->GetHandle();


	// Load the vertex shader.
	WRL::ComPtr<ID3DBlob> vertexShaderBlob;
	AbortIfFailed(D3DReadFileToBlob(L"C:\\dev\\Raygen-dx12\\build\\source\\Debug\\VertexShader.cso",
		&vertexShaderBlob)); // WIP: compile during runtime

	// Load the pixel shader.
	WRL::ComPtr<ID3DBlob> pixelShaderBlob;
	AbortIfFailed(D3DReadFileToBlob(L"C:\\dev\\Raygen-dx12\\build\\source\\Debug\\PixelShader.cso", &pixelShaderBlob));

	// Create the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[]{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Allow input layout and deny unnecessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
													| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
													| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
													| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
													| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[1]{};
	rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// Serialize the root signature.
	WRL::ComPtr<ID3DBlob> rootSignatureBlob;
	WRL::ComPtr<ID3DBlob> errorBlob;
	AbortIfFailed(D3DX12SerializeVersionedRootSignature(
		&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	// Create the root signature.
	AbortIfFailed(device->CreateRootSignature(
		0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

	struct PipelineStateStream {
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
	} pipelineStateStream;

	CD3DX12_RASTERIZER_DESC rdesc{ CD3DX12_DEFAULT{} };
	rdesc.FrontCounterClockwise = TRUE;

	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	pipelineStateStream.pRootSignature = m_rootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.Rasterizer = rdesc;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { sizeof(PipelineStateStream), &pipelineStateStream };

	AbortIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void Layer_::UpdateDSV(int32_t width, int32_t height)
{
	Device->Flush();
	// Resize screen dependent resources.
	// Create a depth buffer.
	D3D12_CLEAR_VALUE optimizedClearValue{};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };

	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	AbortIfFailed(Device->GetHandle()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &optimizedClearValue, IID_PPV_ARGS(&depthBuffer)));

	// Update the depth-stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv{};
	dsv.Format = DXGI_FORMAT_D32_FLOAT;
	dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv.Texture2D.MipSlice = 0;
	dsv.Flags = D3D12_DSV_FLAG_NONE;

	Device->GetHandle()->CreateDepthStencilView(
		depthBuffer.Get(), &dsv, DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}
