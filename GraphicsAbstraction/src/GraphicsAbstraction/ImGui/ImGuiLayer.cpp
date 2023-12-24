#include "ImGuiLayer.h"

#include <array>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanContext.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanSwapchain.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandPool.h>
#include <Platform/GraphicsAPI/Vulkan/Mappings/VulkanCommandBuffer.h>
#include <GraphicsAbstraction/Core/Window.h>

namespace GraphicsAbstraction {

	struct ImGuiLayerData
	{
		VkDescriptorPool Pool;
		VulkanContextReference Context;

		ImGuiLayerData(const VulkanContextReference& context)
			: Context(context) { }
	};

	static ImGuiLayerData* s_Data;

	void ImGuiLayer::Init(const std::shared_ptr<CommandPool>& commandPool, const std::shared_ptr<Swapchain>& swapchain, const std::shared_ptr<Window>& window)
	{
		s_Data = new ImGuiLayerData(VulkanContext::GetReference());

		auto vulkanSwapchain = std::static_pointer_cast<VulkanSwapchain>(swapchain);
		auto vulkanCommandPool = std::static_pointer_cast<VulkanCommandPool>(commandPool);

		constexpr auto poolSizes = std::array { 
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = 1000,
			.poolSizeCount = (uint32_t)poolSizes.size(),
			.pPoolSizes = poolSizes.data()
		};

		VK_CHECK(vkCreateDescriptorPool(s_Data->Context->Device, &poolInfo, nullptr, &s_Data->Pool));

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window->GetNativeWindow());
		ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

		ImGui_ImplVulkan_InitInfo initInfo = {
			.Instance = s_Data->Context->Instance,
			.PhysicalDevice = s_Data->Context->ChosenGPU,
			.Device = s_Data->Context->Device,
			.Queue = s_Data->Context->GraphicsQueue,
			.DescriptorPool = s_Data->Pool,
			.MinImageCount = 3,
			.ImageCount = 3,
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
			.UseDynamicRendering = true,
			.ColorAttachmentFormat = vulkanSwapchain->ImageFormat,
		};
		ImGui_ImplVulkan_Init(&initInfo, VK_NULL_HANDLE);

		// TODO: change below, I don't want to make an "immediate submit" system cause it'll be bad for multithreading but this also stinks
		VkFence fence;
		VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};
		vkCreateFence(s_Data->Context->Device, &fenceInfo, nullptr, &fence);

		VkCommandBuffer cmd = vulkanCommandPool->MainCommandBuffer;

		VkCommandBufferBeginInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		vkBeginCommandBuffer(cmd, &info);
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		vkEndCommandBuffer(cmd);

		VkCommandBufferSubmitInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = cmd
		};

		VkSubmitInfo2 submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &bufferInfo
		};

		VK_CHECK(vkQueueSubmit2(s_Data->Context->GraphicsQueue, 1, &submitInfo, fence));
		VK_CHECK(vkWaitForFences(s_Data->Context->Device, 1, &fence, true, UINT64_MAX));
		vkDestroyFence(s_Data->Context->Device, fence, nullptr);

		// clear font textures from cpu
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void ImGuiLayer::Shutdown()
	{
		vkDeviceWaitIdle(s_Data->Context->Device);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		vkDestroyDescriptorPool(s_Data->Context->Device, s_Data->Pool, nullptr);
		delete s_Data;
	}

	void ImGuiLayer::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::DrawFrame(const std::shared_ptr<CommandBuffer>& cmd, const std::shared_ptr<Image>& image)
	{
		auto vulkanCommandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(cmd);
		auto vulkanImage = std::static_pointer_cast<VulkanImage>(image);

		ImGuiIO& io = ImGui::GetIO();

		ImGui::Render();

		VkRenderingAttachmentInfo colorAttachment = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = vulkanImage->View,
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE
		};

		VkExtent2D extent = {
			.width = (uint32_t)vulkanImage->Width,
			.height = (uint32_t)vulkanImage->Height
		};

		VkRect2D rect = {
			.extent = extent
		};

		VkRenderingInfo info = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = rect,
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachment
		};

		vulkanImage->TransitionLayout(vulkanCommandBuffer->CommandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		vkCmdBeginRendering(vulkanCommandBuffer->CommandBuffer, &info);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vulkanCommandBuffer->CommandBuffer);
		vkCmdEndRendering(vulkanCommandBuffer->CommandBuffer);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = { 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = { 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = { 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = { 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = { 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = { 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = { 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = { 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = { 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = { 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = { 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = { 0.15f, 0.1505f, 0.151f, 1.0f };
	}

}