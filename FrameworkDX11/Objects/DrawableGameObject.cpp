#include "stdafx.h"
#include "DrawableGameObject.h"
#include "Structures.h"
#include "resource.h"

DrawableGameObject::DrawableGameObject( std::string name )
{
	m_sName = name;
	m_pTransform = new Transform();
	m_pAppearance = new Appearance();
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

	if ( m_pAppearance )
	{
		delete m_pAppearance;
		m_pAppearance = nullptr;
	}
}

void DrawableGameObject::Update( float dt, ID3D11DeviceContext* pContext )
{
	static float cummulativeTime = 0;
	cummulativeTime += dt;
	m_pAppearance->Update( pContext );
}

void DrawableGameObject::Draw( ID3D11DeviceContext* pContext )
{
	m_pAppearance->Draw( pContext );
}