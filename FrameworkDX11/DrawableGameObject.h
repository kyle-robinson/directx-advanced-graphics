#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"
#include "Transform.h"
#include"Appearance.h"
using namespace DirectX;


/// <summary>
/// manage a drawable object
/// created from https://github.com/ThomasMillard123/FGAGC
/// </summary>
class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();

	void cleanup();


	void								update(float t, ID3D11DeviceContext* pContext);
	void								draw(ID3D11DeviceContext* pContext);

	Appearance* GetAppearance() const { return _pApperance; }
	Transform* GetTransfrom() const { return _pTransform; }



protected:

	Transform* _pTransform;
	Appearance* _pApperance;

};

