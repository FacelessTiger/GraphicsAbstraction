#include "ModelImporter.h"

#include <Core/Log.h>
#include <Core/Assert.h>
#include <Scene/Scene.h>
#include <Scene/Entity.h>
#include <Assets/Mesh.h>
#include <Assets/Material.h>
#include <Assets/AssetManager.h>

#include <glm/gtx/quaternion.hpp>
#include <Renderer/Renderer.h>

namespace GraphicsAbstraction {

	TempScene ModelImporter::LoadModels(const std::filesystem::path path, Ref<Scene>& tempScene)
	{
		/*fastgltf::GltfDataBuffer data;
		data.loadFromFile(path);

		fastgltf::Asset gltf;
		fastgltf::Parser parser;
		constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

		auto load = parser.loadBinaryGLTF(&data, path.parent_path(), gltfOptions);
		if (load)
		{
			gltf = std::move(load.get());
		}
		else
		{
			GA_CORE_FATAL("Failed to load gltf: {}", fastgltf::to_underlying(load.error()));
			GA_CORE_ASSERT(false);
		}

		std::vector<uint32_t> indices;
		std::vector<Vertex> vertices;

		std::vector<CullMesh> cullInputs;
		std::vector<Object> objects;
		std::vector<OldMaterial> materials;

		std::vector<AssetHandle> builtInMeshes;
		std::vector<AssetHandle> builtInMaterials;

		TempScene scene;
		for (fastgltf::Material& mat : gltf.materials)
		{
			Ref<Material> builtInMaterial = CreateRef<Material>();
			builtInMaterial->Name = mat.name;
			builtInMaterials.push_back(AssetManager::AddAsset(builtInMaterial));

			OldMaterial material;
			material.albedo.x = mat.pbrData.baseColorFactor[0];
			material.albedo.y = mat.pbrData.baseColorFactor[1];
			material.albedo.z = mat.pbrData.baseColorFactor[2];

			material.metallic = mat.pbrData.metallicFactor;
			material.roughness = mat.pbrData.roughnessFactor;
			material.ao = 1.0f;
			materials.push_back(material);
		}

		uint32_t i = 0;
		for (fastgltf::Mesh& mesh : gltf.meshes)
		{
			Ref<Mesh> builtInMesh = CreateRef<Mesh>();
			builtInMesh->Name = mesh.name;

			vertices.clear();

			uint32_t meshIndex = scene.Meshes.size();
			scene.Meshes.push_back({});

			for (auto&& p : mesh.primitives)
			{
				DrawIndexedIndirectCommand command((uint32_t)gltf.accessors[p.indicesAccessor.value()].count, 1, (uint32_t)indices.size(), 0, i);
				size_t initialVertex = vertices.size();

				// load indexes
				{
					fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
					//indices.reserve(indices.size() + indexAccessor.count);

					fastgltf::iterateAccessor<uint16_t>(gltf, indexAccessor, [&](uint32_t index) {
						indices.push_back(index + (uint32_t)initialVertex);
					});
				}

				// load vertex positions
				{
					fastgltf::Accessor& positionAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
					vertices.resize(vertices.size() + positionAccessor.count);

					fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, positionAccessor, [&](glm::vec3 pos, size_t index) {
						Vertex vertex = {
							.position = pos,
							.uvX = 0,
							.normal = { 1, 0, 0 },
							.uvY = 0,
							.color = glm::vec4(1.0f),
						};
						vertices[index + initialVertex] = vertex;
					});
				}

				// load vertex normals
				auto normals = p.findAttribute("NORMAL");
				if (normals != p.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normals->second], [&](glm::vec3 normal, size_t index) {
						vertices[index + initialVertex].normal = normal;
					});
				}

				// load UVs
				auto uvs = p.findAttribute("TEXCOORD_0");
				if (uvs != p.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uvs->second], [&](glm::vec2 uv, size_t index) {
						vertices[index + initialVertex].uvX = uv.x;
						vertices[index + initialVertex].uvY = uv.y;
					});
				}

				// load vertex colors
				auto colors = p.findAttribute("COLOR_0");
				if (colors != p.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[colors->second], [&](glm::vec4 color, size_t index) {
						vertices[index + initialVertex].color = color;
					});
				}

				uint32_t vertexBufferSize = (uint32_t)(vertices.size() * sizeof(Vertex));
				auto vertexBuffer = Buffer::Create(vertexBufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);

				auto staging = Buffer::Create(vertexBufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
				staging->SetData(vertices.data());
				Renderer::CopyNextFrame(staging, vertexBuffer, vertexBufferSize);

				scene.VertexBuffers.push_back(vertexBuffer);
				objects.push_back({ vertexBuffer->GetHandle(), (uint32_t)*p.materialIndex, meshIndex });
				builtInMesh->Primitives.push_back({ builtInMaterials[*p.materialIndex] });

				glm::vec3 minPos = vertices[initialVertex].position;
				glm::vec3 maxPos = vertices[initialVertex].position;
				for (int i = initialVertex; i < vertices.size(); i++)
				{
					minPos = glm::min(minPos, vertices[i].position);
					maxPos = glm::max(maxPos, vertices[i].position);
				}
				
				CullMesh meshAsset = { .command = command };
				meshAsset.bounds.origin = (maxPos + minPos) / 2.0f;
				meshAsset.bounds.extents = (maxPos - minPos) / 2.0f;
				meshAsset.bounds.sphereRadius = glm::length(meshAsset.bounds.extents);
				cullInputs.push_back(meshAsset);

				i++;
			}

			builtInMeshes.push_back(AssetManager::AddAsset(builtInMesh));
		}

		uint32_t indexBufferSize = (uint32_t)(indices.size() * sizeof(uint32_t));
		uint32_t cullInputSize = (uint32_t)(cullInputs.size() * sizeof(CullMesh));
		uint32_t objectsSize = (uint32_t)(objects.size() * sizeof(Object));
		uint32_t materialsSize = (uint32_t)(materials.size() * sizeof(OldMaterial));
		scene.IndexBuffer = Buffer::Create(indexBufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		scene.CullInput = Buffer::Create(cullInputSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		scene.Objects = Buffer::Create(objectsSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);
		scene.Materials = Buffer::Create(materialsSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);

		auto staging = Buffer::Create(indexBufferSize + cullInputSize + objectsSize + materialsSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
		staging->SetData(indices.data(), indexBufferSize);
		staging->SetData(cullInputs.data(), cullInputSize, indexBufferSize);
		staging->SetData(objects.data(), objectsSize, indexBufferSize + cullInputSize);
		staging->SetData(materials.data(), materialsSize, indexBufferSize + cullInputSize + objectsSize);
		Renderer::CopyNextFrame(staging, scene.IndexBuffer, indexBufferSize);
		Renderer::CopyNextFrame(staging, scene.CullInput, cullInputSize, indexBufferSize);
		Renderer::CopyNextFrame(staging, scene.Objects, objectsSize, indexBufferSize + cullInputSize);
		Renderer::CopyNextFrame(staging, scene.Materials, materialsSize, indexBufferSize + cullInputSize + objectsSize);

		for (fastgltf::Node& node : gltf.nodes)
		{
			Entity entity = tempScene->CreateEntity(node.name.c_str());
			std::shared_ptr<Node> newNode = std::make_shared<Node>();
			if (node.meshIndex.has_value())
			{
				entity.AddComponent<MeshComponent>(builtInMeshes[*node.meshIndex]);
				scene.Meshes[*node.meshIndex].Node = newNode;
			}

			scene.Nodes.push_back(newNode);
			std::visit(fastgltf::visitor { [&](fastgltf::Node::TransformMatrix matrix) {
				memcpy(&newNode->LocalTransform, matrix.data(), sizeof(matrix));
			}, 
			[&](fastgltf::Node::TRS transform) {
				glm::vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
				glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2]);
				glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

				glm::mat4 tm = glm::translate(glm::mat4(1.0f), tl);
				glm::mat4 rm = glm::toMat4(rot);
				glm::mat4 sm = glm::scale(glm::mat4(1.0f), sc);

				newNode->LocalTransform = tm * rm * sm;
			} },
			node.transform);
		}

		for (int i = 0; i < gltf.nodes.size(); i++)
		{
			fastgltf::Node& node = gltf.nodes[i];
			std::shared_ptr<Node>& sceneNode = scene.Nodes[i];

			for (auto& c : node.children)
			{
				sceneNode->Children.push_back(scene.Nodes[c]);
				scene.Nodes[c]->Parent = sceneNode;
			}
		}

		for (auto& node : scene.Nodes)
		{
			if (node->Parent.lock() == nullptr)
			{
				scene.TopNodes.push_back(node);
				node->RefreshTransform(glm::mat4(1.0f));
			}
		}*/

		return TempScene();
	}

}