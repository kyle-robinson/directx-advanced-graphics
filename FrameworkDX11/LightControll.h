#pragma once


#include"Light.h"
#include<vector>

/// <summary>
/// manages the lights in the appliaction
/// </summary>

class LightControll
{


public:
	LightControll();
	~LightControll();

	vector<Light_Data*> GetLightList() { return _pLightData; }
	Light_Data* GetLight(string Name);
	Light_Data* GetLight(int No);

	void AddLight(string Name,bool Enabled, LightType _LightType, XMFLOAT4 Pos, XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation);
	void AddLight(string Name, bool Enabled, LightType _LightType, XMFLOAT4 Pos, XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void AddLight(Light light);


	void update(float t, ID3D11DeviceContext* pContext);
	void draw(ID3D11DeviceContext* pContext, ID3D11Buffer* _pConstantBuffer, ConstantBuffer* CB);

	void RemoveAllLights();


private:

	vector<Light_Data*> _pLightData;

	void CleanUp();
};

