#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
	Camera( XMFLOAT3 position, FLOAT width, FLOAT height, FLOAT nearPlane, FLOAT farPlane );

	const XMFLOAT4X4& GetView() const;
	const XMMATRIX& GetViewMatrix() const;
	const XMFLOAT4X4& GetProjection() const;
	const XMMATRIX& GetProjectionMatrix() const;

	const XMVECTOR& GetPositionVec() const;
	const XMFLOAT4& GetPositionF4() const;
	const XMFLOAT3& GetPosition() const;
	const XMVECTOR& GetRotationVec() const;
	const XMFLOAT4& GetRotationF4() const;
	const XMFLOAT3& GetRotation() const;

	void SetPosition( const XMVECTOR& pos );
	void SetPosition( const XMFLOAT3& pos );
	void SetPosition( float x, float y, float z );
	void AdjustPosition( const XMVECTOR& pos );
	void AdjustPosition( float x, float y, float z );
	void SetRotation( const XMVECTOR& rot );
	void SetRotation( const XMFLOAT3& rot );
	void SetRotation( float x, float y, float z );
	void AdjustRotation( const XMVECTOR& rot );
	void AdjustRotation( float x, float y, float z );
	void SetLookAtPos( XMFLOAT3 lookAtPos );

	const XMVECTOR& GetForwardVector( bool omitY = false );
	const XMVECTOR& GetRightVector( bool omitY = false );
	const XMVECTOR& GetBackwardVector( bool omitY = false );
	const XMVECTOR& GetLeftVector( bool omitY = false );
	const XMVECTOR& GetUpVector( bool omitY = false );
	const XMVECTOR& GetDownVector( bool omitY = false );

	inline float GetFov() const noexcept { return m_fFov; }
	inline void SetFov( float fov ) noexcept { m_fFov = fov; }
	inline void AdjustFov( float fov ) noexcept { m_fFov += fov; UpdateProjection(); }
	inline FLOAT GetNear() const noexcept { return m_fNear; }
	inline void SetNear( FLOAT nearDepth ) noexcept { m_fNear = nearDepth; UpdateProjection(); }
	inline FLOAT GetFar() const noexcept { return m_fFar; }
	inline void SetFar( FLOAT farDepth ) noexcept { m_fFar = farDepth; UpdateProjection(); }
	inline bool IsPerspective() const noexcept { return m_bPerspective; }
	inline void SetPerspective() noexcept { m_bPerspective = true; UpdateProjection(); }
	inline void SetOrthographic() noexcept { m_bPerspective = false; UpdateProjection(); }

	inline float GetSpeed() const noexcept { return m_fCameraSpeed; }
	inline void SetSpeed( float speed ) noexcept { m_fCameraSpeed = speed; }
	inline std::string GetName() const noexcept { return m_sCamName; }
	inline void SetName( std::string name ) noexcept { m_sCamName = name; }
private:
	void UpdateProjection();
	void UpdatePerspective();
	void UpdateOrthographic();
	void UpdateViewMatrix();
	void UpdateDirectionVectors();

	XMVECTOR posVector;
	XMVECTOR rotVector;
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	const XMVECTOR DEFAULT_FORWARD_VECTOR = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
	const XMVECTOR DEFAULT_BACKWARD_VECTOR = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
	const XMVECTOR DEFAULT_UP_VECTOR = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	const XMVECTOR DEFAULT_DOWN_VECTOR = XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f );
	const XMVECTOR DEFAULT_LEFT_VECTOR = XMVectorSet( -1.0f, 0.0f, 0.0f, 0.0f );
	const XMVECTOR DEFAULT_RIGHT_VECTOR = XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f );

	XMVECTOR vec_forward;
	XMVECTOR vec_left;
	XMVECTOR vec_right;
	XMVECTOR vec_backward;
	XMVECTOR vec_up;
	XMVECTOR vec_down;

	XMVECTOR vec_forward_noY;
	XMVECTOR vec_left_noY;
	XMVECTOR vec_right_noY;
	XMVECTOR vec_backward_noY;
	XMVECTOR vec_up_noY;
	XMVECTOR vec_down_noY;

	FLOAT m_fFar;
	FLOAT m_fNear;
	FLOAT m_fWindowWidth;
	FLOAT m_fWindowHeight;
	float m_fFov = 75.0f;
	std::string m_sCamName;
	bool m_bPerspective = true;
	float m_fCameraSpeed = 0.1f;
};

#endif