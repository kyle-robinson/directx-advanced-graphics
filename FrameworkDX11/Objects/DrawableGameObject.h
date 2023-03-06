#pragma once
#ifndef DRAWABLEGAMEOBJECT_H_
#define DRAWABLEGAMEOBJECT_H_

#include "Transform.h"
#include "Appearance.h"

class DrawableGameObject
{
public:
	DrawableGameObject( std::string name );
	~DrawableGameObject();

	void Update( float dt, ID3D11DeviceContext* pContext );
	void Draw( ID3D11DeviceContext* pContext );
	void CleanUp();

	inline std::string GetName() const noexcept { return m_sName; }
	inline void SetName( std::string name ) noexcept { m_sName = name; }

	inline Appearance* GetAppearance() const noexcept { return m_pAppearance; }
	inline Transform* GetTransform() const noexcept { return m_pTransform; }

protected:
	std::string m_sName;
	Transform* m_pTransform;
	Appearance* m_pAppearance;
};

#endif