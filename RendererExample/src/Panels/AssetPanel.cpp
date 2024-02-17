#include "AssetPanel.h"

#include <Assets/AssetManager.h>
#include <Assets/Material.h>
#include <Renderer/Texture.h>
#include <Renderer/Renderer.h>
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

namespace GraphicsAbstraction {

	void AssetPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Panel");

		for (const auto& [handle, asset] : AssetManager::GetMap())
		{
			AssetType type = asset->GetType();
			if (type == AssetType::Material)
			{
				if (ImGui::Button(asset->Name.c_str()))
				{
					m_SelectedMaterial = handle;
				}
			}
			else
			{
				ImGui::Text("%s: %s", AssetTypeToString(type).data(), asset->Name.c_str());
			}
		}

		ImGui::End();

		ImGui::Begin("Material viewer");

		if (m_SelectedMaterial)
		{
			Material& material = (Material&)*AssetManager::GetAsset(m_SelectedMaterial);
			Texture& albedoTexture = (Texture&)*AssetManager::GetAsset(material.AlbedoTexture);
			Texture& metallicRoughnessTexture = (Texture&)*AssetManager::GetAsset(material.MetallicRoughnessTexture);

			ImGui::Text("Material name: %s", material.Name.c_str());
			if (ImGui::ColorEdit4("Albedo factor", glm::value_ptr(material.AlbedoFactor))) Renderer::UpdateMaterialAlbedoFactor(material.RenderHandle, material.AlbedoFactor);
			ImGui::Image((ImTextureID)albedoTexture.GetImage()->GetSampledHandle(), { 128, 128 });
			ImGui::Image((ImTextureID)metallicRoughnessTexture.GetImage()->GetSampledHandle(), { 128, 128 });
			ImGui::Text("Metallic factor: %f", material.MetallicFactor);
			ImGui::Text("Roughness factor: %f", material.RoughnessFactor);
		}

		ImGui::End();
	}

}