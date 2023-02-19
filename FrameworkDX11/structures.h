#pragma once
using namespace std;
using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------


struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vOutputColor;
	XMFLOAT4 camPos;
};

struct _Material
{
	_Material()
		: Emissive(0.0f, 0.0f, 0.0f, 1.0f)
		, Ambient(0.1f, 0.1f, 0.1f, 1.0f)
		, Diffuse(1.0f, 1.0f, 1.0f, 1.0f)
		, Specular(1.0f, 1.0f, 1.0f, 1.0f)
		, SpecularPower(128.0f)
		, UseTexture(false)
	{}

	DirectX::XMFLOAT4   Emissive;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   Ambient;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   Diffuse;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   Specular;
	//----------------------------------- (16 byte boundary)
	float               SpecularPower;
	// Add some padding complete the 16 byte boundary.
	int                 UseTexture;
	// Add some padding to complete the 16 byte boundary.
	float               Padding[2];
	//----------------------------------- (16 byte boundary)

	float               HightScale;
	float               MaxLayers;
	float               MinLayers;
	float               Padding1;

}; // Total:                                80 bytes (5 * 16)

struct MaterialPropertiesConstantBuffer
{
	_Material   Material;
};

enum LightType
{
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};

#define MAX_LIGHTS 2

struct Light
{
	Light()
		: Position(0.0f, 0.0f, 0.0f, 1.0f)
		, Direction(0.0f, 0.0f, 1.0f, 0.0f)
		, Color(1.0f, 1.0f, 1.0f, 1.0f)
		, SpotAngle(DirectX::XM_PIDIV2)
		, ConstantAttenuation(1.0f)
		, LinearAttenuation(0.0f)
		, QuadraticAttenuation(0.0f)
		, LightType(DirectionalLight)
		, Enabled(0)
	{}

	DirectX::XMFLOAT4    Position;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4    Direction;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4    Color;
	//----------------------------------- (16 byte boundary)
	float       SpotAngle;
	float       ConstantAttenuation;
	float       LinearAttenuation;
	float       QuadraticAttenuation;
	//----------------------------------- (16 byte boundary)
	int         LightType;
	int         Enabled;
	// Add some padding to make this struct size a multiple of 16 bytes.
	int         Padding[2];
	//----------------------------------- (16 byte boundary)
	XMMATRIX mView;
	XMMATRIX mProjection;
};  // Total:                              80 bytes ( 5 * 16 )


struct LightPropertiesConstantBuffer
{
	LightPropertiesConstantBuffer()
		: EyePosition(0, 0, 0, 1)
		, GlobalAmbient(0.2f, 0.2f, 0.8f, 1.0f)
	{}

	DirectX::XMFLOAT4   EyePosition;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4   GlobalAmbient;
	//----------------------------------- (16 byte boundary)
	Light               Lights[MAX_LIGHTS]; // 80 * 8 bytes
};  // Total:                                  672 bytes (42 * 16)




struct PostProcessingCB
{
	DirectX::XMFLOAT4  Color;
	//----------
	int UseHDR;
	int UseBloom;
	int UseDepthOfF;
	int UseColour;
	//------------
	int UseBlur;
	float fadeAmount;
	float FarPlane;
	float focalwidth;
	//--------------
	float focalDistance;
	float blerAttenuation;
	int pad1;
	int pad2;
};


struct TerrainCB
{
	DirectX::XMFLOAT4   gEyePosition;
	//----------------------------------- (16 byte boundary)
	float MinDist;
	float MaxDist;
	float MinTess;
	float MaxTess;
	//----------------------------------- (16 byte boundary)
	float Layer1MaxHight;
	float Layer2MaxHight;
	float Layer3MaxHight;
	float Layer4MaxHight;
	//----------------------------------- (16 byte boundary)
	float Layer5MaxHight;
	float pad1;
	float pad2;
	float pad3;
	//----------------------------------- (16 byte boundary)
	XMFLOAT4 WorldFrustumPlanes[6];
};  // Total:                                  672 bytes (42 * 16)