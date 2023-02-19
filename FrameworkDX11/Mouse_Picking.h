//#pragma once
//#include <d3d11_1.h>
//#include <d3dcompiler.h>
//#include <directxmath.h>
//#include <directxcolors.h>
//#include<DirectXCollision.h>
//
////#include"Vector3.h"
////#include"Observer.h"
//using namespace std;
//using namespace DirectX;
//
///// <summary>
///// class to check the mouse picking 
///// </summary>
//class Mouse_Picking:public Observer
//{
//
//public:
//	Mouse_Picking();
//	~Mouse_Picking();
//
//	bool IsPicked(XMFLOAT2 mousePosititon, BoundingSphere boundingCollsion);
//
//	void OnNotify(const void* entertity, Events event);
//	XMFLOAT3 getRayDir() { return _rayDir; }
//	XMFLOAT3 getRayOri() { return _rayOri; }
//
//	/*Vector3 getRayDirVec3() { return { _rayDir.x,_rayDir.y,_rayDir.z }; }
//	Vector3 getRayOriVec3() { return { _rayOri.x,_rayOri.y,_rayOri.z }; }*/
//
//
//private:
//
//	//camardata
//	XMFLOAT4X4 CurrentProjection;
//	XMFLOAT4X4 CurrentView;
//	XMFLOAT3 CurrentEye;
//
//	//mouse data
//	XMFLOAT2 MousePosition;
//	//screen width/hight
//	XMFLOAT2 ScreenDimetions;
//
//
//	//picking data
//	XMFLOAT3 _rayOri;
//	XMFLOAT3 _rayDir;	
//
//};
//
////bounding data