#include "stdafx.h"
#include "CameraController.h"

CameraController::CameraController()
{
	m_iCurrentCamNumber = 0;
}

CameraController::~CameraController()
{
	CleanUp();
}

void CameraController::AddCam( Camera* cam )
{
	m_vCamList.push_back( cam );
}

std::vector<Camera*> CameraController::GetCamList()
{
	return m_vCamList;
}

void CameraController::RemoveAllCams()
{
	for ( int i = 0; i < m_vCamList.size(); i++ )
	{
		delete m_vCamList[i];
	}

	m_vCamList.clear();
}

void CameraController::RemoveCam( int num )
{
	delete m_vCamList[num];
}

Camera* CameraController::GetCurentCam()
{
	return m_vCamList[m_iCurrentCamNumber];
}

Camera* CameraController::GetCam( int num )
{
	return m_vCamList[num];
}

Camera* CameraController::GetCam( std::string name )
{
	return nullptr;
}

bool CameraController::SetCam( int num )
{
	if ( m_vCamList.size() < num || 0 > num )
	{
		return false;
	}

	m_iCurrentCamNumber = num;
	return true;
}

bool CameraController::SetCam( std::string name )
{
	return false;
}

void CameraController::Update()
{
	for ( Camera* cam : m_vCamList )
	{
		cam->Update();
	}
}

void CameraController::CleanUp()
{
	RemoveAllCams();
	m_iCurrentCamNumber = 0;
}