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

void LightController::Update( float dt, ID3D11DeviceContext* pContext )
{
    for ( auto lightData : m_vLightData )
    {
        lightData->Update( dt, pContext );
    }
}

void LightController::Draw( ID3D11DeviceContext* pContext, ID3D11Buffer* buffer, ConstantBuffer* cbuffer )
{
    for ( auto lightData : m_vLightData )
    {
        XMFLOAT4X4 worldAsFloat1 = lightData->GetLightObject()->GetTransfrom()->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4( &worldAsFloat1 );
        cbuffer->mWorld = XMMatrixTranspose( mGO );
        pContext->UpdateSubresource( buffer, 0, nullptr, cbuffer, 0, 0 );
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