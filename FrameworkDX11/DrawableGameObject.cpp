#include "DrawableGameObject.h"

using namespace std;
using namespace DirectX;

DrawableGameObject::DrawableGameObject()
{

	_pTransform = new Transform();
	_pApperance = new Appearance();

}


DrawableGameObject::~DrawableGameObject()
{
	cleanup();
}

void DrawableGameObject::cleanup()
{

	if (_pTransform)
		delete _pTransform;
	_pTransform = nullptr;

	if (_pApperance)
		delete _pApperance;
	_pApperance = nullptr;

}



void DrawableGameObject::update(float t, ID3D11DeviceContext* pContext)
{
	static float cummulativeTime = 0;
	cummulativeTime += t;

	//// Cube:  Rotate around origin

	_pApperance->Update(pContext);

}

void DrawableGameObject::draw(ID3D11DeviceContext* pContext)
{
	
	_pApperance->Draw(pContext);
}
