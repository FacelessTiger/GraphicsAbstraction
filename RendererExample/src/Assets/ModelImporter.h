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

	struct GeoSurface
	{
		uint32_t StartIndex;
		uint32_t Count;
	};

	struct Mesh
	{
		//std::string Name;
		uint32_t StartIndex;
		uint32_t Count;

		std::vector<glm::vec2> Positions;
		std::vector<uint32_t> Indices;

		//std::vector<GeoSurface> Surfaces;
		//Ref<Buffer> VertexBuffer, IndexBuffer;
	};

	struct Scene
	{
		std::vector<Mesh> Meshes;

		Ref<Buffer> IndexBuffer, VertexBuffer;
	};

	class ModelImporter
	{
	public:
		static Scene LoadModels(const std::filesystem::path path);
	};

}