#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include <Renderer/EditorCamera.h>
#include <Assets/Material.h>

namespace GraphicsAbstraction {

	class RenderProcedure;

	struct Vertex
	{
		glm::vec3 position;
		float uvX;
		glm::vec3 normal;
		float uvY;
		glm::vec4 color;
	};

	struct Submesh
	{
		std::vector<Vertex> Vertices;
		uint32_t IndexCount, IndexOffset;
		uint32_t MaterialHandle;
	};

	class Renderer
	{
	public:
		static void Init(Ref<Window>& window, bool seperateDisplayImage);
		static void Shutdown();

		static uint32_t UploadMaterial(const Ref<Material>& material);
		static uint32_t UploadMesh(const std::vector<Submesh>& submeshes, const std::vector<uint32_t>& indices, const glm::mat4& initialModel);
		static uint32_t UploadLight(const glm::vec3& position, const glm::vec3& color);

		static void UpdateLight(uint32_t lightHandle, const glm::vec3& position, const glm::vec3& color);
		static void UpdateTransform(uint32_t meshHandle, const glm::mat4& transform);
		static void Render(const EditorCamera& camera);

		static void SetImGuiCallback(std::function<void()> callback);
		static void Resize(uint32_t width, uint32_t height);
		static void SetVsync(bool vsync);

		static void CopyNextFrame(const Ref<Buffer>& srcBuffer, const Ref<Image>& dstImage);
		static void CopyNextFrame(const Ref<Buffer>& srcBuffer, const Ref<Buffer>& dstBuffer, uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0);
		static Ref<Image> GetDrawImage();
	private:
		static void Draw(Ref<CommandList>& cmd, const EditorCamera& camera);
	};

}