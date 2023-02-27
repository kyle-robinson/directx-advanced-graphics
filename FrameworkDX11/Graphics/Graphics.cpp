#include "stdafx.h"
#include "Graphics.h"

#define NUM_VIEWPORTS 4

Graphics::Graphics()
{
    m_pShaderController = new ShaderController();
    m_pSamplerController = new SamplerController();
    m_pRasterizerController = new RasterizerController();
    m_pRenderTargetController = new RenderTargetController();
}

Graphics::~Graphics()
{
    delete m_pShaderController;
    m_pShaderController = nullptr;

    delete m_pSamplerController;
    m_pSamplerController = nullptr;

    delete m_pRasterizerController;
    m_pRasterizerController = nullptr;

    delete m_pRenderTargetController;
    m_pRenderTargetController = nullptr;
}

void Graphics::Initialize( HWND hWnd, UINT width, UINT height )
{
	m_viewWidth = width;
	m_viewHeight = height;
	InitializeDirectX( hWnd );
    InitializeShaders();
    InitializeRenderTargets();
    InitializeRasterizerStates();
    InitializeSamplerStates();
}

void Graphics::InitializeDirectX( HWND hWnd )
{
	m_pSwapChain = std::make_shared<Bind::SwapChain>( m_pContext.GetAddressOf(), m_pDevice.GetAddressOf(), hWnd, m_viewWidth, m_viewHeight );
    m_pBackBuffer = std::make_shared<Bind::BackBuffer>( m_pDevice.Get(), m_pSwapChain->Get() );
    m_pDepthStencil = std::make_shared<Bind::DepthStencil>( m_pDevice.Get(), m_viewWidth, m_viewHeight );
    for ( unsigned int i = 0; i < NUM_VIEWPORTS; i++ )
    {
        if ( i == 2 )
		{
			m_pViewports.push_back( std::make_shared<Bind::Viewport>( m_pContext.Get(), m_viewWidth / 2, m_viewHeight / 2 ) );
		}
		else
		{
			m_pViewports.push_back( std::make_shared<Bind::Viewport>( m_pContext.Get(), m_viewWidth, m_viewHeight ) );
		}
    }
    m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

void Graphics::InitializeShaders()
{
    try
    {
        // Standard shaders
        HRESULT hr = m_pShaderController->NewShader( "Basic", L"DefaultNoNormal.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "NormalMap", L"Default.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "NormalMap_TBN", L"TBN.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxMap", L"Parallax2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxMap_TBN", L"Parallax1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcclusionMap", L"ParallaxOcclusion2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcclusionMap_TBN", L"ParallaxOcclusion1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcclusionShadowMap", L"ParallaxOcclusionShadow2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "ParallaxOcclusionShadowMap_TBN", L"ParallaxOcclusionShadow1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "Depth", L"Depth.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewShader( "DepthLight", L"DepthLight.hlsl", m_pDevice.Get(), m_pContext.Get() );
        COM_ERROR_IF_FAILED( hr, "Failed to create a STANDARD SHADER!" );

        // Fullscreen shaders
        hr = m_pShaderController->NewFullScreenShader( "SolidColour", L"SolidColour.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Alpha", L"Bloom.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Fade", L"Fade.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Gaussian1", L"Gaussian1.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Gaussian2", L"Gaussian2.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "DepthOfField", L"DepthOfField.hlsl", m_pDevice.Get(), m_pContext.Get() );
        hr = m_pShaderController->NewFullScreenShader( "Final", L"FinalPass.hlsl", m_pDevice.Get(), m_pContext.Get() );
        COM_ERROR_IF_FAILED( hr, "Failed to create a FULLSCREEN SHADER!" );

        // Geometry shaders
        hr = m_pShaderController->NewGeometryShader( "BillBoard", L"Billboard.hlsl", m_pDevice.Get(), m_pContext.Get() );
        COM_ERROR_IF_FAILED( hr, "Failed to create a GEOMETRY SHADER!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}

void Graphics::InitializeRenderTargets()
{
    m_pRenderTargetController->CreateRenderTarget( "RTT", m_viewWidth, m_viewHeight, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "Depth", m_viewWidth, m_viewHeight, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "DepthOfField", m_viewWidth, m_viewHeight, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "Fade", m_viewWidth, m_viewHeight, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "Gaussian1", m_viewWidth / 2, m_viewHeight / 2, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "Gaussian2", m_viewWidth / 2, m_viewHeight / 2, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "DownSample", m_viewWidth / 2, m_viewHeight / 2, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "UpSample", m_viewWidth, m_viewHeight, m_pDevice.Get() );
    m_pRenderTargetController->CreateRenderTarget( "Alpha", m_viewWidth, m_viewHeight, m_pDevice.Get() );
}

void Graphics::InitializeRasterizerStates()
{
    m_pRasterizerController->CreateState( "None", Bind::Rasterizer::Type::NONE, m_pDevice.Get() );
    m_pRasterizerController->CreateState( "Back", Bind::Rasterizer::Type::BACK, m_pDevice.Get() );
    m_pRasterizerController->CreateState( "Wireframe", Bind::Rasterizer::Type::WIREFRAME, m_pDevice.Get() );
}

void Graphics::InitializeSamplerStates()
{
    m_pSamplerController->CreateState( "Wrap", Bind::Sampler::Type::WRAP, m_pDevice.Get() );
    m_pSamplerController->CreateState( "Border", Bind::Sampler::Type::BORDER, m_pDevice.Get() );
}