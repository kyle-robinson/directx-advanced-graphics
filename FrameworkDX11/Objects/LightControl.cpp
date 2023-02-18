#include "stdafx.h"
#include "LightControl.h"
#include "Camera.h"
#include <imgui/imgui.h>

LightControl::LightControl( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    try
    {
        // Initialize constant buffer
        HRESULT hr = m_cbLights.Initialize( pDevice, pContext );
        COM_ERROR_IF_FAILED( hr, "Failed to create 'Light Control' constant buffer!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}

LightControl::~LightControl()
{
	for ( auto light : m_pLights )
		delete light;
}

std::vector<Light*> LightControl::GetLightList()
{
    return m_pLights;
}

Light* LightControl::GetLight( std::string lightName )
{
    for ( auto light : m_pLights )
        if ( light->GetName() == lightName )
            return light;
}

Light* LightControl::GetLight( int lightNum )
{
    return m_pLights[lightNum];
}

void LightControl::AddLight( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera )
{
    Light* light = new Light( name, pDevice, pContext, cbuffer, camera );
    m_pLights.push_back( std::move( light ) );
}

void LightControl::AddLight( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera,
    BOOL enabled, LightType type, XMFLOAT4 pos, XMFLOAT4 col, FLOAT angle, FLOAT constAtten, FLOAT linAtten, FLOAT quadAtten, FLOAT intensity )
{
    Light* light = new Light( name, pDevice, pContext, cbuffer, camera, enabled, type, pos, col, angle, constAtten, linAtten, quadAtten, intensity );
    m_pLights.push_back( std::move( light ) );
}

void LightControl::SetupLightsForRender( XMFLOAT4 eyePosition )
{
    m_cbLights.data.GlobalAmbient = m_fGlobalAmbient;
    m_cbLights.data.EyePosition = eyePosition;
	for ( unsigned int i = 0; i < m_pLights.size(); i++ )
        m_cbLights.data.Lights[i] = m_pLights[i]->GetCB().data;
    if ( !m_cbLights.ApplyChanges() ) return;
}

void LightControl::Draw( const XMMATRIX& view, const XMMATRIX& projection )
{
    for ( auto light : m_pLights )
        light->Draw( view, projection );
}

void LightControl::Update( Camera& camera )
{
    for ( auto light : m_pLights )
        light->Update( camera );
}

void LightControl::RemoveAllLights()
{
    for ( int i = 0; i < m_pLights.size(); i++ )
    {
        delete m_pLights[i];
        m_pLights[i] = nullptr;
    }
    m_pLights.clear();
}

void LightControl::CleanUp()
{
    RemoveAllLights();
}

void LightControl::SpawnControlWindows()
{
    if ( ImGui::Begin( "Light Data", FALSE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
    {
        ImGui::Text( "Global Ambient" );
        static XMFLOAT4 globalAmbient = XMFLOAT4( m_fGlobalAmbient );
		ImGui::SliderFloat4( "##Global Ambient", &globalAmbient.x, 0.0f, 1.0f, "%.1f" );
		m_fGlobalAmbient = globalAmbient;
        ImGui::NewLine();
        for ( auto light : m_pLights )
            light->SpawnControlWindow();
    }
    ImGui::End();
}