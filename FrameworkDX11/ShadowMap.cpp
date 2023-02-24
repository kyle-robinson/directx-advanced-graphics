#include "stdafx.h"
#include "ShadowMap.h"

ShadowMap::ShadowMap( ID3D11Device* device, UINT width, UINT height ) :
	m_uWidth( width ), m_uHeight( height ), m_pDepthMapSRV( nullptr ), m_pDepthMapDSV( nullptr )
{

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_uWidth;
	texDesc.Height = m_uHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* depthMap = nullptr;
	HRESULT hr = device->CreateTexture2D( &texDesc, 0, &depthMap );
	if ( FAILED( hr ) )
		return;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView( depthMap, &dsvDesc, &m_pDepthMapDSV );
	if ( FAILED( hr ) )
		return;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView( depthMap, &srvDesc, &m_pDepthMapSRV );
	if ( FAILED( hr ) )
		return;

	depthMap->Release();
}

ShadowMap::~ShadowMap()
{
	CleanUp();
}

ID3D11ShaderResourceView* ShadowMap::DepthMapSRV()
{
	return m_pDepthMapSRV;
}

void ShadowMap::SetShadowMap( ID3D11DeviceContext* pContext )
{
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	pContext->OMSetRenderTargets( 1, renderTargets, m_pDepthMapDSV );
	pContext->ClearDepthStencilView( m_pDepthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

void ShadowMap::CleanUp()
{
	if ( m_pDepthMapSRV )
	{
		m_pDepthMapSRV->Release();
	}

	if ( m_pDepthMapDSV )
	{
		m_pDepthMapDSV->Release();
	}
}