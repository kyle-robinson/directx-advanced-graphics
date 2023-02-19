#include "Light.h"

Light_Data::Light_Data()
{
}

Light_Data::Light_Data(string Name, bool Enabled, LightType _LightType, XMFLOAT4 Pos, XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation):Name(Name)
{
	_LightData.Enabled = Enabled;
	_LightData.LightType = _LightType;
	_LightData.Position = Pos;
	_LightData.Color = Colour;
	_LightData.SpotAngle = Angle;
	_LightData.ConstantAttenuation = ConstantAttenuation;
	_LightData.LinearAttenuation = LinearAttenuation;
	_LightData.QuadraticAttenuation = QuadraticAttenuation;


	XMVECTOR LightDirection = XMVectorSet(-Pos.x, -Pos.y, -Pos.z, 0.0f);
	LightDirection = XMVector3Normalize(LightDirection);
	XMStoreFloat4(&_LightData.Direction, LightDirection);

}

Light_Data::Light_Data(string Name, bool Enabled, LightType _LightType, XMFLOAT4 Pos, XMFLOAT4 Colour, float Angle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext):Name(Name)
{
	_LightData.Enabled = Enabled;
	_LightData.LightType = _LightType;
	_LightData.Position = Pos;
	_LightData.Color = Colour;
	_LightData.SpotAngle = Angle;
	_LightData.ConstantAttenuation = ConstantAttenuation;
	_LightData.LinearAttenuation = LinearAttenuation;
	_LightData.QuadraticAttenuation = QuadraticAttenuation;


	XMVECTOR LightDirection = XMVectorSet(-Pos.x, -Pos.y, -Pos.z, 0.0f);
	LightDirection = XMVector3Normalize(LightDirection);
	XMStoreFloat4(&_LightData.Direction, LightDirection);

	LightObject = new DrawableGameObject();
	LightObject->GetAppearance()->initMesh(pd3dDevice, pContext);
	LightObject->GetTransfrom()->SetScale(0.2f, 0.2f, 0.2f);


	MaterialPropertiesConstantBuffer material;
	material.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	material.Material.SpecularPower = 32.0f;
	material.Material.UseTexture = true;
	material.Material.Emissive = XMFLOAT4(100.0f, 100.0f, 100.0f, 1.0f);
	material.Material.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	material.Material.HightScale = 0.1f;
	material.Material.MaxLayers = 15.0f;
	material.Material.MinLayers = 10.0f;

	LightObject->GetAppearance()->SetMaterial(material);

	CamLight = new Camera(XMFLOAT3{ Pos.x, Pos.y, Pos.z }, XMFLOAT3{ 0, 0, 0 }, XMFLOAT3{ 0,1,0 }, 1280, 720, 0.01f, 100.0f);
	Shadow = new ShadowMap(pd3dDevice, 1280, 720);
	
}

Light_Data::Light_Data(Light LightData): _LightData(LightData)
{
	
}

Light_Data::~Light_Data()
{
	CleanUP();
}

void Light_Data::update(float t, ID3D11DeviceContext* pContext)
{
	if (_LightData.Enabled) {
		if (LightObject->GetAppearance()) {
			LightObject->GetTransfrom()->SetPosition(_LightData.Position.x, _LightData.Position.y, _LightData.Position.z);
			
			LightObject->update(t, pContext);
		}
		
	}
	
	CamLight->SetPosition(XMFLOAT3{ _LightData.Position.x,_LightData.Position.y,_LightData.Position.z });
	CamLight->Update();
}

void Light_Data::draw(ID3D11DeviceContext* pContext)
{
	if (_LightData.Enabled) {
		if (LightObject->GetAppearance()) {
			LightObject->draw(pContext);
		}
	}
}

Light Light_Data::GetLightData()
{
	XMFLOAT4X4 fla = CamLight->GetView();
	XMMATRIX a = XMLoadFloat4x4(&fla);
	XMFLOAT4X4 flb = CamLight->GetProjection();
	XMMATRIX b = XMLoadFloat4x4(&flb);


	_LightData.mView = XMMatrixTranspose(a);
	_LightData.mProjection = XMMatrixTranspose(b);

	return _LightData;
}

void Light_Data::SetLightData(Light LightData)
{
	_LightData = LightData;
}

DrawableGameObject* Light_Data::GetLightObject()
{
	return LightObject;
}

string Light_Data::GetName()
{
	return Name;
}

void Light_Data::setName(string name)
{
	Name = name;
}

void Light_Data::setColour(XMFLOAT4 Colour)
{
	_LightData.Color = Colour;

}

void Light_Data::setPos(XMFLOAT4 Pos)
{
	_LightData.Position = Pos;
	CamLight->SetPosition(XMFLOAT3{ Pos.x,Pos.y,Pos.z });
}

void Light_Data::setDirection(XMFLOAT4 dir)
{
	XMVECTOR LightDirection = XMVectorSet(dir.x, dir.y, dir.z, 0.0f);
	LightDirection = XMVector3Normalize(LightDirection);
	XMStoreFloat4(&_LightData.Direction, LightDirection);

	
}

void Light_Data::SetEnabled(bool enabled)
{

	_LightData.Enabled = XMConvertToRadians(enabled);


}

void Light_Data::SetAttenuation(float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation)
{
	_LightData.ConstantAttenuation = ConstantAttenuation;
	_LightData.LinearAttenuation = LinearAttenuation;
	_LightData.QuadraticAttenuation = QuadraticAttenuation;

}

void Light_Data::SetAngle(float Angle)
{
	_LightData.SpotAngle = Angle;
}

void Light_Data::CreateShdowMap(ID3D11DeviceContext* pContext, vector<DrawableGameObject*> Objects,ID3D11Buffer** _pConstantBuffer)
{
	Shadow->SetShadowMap(pContext);

	ConstantBuffer cb1;
	cb1.mView = GetLightData().mView;
	cb1.mProjection = GetLightData().mProjection;
	cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	


	for (DrawableGameObject* Object : Objects) {

		XMFLOAT4X4 WorldAsFloat = Object->GetTransfrom()->GetWorldMatrix();
		XMMATRIX mGO = XMLoadFloat4x4(&WorldAsFloat);
		cb1.mWorld = XMMatrixTranspose(mGO);
		pContext->UpdateSubresource(*_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
		Object->draw(pContext);
	}

}

void Light_Data::CreatView()
{
	XMVECTOR up;


	// Setup the vector that points upwards.
	up = { 0.0f,1.0f,0.0f };
	_at = { 0,0,0 };

	// Create the view matrix from the three vectors.
	
	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(XMLoadFloat4(&_LightData.Position), XMLoadFloat3(&_at), up));

	
	
}

void Light_Data::CreateProjectionMatrix(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;


	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)XM_PI / 2.0f;
	screenAspect = 1.0f;

	// Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(0.25f * XM_PI, 640 / 100,0.01f, 1000));
}

void Light_Data::CleanUP()
{

	if (LightObject)
		delete LightObject;
	LightObject = nullptr;



}
