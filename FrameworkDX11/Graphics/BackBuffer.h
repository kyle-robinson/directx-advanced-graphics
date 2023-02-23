#ifndef BACKBUFFER_H
#define BACKBUFFER_H

#include <d3d11.h>
#include <Windows.h>
#include <wrl/client.h>
#include "ErrorLogger.h"
#include "DepthStencil.h"

extern UINT MAX_QUALITY;
extern UINT SAMPLE_COUNT;

namespace Bind
{
	class DepthStencil;
	class BackBuffer
	{
	public:
		BackBuffer( ID3D11Device* device, IDXGISwapChain* swapChain )
		{
			try
			{
				Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
				HRESULT hr = swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )pBackBuffer.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create swap chain!" );

				CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc( D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM );
				hr = device->CreateRenderTargetView( pBackBuffer.Get(), &rtvDesc, backBufferView.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create render target view!" );
			}
			catch ( COMException& exception )
			{
				ErrorLogger::Log( exception );
				return;
			}
		}
		inline void Bind( ID3D11DeviceContext* context, DepthStencil* depthStencil, float clearColor[4] ) noexcept
		{
			context->OMSetRenderTargets( 1u, backBufferView.GetAddressOf(), depthStencil->GetDSV() );
			context->ClearRenderTargetView( backBufferView.Get(), clearColor );
		}
		inline ID3D11RenderTargetView* Get() noexcept
		{
			return backBufferView.Get();
		}
		inline ID3D11RenderTargetView** GetPtr() noexcept
		{
			return backBufferView.GetAddressOf();
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferView;
	};
}

#endif