#include "Transform.h"

Transform::Transform()
{   
	XMStoreFloat4x4(&_Parent,XMMatrixIdentity());
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	_Position = XMFLOAT3();
	_Rotation = XMFLOAT3();
	_Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
}

Transform::~Transform()
{
}

void Transform::SetPosition(XMFLOAT3 position)
{
	_Position = position;
	_Vec3Pos.x = position.x; _Vec3Pos.y = position.y; _Vec3Pos.z = position.z;
}

void Transform::SetPosition(float x, float y, float z)
{
	_Position.x = x; _Position.y = y; _Position.z = z;
	_Vec3Pos.x = x; _Vec3Pos.y = y; _Vec3Pos.z = z;
}

void Transform::SetRotation(XMFLOAT3 rotation)
{
	_Rotation = rotation;
	//convert to quatunion
	ConvertToQuatunion();
}

void Transform::SetRotation(float x, float y, float z)
{
	_Rotation.x = x; _Rotation.y = y; _Rotation.z = z;
	//convert to quatunion
	ConvertToQuatunion();
}

XMFLOAT4X4 Transform::GetWorldMatrix()
{
	// Calculate world matrix
	XMMATRIX scale = XMMatrixScaling(_Scale.x, _Scale.y, _Scale.z);
	XMMATRIX TranslationRotationMatrix;
	CalculateTransformMatrixRowMajor(TranslationRotationMatrix, _Vec3Pos, _Orientation);
	XMStoreFloat4x4(&_world, scale * TranslationRotationMatrix);
	//appliey parent world matrix
	if (!XMMatrixIsIdentity(XMLoadFloat4x4(&_Parent)))
	{
		XMStoreFloat4x4(&_world, XMLoadFloat4x4(&_world) * XMLoadFloat4x4(&_Parent));
	}
	return _world;
}

void Transform::ConvertToQuatunion()
{
	XMMATRIX rotation = XMMatrixRotationX(XMConvertToRadians(_Rotation.x)) * XMMatrixRotationY(XMConvertToRadians(_Rotation.y)) * XMMatrixRotationZ(XMConvertToRadians(_Rotation.z));
	XMFLOAT4 orientation;
	XMStoreFloat4(&orientation, XMQuaternionRotationMatrix(rotation));
	_Orientation.r = orientation.w;
	_Orientation.i = orientation.x;
	_Orientation.j = orientation.y;
	_Orientation.k = orientation.z;
}
