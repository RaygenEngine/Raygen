#include "pch.h"

#include "Device.h"
#include "CommandQueue.h"

Device_::Device_()
{
#if defined(_DEBUG)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	WRL::ComPtr<ID3D12Debug1> debugInterface;
	AbortIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
	// Enable these if you want full validation (will slow down rendering a lot).
	// debugInterface->SetEnableGPUBasedValidation(TRUE);
	// debugInterface->SetEnableSynchronizedCommandQueueValidation(TRUE);
#endif

	auto dxgiAdapter = GetAdapter(false);
	if (!dxgiAdapter) {
		// If no supporting DX12 adapters exist, fall back to WARP
		dxgiAdapter = GetAdapter(true);
	}

	if (dxgiAdapter) {
		m_d3d12Device = CreateDevice(dxgiAdapter);
	}
	else {
		LOG_ABORT("DXGI adapter enumeration failed.");
	}

	for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
		m_descriptorHandleIncrementSize[i]
			= m_d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
}

WRL::ComPtr<ID3D12DescriptorHeap> Device_::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors)
{
	WRL::ComPtr<ID3D12DescriptorHeap> d3d12DescriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;

	AbortIfFailed(m_d3d12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&d3d12DescriptorHeap)));

	return d3d12DescriptorHeap;
}

WRL::ComPtr<ID3D12CommandAllocator> Device_::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
	WRL::ComPtr<ID3D12CommandAllocator> d3d12CommandAllocator;
	AbortIfFailed(m_d3d12Device->CreateCommandAllocator(type, IID_PPV_ARGS(&d3d12CommandAllocator)));

	return d3d12CommandAllocator;
}

WRL::ComPtr<ID3D12GraphicsCommandList2> Device_::CreateCommandList(
	ID3D12CommandAllocator* d3d12CommandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	WRL::ComPtr<ID3D12GraphicsCommandList2> d3d12CommandList;
	AbortIfFailed(
		m_d3d12Device->CreateCommandList(0, type, d3d12CommandAllocator, nullptr, IID_PPV_ARGS(&d3d12CommandList)));

	AbortIfFailed(d3d12CommandList->Close());

	return d3d12CommandList;
}

WRL::ComPtr<IDXGIAdapter4> Device_::GetAdapter(bool bUseWarp)
{
	WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	AbortIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

	WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1;
	WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (bUseWarp) {
		AbortIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
		AbortIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
	}
	else {
		SIZE_T maxDedicatedVideoMemory = 0;
		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			// Check to see if the adapter can create a D3D12 device without actually
			// creating it. The adapter with the largest dedicated video memory
			// is favored.
			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0
				&& SUCCEEDED(
					D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))
				&& dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory) {
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				AbortIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}

WRL::ComPtr<ID3D12Device2> Device_::CreateDevice(WRL::ComPtr<IDXGIAdapter4> adapter)
{
	WRL::ComPtr<ID3D12Device2> d3d12Device2;
	AbortIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));
	//    NAME_D3D12_OBJECT(d3d12Device2);

	// Enable debug messages in debug mode.
#if defined(_DEBUG)
	WRL::ComPtr<ID3D12InfoQueue> pInfoQueue;
	if (SUCCEEDED(d3d12Device2.As(&pInfoQueue))) {
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Suppress whole categories of messages
		// D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES,
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		// NewFilter.DenyList.NumCategories = _countof(Categories);
		// NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		AbortIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
	}
#endif

	return d3d12Device2;
}

void DeviceQueues::AddQueue(D3D12_COMMAND_LIST_TYPE type)
{
	queues[GetIndexFromType(type)] = std::make_unique<CommandQueue>(type);
}

void DeviceQueues::FlushAll()
{
	for (auto& q : queues) {
		if (q) {
			q->Flush();
		}
	}
}
