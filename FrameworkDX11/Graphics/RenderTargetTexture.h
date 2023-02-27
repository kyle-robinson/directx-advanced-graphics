#pragma once
#ifndef RENDERTARGETTEXTURE_H
#define RENDERTARGETTEXTURE_H

class RenderTargetTexture
{
public:
	RenderTargetTexture();
	RenderTargetTexture( ID3D11Device* pDevice, UINT width, UINT height );
	RenderTargetTexture( std::string name, ID3D11Device* pDevice, UINT width, UINT height );
	~RenderTargetTexture();

	bool Initialize( ID3D11Device* pDevice, UINT width, UINT height );
	void SetRenderTarget( ID3D11DeviceContext* pContext );

	inline std::string GetName() const noexcept { return m_sName; }
	inline ID3D11ShaderResourceView* GetTexture() const noexcept { return m_pShaderResourceView; }

private:
	void CleanUp();

	// Front buffer
	ID3D11Texture2D* m_pRenderTargetTexture;
	ID3D11RenderTargetView* m_pRenderTargetView;

	// Depth buffer
	ID3D11Texture2D* m_pDepthStencil = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView;

	ID3D11ShaderResourceView* m_pShaderResourceView;
	std::string m_sName;
};

#endif