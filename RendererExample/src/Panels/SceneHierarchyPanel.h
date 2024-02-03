#pragma once

#include <Scene/Scene.h>
#include <Scene/Entity.h>

namespace GraphicsAbstraction {

	class SceneHierarchyPanel
	{
	public:
		void SetContext(const Ref<Scene>& context);

		void OnImGuiRender();
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};

}