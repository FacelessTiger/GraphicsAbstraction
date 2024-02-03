#include "Scene.h"

#include <Scene/Entity.h>
#include <Renderer/Renderer.h>

namespace GraphicsAbstraction {

	Entity Scene::CreateEntity(const std::string& name)
	{
		UUID uuid;

		Entity entity(m_Registry.create(), this);
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<RelationshipComponent>();
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);

		m_EntityMap[uuid] = entity;
		return entity;
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };

		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{ }

	template<>
	void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent& component)
	{ }

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{ }

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{ }

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{ }

	template<>
	void Scene::OnComponentAdded(Entity entity, LightComponent& component)
	{
		component.RenderHandle = Renderer::UploadLight(glm::vec3(0), component.Color);
	}

}