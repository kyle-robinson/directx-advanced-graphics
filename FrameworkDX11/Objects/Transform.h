#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include"Quaternion.h"

class Transform
{
public:
	Transform();
	~Transform();

	inline void SetParent( XMFLOAT4X4 parent ) noexcept { m_mParent = parent; }

	void SetPosition( XMFLOAT3 position ) noexcept;
	void SetPosition( float x, float y, float z ) noexcept;
	inline XMFLOAT3 GetPosition() const noexcept { return m_fPosition; }
	inline Vector3 GetVec3Pos() const noexcept { return m_vPosition; }

	inline XMFLOAT3 GetScale() const noexcept { return m_fScale; }
	inline void SetScale( XMFLOAT3 scale ) noexcept { m_fScale = scale; }
	inline void SetScale( float x, float y, float z ) { m_fScale.x = x; m_fScale.y = y; m_fScale.z = z; }

	void SetRotation( XMFLOAT3 rotation ) noexcept;
	void SetRotation( float x, float y, float z ) noexcept;
	inline XMFLOAT3 GetRotation() const noexcept { return m_fRotation; }

	XMFLOAT4X4 GetWorldMatrix();
	inline Quaternion GetOrination() const noexcept { return m_qOrientation; }
	inline void Setorination( Quaternion orientation ) noexcept { m_qOrientation = orientation; }

private:
	void ConvertToQuatunion();

	// Object data
	Vector3 m_vPosition;
	XMFLOAT3 m_fPosition;
	XMFLOAT3 m_fScale;
	XMFLOAT3 m_fRotation;

	// Matrix/quaternion data
	XMFLOAT4X4 m_mParent;
	XMFLOAT4X4 m_mWorld;
	Quaternion m_qOrientation;
};

#endif