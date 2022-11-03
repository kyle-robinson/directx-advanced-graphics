#pragma once
#ifndef DEPTHSTENCIL_H
#define DEPTHSTENCIL_H

#include <d3d11.h>
#include <Windows.h>
#include <wrl/client.h>
#include "ErrorLogger.h"

extern UINT MAX_QUALITY;
extern UINT SAMPLE_COUNT;

namespace Bind
{
	class DepthStencil
	{
	public:
		DepthStencil( ID3D11Device* device, int width, int height )
		{
			try
			{
				CD3D11_TEXTURE2D_DESC depthStencilDesc( DXGI_FORMAT_D24_UNORM_S8_UINT, static_cast<UINT>( width ), static_cast<UINT>( height ) );
				depthStencilDesc.MipLevels = 1u;
				depthStencilDesc.SampleDesc.Count = SAMPLE_COUNT;
				depthStencilDesc.SampleDesc.Quality = MAX_QUALITY;
				depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

				Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
				HRESULT hr = device->CreateTexture2D( &depthStencilDesc, NULL, depthStencilBuffer.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create depth stencil texture!" );

				CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc( D3D11_DSV_DIMENSION_TEXTURE2D );
				hr = device->CreateDepthStencilView( depthStencilBuffer.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create depth stencil view!" );
			}
			catch ( COMException& exception )
			{
				ErrorLogger::Log( exception );
				return;
			}
		}
		inline void ClearDepthStencil( ID3D11DeviceContext* context ) noexcept
		{
			context->ClearDepthStencilView( depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u );
		}
		inline ID3D11DepthStencilView* GetDepthStencilView() noexcept
		{
			return depthStencilView.Get();
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	};
}

#endif