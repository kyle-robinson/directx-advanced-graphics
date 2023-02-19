#ifndef SHADOWMAP_H
#define SHADOWMAP_H

class ShadowMap
{
public:
	ShadowMap( ID3D11Device* device, UINT width, UINT height );
	~ShadowMap();

	void SetShadowMap( ID3D11DeviceContext* pContext );

	inline ID3D11DepthStencilView* GetDepthStencilPtr() const noexcept { return m_pDepthMapDSV.Get(); }
	inline ID3D11DepthStencilView* const* GetDepthStencilDPtr() const noexcept { return m_pDepthMapDSV.GetAddressOf(); }

	inline ID3D11ShaderResourceView* GetDepthTexturePtr() const noexcept { return m_pDepthMapSRV.Get(); }
	inline ID3D11ShaderResourceView* const* GetDepthTextureDPtr() const noexcept { return m_pDepthMapSRV.GetAddressOf(); }

private:
	UINT m_uWidth, m_uHeight;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDepthMapDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pDepthMapSRV;
};

#endif