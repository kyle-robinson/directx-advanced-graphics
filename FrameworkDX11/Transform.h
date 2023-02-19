#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>
#include"Quaternion.h"
using namespace DirectX;
using namespace std;
/// <summary>
/// class to hold all transform data of a object
/// created from https://github.com/ThomasMillard123/FGAGC
/// </summary>
class Transform
{
public:
	Transform();
	~Transform();


	void SetParent(XMFLOAT4X4 parent) { _Parent = parent; }

	//position get/sets
	void SetPosition(XMFLOAT3 position);
	void SetPosition(float x, float y, float z);
	XMFLOAT3 GetPosition() const { return _Position; }
	Vector3 GetVec3Pos()const { return _Vec3Pos; }

	//scale get/sets
	void SetScale(XMFLOAT3 scale) { _Scale = scale; }
	void SetScale(float x, float y, float z) { _Scale.x = x; _Scale.y = y; _Scale.z = z; }

	XMFLOAT3 GetScale() const { return _Scale; }
	//rotation get/sets
	void SetRotation(XMFLOAT3 rotation);
	void SetRotation(float x, float y, float z);
	XMFLOAT3 GetRotation() const { return _Rotation; }
	//Orientation get/sets
	Quaternion GetOrination() const { return _Orientation;}
	void Setorination(Quaternion orientation) { _Orientation = orientation; }

	//get world
	XMFLOAT4X4 GetWorldMatrix();

private:
	void ConvertToQuatunion();

private:
	//object data
	XMFLOAT3 _Position;
	Vector3 _Vec3Pos;
	XMFLOAT3 _Scale;
	XMFLOAT3 _Rotation;
	
	Quaternion _Orientation;

	//matrix data
	XMFLOAT4X4 _Parent;
	XMFLOAT4X4 _world;

};

