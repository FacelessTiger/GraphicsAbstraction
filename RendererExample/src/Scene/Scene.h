#pragma once

#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include <Scene/Components.h>
#include <entt.hpp>

namespace GraphicsAbstraction {

	class Entity;
	class SceneHierarchyPanel;

	class Scene : public RefCounted
	{
	public:
		virtual ~Scene() = default;

		Entity CreateEntity(const std::string& name = std::string());
		Entity GetEntityByUUID(UUID uuid);
	private:
		friend class Entity;
		friend class SceneHierarchyPanel;

		entt::registry m_Registry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;
	};

}