#pragma once
#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include <d3d11.h>
#include <Windows.h>
#include <wrl/client.h>
#include "ErrorLogger.h"
#include "DepthStencil.h"

namespace Bind
{
	class DepthStencil;
	class RenderTarget
	{
	public:
		RenderTarget( ID3D11Device* device, IDXGISwapChain* swapChain )
		{
			try
			{
				Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
				HRESULT hr = swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )pBackBuffer.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create swap chain!" );
				hr = device->CreateRenderTargetView( pBackBuffer.Get(), nullptr, backBuffer.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create render target view!" );
			}
			catch ( COMException& exception )
			{
				ErrorLogger::Log( exception );
				return;
			}
		}
		void BindAsBuffer( ID3D11DeviceContext* context, DepthStencil* depthStencil, float clearColor[4] ) noexcept
		{
			context->OMSetRenderTargets( 1, backBuffer.GetAddressOf(), depthStencil->GetDepthStencilView() );
			context->ClearRenderTargetView( backBuffer.Get(), clearColor );
		}
		void BindAsNull( ID3D11DeviceContext* context ) noexcept
		{
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullRenderTarget = nullptr;
			context->OMSetRenderTargets( 1u, nullRenderTarget.GetAddressOf(), nullptr );
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nullShaderResourceView = nullptr;
			context->PSSetShaderResources( 0u, 1u, nullShaderResourceView.GetAddressOf() );
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBuffer;
	};
}

#endif