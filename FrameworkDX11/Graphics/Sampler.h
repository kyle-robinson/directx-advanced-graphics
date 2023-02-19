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
			ANISOTROPIC_WRAP,
			ANISOTROPIC_CLAMP,
			BILINEAR,
			LINEAR,
			POINT
		};
		enum class UVType
		{
			BORDER,
			CLAMP,
			WRAP
		};
		Sampler( ID3D11Device* device, Type type, UVType uvType, UINT slot = 0u )
			: type( type ), uvType( uvType ), slot( slot )
		{
			try
			{
				CD3D11_SAMPLER_DESC samplerDesc( CD3D11_DEFAULT{} );
				samplerDesc.ComparisonFunc = [type]() mutable
				{
					switch ( type )
					{
					case Type::ANISOTROPIC_CLAMP:
						return D3D11_COMPARISON_LESS_EQUAL;
					case Type::POINT:
					case Type::LINEAR:
					case Type::BILINEAR:
					case Type::ANISOTROPIC_WRAP:
						return D3D11_COMPARISON_NEVER;
					}
				}();
				samplerDesc.Filter = [type]() mutable
				{
					switch ( type )
					{
					case Type::ANISOTROPIC_WRAP:
					case Type::ANISOTROPIC_CLAMP:
						return D3D11_FILTER_ANISOTROPIC;

					case Type::LINEAR:
					case Type::BILINEAR:
						return D3D11_FILTER_MIN_MAG_MIP_LINEAR;

					case Type::POINT:
						return D3D11_FILTER_MIN_MAG_MIP_POINT;
					}
				}();
				samplerDesc.AddressU = [uvType]() mutable
				{
					switch ( uvType )
					{
					case UVType::BORDER:
						return D3D11_TEXTURE_ADDRESS_BORDER;
					case UVType::CLAMP:
						return D3D11_TEXTURE_ADDRESS_CLAMP;
					case UVType::WRAP:
						return D3D11_TEXTURE_ADDRESS_WRAP;
					}
				}();
				samplerDesc.AddressV = [uvType]() mutable
				{
					switch ( uvType )
					{
					case UVType::BORDER:
						return D3D11_TEXTURE_ADDRESS_BORDER;
					case UVType::CLAMP:
						return D3D11_TEXTURE_ADDRESS_CLAMP;
					case UVType::WRAP:
						return D3D11_TEXTURE_ADDRESS_WRAP;
					}
				}();
				samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

				HRESULT hr = device->CreateSamplerState( &samplerDesc, pSampler.GetAddressOf() );
				COM_ERROR_IF_FAILED( hr, "Failed to create sampler state!" );
			}
			catch ( COMException& exception )
			{
				ErrorLogger::Log( exception );
				return;
			}
		}
		inline void Bind( ID3D11DeviceContext* context ) noexcept
		{
			context->PSSetSamplers( slot, 1u, pSampler.GetAddressOf() );
		}
	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
		UVType uvType;
		Type type;
		UINT slot;
	};
}

#endif