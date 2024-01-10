#pragma once

#include <GraphicsAbstraction/Renderer/Shader.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <dxc/dxcapi.h>

namespace GraphicsAbstraction {

	class D3D12Shader : public Shader
	{
	public:
		D3D12_SHADER_BYTECODE Shader;
		uint32_t ID;

		ShaderStage Stage;
	public:
		D3D12Shader(const std::string& path, ShaderStage stage);
		virtual ~D3D12Shader();

		static D3D12Shader* GetShaderByID(uint32_t id);
	private:
		std::string ReadAndPreProcessFile(const std::string& path);
	private:
		Microsoft::WRL::ComPtr<IDxcBlob> m_Blob;
	};

}