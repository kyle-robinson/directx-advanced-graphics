#include "stdafx.h"
#include "Camera.h"

Camera::Camera( XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth )
	: m_fEye( position ), m_fAt( at ), m_fUp( up ), m_fWindowWidth( windowWidth ), m_fWindowHeight( windowHeight ), m_fNearDepth( nearDepth ), m_fFarDepth( farDepth )
{
	m_fRot = { 0.0f,0.0f,0.0f };
	Update();
}

Camera::~Camera() {}

void Camera::Update()
{
	// Get quaternion rotation
	XMFLOAT4 fOrientation;
	Quaternion qOrientation;
	XMMATRIX mRotation;

	XMMATRIX rotation = XMMatrixRotationX( m_fRot.x ) * XMMatrixRotationY( m_fRot.y ) * XMMatrixRotationZ( m_fRot.z );
	XMStoreFloat4( &fOrientation, XMQuaternionRotationMatrix( rotation ) );
	qOrientation.r = fOrientation.w;
	qOrientation.i = fOrientation.x;
	qOrientation.j = fOrientation.y;
	qOrientation.k = fOrientation.z;
	CalculateTransformMatrixRowMajor( mRotation, { 0,0,0 }, qOrientation );

	// Initialize the view matrix
	XMVECTOR camTarget = XMVector3Transform( XMLoadFloat3( &m_fDefaultForward ), mRotation );
	camTarget += XMLoadFloat3( &m_fEye );
	XMVECTOR upDir = XMVector3TransformCoord( XMLoadFloat3( &m_fDefaultUp ), mRotation );
	XMStoreFloat4x4( &m_mView, XMMatrixLookAtLH( XMLoadFloat3( &m_fEye ), camTarget, upDir ) );

	XMStoreFloat3( &m_fVecForward, XMVector3TransformCoord( XMLoadFloat3( &m_fDefaultForward ), mRotation ) );
	XMStoreFloat3( &m_fVecBack, XMVector3TransformCoord( XMLoadFloat3( &m_fDefaultBack ), mRotation ) );
	XMStoreFloat3( &m_fVecLeft, XMVector3TransformCoord( XMLoadFloat3( &m_fDefaultLeft ), mRotation ) );
	XMStoreFloat3( &m_fVecRight, XMVector3TransformCoord( XMLoadFloat3( &m_fDefaultRight ), mRotation ) );
	XMStoreFloat3( &m_fVecUp, XMVector3TransformCoord( XMLoadFloat3( &m_fDefaultUp ), mRotation ) );
	XMStoreFloat3( &m_fVecDown, XMVector3TransformCoord( XMLoadFloat3( &m_fDefaultDown ), mRotation ) );

	// Initialize the projection matrix
	XMStoreFloat4x4( &m_mProjection, XMMatrixPerspectiveFovLH( 0.25f * XM_PI, m_fWindowWidth / m_fWindowHeight, m_fNearDepth, m_fFarDepth ) );
}

void Camera::UpdatePointAt()
{
	XMStoreFloat4x4( &m_mView, XMMatrixLookAtLH( XMLoadFloat3( &m_fEye ), XMLoadFloat3( &m_fAt ), XMLoadFloat3( &m_fUp ) ) );
	XMStoreFloat4x4( &m_mProjection, XMMatrixPerspectiveFovLH( XM_PI / 2, 1.0f, m_fNearDepth, m_fFarDepth ) );
}

void Camera::Reshape( FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth )
{
	m_fWindowWidth = windowWidth;
	m_fWindowHeight = windowHeight;
	m_fNearDepth = nearDepth;
	m_fFarDepth = farDepth;
}

void Camera::SetPosition( XMFLOAT3 position )
{
	m_fEye = position;
}

void Camera::AdjustPos( XMFLOAT3 position )
{
	m_fEye.x += position.x;
	m_fEye.y += position.y;
	m_fEye.z += position.z;
}

void Camera::SetRot( XMFLOAT3 rot )
{
	m_fRot = rot;
	if ( m_fRot.x >= XMConvertToRadians( 90.0f ) )
		m_fRot.x = XMConvertToRadians( 90.0f );

	if ( m_fRot.x <= XMConvertToRadians( -90.0f ) )
		m_fRot.x = XMConvertToRadians( -90.0f );
}

void Camera::AdjustRot( XMFLOAT3 rot )
{
	m_fRot.x += rot.x;
	m_fRot.y += rot.y;
	m_fRot.z += rot.z;

	if ( m_fRot.x >= XMConvertToRadians( 90.0f ) )
		m_fRot.x = XMConvertToRadians( 90.0f );

	if ( m_fRot.x <= XMConvertToRadians( -90.0f ) )
		m_fRot.x = XMConvertToRadians( -90.0f );
}

XMFLOAT4X4 Camera::GetViewProjection() const
{
	XMMATRIX view = XMLoadFloat4x4( &m_mView );
	XMMATRIX projection = XMLoadFloat4x4( &m_mProjection );
	XMFLOAT4X4 viewProj;
	XMStoreFloat4x4( &viewProj, view * projection );
	return viewProj;
}

XMFLOAT4 Camera::GetPositionFloat4() noexcept
{
	XMFLOAT4 Pos;
	Pos.x = m_fEye.x;
	Pos.y = m_fEye.y;
	Pos.z = m_fEye.z;
	Pos.w = 1.0f;
	return Pos;
}