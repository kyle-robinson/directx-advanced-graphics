#include "CameraController.h"

CameraController::CameraController()
{
	_CurrentCamNumber = 0;
}

CameraController::~CameraController()
{
	CleanUp();
}

void CameraController::AddCam(Camera* cam)
{

	CamList.push_back(cam);

}

vector<Camera*> CameraController::GetCamList()
{
    return CamList;
}

void CameraController::RemoveAllCam()
{
	for (int i = 0; i < CamList.size(); i++)
	{
		delete CamList[i];
	}

	CamList.clear();
}

void CameraController::RemoveCam(int CamNum)
{
	delete CamList[CamNum];

}

Camera* CameraController::GetCurentCam()
{
	return CamList[_CurrentCamNumber];
}

Camera* CameraController::GetCam(int CamNo)
{
	return CamList[CamNo];
}

Camera* CameraController::GetCam(string CamName)
{
	return nullptr;
}

bool CameraController::SetCam(int CamNo)
{
	if (CamList.size() < CamNo || 0 > CamNo) {
		return false;
	}

	_CurrentCamNumber = CamNo;
	return true;
}

bool CameraController::SetCam(string CamName)
{
	return false;
}

void CameraController::Update()
{
	for (Camera* cam : CamList) {

		cam->Update();
	}



}

void CameraController::CleanUp()
{
	RemoveAllCam();
	_CurrentCamNumber = 0;
}
