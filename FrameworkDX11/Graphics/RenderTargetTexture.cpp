#include "stdafx.h"
#include "RenderTargetTexture.h"

RenderTargetTexture::RenderTargetTexture()
{}

RenderTargetTexture::RenderTargetTexture( ID3D11Device* pDevice, UINT width, UINT height )
{
    Initialize( pDevice, width, height );
}

RenderTargetTexture::RenderTargetTexture( std::string name, ID3D11Device* pDevice, UINT width, UINT height ) : m_sName( name )
{
    Initialize( pDevice, width, height );
}

RenderTargetTexture::~RenderTargetTexture()
{
    CleanUp();
}

bool RenderTargetTexture::Initialize( ID3D11Device* pDevice, UINT width, UINT height )
{
    try
    {
        // Render target texture
        D3D11_TEXTURE2D_DESC textureDesc;
        ZeroMemory( &textureDesc, sizeof( textureDesc ) );
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;
        HRESULT hr = pDevice->CreateTexture2D( &textureDesc, NULL, &m_pRenderTargetTexture );
        COM_ERROR_IF_FAILED( hr, "Failed to create a RENDER TARGET TEXTURE!" );

        // Render target view
        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        renderTargetViewDesc.Format = textureDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;
        hr = pDevice->CreateRenderTargetView( m_pRenderTargetTexture, &renderTargetViewDesc, &m_pRenderTargetView );
        COM_ERROR_IF_FAILED( hr, "Failed to create a RENDER TARGET VIEW!" );

        // Depth stencil texture
        D3D11_TEXTURE2D_DESC descDepthRTT = {};
        descDepthRTT.Width = width;
        descDepthRTT.Height = height;
        descDepthRTT.MipLevels = 1;
        descDepthRTT.ArraySize = 1;
        descDepthRTT.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepthRTT.SampleDesc.Count = 1;
        descDepthRTT.SampleDesc.Quality = 0;
        descDepthRTT.Usage = D3D11_USAGE_DEFAULT;
        descDepthRTT.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepthRTT.CPUAccessFlags = 0;
        descDepthRTT.MiscFlags = 0;
        hr = pDevice->CreateTexture2D( &descDepthRTT, nullptr, &m_pDepthStencil );
        COM_ERROR_IF_FAILED( hr, "Failed to create a DEPTH STENCIL TEXTURE!" );

        // Depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSVRTT = {};
        descDSVRTT.Format = descDepthRTT.Format;
        descDSVRTT.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSVRTT.Texture2D.MipSlice = 0;
        hr = pDevice->CreateDepthStencilView( m_pDepthStencil, &descDSVRTT, &m_pDepthStencilView );
        COM_ERROR_IF_FAILED( hr, "Failed to create a DEPTH STENCIL VIEW!" );

        // Shader resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        shaderResourceViewDesc.Format = textureDesc.Format;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;
        hr = pDevice->CreateShaderResourceView( m_pRenderTargetTexture, &shaderResourceViewDesc, &m_pShaderResourceView );
        COM_ERROR_IF_FAILED( hr, "Failed to create a SHADER RESOURCE VIEW!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    return true;
}

void RenderTargetTexture::SetRenderTarget( ID3D11DeviceContext* pContext )
{
    pContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );
    pContext->ClearRenderTargetView( m_pRenderTargetView, Colors::Black );
    pContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

void RenderTargetTexture::CleanUp()
{
    if ( m_pRenderTargetTexture ) m_pRenderTargetTexture->Release();
    if ( m_pRenderTargetView ) m_pRenderTargetView->Release();
    if ( m_pDepthStencil ) m_pDepthStencil->Release();
    if ( m_pDepthStencilView ) m_pDepthStencilView->Release();
    if ( m_pShaderResourceView ) m_pShaderResourceView->Release();
}