#include "stdafx.h"
#include "Transform.h"

Transform::Transform()
{
	XMStoreFloat4x4( &m_mParent, XMMatrixIdentity() );
	XMStoreFloat4x4( &m_mWorld, XMMatrixIdentity() );
	m_fPosition = XMFLOAT3();
	m_fRotation = XMFLOAT3();
	m_fScale = XMFLOAT3( 1.0f, 1.0f, 1.0f );
}

Transform::~Transform() {}

void Transform::SetPosition( XMFLOAT3 position ) noexcept
{
	m_fPosition = position;
	m_vPosition.x = position.x; m_vPosition.y = position.y; m_vPosition.z = position.z;
}

void Transform::SetPosition( float x, float y, float z ) noexcept
{
	m_fPosition.x = x;
	m_fPosition.y = y;
	m_fPosition.z = z;

	m_vPosition.x = x;
	m_vPosition.y = y;
	m_vPosition.z = z;
}

void Transform::SetRotation( XMFLOAT3 rotation ) noexcept
{
	m_fRotation = rotation;
	ConvertToQuatunion();
}

void Transform::SetRotation( float x, float y, float z ) noexcept
{
	m_fRotation.x = x;
	m_fRotation.y = y;
	m_fRotation.z = z;
	ConvertToQuatunion();
}

XMFLOAT4X4 Transform::GetWorldMatrix()
{
	// Calculate world matrix
	XMMATRIX scale = XMMatrixScaling( m_fScale.x, m_fScale.y, m_fScale.z );
	XMMATRIX TranslationRotationMatrix;
	CalculateTransformMatrixRowMajor( TranslationRotationMatrix, m_vPosition, m_qOrientation );
	XMStoreFloat4x4( &m_mWorld, scale * TranslationRotationMatrix );

	// Apply parent world matrix
	if ( !XMMatrixIsIdentity( XMLoadFloat4x4( &m_mParent ) ) )
	{
		XMStoreFloat4x4( &m_mWorld, XMLoadFloat4x4( &m_mWorld ) * XMLoadFloat4x4( &m_mParent ) );
	}
	return m_mWorld;
}

void Transform::ConvertToQuatunion()
{
	XMMATRIX rotation =
		XMMatrixRotationX( XMConvertToRadians( m_fRotation.x ) ) *
		XMMatrixRotationY( XMConvertToRadians( m_fRotation.y ) ) *
		XMMatrixRotationZ( XMConvertToRadians( m_fRotation.z ) );

	XMFLOAT4 orientation;
	XMStoreFloat4( &orientation, XMQuaternionRotationMatrix( rotation ) );
	m_qOrientation.r = orientation.w;
	m_qOrientation.i = orientation.x;
	m_qOrientation.j = orientation.y;
	m_qOrientation.k = orientation.z;
}