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
		template<typename T, typename OnAdded>
		void DisplayAddComponentEntry(const std::string& entryName, OnAdded onAdded);

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};

}