#include "stdafx.h"
#include "ShadowMap.h"

ShadowMap::ShadowMap( ID3D11Device* device, UINT width, UINT height )
	: m_uWidth( width ), m_uHeight( height ), m_pDepthMapSRV( nullptr ), m_pDepthMapDSV( nullptr )
{
	try
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = m_uWidth;
		texDesc.Height = m_uHeight;
		texDesc.MipLevels = 1u;
		texDesc.ArraySize = 1u;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0u;
		texDesc.MiscFlags = 0u;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthMap;
		HRESULT hr = device->CreateTexture2D( &texDesc, nullptr, pDepthMap.GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create ShadowMap texture!" );

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = 0u;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;
		hr = device->CreateDepthStencilView( pDepthMap.Get(), &dsvDesc, m_pDepthMapDSV.GetAddressOf());
		COM_ERROR_IF_FAILED( hr, "Failed to create ShadowMap depth stencil view!" );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1u;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		hr = device->CreateShaderResourceView( pDepthMap.Get(), &srvDesc, m_pDepthMapSRV.GetAddressOf());
		COM_ERROR_IF_FAILED( hr, "Failed to create ShadowMap shader resource view!" );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
	}
}

ShadowMap::~ShadowMap() {}

void ShadowMap::SetShadowMap( ID3D11DeviceContext* pContext )
{
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	pContext->OMSetRenderTargets( 1u, renderTargets, m_pDepthMapDSV.Get() );
	pContext->ClearDepthStencilView( m_pDepthMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u );
}