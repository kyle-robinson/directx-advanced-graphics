#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include"DrawableGameObject.h"
#include"Camera.h"
#include"ShadowMap.h"
using namespace DirectX;
/// <summary>
/// controlls a single lights data
/// </summary>
class Light_Data
{
public:
	Light_Data();
	Light_Data(string Name,bool Enabled, LightType _LightType, XMFLOAT4 Pos,XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation);
	Light_Data(string Name, bool Enabled, LightType _LightType, XMFLOAT4 Pos, XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	Light_Data(Light LightData);
	~Light_Data();


	void update(float t, ID3D11DeviceContext* pContext);
	void draw(ID3D11DeviceContext* pContext);

	Light GetLightData();
	void SetLightData(Light LightData);
	DrawableGameObject* GetLightObject();


	string GetName();
	void setName(string name);

	void CreatView();
	void CreateProjectionMatrix(float screenDepth, float screenNear);

	void setColour(XMFLOAT4 Colour);
	void setPos(XMFLOAT4 Pos);
	void setDirection(XMFLOAT4 dir);
	void SetEnabled(bool enabled);
	void SetAttenuation(float ConstantAttenuation=0, float LinearAttenuation = 0, float QuadraticAttenuation = 0);
	void SetAngle(float Angle);
	Camera* CamLight;

	ShadowMap* GetShadow() { return Shadow; }

	void CreateShdowMap(ID3D11DeviceContext* pContext, vector<DrawableGameObject*> Objects,ID3D11Buffer** _pConstantBuffer);

private:

	Light _LightData;
	DrawableGameObject* LightObject;
	string Name;
	


	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
	XMFLOAT3 _at;

	ShadowMap* Shadow;
	void CleanUP();
};

