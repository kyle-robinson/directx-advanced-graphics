#pragma once
#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <vector>
#include "Camera.h"

class CameraController
{
public:
	CameraController();
	~CameraController();

	void AddCam( Camera* cam );
	std::vector<Camera*> GetCamList();

	void RemoveAllCams();
	void RemoveCam( int num );

	Camera* GetCurrentCam();
	Camera* GetCam( int num );
	Camera* GetCam( std::string name );

	bool SetCam( int num );
	bool SetCam( std::string name );

private:
	void CleanUp();

	std::vector<Camera*> m_vCamList;
	int m_iCurrentCamNumber;
};

#endif