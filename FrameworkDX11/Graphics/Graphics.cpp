#include "stdafx.h"
#include "Graphics.h"

bool Graphics::Initialize( HWND hWnd, UINT width, UINT height )
{
	m_viewWidth = width;
	m_viewHeight = height;

	InitializeDirectX( hWnd );

	if ( !InitializeShaders() )
		return false;

	if ( !InitializeRTT() )
		return false;
	
	return true;
}

void Graphics::InitializeDirectX( HWND hWnd )
{
	m_pSwapChain = std::make_shared<Bind::SwapChain>( m_pContext.GetAddressOf(), m_pDevice.GetAddressOf(), hWnd, m_viewWidth, m_viewHeight );
    m_pBackBuffer = std::make_shared<Bind::BackBuffer>( m_pDevice.Get(), m_pSwapChain->GetSwapChain() );
    m_pDepthStencil = std::make_shared<Bind::DepthStencil>( m_pDevice.Get(), m_viewWidth, m_viewHeight );
	m_pViewport = std::make_shared<Bind::Viewport>( m_pContext.Get(), m_viewWidth, m_viewHeight );
    
	m_pRenderTarget = std::make_shared<Bind::RenderTarget>( m_pDevice.Get(), m_viewWidth, m_viewHeight );
	m_pRenderTargetNormalDepth = std::make_shared<Bind::RenderTarget>( m_pDevice.Get(), m_viewWidth, m_viewHeight );
	for ( uint32_t i = 0u; i < BUFFER_COUNT; i++ )
		m_pRenderTargetsDeferred.emplace( (Bind::RenderTarget::Type)i, std::make_shared<Bind::RenderTarget>( m_pDevice.Get(), m_viewWidth, m_viewHeight, (Bind::RenderTarget::Type)i ) );
    
    m_pRasterizerStates.emplace( Bind::Rasterizer::Type::SOLID, std::make_shared<Bind::Rasterizer>( m_pDevice.Get(), Bind::Rasterizer::Type::SOLID ) );
    m_pRasterizerStates.emplace( Bind::Rasterizer::Type::SKYSPHERE, std::make_shared<Bind::Rasterizer>( m_pDevice.Get(), Bind::Rasterizer::Type::SKYSPHERE ) );
    m_pRasterizerStates.emplace( Bind::Rasterizer::Type::WIREFRAME, std::make_shared<Bind::Rasterizer>( m_pDevice.Get(), Bind::Rasterizer::Type::WIREFRAME ) );

    m_pSamplerStates.emplace( Bind::Sampler::Type::ANISOTROPIC_WRAP, std::make_shared<Bind::Sampler>( m_pDevice.Get(), Bind::Sampler::Type::ANISOTROPIC_WRAP, false, 0u ) );
	m_pSamplerStates.emplace( Bind::Sampler::Type::ANISOTROPIC_CLAMP, std::make_shared<Bind::Sampler>( m_pDevice.Get(), Bind::Sampler::Type::ANISOTROPIC_CLAMP, true, 1u ) );
	m_pSamplerStates.emplace( Bind::Sampler::Type::BILINEAR, std::make_shared<Bind::Sampler>( m_pDevice.Get(), Bind::Sampler::Type::BILINEAR ) );
	m_pSamplerStates.emplace( Bind::Sampler::Type::POINT, std::make_shared<Bind::Sampler>( m_pDevice.Get(), Bind::Sampler::Type::POINT ) );

	m_pSamplerStates[Bind::Sampler::Type::ANISOTROPIC_WRAP]->Bind( m_pContext.Get() );
	m_pSamplerStates[Bind::Sampler::Type::ANISOTROPIC_CLAMP]->Bind( m_pContext.Get() );
    m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}

bool Graphics::InitializeShaders()
{
	try
	{
		// Define input layout for cube
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// Create the cube shaders
		HRESULT hr = m_vertexShader.Initialize( m_pDevice, L"Resources\\Shaders\\shader_VS.hlsl", layout, ARRAYSIZE( layout ) );
		COM_ERROR_IF_FAILED( hr, "Failed to create cube vertex shader!" );
		hr = m_pixelShader.Initialize( m_pDevice, L"Resources\\Shaders\\shader_PS.hlsl" );
		COM_ERROR_IF_FAILED( hr, "Failed to create cube pixel shader!" );

		// Define input layout for normal/depth pass
		D3D11_INPUT_ELEMENT_DESC layoutDR[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// Create the cube shaders
		hr = m_vertexShaderDR.Initialize( m_pDevice, L"Resources\\Shaders\\shaderDR_VS.hlsl", layoutDR, ARRAYSIZE( layoutDR ) );
		COM_ERROR_IF_FAILED( hr, "Failed to create deferred vertex shader!" );
		hr = m_pixelShaderDR.Initialize( m_pDevice, L"Resources\\Shaders\\shaderDR_PS.hlsl" );
		COM_ERROR_IF_FAILED( hr, "Failed to create deferred pixel shader!" );

		// Define input layout for models
		D3D11_INPUT_ELEMENT_DESC layoutOBJ[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// Create the model shaders
		hr = m_vertexShaderOBJ.Initialize( m_pDevice, L"Resources\\Shaders\\shaderOBJ_VS.hlsl", layoutOBJ, ARRAYSIZE( layoutOBJ ) );
		COM_ERROR_IF_FAILED( hr, "Failed to create model vertex shader!" );
		hr = m_pixelShaderOBJ.Initialize( m_pDevice, L"Resources\\Shaders\\shaderOBJ_PS.hlsl" );
		COM_ERROR_IF_FAILED( hr, "Failed to create model pixel shader!" );

		// Define input layout for textures
		D3D11_INPUT_ELEMENT_DESC layoutTEX[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		// Create the texture shaders
		hr = m_vertexShaderTEX.Initialize( m_pDevice, L"Resources\\Shaders\\shaderTEX_VS.hlsl", layoutTEX, ARRAYSIZE( layoutTEX ) );
		COM_ERROR_IF_FAILED( hr, "Failed to create texture vertex shader!" );
		hr = m_pixelShaderTEX.Initialize( m_pDevice, L"Resources\\Shaders\\shaderTEX_PS.hlsl" );
		COM_ERROR_IF_FAILED( hr, "Failed to create texture pixel shader!" );

		// Define input layout for normal/depth pass
		D3D11_INPUT_ELEMENT_DESC layoutNRM[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		// Create the normal/depth pass shaders
		hr = m_vertexShaderNRM.Initialize( m_pDevice, L"Resources\\Shaders\\shaderNRM_VS.hlsl", layoutNRM, ARRAYSIZE( layoutNRM ) );
		COM_ERROR_IF_FAILED( hr, "Failed to create normal/depth vertex shader!" );
		hr = m_pixelShaderNRM.Initialize( m_pDevice, L"Resources\\Shaders\\shaderNRM_PS.hlsl" );
		COM_ERROR_IF_FAILED( hr, "Failed to create normal/depth pixel shader!" );

		// Define input layout for RTT
		D3D11_INPUT_ELEMENT_DESC layoutPP[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// Create the RTT shaders
		hr = m_vertexShaderPP.Initialize( m_pDevice, L"Resources\\Shaders\\shaderPP_VS.hlsl", layoutPP, ARRAYSIZE( layoutPP ) );
		COM_ERROR_IF_FAILED( hr, "Failed to create RTT vertex shader!" );
		hr = m_pixelShaderPP.Initialize( m_pDevice, L"Resources\\Shaders\\shaderPP_PS.hlsl" );
		COM_ERROR_IF_FAILED( hr, "Failed to create RTT pixel shader!" );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}

	return true;
}

bool Graphics::InitializeRTT()
{
	// Initialize quad for post-processing
	if ( !m_quad.Initialize( m_pDevice.Get() ) )
		return false;
	return true;
}

void Graphics::BeginFrame()
{
	// Clear render target/depth stencil
    m_pRenderTarget->Bind( m_pContext.Get(), m_pDepthStencil.get(), m_clearColor );
    m_pDepthStencil->ClearDepthStencil( m_pContext.Get() );
}

void Graphics::BeginFrameNormal()
{
	// Clear render target/depth stencil
	m_pRenderTargetNormalDepth->Bind( m_pContext.Get(), m_pDepthStencil.get(), m_clearColor );
	m_pDepthStencil->ClearDepthStencil( m_pContext.Get() );
}

void Graphics::BeginFrameDeferred()
{
	// Clear render target/depth stencil
	ID3D11RenderTargetView* renderTargets[] = {
		m_pRenderTargetsDeferred[Bind::RenderTarget::Type::POSITION]->GetRenderTarget(),
		m_pRenderTargetsDeferred[Bind::RenderTarget::Type::ALBEDO]->GetRenderTarget(),
		m_pRenderTargetsDeferred[Bind::RenderTarget::Type::NORMAL]->GetRenderTarget(),
		m_pRenderTargetsDeferred[Bind::RenderTarget::Type::TANGENT]->GetRenderTarget(),
		m_pRenderTargetsDeferred[Bind::RenderTarget::Type::BINORMAL]->GetRenderTarget(),
		m_pRenderTargetsDeferred[Bind::RenderTarget::Type::NORMALMAP]->GetRenderTarget()
	};
	m_pContext->OMSetRenderTargets( BUFFER_COUNT, renderTargets, m_pDepthStencil->GetDepthStencilView() );
	m_pContext->ClearRenderTargetView( m_pRenderTargetsDeferred[Bind::RenderTarget::Type::POSITION]->GetRenderTarget(), m_clearColor );
	m_pContext->ClearRenderTargetView( m_pRenderTargetsDeferred[Bind::RenderTarget::Type::ALBEDO]->GetRenderTarget(), m_clearColor );
	m_pContext->ClearRenderTargetView( m_pRenderTargetsDeferred[Bind::RenderTarget::Type::NORMAL]->GetRenderTarget(), m_clearColor );
	m_pContext->ClearRenderTargetView( m_pRenderTargetsDeferred[Bind::RenderTarget::Type::TANGENT]->GetRenderTarget(), m_clearColor );
	m_pContext->ClearRenderTargetView( m_pRenderTargetsDeferred[Bind::RenderTarget::Type::BINORMAL]->GetRenderTarget(), m_clearColor );
	m_pContext->ClearRenderTargetView( m_pRenderTargetsDeferred[Bind::RenderTarget::Type::NORMALMAP]->GetRenderTarget(), m_clearColor );
	m_pDepthStencil->ClearDepthStencil( m_pContext.Get() );
}

void Graphics::UpdateRenderStateSkysphere( bool useDeferred, bool useGBuffer )
{
	// Set render state for skysphere
    m_pRasterizerStates[Bind::Rasterizer::Type::SKYSPHERE]->Bind( m_pContext.Get() );
	if ( useDeferred )
	{
		useGBuffer ?
			Shaders::BindShaders( m_pContext.Get(), m_vertexShaderOBJ, m_pixelShaderOBJ ) :
			Shaders::BindShaders( m_pContext.Get(), m_vertexShaderDR, m_pixelShaderDR );
	}
	else
	{
		Shaders::BindShaders( m_pContext.Get(), m_vertexShaderOBJ, m_pixelShaderOBJ );
	}
}

void Graphics::UpdateRenderStateCube( bool useDeferred, bool useGBuffer )
{
	// Set default render state for cubes
    m_pRasterizerStates[Bind::Rasterizer::Type::SOLID]->Bind( m_pContext.Get() );
	if ( useDeferred )
	{
		useGBuffer ?
			Shaders::BindShaders( m_pContext.Get(), m_vertexShader, m_pixelShader ) :
			Shaders::BindShaders( m_pContext.Get(), m_vertexShaderDR, m_pixelShaderDR );
	}
	else
	{
		Shaders::BindShaders( m_pContext.Get(), m_vertexShader, m_pixelShader );
	}
}

void Graphics::UpdateRenderStateObject( bool useDeferred, bool useGBuffer )
{
	// Set default render state for objects
    m_pRasterizerStates[Bind::Rasterizer::Type::SOLID]->Bind( m_pContext.Get() );
	if ( useDeferred )
	{
		useGBuffer ?
			Shaders::BindShaders( m_pContext.Get(), m_vertexShaderOBJ, m_pixelShaderOBJ ) :
			Shaders::BindShaders( m_pContext.Get(), m_vertexShaderDR, m_pixelShaderDR );
	}
	else
	{
		Shaders::BindShaders( m_pContext.Get(), m_vertexShaderOBJ, m_pixelShaderOBJ );
	}
}

void Graphics::UpdateRenderStateTexture( bool useDeferred, bool useGBuffer )
{
	// Set default render state for objects
    m_pRasterizerStates[Bind::Rasterizer::Type::SOLID]->Bind( m_pContext.Get() );
	if ( useDeferred )
	{
		useGBuffer ?
			Shaders::BindShaders( m_pContext.Get(), m_vertexShaderTEX, m_pixelShaderTEX ) :
			Shaders::BindShaders( m_pContext.Get(), m_vertexShaderDR, m_pixelShaderDR );
	}
	else
	{
		Shaders::BindShaders( m_pContext.Get(), m_vertexShaderTEX, m_pixelShaderTEX );
	}
}

void Graphics::BeginRenderSceneToTexture()
{
	// Bind new render target
	m_pBackBuffer->Bind( m_pContext.Get(), m_pDepthStencil.get(), m_clearColor );
}

void Graphics::RenderSceneToTexture(
	ID3D11Buffer* const* cbMotionBlur,
	ID3D11Buffer* const* cbFXAA,
	ID3D11Buffer* const* cbSSAO,
	ID3D11ShaderResourceView* const* pNoiseTexture )
{
	// Render fullscreen texture to new render target
	Shaders::BindShaders( m_pContext.Get(), m_vertexShaderPP, m_pixelShaderPP );
	m_pContext->PSSetConstantBuffers( 0u, 1u, cbMotionBlur );
	m_pContext->PSSetConstantBuffers( 1u, 1u, cbFXAA );	
	m_pContext->PSSetConstantBuffers( 2u, 1u, cbSSAO );	
	m_quad.SetupBuffers( m_pContext.Get() );

	m_pContext->PSSetShaderResources( 0u, 1u, m_pRenderTarget->GetShaderResourceViewPtr() );
	m_pContext->PSSetShaderResources( 1u, 1u, m_pDepthStencil->GetShaderResourceViewPtr() );
	m_pContext->PSSetShaderResources( 2u, 1u, m_pRenderTargetNormalDepth->GetShaderResourceViewPtr() );
	m_pContext->PSSetShaderResources( 3u, 1u, pNoiseTexture );

	Bind::Rasterizer::DrawSolid( m_pContext.Get(), m_quad.GetIndexBuffer().IndexCount() ); // always draw as solid
	m_pSamplerStates[Bind::Sampler::Type::ANISOTROPIC_WRAP]->Bind( m_pContext.Get() );
}

void Graphics::RenderSceneToTextureNormalDepth( ID3D11Buffer* const* cbMatrices )
{
	// Render fullscreen texture to new render target
	Shaders::BindShaders( m_pContext.Get(), m_vertexShaderNRM, m_pixelShaderNRM );
	m_pContext->PSSetConstantBuffers( 0u, 1u, cbMatrices );
	m_quad.SetupBuffers( m_pContext.Get() );
	Bind::Rasterizer::DrawSolid( m_pContext.Get(), m_quad.GetIndexBuffer().IndexCount() ); // always draw as solid
	m_pSamplerStates[Bind::Sampler::Type::ANISOTROPIC_WRAP]->Bind( m_pContext.Get() );
}

void Graphics::EndFrame()
{
	// Unbind render target
	m_pRenderTarget->BindNull( m_pContext.Get() );
	m_pRenderTargetNormalDepth->BindNull( m_pContext.Get() );
	for ( uint32_t i = 0u; i < BUFFER_COUNT; i++ )
		m_pRenderTargetsDeferred[(Bind::RenderTarget::Type)i]->BindNull( m_pContext.Get() );
	m_pBackBuffer->BindNull( m_pContext.Get() );

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