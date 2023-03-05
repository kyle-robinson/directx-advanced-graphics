#include "stdafx.h"
#include "Camera.h"

Camera::Camera( XMFLOAT3 position, FLOAT width, FLOAT height, FLOAT nearPlane, FLOAT farPlane )
	: m_fWindowWidth( width ), m_fWindowHeight( height ), m_fNear( nearPlane ), m_fFar( farPlane )
{
	pos = position;
	posVector = XMLoadFloat3( &pos );
	rot = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	rotVector = XMLoadFloat3( &rot );
	UpdateViewMatrix();
	UpdateProjection();
}

void Camera::UpdateProjection()
{
	if ( m_bPerspective )
	{
		UpdatePerspective();
	}
	else
	{
		UpdateOrthographic();
	}
}

void Camera::UpdatePerspective()
{
	projectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians( m_fFov ), m_fWindowWidth / m_fWindowHeight, m_fNear, m_fFar );
}

void Camera::UpdateOrthographic()
{
	float zoom = m_fFov * 0.0001f;
	projectionMatrix = XMMatrixOrthographicLH( m_fWindowWidth * zoom, m_fWindowHeight * zoom, m_fNear, m_fFar );
	SetLookAtPos( XMFLOAT3( 0.0f, 0.0f, 0.0f ) );
}

const XMFLOAT4X4& Camera::GetView() const
{
	XMFLOAT4X4 view;
	XMStoreFloat4x4( &view, viewMatrix );
	return view;
}

const XMMATRIX& Camera::GetViewMatrix() const
{
	return viewMatrix;
}

const XMFLOAT4X4& Camera::GetProjection() const
{
	XMFLOAT4X4 projection;
	XMStoreFloat4x4( &projection, projectionMatrix );
	return projection;
}

const XMMATRIX& Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

const XMVECTOR& Camera::GetPositionVec() const
{
	return posVector;
}

const XMFLOAT4& Camera::GetPositionF4() const
{
	return XMFLOAT4( pos.x, pos.y, pos.z, 1.0f );
}

const XMFLOAT3& Camera::GetPosition() const
{
	return pos;
}

const XMVECTOR& Camera::GetRotationVec() const
{
	return rotVector;
}

const XMFLOAT4& Camera::GetRotationF4() const
{
	return XMFLOAT4( rot.x, rot.y, rot.z, 1.0f );
}

const XMFLOAT3& Camera::GetRotation() const
{
	return rot;
}

void Camera::SetPosition( const XMVECTOR& pos )
{
	XMStoreFloat3( &this->pos, pos );
	posVector = pos;
	UpdateViewMatrix();
}

void Camera::SetPosition( const XMFLOAT3& pos )
{
	posVector = XMLoadFloat3( &pos );
	UpdateViewMatrix();
}

void Camera::SetPosition( float x, float y, float z )
{
	pos = XMFLOAT3( x, y, z );
	posVector = XMLoadFloat3( &pos );
	UpdateViewMatrix();
}

void Camera::AdjustPosition( const XMVECTOR& pos )
{
	posVector += pos;
	XMStoreFloat3( &this->pos, posVector );
	UpdateViewMatrix();
}

void Camera::AdjustPosition( float x, float y, float z )
{
	pos.x += x;
	pos.y += y;
	pos.z += z;
	posVector = XMLoadFloat3( &pos );
	UpdateViewMatrix();
}

void Camera::SetRotation( const XMVECTOR& rot )
{
	rotVector = rot;
	XMStoreFloat3( &this->rot, rot );
	UpdateViewMatrix();
}

void Camera::SetRotation( const XMFLOAT3& rot )
{
	rotVector = XMLoadFloat3( &rot );
	UpdateViewMatrix();
}

void Camera::SetRotation( float x, float y, float z )
{
	rot = XMFLOAT3( x, y, z );
	rotVector = XMLoadFloat3( &rot );
	UpdateViewMatrix();
}

void Camera::AdjustRotation( const XMVECTOR& rot )
{
	rotVector += rot;
	XMStoreFloat3( &this->rot, rotVector );
	UpdateViewMatrix();
}

void Camera::AdjustRotation( float x, float y, float z )
{
	rot.x += x;
	rot.y += y;
	rot.z += z;
	rotVector = XMLoadFloat3( &rot );
	UpdateViewMatrix();
}

void Camera::SetLookAtPos( XMFLOAT3 lookAtPos )
{
	//Verify that look at pos is not the same as cam pos. They cannot be the same as that wouldn't make sense and would result in undefined behavior.
	if ( lookAtPos.x == pos.x && lookAtPos.y == pos.y && lookAtPos.z == pos.z )
		return;

	lookAtPos.x = pos.x - lookAtPos.x;
	lookAtPos.y = pos.y - lookAtPos.y;
	lookAtPos.z = pos.z - lookAtPos.z;

	float pitch = 0.0f;
	if ( lookAtPos.y != 0.0f )
	{
		const float distance = sqrt( lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z );
		pitch = atan( lookAtPos.y / distance );
	}

	float yaw = 0.0f;
	if ( lookAtPos.x != 0.0f )
	{
		yaw = atan( lookAtPos.x / lookAtPos.z );
	}
	if ( lookAtPos.z > 0 )
		yaw += XM_PI;

	SetRotation( pitch, yaw, 0.0f );
}

const XMVECTOR& Camera::GetForwardVector( bool omitY )
{
	if ( omitY )
		return vec_forward_noY;
	else
		return vec_forward;
}

const XMVECTOR& Camera::GetRightVector( bool omitY )
{
	if ( omitY )
		return vec_right_noY;
	else
		return vec_right;
}

const XMVECTOR& Camera::GetBackwardVector( bool omitY )
{
	if ( omitY )
		return vec_backward_noY;
	else
		return vec_backward;
}

const XMVECTOR& Camera::GetLeftVector( bool omitY )
{
	if ( omitY )
		return vec_left_noY;
	else
		return vec_left;
}

const XMVECTOR& Camera::GetUpVector( bool omitY )
{
	if ( omitY )
		return vec_up_noY;
	else
		return vec_up;
}

const XMVECTOR& Camera::GetDownVector( bool omitY )
{
	if ( omitY )
		return vec_down_noY;
	else
		return vec_down;
}

void Camera::UpdateViewMatrix() //Updates view matrix and also updates the movement vectors
{
	//Calculate camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw( rot.x, rot.y, rot.z );
	//Calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord( DEFAULT_FORWARD_VECTOR, camRotationMatrix );
	//Adjust cam target to be offset by the camera's current position
	camTarget += posVector;
	//Calculate up direction based on current rotation
	XMVECTOR upDir = XMVector3TransformCoord( DEFAULT_UP_VECTOR, camRotationMatrix );
	//Rebuild view matrix
	viewMatrix = XMMatrixLookAtLH( posVector, camTarget, upDir );

	UpdateDirectionVectors();
}

void Camera::UpdateDirectionVectors()
{
	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw( rot.x, rot.y, 0.0f );
	vec_forward = XMVector3TransformCoord( DEFAULT_FORWARD_VECTOR, vecRotationMatrix );
	vec_backward = XMVector3TransformCoord( DEFAULT_BACKWARD_VECTOR, vecRotationMatrix );
	vec_left = XMVector3TransformCoord( DEFAULT_LEFT_VECTOR, vecRotationMatrix );
	vec_right = XMVector3TransformCoord( DEFAULT_RIGHT_VECTOR, vecRotationMatrix );
	vec_up = XMVector3TransformCoord( DEFAULT_UP_VECTOR, vecRotationMatrix );
	vec_down = XMVector3TransformCoord( DEFAULT_DOWN_VECTOR, vecRotationMatrix );

	XMMATRIX vecRotationMatrixNoY = XMMatrixRotationRollPitchYaw( 0.0f, rot.y, 0.0f );
	vec_forward_noY = XMVector3TransformCoord( DEFAULT_FORWARD_VECTOR, vecRotationMatrixNoY );
	vec_backward_noY = XMVector3TransformCoord( DEFAULT_BACKWARD_VECTOR, vecRotationMatrixNoY );
	vec_left_noY = XMVector3TransformCoord( DEFAULT_LEFT_VECTOR, vecRotationMatrixNoY );
	vec_right_noY = XMVector3TransformCoord( DEFAULT_RIGHT_VECTOR, vecRotationMatrixNoY );
	vec_up_noY = XMVector3TransformCoord( DEFAULT_UP_VECTOR, vecRotationMatrix );
	vec_down_noY = XMVector3TransformCoord( DEFAULT_DOWN_VECTOR, vecRotationMatrix );
}