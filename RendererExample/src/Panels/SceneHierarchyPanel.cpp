#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <Assets/Mesh.h>
#include <Assets/AssetManager.h>
#include <Renderer/Renderer.h>

namespace GraphicsAbstraction {

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			m_Context->m_Registry.each([&](auto entityID)
				{
					Entity entity(entityID, m_Context.Get());
					if (entity.GetComponent<RelationshipComponent>().Parent == -1)
						DrawEntityNode(entity);
				});

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}
		}

		ImGui::Begin("Properties");
		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);
		ImGui::End();

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& relationship = entity.GetComponent<RelationshipComponent>();
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
			m_SelectionContext = entity;

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			UUID uuid = entity.GetUUID();

			ImGui::SetDragDropPayload("ENTITY_DRAG", &uuid, sizeof(UUID));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
			{
				UUID uuid = *(UUID*)payload->Data;

				auto& childComponent = m_Context->GetEntityByUUID(uuid).GetComponent<RelationshipComponent>();
				if (childComponent.Parent == -1)
				{
					childComponent.Parent = entity.GetUUID();
					relationship.Children.push_back(uuid);
				}
			}

			ImGui::EndDragDropTarget();
		}

		if (opened)
		{
			for (const auto& uuid : relationship.Children)
			{
				Entity child = m_Context->GetEntityByUUID(uuid);
				DrawEntityNode(child);
			}

			ImGui::TreePop();
		}
	}

	template<typename T, typename UIFunction, typename RemoveCallback>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, RemoveCallback removeCallback)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

			if (ImGui::Button("+", { lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
			{
				removeCallback(component);
				entity.RemoveComponent<T>();
			}
		}
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		DrawComponent<T>(name, entity, uiFunction, [](T&) { });
	}

	static bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		bool changed = false;

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) 
			changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();

		return changed;
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<LightComponent>("Light", [&](LightComponent& component)
			{
				TransformComponent& transform = m_SelectionContext.GetComponent<TransformComponent>();
				component.RenderHandle = Renderer::UploadLight(transform.Translation, component.Color);
			});
			/*DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
			DisplayAddComponentEntry<TextComponent>("Text");*/

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [&](TransformComponent& component)
		{
			bool changed = false;
			if (DrawVec3Control("Translation", component.Translation)) changed = true;
			glm::vec3 rotation = glm::degrees(glm::eulerAngles(component.Rotation));
			if (DrawVec3Control("Rotation", rotation))
			{
				changed = true;
				component.Rotation = glm::quat(glm::radians(rotation));
			}
			if (DrawVec3Control("Scale", component.Scale, 1.0f)) changed = true;

			if (changed)
			{
				if (entity.HasComponent<MeshComponent>())
				{
					Renderer::UpdateTransform(entity.GetComponent<MeshComponent>().RenderHandle, entity.GetWorldTransform());
				}

				if (entity.HasComponent<LightComponent>())
				{
					LightComponent& light = entity.GetComponent<LightComponent>();
					Renderer::UpdateLight(light.RenderHandle, component.Translation, light.Color);
				}
			}
		});

		DrawComponent<MeshComponent>("Mesh", entity, [](MeshComponent& component)
		{
			Mesh& mesh = (Mesh&)*AssetManager::GetAsset(component.Mesh);

			ImGui::Text("Mesh name: %s", mesh.Name.c_str());
			for (auto& primitive : mesh.Primitives)
				ImGui::Text("Material name: %s", AssetManager::GetAsset(primitive.Material)->Name.c_str());
		});

		DrawComponent<LightComponent>("Light", entity, 
		[&](LightComponent& component) {
			if (DrawVec3Control("Color", component.Color, 1.0f))
			{
				TransformComponent& transform = entity.GetComponent<TransformComponent>();
				Renderer::UpdateLight(component.RenderHandle, transform.Translation, component.Color);
			}
		},
		[&](LightComponent& component) {
			Renderer::RemoveLight(component.RenderHandle);
		});
	}

	template<typename T, typename OnAdded>
	inline void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName, OnAdded onAdded)
	{
		if (!m_SelectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				onAdded(m_SelectionContext.AddComponent<T>());
				ImGui::CloseCurrentPopup();
			}
		}
	}

}
