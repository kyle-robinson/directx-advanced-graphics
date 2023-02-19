//#include "Mouse_Picking.h"
////#include"EventSystem.h"
//Mouse_Picking::Mouse_Picking()
//{
//	/*EventSystem::Instance()->AddObserver(this, Events::WindowSizeUpdate);
//	EventSystem::Instance()->AddObserver(this, Events::EyePos);
//	EventSystem::Instance()->AddObserver(this, Events::ProjectionMatrix);
//	EventSystem::Instance()->AddObserver(this, Events::ViewMatix);*/
//}
//
//Mouse_Picking::~Mouse_Picking()
//{
//	//EventSystem::Instance()->RemoveObserver(this);
//}
//
//bool Mouse_Picking::IsPicked(XMFLOAT2 mousePosititon,BoundingSphere boundingCollsion)
//{
//	MousePosition = mousePosititon;
//	//NDC
//	float NDC_X = (2.0f * mousePosititon.x) / ScreenDimetions.x - 1.0f;
//	float NDC_Y = 1.0f - (2.0f * mousePosititon.y) / ScreenDimetions.y;
//
//
//	XMVECTOR rayOrigin = XMVectorSet(NDC_X, NDC_Y, 0.0f, 0.0f);
//
//	//invert projection and View
//	XMMATRIX P = XMLoadFloat4x4(&CurrentProjection);
//	XMMATRIX InvertP = XMMatrixInverse(nullptr, XMLoadFloat4x4(&CurrentProjection));
//	XMMATRIX V = XMLoadFloat4x4(&CurrentView);
//	XMMATRIX InvertV = XMMatrixInverse(nullptr, V);
//	XMVECTOR eyePos = XMLoadFloat3(&CurrentEye);
//
//	//covert ray to world
//	rayOrigin = XMVector3Transform(rayOrigin, InvertP);
//	rayOrigin = XMVector3Transform(rayOrigin, InvertV);
//	//get the ray direction
//	XMVECTOR rayDir = rayOrigin - eyePos;
//	rayDir = XMVector3Normalize(rayDir);
//	
//
//	XMStoreFloat3(&_rayOri, rayOrigin);
//	XMStoreFloat3(&_rayDir, rayDir);
//
//
//	//find what was was hit using bounding sphere
//	float distance = 10.0f;
//	if (boundingCollsion.Intersects(rayOrigin, rayDir, distance)) {
//		return true;
//	}
//
//    return false;
//}
//
////void Mouse_Picking::OnNotify(const void* entertity, Events event)
////{
////	switch (event)
////	{
////	case Events::WindowSizeUpdate:
////		ScreenDimetions = *(XMFLOAT2*)entertity;
////		break;
////	case Events::EyePos:
////		CurrentEye = *(XMFLOAT3*)entertity;
////		break;
////	case Events::ViewMatix:
////		CurrentView = *(XMFLOAT4X4*)entertity;
////		break;
////	case Events::ProjectionMatrix:
////		CurrentProjection = *(XMFLOAT4X4*)entertity;
////		break;
////	}
////
////
////}
