#pragma once

#include <filesystem>
#include <GraphicsAbstraction/Renderer/Buffer.h>
#include <glm/glm.hpp>

namespace GraphicsAbstraction {

	struct Vertex
	{
		glm::vec3 position;
		float uvX;
		glm::vec3 normal;
		float uvY;
		glm::vec4 color;
	};

	struct GeoSurface
	{
		uint32_t StartIndex;
		uint32_t Count;
	};

	struct Mesh
	{
		std::string Name;

		std::vector<GeoSurface> Surfaces;
		Ref<Buffer> VertexBuffer, IndexBuffer;
	};

	class ModelImporter
	{
	public:
		static std::vector<Mesh> LoadModels(const std::filesystem::path path);
	};

}