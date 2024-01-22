#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include <GraphicsAbstraction/Renderer/Shared/PipelineKeys.h>

#include <InternalManagers/ResourceHandle.h>
#include <InternalManagers/PipelineManager.h>
#include <InternalManagers/CommandSignatureManager.h>
#include <InternalManagers/DeletionQueue.h>
#include <InternalManagers/Utils.h>

#include <d3d12.h>
#include <d3dx12/d3dx12.h>
#include <dxc/dxcapi.h>
#include <dxgi1_6.h>
#include <comdef.h>
#include <codecvt>
#include <wrl.h>

#ifndef GA_DIST
	#define D3D12_CHECK(x)																											\
			do																														\
			{																														\
				HRESULT err = x;																									\
				if (FAILED(err))																									\
					assert(false && err);																							\
			} while (0)
#else
	#define D3D12_CHECK(x) x
#endif

namespace GraphicsAbstraction {

	using namespace Microsoft::WRL;

	template<>
	struct Impl<GraphicsContext> : public RefCounted
	{
		static Ref<Impl<GraphicsContext>> Reference;
		ComPtr<ID3D12Device2> Device;
		ComPtr<ID3D12CommandQueue> GraphicsQueue;

		ComPtr<ID3D12RootSignature> BindlessRootSignature;
		ComPtr<ID3D12DescriptorHeap> BindlessDescriptorHeap, BindlessSamplerHeap;

		PipelineManager* PipelineManager;
		CommandSignatureManager* CommandSignatureManager;
		std::vector<DeletionQueue> FrameDeletionQueues;
		uint32_t FrameInFlight = 0;

		Impl(uint32_t frameInFlightCount);
		virtual ~Impl();

		ComPtr<IDXGIAdapter4> SetupAdapter();
		void SetupDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
		void SetupBindless();

		inline DeletionQueue& GetFrameDeletionQueue() { return FrameDeletionQueues[FrameInFlight]; }
	};

	template<>
	struct Impl<Queue>
	{
		ComPtr<ID3D12CommandQueue> Queue;
		D3D12_COMMAND_LIST_TYPE Type;

		Ref<Fence> DeletionFence;

		Impl(ComPtr<ID3D12CommandQueue> queue, D3D12_COMMAND_LIST_TYPE type);
	};

	template<>
	struct Impl<Image>
	{
		ComPtr<ID3D12Resource> Image;
		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		ResourceHandle Handle;

		D3D12_RESOURCE_STATES State;
		ImageFormat Format;
		ImageUsage Usage;
		uint32_t Width, Height;

		Ref<Impl<GraphicsContext>> Context;
		ComPtr<ID3D12DescriptorHeap> Heap; // only used for depthstencil and color attachment

		Impl(const glm::vec2& size, ImageFormat format, ImageUsage usage);
		Impl(ComPtr<ID3D12Resource> image, D3D12_RESOURCE_STATES state, ImageFormat format, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);

		void TransitionState(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState);

		void Create();
		void CreateViews(DXGI_FORMAT d3d12Format);
	};

	template<>
	struct Impl<Buffer>
	{
		ComPtr<ID3D12Resource> Resource;
		ResourceHandle Handle;
		D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;

		void* Data = nullptr;
		uint32_t Size;
		BufferUsage Usage;

		Ref<Impl<GraphicsContext>> Context;

		Impl(uint32_t size, BufferUsage usage, BufferFlags flags);
		void TransitionState(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES newState);
	};

	template<>
	struct Impl<Swapchain>
	{
		ComPtr<IDXGISwapChain4> Swapchain;
		DXGI_FORMAT ImageFormat;
		std::vector<Ref<Image>> Images;

		uint32_t ImageIndex = 0;
		bool Dirty = false;

		uint32_t Width, Height;
		bool Vsync;

		Ref<Impl<GraphicsContext>> Context;
		ComPtr<ID3D12DescriptorHeap> RTVHeap;

		void CreateSwapchain(HWND hwnd);
		bool CheckTearingSupport();
		void ResizeImpl();

		void UpdateRenderTargetViews();

		Impl(const Ref<Window>& window, const glm::vec2& size, bool enableVSync);
	};

	template<>
	struct Impl<Fence>
	{
		ComPtr<ID3D12Fence> Fence;
		uint64_t Value = 0;

		Ref<Impl<GraphicsContext>> Context;
		HANDLE Event;

		Impl();
	};

	template<>
	struct Impl<CommandList>
	{
		ComPtr<ID3D12GraphicsCommandList> CommandList;
		Ref<Impl<GraphicsContext>> Context;

		GraphicsPipelineKey GraphicsPipelineKey;
		ComputePipelineKey ComputePipelineKey;
		bool GraphicsPipelineStateChanged = false;
		bool ComputePipelineStateChanged = false;
		bool DefaultDynamicStateSet = false;

		Impl(ComPtr<ID3D12GraphicsCommandList> commandList);
		void SetGraphicsPipeline();
	};

	template<>
	struct Impl<Shader>
	{
		D3D12_SHADER_BYTECODE Shader;
		uint32_t ID;

		ShaderStage Stage;
		ComPtr<IDxcBlob> Blob;

		Impl(const std::string& path, ShaderStage stage);
		static Impl<GraphicsAbstraction::Shader>* GetShaderByID(uint32_t id);

		std::string ReadAndPreProcessFile(const std::string& path);
	};

	template<>
	struct Impl<CommandAllocator>
	{
		ComPtr<ID3D12CommandAllocator> Allocator;
		ComPtr<ID3D12GraphicsCommandList> MainCommandList;

		Ref<Impl<GraphicsContext>> Context;

		Impl(const Ref<Queue>& queue);
	};

	template<>
	struct Impl<Sampler>
	{
		ResourceHandle Handle = { ResourceType::Sampler };
		Ref<Impl<GraphicsContext>> Context;

		Impl(Filter min, Filter mag);
	};

}