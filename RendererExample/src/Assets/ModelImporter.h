#pragma once

#include <filesystem>
#include <GraphicsAbstraction/Renderer/Buffer.h>

#include <glm/glm.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

namespace GraphicsAbstraction {

	struct Vertex
	{
		glm::vec3 position;
		float uvX;
		glm::vec3 normal;
		float uvY;
		glm::vec4 color;
	};

	struct Mesh
	{
		uint32_t StartIndex;
		uint32_t Count;

		Ref<Buffer> VertexBuffer;
	};

	struct Scene
	{
		std::vector<Mesh> Meshes;

		Ref<Buffer> IndexBuffer;
	};

	class ModelImporter
	{
	public:
		static Scene LoadModels(const std::filesystem::path path);
	};

}