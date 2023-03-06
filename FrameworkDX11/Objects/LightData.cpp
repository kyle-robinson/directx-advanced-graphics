#include "stdafx.h"
#include "LightData.h"

LightData::LightData() {}

LightData::LightData( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation ) : m_sName( name )
{
	m_lightData.Enabled = enabled;
	m_lightData.LightType = lightType;
	m_lightData.Position = pos;
	m_lightData.Color = colour;
	m_lightData.SpotAngle = angle;
	m_lightData.ConstantAttenuation = constantAttenuation;
	m_lightData.LinearAttenuation = linearAttenuation;
	m_lightData.QuadraticAttenuation = quadraticAttenuation;

	XMVECTOR lightDirection = XMVectorSet( -pos.x, -pos.y, -pos.z, 0.0f );
	lightDirection = XMVector3Normalize( lightDirection );
	XMStoreFloat4( &m_lightData.Direction, lightDirection );
}

LightData::LightData( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation, ID3D11Device* pDevice, ID3D11DeviceContext* pContext ) : m_sName( name )
{
	m_lightData.Enabled = enabled;
	m_lightData.LightType = lightType;
	m_lightData.Position = pos;
	m_lightData.Color = colour;
	m_lightData.SpotAngle = angle;
	m_lightData.ConstantAttenuation = constantAttenuation;
	m_lightData.LinearAttenuation = linearAttenuation;
	m_lightData.QuadraticAttenuation = quadraticAttenuation;

	XMVECTOR lightDirection = XMVectorSet( -pos.x, -pos.y, -pos.z, 0.0f );
	lightDirection = XMVector3Normalize( lightDirection );
	XMStoreFloat4( &m_lightData.Direction, lightDirection );

	m_pLightObject = new DrawableGameObject( name );
	m_pLightObject->GetAppearance()->InitMesh_Cube( pDevice, pContext );
	m_pLightObject->GetTransform()->SetScale( 0.2f, 0.2f, 0.2f );

	MaterialPropertiesCB materialData;
	materialData.Material.Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	materialData.Material.Specular = XMFLOAT4( 1.0f, 0.2f, 0.2f, 1.0f );
	materialData.Material.SpecularPower = 32.0f;
	materialData.Material.UseTexture = true;
	materialData.Material.Emissive = XMFLOAT4( 100.0f, 100.0f, 100.0f, 1.0f );
	materialData.Material.Ambient = XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	materialData.Material.HeightScale = 0.1f;
	materialData.Material.MaxLayers = 15.0f;
	materialData.Material.MinLayers = 10.0f;
	m_pLightObject->GetAppearance()->SetMaterialData( materialData );

	m_pCamLight = new Camera( XMFLOAT3{ pos.x, pos.y, pos.z }, 1280, 720, 0.01f, 100.0f );
	m_pCamLight->SetName( name + " Camera" );
	m_pShadow = new ShadowMap( pDevice, 1280, 720 );
}

LightData::LightData( Light lightData ) : m_lightData( lightData ) {}

LightData::~LightData()
{
	CleanUp();
}

void LightData::Update( float dt, ID3D11DeviceContext* pContext, std::string camName )
{
	bool camActive = camName == ( m_sName + " Camera" );

	if ( m_lightData.Enabled )
	{
		if ( m_pLightObject->GetAppearance() )
		{
			if ( camActive )
				m_pLightObject->GetTransform()->SetPosition( m_pCamLight->GetPosition() );
			else
				m_pLightObject->GetTransform()->SetPosition( m_lightData.Position.x, m_lightData.Position.y, m_lightData.Position.z );
			m_pLightObject->Update( dt, pContext );
		}
	}

	if ( camActive )
		m_lightData.Position = m_pCamLight->GetPositionF4();
	else
		m_pCamLight->SetPosition( XMFLOAT3{ m_lightData.Position.x, m_lightData.Position.y, m_lightData.Position.z } );
}

void LightData::Draw( ID3D11DeviceContext* pContext )
{
	if ( m_lightData.Enabled )
	{
		if ( m_pLightObject->GetAppearance() )
		{
			m_pLightObject->Draw( pContext );
		}
	}
}

Light LightData::GetLightData()
{
	XMFLOAT4X4 fla = m_pCamLight->GetView();
	XMMATRIX a = XMLoadFloat4x4( &fla );
	XMFLOAT4X4 flb = m_pCamLight->GetProjection();
	XMMATRIX b = XMLoadFloat4x4( &flb );
	m_lightData.mView = XMMatrixTranspose( a );
	m_lightData.mProjection = XMMatrixTranspose( b );
	return m_lightData;
}

void LightData::SetLightData( Light lightData )
{
	m_lightData = lightData;
}

DrawableGameObject* LightData::GetLightObject()
{
	return m_pLightObject;
}

std::string LightData::GetName()
{
	return m_sName;
}

void LightData::SetName( std::string name )
{
	m_sName = name;
}

void LightData::SetColour( XMFLOAT4 colour )
{
	m_lightData.Color = colour;
}

void LightData::SetPos( XMFLOAT4 pos )
{
	m_lightData.Position = pos;
}

void LightData::SetDirection( XMFLOAT4 dir )
{
	XMVECTOR lightDirection = XMVectorSet( dir.x, dir.y, dir.z, 0.0f );
	lightDirection = XMVector3Normalize( lightDirection );
	XMStoreFloat4( &m_lightData.Direction, lightDirection );
}

void LightData::SetEnabled( bool enabled )
{
	m_lightData.Enabled = XMConvertToRadians( enabled );
}

void LightData::SetAttenuation( float constantAttenuation, float linearAttenuation, float quadraticAttenuation )
{
	m_lightData.ConstantAttenuation = constantAttenuation;
	m_lightData.LinearAttenuation = linearAttenuation;
	m_lightData.QuadraticAttenuation = quadraticAttenuation;
}

void LightData::SetAngle( float angle )
{
	m_lightData.SpotAngle = angle;
}

void LightData::CreateShadowMap( ID3D11DeviceContext* pContext, std::vector<DrawableGameObject*> objects, ConstantBuffer<MatrixBuffer>& buffer )
{
	m_pShadow->SetShadowMap( pContext );

	buffer.data.mView = GetLightData().mView;
	buffer.data.mProjection = GetLightData().mProjection;
	buffer.data.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );

	for ( DrawableGameObject* object : objects )
	{
		XMFLOAT4X4 worldAsFloat = object->GetTransform()->GetWorldMatrix();
		XMMATRIX mGO = XMLoadFloat4x4( &worldAsFloat );
		buffer.data.mWorld = XMMatrixTranspose( mGO );
		if ( !buffer.ApplyChanges() )
			return;
		object->Draw( pContext );
	}
}

void LightData::CreateView()
{
	XMVECTOR up = { 0.0f, 1.0f, 0.0f };
	m_fAt = { 0.0f, 0.0f, 0.0f };
	XMStoreFloat4x4( &m_mView, XMMatrixLookAtLH( XMLoadFloat4( &m_lightData.Position ), XMLoadFloat3( &m_fAt ), up ) );
}

void LightData::CreateProjectionMatrix( float screenDepth, float screenNear )
{
	float fieldOfView, screenAspect;
	fieldOfView = (float)XM_PI / 2.0f;
	screenAspect = 1.0f;
	XMStoreFloat4x4( &m_mProjection, XMMatrixPerspectiveFovLH( 0.25f * XM_PI, 640 / 100, 0.01f, 1000 ) );
}

void LightData::CleanUp()
{
	if ( m_pLightObject )
	{
		delete m_pLightObject;
		m_pLightObject = nullptr;
	}
}