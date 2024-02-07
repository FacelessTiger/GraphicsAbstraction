#pragma once

#include <Assets/Asset.h>

#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <entt.hpp>

namespace GraphicsAbstraction {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID id)
			: ID(id)
		{ }
	};

	struct RelationshipComponent
	{
		UUID Parent = (uint64_t)-1;
		std::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag)
		{ }
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::quat Rotation = { 0.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{ }

		glm::mat4 GetLocalTransform() const
		{
			glm::mat4 rotation = glm::toMat4(Rotation);
			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct MeshComponent
	{
		AssetHandle Mesh = 0;
		UUID RenderHandle;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(AssetHandle mesh, UUID renderHandle)
			: Mesh(mesh), RenderHandle(renderHandle)
		{ }
	};

	struct LightComponent
	{
		glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
		UUID RenderHandle;

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
		LightComponent(const glm::vec3& color)
			: Color(color)
		{ }
	};

}