#include "stdafx.h"
#include "FXAA.h"
#include <imgui/imgui.h>

bool FXAA::Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	try
	{
		HRESULT hr = m_cbFXAA.Initialize( pDevice, pContext );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'FXAA' constant buffer!" );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}
	return true;
}

void FXAA::UpdateCB( int width, int height )
{
	// Setup mapping data
	FXAAData fxaaData;
	fxaaData.TextureSizeInverse = XMFLOAT2( 1.0f / width, 1.0f / height );
	fxaaData.SpanMax = m_fSpanMax;
	fxaaData.ReduceMin = m_fReduceMin;
	fxaaData.ReduceMul = m_fReduceMul;
	fxaaData.UseFXAA = m_bUseFXAA;

	// Add to constant buffer
	m_cbFXAA.data.FXAA = fxaaData;
    if ( !m_cbFXAA.ApplyChanges() ) return;
}

void FXAA::SpawnControlWindow( bool usingMotionBlur, bool usingSSAO )
{
	ImGui::SameLine();

	static bool useFXAA = m_bUseFXAA;
	if ( usingMotionBlur || usingSSAO )
	{
		useFXAA = false;
		m_bUseFXAA = useFXAA;
		return;
	}

	ImGui::Checkbox( "Use FXAA?", &useFXAA );
	m_bUseFXAA = useFXAA;

	if ( m_bUseFXAA )
	{
		ImGui::Text( "Max Span" );
		ImGui::SliderFloat( "##Max Span", &m_fSpanMax, 4.0f, 12.0f, "%1.f" );
	}
}