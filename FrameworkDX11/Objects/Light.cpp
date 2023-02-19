#include "stdafx.h"
#include "Light.h"
#include <imgui/imgui.h>

Light::Light() {}

Light::Light( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera )
{
    m_sName = name;
    SetModel( pDevice, pContext, cbuffer );
    SetCBData( pDevice, pContext, camera );
    m_lightCamera.Initialize( XMFLOAT3( m_fPosition.x, m_fPosition.y, m_fPosition.z ), 1280.0f, 720.0f );
}

Light::Light( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera,
    BOOL enabled, LightType type, XMFLOAT4 pos, XMFLOAT4 col, FLOAT angle, FLOAT constAtten, FLOAT linAtten, FLOAT quadAtten, FLOAT intensity )
{
	m_sName = name;
	m_bEnabled = enabled;
	m_eType = type;
	m_fPosition = pos;
	m_fColor = col;
    m_fSpotAngle = angle;
	m_fConstantAttenuation = constAtten;
	m_fLinearAttenuation = linAtten;
	m_fQuadraticAttenuation = quadAtten;
	m_fIntensity = intensity;
    SetModel( pDevice, pContext, cbuffer );
    SetCBData( pDevice, pContext, camera );
    m_lightCamera.Initialize( XMFLOAT3( m_fPosition.x, m_fPosition.y, m_fPosition.z ), 1280.0f, 720.0f );
}

Light::~Light() {}

bool Light::SetModel( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer )
{
#if defined ( _x64 )
    // Initialize light model
    if ( !m_objLight.Initialize( "Resources\\Models\\light.obj", pDevice, pContext, cbuffer ) )
		return false;
    m_objLight.SetInitialScale( 0.1f, 0.1f, 0.1f );
#endif
	return true;
}

bool Light::SetCBData( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, Camera& camera )
{
    try
    {
        // Initialize constant buffer
        HRESULT hr = m_cbLight.Initialize( pDevice, pContext );
        COM_ERROR_IF_FAILED( hr, "Failed to create 'Light' constant buffer!" );
        Update( camera ); // Update constant buffer with default values
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }
    return true;
}

void Light::Draw( const XMMATRIX& view, const XMMATRIX& projection )
{
#if defined ( _x64 )
    if ( !m_bAttachedToCamera )
        m_objLight.Draw( view, projection );
#endif
}

void Light::Update( Camera& camera )
{
    m_lightCamera.SetPosition( XMFLOAT3( m_fPosition.x, m_fPosition.y, m_fPosition.z ) );
    m_lightCamera.UpdateMatrix();

    XMFLOAT4 cameraPosition =
    {
        camera.GetPositionFloat3().x,
        camera.GetPositionFloat3().y,
        camera.GetPositionFloat3().z,
        0.0f
    };

	// Setup light data
    LightData light;
    light.Enabled = m_bEnabled;
    light.LightType = m_eType;
    light.Color = m_fColor;
    light.SpotAngle = XMConvertToRadians( m_fSpotAngle );
    light.ConstantAttenuation = m_fConstantAttenuation;
    light.LinearAttenuation = m_fLinearAttenuation;
    light.QuadraticAttenuation = m_fQuadraticAttenuation;
    light.Intensity = m_fIntensity;

    // Setup light
    if ( m_bAttachedToCamera )
        m_fPosition = cameraPosition;
    light.Position = m_fPosition;

#if defined ( _x64 )
    // Update model position
    m_objLight.SetPosition( XMFLOAT3( m_fPosition.x, m_fPosition.y, m_fPosition.z ) );
#endif

    XMVECTOR lightDirection = XMVectorSet(
        -m_fPosition.x,
        -m_fPosition.y,
        -m_fPosition.z,
        0.0f
    );
    lightDirection = XMVector3Normalize( lightDirection );
    XMStoreFloat4( &light.Direction, lightDirection );

    // Update camera data
	light.View = XMMatrixTranspose( m_lightCamera.GetViewMatrix() );
	light.Projection = XMMatrixTranspose( m_lightCamera.GetProjectionMatrix() );

    // Add to constant buffer
    m_cbLight.data = light;
    if ( !m_cbLight.ApplyChanges() ) return;
}

void Light::SpawnControlWindow()
{
	static std::string type = "Point";
    switch ( m_eType )
    {
	case LightType::SpotLight: type = "Spot"; break;
	case LightType::PointLight: type = "Point"; break;
	case LightType::DirectionalLight: type = "Directional"; break;
    }

    if ( ImGui::CollapsingHeader( std::string( m_sName ).append( " (" ).append( type ).append( ")" ).c_str() ) )
    {
        static bool enabled = (bool)m_bEnabled;
        ImGui::Checkbox( std::string( "##Enabled" ).append( m_sName ).c_str(), &enabled );
        ImGui::SameLine();
        ImGui::Text( "Enabled?" );
        m_bEnabled = (BOOL)enabled;

        ImGui::Checkbox( std::string( "##Attached" ).append( m_sName ).c_str(), &m_bAttachedToCamera );
        ImGui::SameLine();
        ImGui::Text( "Attached To Camera?" );
        ImGui::NewLine();

        if ( !m_bAttachedToCamera )
        {
            ImGui::Text( "Position" );
		    ImGui::DragFloat4( std::string( "##Position" ).append( m_sName ).c_str(), &m_fPosition.x, 0.1f, -10.0f, 10.0f, "%.1f" );
		    ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();
        }

        ImGui::Text( "Color" );
	    ImGui::DragFloat4( std::string( "##Color" ).append( m_sName ).c_str(), &m_fColor.x, 0.1f, 0.0f, 1.0f, "%.1f" );

        ImGui::Text( "Constant Attenuation" );
	    ImGui::DragFloat( std::string( "##Constant" ).append( m_sName ).c_str(), &m_fConstantAttenuation, 0.1f, 0.0f, 1.0f, "%.1f" );

        ImGui::Text( "Linear Attenuation" );
	    ImGui::DragFloat( std::string( "##Linear" ).append( m_sName ).c_str(), &m_fLinearAttenuation, 0.1f, 0.0f, 1.0f, "%.1f" );

        ImGui::Text( "Quadratic Attenuation" );
	    ImGui::DragFloat( std::string( "##Quadratic" ).append( m_sName ).c_str(), &m_fQuadraticAttenuation, 0.1f, 0.0f, 1.0f, "%.1f" );

        ImGui::Text( "Intensity" );
	    ImGui::DragFloat( std::string( "##Intensity" ).append( m_sName ).c_str(), &m_fIntensity, 0.1f, 1.0f, 10.0f, "%1.f" );

        ImGui::Text( "Spot Angle" );
        ImGui::DragFloat( std::string( "##Spot Angle" ).append( m_sName ).c_str(), &m_fSpotAngle, 1.0f, -90.0f, 90.0f, "%1.f" );
    }
}