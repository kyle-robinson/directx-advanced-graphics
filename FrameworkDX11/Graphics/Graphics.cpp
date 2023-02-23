#include "stdafx.h"
#include "Graphics.h"

void Graphics::Initialize( HWND hWnd, UINT width, UINT height )
{
	m_viewWidth = width;
	m_viewHeight = height;
	InitializeDirectX( hWnd );
}

void Graphics::InitializeDirectX( HWND hWnd )
{
	m_pSwapChain = std::make_shared<Bind::SwapChain>( m_pContext.GetAddressOf(), m_pDevice.GetAddressOf(), hWnd, m_viewWidth, m_viewHeight );
    m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}