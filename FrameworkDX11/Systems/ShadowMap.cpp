#include "stdafx.h"
#include "ShadowMap.h"
#include <imgui/imgui.h>

bool ShadowMap::Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, UINT width, UINT height )
{
	try
	{
		HRESULT hr = m_cbShadows.Initialize( pDevice, pContext );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'Shadow Map' constant buffer!" );

		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1u;
		texDesc.ArraySize = 1u;
		texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0u;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0u;
		texDesc.MiscFlags = 0u;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthMap;
		hr = pDevice->CreateTexture2D( &texDesc, 0u, depthMap.GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'Shadow Map' depth texture!" );

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = 0u;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0u;
		hr = pDevice->CreateDepthStencilView( depthMap.Get(), &dsvDesc, m_depthDSV.GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'Shadow Map' depth stencil view!" );

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0u;
		hr = pDevice->CreateShaderResourceView( depthMap.Get(), &srvDesc, m_depthSRV.GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'Shadow Map' shader resource view!" );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}
	return true;
}

void ShadowMap::BeginFrame( ID3D11DeviceContext* pContext )
{
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	pContext->OMSetRenderTargets( 1u, renderTargets, m_depthDSV.Get() );
	pContext->ClearDepthStencilView( m_depthDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u );
}

void ShadowMap::UpdateCB()
{
	// Setup mapping data
	ShadowData sdData;
	sdData.UseShadows = m_bUseShadows;

	// Add to constant buffer
	m_cbShadows.data.Shadows = sdData;
    if ( !m_cbShadows.ApplyChanges() ) return;
}

void ShadowMap::SpawnControlWindow()
{
	if ( ImGui::Begin( "Shadow Mapping", FALSE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
	{
		static bool useShadows = m_bUseShadows;
		ImGui::Checkbox( "Use Shadows?", &useShadows );
		m_bUseShadows = useShadows;
	}
	ImGui::End();
}