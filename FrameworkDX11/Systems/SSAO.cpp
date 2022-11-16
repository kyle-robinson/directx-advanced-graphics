#include "stdafx.h"
#include "SSAO.h"
#include "Camera.h"
#include <functional>
#include <imgui/imgui.h>

bool SSAO::Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	try
	{
		HRESULT hr = m_cbSSAO.Initialize( pDevice, pContext );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'SSAO' constant buffer!" );

		// Generate the sample kernel
		srand( static_cast<unsigned>( time( 0 ) ) );
		for ( uint32_t i = 0u; i < m_kernelSize; i++ )
		{
			XMFLOAT3 sample;
			sample.x = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f;
			sample.y = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f;
			sample.z = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX );

			float scale = (float)i / (float)m_kernelSize;
			std::function<float( float, float, float )> lerp =
				[]( float a, float b, float f ) -> float { return a + f * ( b - a ); };
			scale = lerp( 0.1f, 1.0f, (float)m_kernelSize * (float)m_kernelSize );
			sample.x *= scale;
			sample.y *= scale;
			sample.z *= scale;

			m_fKernelOffsets[i] = sample;
		}

		// Generate the noise texture
		float noiseTextureFloats[192];
		for ( int i = 0; i < 64; i++ )
		{
			int index = i * 3;
			noiseTextureFloats[index] = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f;
			noiseTextureFloats[index + 1] = static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) * 2.0f - 1.0f;
			noiseTextureFloats[index + 2] = 0.0f;
		}

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = noiseTextureFloats;
		data.SysMemPitch = 8u * 12u;
		data.SysMemSlicePitch = 0u;

		D3D11_TEXTURE2D_DESC descTex;
		descTex.Width = 8u;
		descTex.Height = 8u;
		descTex.MipLevels = descTex.ArraySize = 1u;
		descTex.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		descTex.SampleDesc.Count = 1u;
		descTex.SampleDesc.Quality = 0u;
		descTex.Usage = D3D11_USAGE_DEFAULT;
		descTex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		descTex.CPUAccessFlags = 0u;
		descTex.MiscFlags = 0u;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pNoiseTexture;
		pDevice->CreateTexture2D( &descTex, &data, pNoiseTexture.GetAddressOf() );

		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		descSRV.Format = descTex.Format;
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		descSRV.Texture2D.MostDetailedMip = 0u;
		descSRV.Texture2D.MipLevels = 1u;
		pDevice->CreateShaderResourceView( pNoiseTexture.Get(), &descSRV, m_pTextureNoise.GetAddressOf() );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}
	return true;
}

void SSAO::UpdateCB( int width, int height, const Camera& pCamera )
{
	// Setup mapping data
	SSAOData ssaoData;
	XMStoreFloat4x4( &m_ProjectionMatrix, pCamera.GetProjectionMatrix() );
	ssaoData.ProjectionMatrix = XMLoadFloat4x4( &m_ProjectionMatrix );

	m_fScreenSize = XMFLOAT2( width, height );
	ssaoData.ScreenSize = m_fScreenSize;

	m_fNoiseScale = XMFLOAT2( width / m_fNoiseSize, height / m_fNoiseSize );
	ssaoData.NoiseScale = m_fNoiseScale;

	ssaoData.Radius = m_fRadius;
	ssaoData.Power = m_fPower;
	ssaoData.KernelSize = m_kernelSize;
	ssaoData.UseSSAO = m_bUseSSAO;

	for ( int i = 0; i < m_kernelSize; i++ )
		ssaoData.KernelOffsets[i] = m_fKernelOffsets[i];

	// Add to constant buffer
	m_cbSSAO.data.SSAO = ssaoData;
    if ( !m_cbSSAO.ApplyChanges() ) return;
}

void SSAO::SpawnControlWindow( bool usingMotionBlur, bool usingFXAA )
{
	ImGui::SameLine();

	static bool useSSAO = m_bUseSSAO;
	if ( usingMotionBlur || usingFXAA )
	{
		useSSAO = false;
		m_bUseSSAO = useSSAO;
		return;
	}

	ImGui::Checkbox( "Use SSAO?", &useSSAO );
	m_bUseSSAO = useSSAO;

	if ( m_bUseSSAO )
	{
		ImGui::Text( "Occlusion Power" );
		ImGui::SliderFloat( "##Occlusion Power", &m_fPower, 1.0f, 4.0f, "%1.f" );

		ImGui::Text( "Noise Size" );
		ImGui::SliderFloat( "##Noise Size", &m_fNoiseSize, 1.0f, 10.0f, "%1.f" );
	}
}