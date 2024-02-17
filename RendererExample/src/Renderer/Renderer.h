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
		UUID MaterialHandle;
	};

	class Renderer
	{
	public:
		static void Init(Ref<Window>& window, bool seperateDisplayImage);
		static void Shutdown();

		static UUID UploadMaterial(const Ref<Material>& material);
		static UUID UploadMesh(const std::vector<Submesh>& submeshes, const std::vector<uint32_t>& indices);
		static UUID UploadModel(UUID meshHandle, const glm::mat4& initialModel, uint32_t entityID);
		static UUID UploadLight(const glm::vec3& position, const glm::vec3& color);

		static void UpdateLight(UUID lightHandle, const glm::vec3& position, const glm::vec3& color);
		static void UpdateMaterialAlbedoFactor(UUID materialHandle, const glm::vec4& albedoFactor);
		static void UpdateTransform(UUID modelHandle, const glm::mat4& transform);

		static void RemoveLight(UUID lightHandle);
		static void Render(const EditorCamera& camera);

		static void SetImGuiCallback(std::function<void()> callback);
		static void Resize(uint32_t width, uint32_t height);
		static void SetVsync(bool vsync);
		static void SetCullPaused(bool paused);

		static int GetEntityIDAt(uint32_t x, uint32_t y);

		static void CopyImage(void* src, Ref<Image> dstImage, uint32_t size);
		static Ref<Image> GetDrawImage();
	private:
		static void ImmediateSubmit();
		static void Draw(Ref<CommandList>& cmd, const EditorCamera& camera);
	};

}