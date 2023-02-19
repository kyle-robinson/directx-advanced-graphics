#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>

//shadows implmented with the help of 
//https://www.youtube.com/watch?v=CIGNP71FiG0
//https://www.youtube.com/watch?v=EPHjOUQ4rC8&list=PLqCJpWy5Fohd3S7ICFXwUomYW0Wv67pDD&index=64
//https://www.youtube.com/watch?v=tRsJ-b2ngIQ&list=PLqCJpWy5Fohd3S7ICFXwUomYW0Wv67pDD&index=65

using namespace DirectX;
/// <summary>
/// stores the shadow map data
/// </summary>
class ShadowMap
{

public:
	ShadowMap(ID3D11Device* device, UINT width, UINT height);
	~ShadowMap();
	ID3D11ShaderResourceView* DepthMapSRV();
	void SetShadowMap(ID3D11DeviceContext* dc);

private:
	UINT mWidth;
	UINT mHeight;
	ID3D11ShaderResourceView* mDepthMapSRV;
	ID3D11DepthStencilView* mDepthMapDSV;
	D3D11_VIEWPORT mViewport;
	
	void CleanUp();

};

