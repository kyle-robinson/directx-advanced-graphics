#include "stdafx.h"
#include "LightController.h"

LightController::LightController() {}

LightController::~LightController() {}

LightData* LightController::GetLight( std::string name )
{
    for ( auto lightData : m_vLightData )
    {
        if ( lightData->GetName() == name )
        {
            return lightData;
        }
    }
}

LightData* LightController::GetLight( int num )
{
    return m_vLightData[num];
}

void LightController::AddLight( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation )
{
    LightData* lightData = new LightData( name, enabled, lightType, pos, colour, angle, constantAttenuation, linearAttenuation, quadraticAttenuation );
    m_vLightData.push_back( lightData );
}

void LightController::AddLight( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    LightData* lightData = new LightData( name, enabled, lightType, pos, colour, angle, constantAttenuation, linearAttenuation, quadraticAttenuation, pDevice, pContext );
    m_vLightData.push_back( lightData );
}

void LightController::AddLight( Light light )
{
    LightData* lightData = new LightData( light );
    m_vLightData.push_back( lightData );
}

void LightController::Update( float dt, ID3D11DeviceContext* pContext, std::string camName )
{
    for ( auto lightData : m_vLightData )
    {
        lightData->Update( dt, pContext, camName );
    }
}

void LightController::Draw( ID3D11DeviceContext* pContext, ConstantBuffer<MatrixBuffer>& buffer )
{
    for ( auto lightData : m_vLightData )
    {
        XMFLOAT4X4 worldAsFloat1 = lightData->GetLightObject()->GetTransform()->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4( &worldAsFloat1 );
        buffer.data.mWorld = XMMatrixTranspose( mGO );
        if ( !buffer.ApplyChanges() )
            return;
        lightData->Draw( pContext );
    }
}

void LightController::RemoveAllLights()
{
    for ( int i = 0; i < m_vLightData.size(); i++ )
    {
        delete m_vLightData[i];
        m_vLightData[i] = nullptr;
    }
    m_vLightData.clear();
}

void LightController::CleanUp()
{
    RemoveAllLights();
}