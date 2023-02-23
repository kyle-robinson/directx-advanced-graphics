#include "stdafx.h"
#include "Graphics.h"

Graphics::Graphics()
{
    m_pShaderController = new ShaderController();
}

Graphics::~Graphics()
{
    delete m_pShaderController;
    m_pShaderController = nullptr;
}

void Graphics::Initialize( HWND hWnd, UINT width, UINT height )
{
	m_viewWidth = width;
	m_viewHeight = height;
	InitializeDirectX( hWnd );
    InitializeShaders();
}

void Graphics::InitializeDirectX( HWND hWnd )
{
	m_pSwapChain = std::make_shared<Bind::SwapChain>( m_pContext.GetAddressOf(), m_pDevice.GetAddressOf(), hWnd, m_viewWidth, m_viewHeight );
    m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

void Graphics::InitializeShaders()
{
    try
    {
        // Standard shaders
        HRESULT hr = m_pShaderController->NewShader( "NoEffects", L"DefaultNoNormal.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "NormalMap", L"Default.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "NormalMap_TBN_VS", L"TBN.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxMapping_TBN_VS", L"Parallax1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxMapping", L"Parallax2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcMapping_TBN_VS", L"ParallaxOcclusion1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcMapping", L"ParallaxOcclusion2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcShadingMapping_TBN_VS", L"ParallaxOcclusionShadow1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcShadingMapping", L"ParallaxOcclusionShadow2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "Depth(NotFullShader)", L"Depth.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "DepthLight(NotFullShader)", L"DepthLight.hlsl", m_pDevice.Get(), m_pContext.Get() );

        // Fullscreen shaders
        hr = m_pShaderController->NewFullScreenShader( "SolidColour", L"SolidColour.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Gaussian1", L"Gaussian1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Fianl", L"FinalPass.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Alpha", L"Bloom.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Gaussian2", L"Gaussian2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Fade", L"Fade.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "DepthOfField", L"DepthOfField.hlsl", m_pDevice.Get(), m_pContext.Get() );

        // Geometry shaders
        hr = m_pShaderController->NewGeometryShader( "BillBord", L"Billboard.hlsl", m_pDevice.Get(), m_pContext.Get() );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}