#include "Entity.h"

namespace GraphicsAbstraction {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{ }

	glm::mat4 Entity::GetWorldTransform()
	{
		RelationshipComponent& relationship = GetComponent<RelationshipComponent>();
		glm::mat4 worldTransform = GetComponent<TransformComponent>().GetLocalTransform();

		Entity parent = m_Scene->GetEntityByUUID(relationship.Parent);
		while (parent)
		{
			worldTransform *= parent.GetComponent<TransformComponent>().GetLocalTransform();

			relationship = parent.GetComponent<RelationshipComponent>();
			parent = m_Scene->GetEntityByUUID(relationship.Parent);
		}

		return worldTransform;
	}

}