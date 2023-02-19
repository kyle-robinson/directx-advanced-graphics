#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
	: _eye(position), _at(at), _up(up), _windowWidth(windowWidth), _windowHeight(windowHeight), _nearDepth(nearDepth), _farDepth(farDepth)
{
	

	_Rot = { 0.0f,0.0f,0.0f };

	Update();
}

Camera::~Camera()
{
	CleanUp();
}

void Camera::Update()
{
	//get quaturnion rotation
	XMFLOAT4 orientation;
	Quaternion qOrientation;
	XMMATRIX RotationMatrix;

	XMMATRIX rotation = XMMatrixRotationX(_Rot.x) * XMMatrixRotationY(_Rot.y) * XMMatrixRotationZ(_Rot.z);
	XMStoreFloat4(&orientation, XMQuaternionRotationMatrix(rotation));
	qOrientation.r = orientation.w;
	qOrientation.i = orientation.x;
	qOrientation.j = orientation.y;
	qOrientation.k = orientation.z;
	
	CalculateTransformMatrixRowMajor(RotationMatrix, { 0,0,0 }, qOrientation);

    // Initialize the view matrix

	XMVECTOR cmaTarget = XMVector3Transform(XMLoadFloat3(&_DefualtFord), RotationMatrix);
	cmaTarget += XMLoadFloat3(&_eye);
	XMVECTOR upDir = XMVector3TransformCoord(XMLoadFloat3(&_DefualtUP), RotationMatrix);
	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(XMLoadFloat3(&_eye), cmaTarget, upDir));


	XMStoreFloat3(&_VecFord,XMVector3TransformCoord(XMLoadFloat3(&_DefualtFord), RotationMatrix));
	XMStoreFloat3(&_VecBack, XMVector3TransformCoord(XMLoadFloat3(&_DefaultBack), RotationMatrix));
	XMStoreFloat3(&_VecLeft, XMVector3TransformCoord(XMLoadFloat3(&_DefaultLeft), RotationMatrix));
	XMStoreFloat3(&_VecRight, XMVector3TransformCoord(XMLoadFloat3(&_DefaultRight), RotationMatrix));
	
    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(0.25f * XM_PI, _windowWidth / _windowHeight, _nearDepth, _farDepth));



	
}

void Camera::UpdatePointat()
{
	
	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(XMLoadFloat3(&_eye), XMLoadFloat3(&_at), XMLoadFloat3(&_up)));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH( XM_PI/2, 1.0f, _nearDepth, _farDepth));
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
}

void Camera::SetPosition(XMFLOAT3 position)
{
	_eye = position;
}

void Camera::AgustPos(XMFLOAT3 position)
{
	_eye.x += position.x;
	_eye.y += position.y;
	_eye.z += position.z;
}

void Camera::SetRot(XMFLOAT3 rot)
{
	_Rot = rot;
	if (_Rot.x >= XMConvertToRadians(90.0f))
		_Rot.x = XMConvertToRadians(90.0f);

	if (_Rot.x <= XMConvertToRadians(-90.0f))
		_Rot.x = XMConvertToRadians(-90.0f);
}

void Camera::AgustRot(XMFLOAT3 rot)
{
	_Rot.x += rot.x;
	_Rot.y += rot.y;
	_Rot.z += rot.z;


	if (_Rot.x >= XMConvertToRadians(90.0f))
		_Rot.x = XMConvertToRadians(90.0f);

	if (_Rot.x <= XMConvertToRadians(-90.0f))
		_Rot.x = XMConvertToRadians(-90.0f);
}



void Camera::CleanUp()
{
	
}

XMFLOAT4X4 Camera::GetViewProjection() const 
{ 
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

	XMFLOAT4X4 viewProj;

	XMStoreFloat4x4(&viewProj, view * projection);

	return viewProj;
}

XMFLOAT4 Camera::GetPositionFloat4()
{
	XMFLOAT4 Pos;
	Pos.x = _eye.x;
	Pos.y = _eye.y;
	Pos.z = _eye.z;
	Pos.w = 1.0f;

	return Pos;
}
