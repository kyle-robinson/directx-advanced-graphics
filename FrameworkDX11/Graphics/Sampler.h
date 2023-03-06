#pragma once
#ifndef SAMPLER_H
#define SAMPLER_H

#include <string>
#include <d3d11.h>
#include <Windows.h>
#include <wrl/client.h>
#include "ErrorLogger.h"

namespace Bind
{
	class Sampler
	{
	public:
		enum class Type
		{
			WRAP,
			BORDER,
			CLAMP
		};
		Sampler( ID3D11Device* device, Type type )
		{
			try
			{
				CD3D11_SAMPLER_DESC samplerDesc( CD3D11_DEFAULT{} );

				if ( type == Type::WRAP )
				{
					samplerDesc.MinLOD = 0;
					samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
					samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
				}

				if ( type == Type::BORDER )
				{
					samplerDesc.BorderColor[0] = 1.0f;
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
					samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
				}

				if ( type == Type::CLAMP )
				{
					samplerDesc.MinLOD = 0;
					samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
					samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
					samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
				}

				HRESULT hr = device->CreateSamplerState( &samplerDesc, pSampler.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create sampler state!" );
			}
			catch ( COMException& exception )
			{
				ErrorLogger::Log( exception );
				return;
			}
		}
		inline void Bind( ID3D11DeviceContext* context, UINT slot ) noexcept
		{
			context->PSSetSamplers( slot, 1u, pSampler.GetAddressOf() );
		}
		inline ID3D11SamplerState* Get() const noexcept
		{
			return pSampler.Get();
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
		Type type;
	};
}

#endif