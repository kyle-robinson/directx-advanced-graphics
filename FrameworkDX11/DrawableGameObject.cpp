#include "stdafx.h"
#include "DrawableGameObject.h"
#include "Structures.h"
#include "resource.h"

DrawableGameObject::DrawableGameObject()
{
	m_pTransform = new Transform();
	m_pApperance = new Appearance();
}

DrawableGameObject::~DrawableGameObject()
{
	CleanUp();
}

void DrawableGameObject::CleanUp()
{
	if ( m_pTransform )
	{
		delete m_pTransform;
		m_pTransform = nullptr;
	}

	if ( m_pApperance )
	{
		delete m_pApperance;
		m_pApperance = nullptr;
	}
}

void DrawableGameObject::Update( float dt, ID3D11DeviceContext* pContext )
{
	static float cummulativeTime = 0;
	cummulativeTime += dt;
	m_pApperance->Update( pContext );
}

void DrawableGameObject::Draw( ID3D11DeviceContext* pContext )
{
	m_pApperance->Draw( pContext );
}