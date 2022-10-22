#include "stdafx.h"
#include "Graphics.h"

bool Graphics::Initialize( HWND hWnd, UINT width, UINT height )
{
	m_viewWidth = width;
	m_viewHeight = height;

	InitializeDirectX( hWnd );
	if ( !InitializeShaders() )
		return false;
	
	return true;
}

void Graphics::InitializeDirectX( HWND hWnd )
{
	m_pSwapChain = std::make_shared<Bind::SwapChain>( m_pContext.GetAddressOf(), m_pDevice.GetAddressOf(), hWnd, m_viewWidth, m_viewHeight);
    m_pRenderTarget = std::make_shared<Bind::RenderTarget>( m_pDevice.Get(), m_pSwapChain->GetSwapChain() );
    m_pDepthStencil = std::make_shared<Bind::DepthStencil>( m_pDevice.Get(), m_viewWidth, m_viewHeight );
	m_pViewport = std::make_shared<Bind::Viewport>( m_pContext.Get(), m_viewWidth, m_viewHeight );
    
    m_pRasterizerStates.emplace( Bind::Rasterizer::Type::SOLID, std::make_shared<Bind::Rasterizer>( m_pDevice.Get(), Bind::Rasterizer::Type::SOLID ) );
    m_pRasterizerStates.emplace( Bind::Rasterizer::Type::WIREFRAME, std::make_shared<Bind::Rasterizer>( m_pDevice.Get(), Bind::Rasterizer::Type::WIREFRAME ) );

    m_pSamplerStates.emplace( Bind::Sampler::Type::ANISOTROPIC, std::make_shared<Bind::Sampler>( m_pDevice.Get(), Bind::Sampler::Type::ANISOTROPIC ) );
	m_pSamplerStates.emplace( Bind::Sampler::Type::BILINEAR, std::make_shared<Bind::Sampler>( m_pDevice.Get(), Bind::Sampler::Type::BILINEAR ) );
	m_pSamplerStates.emplace( Bind::Sampler::Type::POINT, std::make_shared<Bind::Sampler>( m_pDevice.Get(), Bind::Sampler::Type::POINT ) );

    m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

bool Graphics::InitializeShaders()
{
	try
	{
		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// Create the shaders
		HRESULT hr = m_vertexShader.Initialize( m_pDevice, L"Resources\\Shaders\\shader.fx", layout, ARRAYSIZE(layout));
		COM_ERROR_IF_FAILED( hr, "Failed to create vertex shader!" );
		hr = m_pixelShader.Initialize( m_pDevice, L"Resources\\Shaders\\shader.fx" );
		COM_ERROR_IF_FAILED( hr, "Failed to create pixel shader!" );

		// Bind shaders to the pipeline
		Shaders::BindShaders( m_pContext.Get(), m_vertexShader, m_pixelShader );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}

	return true;
}

void Graphics::BeginFrame()
{
	// clear render target/depth stencil
    static float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    m_pRenderTarget->BindAsBuffer( m_pContext.Get(), m_pDepthStencil.get(), clearColor);
    m_pDepthStencil->ClearDepthStencil( m_pContext.Get() );

    // set render state
    m_pRasterizerStates[Bind::Rasterizer::Type::SOLID]->Bind( m_pContext.Get() );
    m_pSamplerStates[Bind::Sampler::Type::ANISOTROPIC]->Bind( m_pContext.Get() );

    // bind shaders
    Shaders::BindShaders( m_pContext.Get(), m_vertexShader, m_pixelShader );
}

void Graphics::EndFrame()
{
	// Unbind render target
	m_pRenderTarget->BindAsNull( m_pContext.Get() );

	// Present frame
	HRESULT hr = m_pSwapChain->GetSwapChain()->Present( 1u, NULL );
	if ( FAILED( hr ) )
	{
		hr == DXGI_ERROR_DEVICE_REMOVED ?
			ErrorLogger::Log( m_pDevice->GetDeviceRemovedReason(), "Swap Chain. Graphics device removed!" ) :
			ErrorLogger::Log( hr, "Swap Chain failed to render frame!" );
		exit( -1 );
	}
}