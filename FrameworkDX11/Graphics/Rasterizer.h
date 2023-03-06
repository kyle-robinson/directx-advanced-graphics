#pragma once
#ifndef RASTERIZER_H
#define RASTERIZER_H

#include <d3d11.h>
#include <Windows.h>
#include <wrl/client.h>
#include "ErrorLogger.h"

namespace Bind
{
	class Rasterizer
	{
	public:
		enum class Type
		{
			NONE,
			BACK,
			WIREFRAME
		};
		Rasterizer( ID3D11Device* device, Type type )
		{
			try
			{
				CD3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT{} );
				rasterizerDesc.MultisampleEnable = TRUE;
				rasterizerDesc.AntialiasedLineEnable = TRUE;

				if ( type == Type::NONE )
				{
					rasterizerDesc.CullMode = D3D11_CULL_NONE;
				}

				if ( type == Type::WIREFRAME )
				{
					rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
					rasterizerDesc.CullMode = D3D11_CULL_NONE;
				}

				HRESULT hr = device->CreateRasterizerState( &rasterizerDesc, pRasterizer.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create rasterizer state!" );
			}
			catch ( COMException& exception )
			{
				ErrorLogger::Log( exception );
				return;
			}
		}
		inline void Bind( ID3D11DeviceContext* context ) noexcept
		{
			context->RSSetState( pRasterizer.Get() );
		}
		inline ID3D11RasterizerState* Get() const noexcept
		{
			return pRasterizer.Get();
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
	};
}

#endif