#pragma once

#include <filesystem>
#include <GraphicsAbstraction/GraphicsAbstraction.h>

#include <glm/glm.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

namespace GraphicsAbstraction {

	class Scene;

	struct OldMaterial
	{
		glm::vec3 albedo;
		float metallic;
		float roughness;
		float ao;
	};

	struct Node
	{
		std::weak_ptr<Node> Parent;
		std::vector<std::shared_ptr<Node>> Children;

		glm::mat4 LocalTransform;
		glm::mat4 WorldTransform;

		void RefreshTransform(const glm::mat4& parentMatrix)
		{
			WorldTransform = parentMatrix * LocalTransform;
			for (auto c : Children)
				c->RefreshTransform(WorldTransform);
		}
	};

	struct CockMesh : public Node
	{
		std::shared_ptr<Node> Node;
	};

	struct TempScene
	{
		Ref<Buffer> IndexBuffer, Objects, Materials, CullInput;
		std::vector<Ref<Buffer>> VertexBuffers;

		std::vector<CockMesh> Meshes;
		std::vector<std::shared_ptr<Node>> Nodes;
		std::vector<std::shared_ptr<Node>> TopNodes;
	};

	class ModelImporter
	{
	public:
		static TempScene LoadModels(const std::filesystem::path path, Ref<Scene>& tempScene);
	};

}