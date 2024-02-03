#pragma once

#include <Assets/Asset.h>
#include <Assets/Material.h>

namespace GraphicsAbstraction {

	class AssetPanel
	{
	public:
		void OnImGuiRender();
	private:
		AssetHandle m_SelectedMaterial = 0;
	};

}