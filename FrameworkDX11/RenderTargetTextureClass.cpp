#include "RenderTargetTextureClass.h"

RenderTargetTextureClass::RenderTargetTextureClass()
{
}

RenderTargetTextureClass::RenderTargetTextureClass(ID3D11Device* _pd3dDevice, UINT width, UINT height)
{
    Inizalize(_pd3dDevice, width, height);
}

RenderTargetTextureClass::RenderTargetTextureClass(string Name, ID3D11Device* _pd3dDevice, UINT width, UINT height): Name(Name)
{
    Inizalize(_pd3dDevice, width, height);
}

RenderTargetTextureClass::~RenderTargetTextureClass()
{
    CleanUp();
}

HRESULT RenderTargetTextureClass::Inizalize(ID3D11Device* _pd3dDevice,UINT width, UINT height)
{
    //RTT
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
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

    hr = _pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_pRrenderTargetTexture);
    if (FAILED(hr))
        return hr;

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    // Setup the description of the render target view.
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;


    hr = _pd3dDevice->CreateRenderTargetView(_pRrenderTargetTexture, &renderTargetViewDesc, &_pRenderTargetView);

    if (FAILED(hr))
        return hr;


    // Create depth stencil texture
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
    hr = _pd3dDevice->CreateTexture2D(&descDepthRTT, nullptr, &_pDepthStencil);
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSVRTT = {};
    descDSVRTT.Format = descDepthRTT.Format;
    descDSVRTT.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSVRTT.Texture2D.MipSlice = 0;

    hr = _pd3dDevice->CreateDepthStencilView(_pDepthStencil, &descDSVRTT, &_pDepthStencilView);
    if (FAILED(hr))
        return hr;


    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    // Setup the description of the shader resource view.
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    // Create the shader resource view.
    hr = _pd3dDevice->CreateShaderResourceView(_pRrenderTargetTexture, &shaderResourceViewDesc, &_pShaderResourceView);
    if (FAILED(hr))
        return hr;

    return hr;
}

HRESULT RenderTargetTextureClass::SetRenderTarget(ID3D11DeviceContext* _pImmediateContext)
{
    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _pDepthStencilView);
    // Clear the back buffer
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, Colors::Black);
    // Clear the depth buffer to 1.0 (max depth)
    _pImmediateContext->ClearDepthStencilView(_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    return S_OK;
}

void RenderTargetTextureClass::CleanUp()
{
    if (_pRrenderTargetTexture)_pRrenderTargetTexture->Release();
    if (_pRenderTargetView)_pRenderTargetView->Release();
    if (_pDepthStencil)_pDepthStencil->Release();
    if (_pDepthStencilView)_pDepthStencilView->Release();
    if (_pShaderResourceView)_pShaderResourceView->Release();
}
