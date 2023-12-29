#pragma once

#include <GraphicsAbstraction/Core/Window.h>
#include <GraphicsAbstraction/Renderer/Image.h>
#include <GraphicsAbstraction/Renderer/CommandPool.h>
#include <GraphicsAbstraction/Renderer/Queue.h>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <GraphicsAbstraction/Renderer/Fence.h>
#include <Renderer/EditorCamera.h>

namespace GraphicsAbstraction {

	class RenderProcedure;

	class Renderer
	{
	public:
		static void Init(Ref<Window>& window, bool seperateDisplayImage);
		static void Shutdown();

		static void AddProcedure(RenderProcedure* procedure);
		static void PreProcess();
		static void Render(const EditorCamera& camera);

		static void SetImGuiCallback(std::function<void()> callback);
		static void Resize(uint32_t width, uint32_t height);
		static void SetVsync(bool vsync);

		static void CopyNextFrame(const Ref<Buffer>& srcBuffer, const Ref<Image>& dstImage);
		static Ref<Image> GetDrawImage();
	};

}