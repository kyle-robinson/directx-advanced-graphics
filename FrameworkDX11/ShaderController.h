#pragma once
#ifndef SHADERCONTROLLER_H
#define SHADERCONTROLLER_H

/// <summary>
/// Controller for all of the shaders in the application.
/// </summary>
class ShaderController
{
public:
	enum class Layout
	{
		Default = 0,
		Instance,
		Terrain,
		Animation
	};

	/// <summary>
	/// Container to hold all the shader data.
	/// </summary>
	struct ShaderData
	{
		ShaderData()
		{
			m_sName = "Empty";
			m_pVertexShader = nullptr;
			m_pGeometryShader = nullptr;
			m_pHullShader = nullptr;
			m_pDomainShader = nullptr;
			m_pPixelShader = nullptr;
			m_pVertexLayout = nullptr;
		}

		ShaderData(
			std::string name,
			ID3D11VertexShader* pVertexShader,
			ID3D11PixelShader* pPixelShader,
			ID3D11InputLayout* pVertexLayout,
			ID3D11GeometryShader* pGeometryShader = nullptr,
			ID3D11HullShader* pHullShader = nullptr,
			ID3D11DomainShader* pDomainShader = nullptr
		)
		{
			m_sName = name;
			m_pVertexShader = pVertexShader;
			m_pGeometryShader = pGeometryShader;
			m_pHullShader = pHullShader;
			m_pDomainShader = pDomainShader;
			m_pPixelShader = pPixelShader;
			m_pVertexLayout = pVertexLayout;
		}

		void CleanUp()
		{
			if ( m_pVertexShader )
				m_pVertexShader->Release();

			if ( m_pPixelShader )
				m_pPixelShader->Release();

			if ( m_pVertexLayout )
				m_pVertexLayout->Release();

			if ( m_pGeometryShader )
				m_pGeometryShader->Release();

			if ( m_pHullShader )
				m_pHullShader->Release();

			if ( m_pDomainShader )
				m_pDomainShader->Release();
		}

		std::string m_sName;
		ID3D11VertexShader* m_pVertexShader;
		ID3D11HullShader* m_pHullShader;
		ID3D11DomainShader* m_pDomainShader;
		ID3D11GeometryShader* m_pGeometryShader;
		ID3D11PixelShader* m_pPixelShader;
		ID3D11InputLayout* m_pVertexLayout;
	};

	ShaderController();
	~ShaderController();

	ShaderData GetFullScreenShaderByName( std::string name );
	ShaderData GetFullScreenShaderByNumber( int num );
	ShaderData GetShaderByName( std::string name );
	ShaderData GetShaderData();
	void SetShaderData( UINT shaderNum );

	bool NewShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	bool NewFullScreenShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	bool NewGeometryShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	bool NewTessellationShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	bool NewAnimationShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

	inline std::vector<ShaderData> GetFSShaderList() const noexcept{ return m_vFullScreenShaderData; }
	inline std::vector<ShaderData> GetShaderList() const noexcept { return m_vShaderData; }
	inline ShaderData GetGeometryData() const noexcept { return m_geometryData; }

private:
	HRESULT CompileShaderFromFile( std::wstring fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut );
	bool NewVertexShader( std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, Layout layout );
	bool NewPixelShader( std::wstring fileName, ID3D11Device* pDevice );
	bool NewGeometryShader( std::wstring fileName, ID3D11Device* pDevice );
	bool NewHullShader( std::wstring fileName, ID3D11Device* pDevice );
	bool NewDomainShader( std::wstring fileName, ID3D11Device* pDevice );
	void CleanUp();

	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11HullShader* m_pHullShader = nullptr;
	ID3D11DomainShader* m_pDomainShader = nullptr;
	ID3D11GeometryShader* m_pGeometryShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pVertexLayout = nullptr;

	ID3D11InputLayout* m_pQuadLayout = nullptr;
	ID3D11VertexShader* m_pQuadVS = nullptr;
	ID3D11PixelShader* m_pQuadPS = nullptr;

	std::vector<ShaderData> m_vFullScreenShaderData;
	std::vector<ShaderData> m_vShaderData;
	ShaderData m_geometryData;
	UINT m_uCurrentShader;
};

#endif