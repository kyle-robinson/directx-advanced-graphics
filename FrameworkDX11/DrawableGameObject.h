#pragma once
#ifndef DRAWABLEGAMEOBJECT_H_
#define DRAWABLEGAMEOBJECT_H_

#include "Transform.h"
#include "Appearance.h"

class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();

	void Update( float dt, ID3D11DeviceContext* pContext );
	void Draw( ID3D11DeviceContext* pContext );
	void CleanUp();

	inline Appearance* GetAppearance() const noexcept { return m_pApperance; }
	inline Transform* GetTransfrom() const noexcept { return m_pTransform; }

protected:
	Transform* m_pTransform;
	Appearance* m_pApperance;
};

#endif