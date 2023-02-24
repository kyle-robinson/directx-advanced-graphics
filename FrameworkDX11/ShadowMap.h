#pragma once
#ifndef SHADOWMAP_H
#define SHADOWMAP_H

// REFERENCES
// https://www.youtube.com/watch?v=CIGNP71FiG0
// https://www.youtube.com/watch?v=EPHjOUQ4rC8&list=PLqCJpWy5Fohd3S7ICFXwUomYW0Wv67pDD&index=64
// https://www.youtube.com/watch?v=tRsJ-b2ngIQ&list=PLqCJpWy5Fohd3S7ICFXwUomYW0Wv67pDD&index=65

class ShadowMap
{
public:
	ShadowMap( ID3D11Device* device, UINT width, UINT height );
	~ShadowMap();

	ID3D11ShaderResourceView* DepthMapSRV();
	void SetShadowMap( ID3D11DeviceContext* pContext );

private:
	void CleanUp();

	UINT m_uWidth;
	UINT m_uHeight;

	D3D11_VIEWPORT m_viewport;
	ID3D11DepthStencilView* m_pDepthMapDSV;
	ID3D11ShaderResourceView* m_pDepthMapSRV;
};

#endif