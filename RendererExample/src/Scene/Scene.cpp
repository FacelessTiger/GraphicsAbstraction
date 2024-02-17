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

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());

		if (entity.HasComponent<LightComponent>()) Renderer::RemoveLight(entity.GetComponent<LightComponent>().RenderHandle);
		m_Registry.destroy(entity);
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };

		return {};
	}

}