#include "LightControll.h"

LightControll::LightControll()
{
}

LightControll::~LightControll()
{
}

Light_Data* LightControll::GetLight(string Name)
{
    for (auto LightData : _pLightData) {
        if (LightData->GetName() == Name) {
            return LightData;
      }
    }
  
}

Light_Data* LightControll::GetLight(int No)
{
    return _pLightData[No];
}

void LightControll::AddLight(string Name, bool Enabled, LightType _LightType, XMFLOAT4 Pos, XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation)
{
    Light_Data* LightDat = new Light_Data(Name, Enabled, _LightType, Pos, Colour, Angle, ConstantAttenuation, LinearAttenuation, QuadraticAttenuation);

    _pLightData.push_back(LightDat);

}

void LightControll::AddLight(string Name, bool Enabled, LightType _LightType, XMFLOAT4 Pos, XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
    Light_Data* LightDat = new Light_Data(Name, Enabled, _LightType, Pos, Colour, Angle, ConstantAttenuation, LinearAttenuation, QuadraticAttenuation, pd3dDevice, pContext);
    _pLightData.push_back(LightDat);
}

void LightControll::AddLight(Light light)
{
    Light_Data* LightDat = new Light_Data(light);

    _pLightData.push_back(LightDat);
}

void LightControll::update(float t, ID3D11DeviceContext* pContext)
{
    for (auto LightData : _pLightData) {
        LightData->update(t, pContext);
    }
}

void LightControll::draw(ID3D11DeviceContext* pContext, ID3D11Buffer* _pConstantBuffer,   ConstantBuffer* CB)
{
    for (auto LightData : _pLightData) {

        XMFLOAT4X4 WorldAsFloat1 = LightData->GetLightObject()->GetTransfrom()->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4(&WorldAsFloat1);
        CB->mWorld = XMMatrixTranspose(mGO);
        pContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, CB, 0, 0);

        LightData->draw(pContext);
    }
}

void LightControll::RemoveAllLights()
{
    for (int i = 0; i < _pLightData.size(); i++)
    {
        delete _pLightData[i];
         _pLightData[i]=nullptr;
    }

    _pLightData.clear();

}



void LightControll::CleanUp()
{
    RemoveAllLights();
}
