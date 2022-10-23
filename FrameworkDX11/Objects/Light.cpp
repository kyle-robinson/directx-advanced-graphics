#include "stdafx.h"
#include "Light.h"
#include "Camera.h"

bool Light::Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	try
	{
		HRESULT hr = m_cbLight.Initialize( pDevice, pContext );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'Light' constant buffer!" );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}
	return true;
}

void Light::UpdateCB( Camera& camera )
{
	// Setup light data
    LightData light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = DirectX::XMFLOAT4( DirectX::Colors::White );
    light.SpotAngle = DirectX::XMConvertToRadians( 45.0f );
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = 1.0f;
    light.QuadraticAttenuation = 1.0f;

    // Setup light
    XMFLOAT4 eyePosition = {
        camera.GetPositionFloat3().x,
        camera.GetPositionFloat3().y,
        camera.GetPositionFloat3().z,
        1.0f
    };
    DirectX::XMFLOAT4 LightPosition( eyePosition );
    light.Position = LightPosition;
    DirectX::XMVECTOR LightDirection = DirectX::XMVectorSet(
        camera.GetCameraTarget().x - LightPosition.x,
        camera.GetCameraTarget().y - LightPosition.y,
        camera.GetCameraTarget().z - LightPosition.z,
        0.0f
    );
    LightDirection = DirectX::XMVector3Normalize( LightDirection );
    DirectX::XMStoreFloat4( &light.Direction, LightDirection );

    // Add to constant buffer
    m_cbLight.data.EyePosition = LightPosition;
    m_cbLight.data.Lights[0] = light;
    if ( !m_cbLight.ApplyChanges() ) return;
}