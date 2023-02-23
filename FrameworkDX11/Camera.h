#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "Quaternion.h"

class Camera
{
public:
	Camera( XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth );
	~Camera();

	void Update();
	void UpdatePointAt();

	inline XMFLOAT4X4 GetView() const noexcept { return m_mView; }
	inline XMFLOAT4X4 GetProjection() const noexcept { return m_mProjection; }
	XMFLOAT4X4 GetViewProjection() const noexcept;

	XMFLOAT4 GetPositionFloat4() noexcept;
	inline XMFLOAT3 GetPosition() const noexcept { return m_fEye; }
	inline XMFLOAT3 GetLookAt() const noexcept { return m_fAt; }
	inline XMFLOAT3 GetUp() const noexcept { return m_fUp; }

	inline XMFLOAT3 GetVecForward() const noexcept { return m_fVecForward; }
	inline XMFLOAT3 GetVecBack() const noexcept { return m_fVecBack; }
	inline XMFLOAT3 GetVecLeft() const noexcept { return m_fVecLeft; }
	inline XMFLOAT3 GetVecRight() const noexcept { return m_fVecRight; }
	inline XMFLOAT3 GetVecUp() const noexcept { return m_fVecUp; }
	inline XMFLOAT3 GetVecDown() const noexcept { return m_fVecDown; }

	void SetPosition( XMFLOAT3 position );
	void AdjustPos( XMFLOAT3 position );

	inline void SetLookAt( XMFLOAT3 lookAt ) noexcept { m_fAt = lookAt; }
	inline void SetUp( XMFLOAT3 up ) noexcept { m_fUp = up; }

	XMFLOAT3 GetRot() { return m_fRot; }
	void SetRot( XMFLOAT3 rot );
	void AdjustRot( XMFLOAT3 rot );
	void Reshape( FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth );

	inline float GetCamSpeed() const noexcept { return m_fCameraSpeed; }
	inline void SetCamSpeed( float speed ) noexcept { m_fCameraSpeed = speed; }

	inline std::string GetCamName() const noexcept { return m_sCamName; }
	inline void SetCamName( std::string name ) noexcept { m_sCamName = name; }

private:
	// Movement data
	const XMFLOAT3 m_fDefaultUp = { 0.0f,1.0f,0.0f };
	const XMFLOAT3 m_fDefaultDown = { 0.0f,-1.0f,0.0f };
	const XMFLOAT3 m_fDefaultForward = { 0.0f,0.0f,1.0f };
	const XMFLOAT3 m_fDefaultBack = { 0.0f,0.0f,-1.0f };
	const XMFLOAT3 m_fDefaultLeft = { -1.0f,0.0f,0.0f };
	const XMFLOAT3 m_fDefaultRight = { 1.0f,0.0f,0.0f };

	XMFLOAT3 m_fVecUp;
	XMFLOAT3 m_fVecDown;
	XMFLOAT3 m_fVecForward;
	XMFLOAT3 m_fVecBack;
	XMFLOAT3 m_fVecLeft;
	XMFLOAT3 m_fVecRight;

	// Camera data
	XMFLOAT3 m_fRot;
	float m_fCameraSpeed = 0.1f;

	XMFLOAT3 m_fEye;
	XMFLOAT3 m_fAt;
	XMFLOAT3 m_fUp;

	FLOAT m_fWindowWidth;
	FLOAT m_fWindowHeight;
	FLOAT m_fNearDepth;
	FLOAT m_fFarDepth;

	// Matrices
	XMFLOAT4X4 m_mView;
	XMFLOAT4X4 m_mProjection;

	// Mouse Data
	XMFLOAT2 m_fMousePos;
	std::string m_sCamName;
};

#endif