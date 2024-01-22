#include "ModelImporter.h"

#include <Core/Log.h>
#include <Core/Assert.h>

#include <Renderer/Renderer.h>

namespace GraphicsAbstraction {

	Scene ModelImporter::LoadModels(const std::filesystem::path path)
	{
		fastgltf::GltfDataBuffer data;
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

		std::vector<uint16_t> indices;
		std::vector<Vertex> vertices;

		Scene scene;
		for (fastgltf::Mesh& mesh : gltf.meshes)
		{
			vertices.clear();

			for (auto&& p : mesh.primitives)
			{
				Mesh meshAsset;
				meshAsset.StartIndex = (uint32_t)indices.size();
				meshAsset.Count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

				size_t initialVertex = vertices.size();

				// load indexes
				{
					fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
					//indices.reserve(indices.size() + indexAccessor.count);

					fastgltf::iterateAccessor<uint16_t>(gltf, indexAccessor, [&](uint16_t index) {
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
				meshAsset.VertexBuffer = Buffer::Create(vertexBufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);

				auto staging = Buffer::Create(vertexBufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
				staging->SetData(vertices.data());
				Renderer::CopyNextFrame(staging, meshAsset.VertexBuffer, vertexBufferSize);

				glm::vec3 minPos = vertices[initialVertex].position;
				glm::vec3 maxPos = vertices[initialVertex].position;
				for (int i = initialVertex; i < vertices.size(); i++)
				{
					minPos = glm::min(minPos, vertices[i].position);
					maxPos = glm::max(maxPos, vertices[i].position);
				}

				meshAsset.Bounds.origin = (maxPos + minPos) / 2.0f;
				meshAsset.Bounds.extents = (maxPos - minPos) / 2.0f;
				meshAsset.Bounds.sphereRadius = glm::length(meshAsset.Bounds.extents);
				scene.Meshes.push_back(meshAsset);
			}

			constexpr bool overrideColors = false;
			if (overrideColors)
			{
				for (Vertex& vertex : vertices) vertex.color = glm::vec4(vertex.normal, 1.0f);
			}
		}

		uint32_t indexBufferSize = (uint16_t)(indices.size() * sizeof(uint16_t));
		scene.IndexBuffer = Buffer::Create(indexBufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst, BufferFlags::DeviceLocal);

		auto staging = Buffer::Create(indexBufferSize, BufferUsage::TransferSrc, BufferFlags::Mapped);
		staging->SetData(indices.data());
		Renderer::CopyNextFrame(staging, scene.IndexBuffer, indexBufferSize);

		return scene;
	}

}