#include "Application.h"

#include <Core/Core.h>
#include <core/Input.h>
#include <Renderer/Renderer.h>
#include <Assets/Material.h>
#include <Assets/Mesh.h>
#include <Assets/ShaderManager.h>
#include <Assets/AssetManager.h>

#include <GraphicsAbstraction/Core/DirectXExport.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <string>
#include <imgui.h>

namespace GraphicsAbstraction {

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_EditorCamera(70.0f, 16.0f / 9.0f, 0.1f)
	{
		GA_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(GA_BIND_EVENT_FN(Application::OnEvent));

		m_Scene = CreateRef<Scene>();
		m_SceneHierarchyPanel.SetContext(m_Scene);

		Renderer::Init(m_Window, true);
		Renderer::SetImGuiCallback(GA_BIND_EVENT_FN(Application::OnImGuiRender));

		uint8_t whiteData[] = { 0xff, 0xff, 0xff, 0xff };
		Ref<Texture> white = CreateRef<Texture>(whiteData, 1, 1);
		m_WhiteTexture = AssetManager::AddAsset(white);

		LoadGLTF("Assets/models/structure.glb");
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);
		ImGuiLayer::OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(GA_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(GA_BIND_EVENT_FN(Application::OnWindowResize));
		dispatcher.Dispatch<MouseButtonPressedEvent>(GA_BIND_EVENT_FN(Application::OnMouseButtonPressed));
	}

	void Application::Run()
	{
		while (m_Running)
		{
			auto t1 = std::chrono::high_resolution_clock::now();

			if (!m_Minimized)
			{
				Renderer::Render(m_EditorCamera);
			}

			m_Window->OnUpdate();
			m_EditorCamera.OnUpdate();

			auto t2 = std::chrono::high_resolution_clock::now();
			m_FrameTime = std::chrono::duration<double, std::milli>(t2 - t1).count();
		}
	}

	void Application::LoadGLTF(const std::filesystem::path path)
	{
		fastgltf::GltfDataBuffer data;
		data.loadFromFile(path);

		fastgltf::Asset gltf;
		fastgltf::Parser parser;
		constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

		auto type = fastgltf::determineGltfFileType(&data);
		if (type == fastgltf::GltfType::glTF)
		{
			auto load = parser.loadGLTF(&data, path.parent_path(), gltfOptions);
			if (load)
			{
				gltf = std::move(load.get());
			}
			else
			{
				GA_CORE_FATAL("Failed to load gltf: {}", fastgltf::to_underlying(load.error()));
				GA_CORE_ASSERT(false);
			}
		}
		else if (type == fastgltf::GltfType::GLB)
		{
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
		}
		else
		{
			GA_CORE_FATAL("Failed to determinte gltf container");
			GA_CORE_ASSERT(false);
		}

		std::vector<AssetHandle> builtInMeshes;
		std::vector<AssetHandle> builtInMaterials;
		std::vector<AssetHandle> builtInTextures;

		for (fastgltf::Image& image : gltf.images)
		{
			Ref<Texture> texture;

			std::visit(fastgltf::visitor{ [](auto& arg) {},
			[&](fastgltf::sources::URI& filepath) {
				GA_CORE_ASSERT(filepath.fileByteOffset == 0);
				GA_CORE_ASSERT(filepath.uri.isLocalPath());

				texture = CreateRef<Texture>(filepath.uri.path().data());
			},
			[&](fastgltf::sources::Vector& vector) {
				texture = CreateRef<Texture>(vector.bytes.data(), vector.bytes.size());
			},
			[&](fastgltf::sources::BufferView& view) {
				auto& bufferView = gltf.bufferViews[view.bufferViewIndex];
				auto& buffer = gltf.buffers[bufferView.bufferIndex];

				std::visit(fastgltf::visitor{ [](auto& arg) {},
				[&](fastgltf::sources::Vector& vector) {
					texture = CreateRef<Texture>(vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength);
				} },
				buffer.data);
			} },
			image.data);

			texture->Name = image.name;
			builtInTextures.push_back(AssetManager::AddAsset(texture));
		}

		for (fastgltf::Material& mat : gltf.materials)
		{
			Ref<Material> builtInMaterial = CreateRef<Material>();
			builtInMaterial->Name = mat.name;
			builtInMaterial->AlbedoFactor.r = mat.pbrData.baseColorFactor[0];
			builtInMaterial->AlbedoFactor.g = mat.pbrData.baseColorFactor[1];
			builtInMaterial->AlbedoFactor.b = mat.pbrData.baseColorFactor[2];
			builtInMaterial->AlbedoFactor.a = mat.pbrData.baseColorFactor[3];
			builtInMaterial->MetallicFactor = mat.pbrData.metallicFactor;
			builtInMaterial->RoughnessFactor = mat.pbrData.roughnessFactor;
			builtInMaterial->AO = 0.5f;
			
			if (mat.pbrData.baseColorTexture.has_value()) builtInMaterial->AlbedoTexture = builtInTextures[*gltf.textures[(*mat.pbrData.baseColorTexture).textureIndex].imageIndex];
			else builtInMaterial->AlbedoTexture = m_WhiteTexture;

			if (mat.pbrData.metallicRoughnessTexture.has_value())  builtInMaterial->MetallicRoughnessTexture = builtInTextures[*gltf.textures[(*mat.pbrData.metallicRoughnessTexture).textureIndex].imageIndex];
			else builtInMaterial->MetallicRoughnessTexture = m_WhiteTexture;

			builtInMaterial->RenderHandle = Renderer::UploadMaterial(builtInMaterial);
			builtInMaterials.push_back(AssetManager::AddAsset(builtInMaterial));
		}

		for (fastgltf::Mesh& mesh : gltf.meshes)
		{
			Ref<Mesh> builtInMesh = CreateRef<Mesh>();
			builtInMesh->Name = mesh.name;

			std::vector<uint32_t> indices;
			std::vector<Submesh> submeshes;

			for (auto&& p : mesh.primitives)
			{
				uint32_t indexOffset = indices.size();
				std::vector<Vertex> vertices;

				// load indexes
				{
					fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
					//indices.reserve(indexAccessor.count);

					fastgltf::iterateAccessor<uint16_t>(gltf, indexAccessor, [&](uint32_t index) {
						indices.push_back(index);
					});
				}

				// load vertex positions
				{
					fastgltf::Accessor& positionAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
					vertices.resize(positionAccessor.count);

					fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, positionAccessor, [&](glm::vec3 pos, size_t index) {
						Vertex vertex = {
							.position = pos,
							.uvX = 0,
							.normal = { 1, 0, 0 },
							.uvY = 0,
							.color = glm::vec4(1.0f),
						};
						vertices[index] = vertex;
					});
				}

				// load vertex normals
				auto normals = p.findAttribute("NORMAL");
				if (normals != p.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normals->second], [&](glm::vec3 normal, size_t index) {
						vertices[index].normal = normal;
					});
				}

				// load UVs
				auto uvs = p.findAttribute("TEXCOORD_0");
				if (uvs != p.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uvs->second], [&](glm::vec2 uv, size_t index) {
						vertices[index].uvX = uv.x;
						vertices[index].uvY = uv.y;
					});
				}

				// load vertex colors
				auto colors = p.findAttribute("COLOR_0");
				if (colors != p.attributes.end())
				{
					fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[colors->second], [&](glm::vec4 color, size_t index) {
						vertices[index].color = color;
					});
				}

				AssetHandle materialHandle;
				if (p.materialIndex.has_value())
					materialHandle = builtInMaterials[*p.materialIndex];
				else
					materialHandle = builtInMaterials[0];

				builtInMesh->Primitives.push_back({ materialHandle });
				submeshes.push_back({
					.Vertices = vertices,
					.IndexCount = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count,
					.IndexOffset = indexOffset,
					.MaterialHandle = ((Material&)*AssetManager::GetAsset(materialHandle)).RenderHandle
				});
			}

			builtInMesh->RenderHandle = Renderer::UploadMesh(submeshes, indices);
			builtInMeshes.push_back(AssetManager::AddAsset(builtInMesh));
		}

		std::vector<Entity> sceneNodes;
		for (fastgltf::Node& node : gltf.nodes)
		{
			Entity entity = m_Scene->CreateEntity(node.name.c_str());
			sceneNodes.push_back(entity);

			TransformComponent& entityTransform = entity.GetComponent<TransformComponent>();
			std::visit(fastgltf::visitor { 
			[&](fastgltf::Node::TransformMatrix matrix) {
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(glm::make_mat4(matrix.data()), entityTransform.Scale, entityTransform.Rotation, entityTransform.Translation, skew, perspective);
			},
			[&](fastgltf::Node::TRS transform) {
				entityTransform.Translation = { transform.translation[0], transform.translation[1], transform.translation[2] };
				entityTransform.Rotation = { transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2] };
				entityTransform.Scale = { transform.scale[0], transform.scale[1], transform.scale[2] };
			} },
			node.transform);

			if (node.meshIndex.has_value())
			{
				AssetHandle meshHandle = builtInMeshes[*node.meshIndex];
				Mesh& mesh = (Mesh&)*AssetManager::GetAsset(meshHandle);

				UUID modelID = Renderer::UploadModel(mesh.RenderHandle, entity.GetWorldTransform(), (uint32_t)entity);
				entity.AddComponent<MeshComponent>(meshHandle, modelID);
			}
		}

		for (int i = 0; i < gltf.nodes.size(); i++)
		{
			fastgltf::Node& node = gltf.nodes[i];
			RelationshipComponent& relationship = sceneNodes[i].GetComponent<RelationshipComponent>();

			for (auto& c : node.children)
			{
				relationship.Children.push_back(sceneNodes[c].GetUUID());
				sceneNodes[c].GetComponent<RelationshipComponent>().Parent = sceneNodes[i].GetUUID();
			}
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return true;
		}

		m_Minimized = false;
		Renderer::Resize(e.GetWidth(), e.GetHeight());

		return true;
	}

	bool Application::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == 0)
		{
			if (m_ViewportHovered && !Input::IsKeyPressed(Key::LeftAlt))
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}

		return false;
	}

	void Application::OnImGuiRender()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleColor();

		ImGui::PopStyleVar(4);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 400.0f;
		ImGui::DockSpace(ImGui::GetID("MyDockspace"));
		style.WindowMinSize.x = minWinSizeX;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportOffset = ImGui::GetWindowPos();
		glm::vec2 viewportPos = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };

		m_ViewportHovered = ImGui::IsWindowHovered();

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize.x != viewportPanelSize.x || m_ViewportSize.y != viewportPanelSize.y)
		{
			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
			m_EditorCamera.SetViewportSize(viewportPanelSize.x, viewportPanelSize.y);
		}
		ImGui::Image((ImTextureID)(uint64_t)Renderer::GetDrawImage()->GetSampledHandle(), { viewportPanelSize.x, viewportPanelSize.y });

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Settings");
		ImGui::Text("%.3fms %ifps", m_FrameTime, (int)(1.0 / m_FrameTime * 1000.0));
		if (ImGui::Checkbox("Vsync", &m_Vsync))
			Renderer::SetVsync(m_Vsync);	
		if (ImGui::Checkbox("Cull paused", &m_CullPaused))
			Renderer::SetCullPaused(m_CullPaused);
		if (ImGui::Button("Serialize shaders")) ShaderManager::Serialize();

		auto[mx, my] = ImGui::GetMousePos();
		mx -= viewportPos.x;
		my -= viewportPos.y;

		if (mx >= 0 && my >= 0 && mx < m_ViewportSize.x && my < m_ViewportSize.y)
		{
			mx *= (1920.0f / m_ViewportSize.x);
			my *= (1080.0f / m_ViewportSize.y);
			ImGui::Text("Mouse pos: (%d, %d)", (int)mx, (int)my);

			int pixelData = Renderer::GetEntityIDAt((int)mx, (int)my);
			ImGui::Text("Entity id: %d", pixelData);
			m_HoveredEntity = (pixelData == -1) ? Entity() : Entity((entt::entity)pixelData, m_Scene.Get());
		}
		ImGui::End();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_AssetPanel.OnImGuiRender();

		ImGui::End();
	}

}