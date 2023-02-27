#include "stdafx.h"
#include "RenderTargetController.h"

RenderTargetController::RenderTargetController() {}

RenderTargetController::~RenderTargetController()
{
	CleanUp();
}

RenderTargetTexture* RenderTargetController::GetRenderTarget( int num )
{
	return m_vTargetTextures[num];
}

RenderTargetTexture* RenderTargetController::GetRenderTarget( std::string name )
{

	for ( RenderTargetTexture* renderTarget : m_vTargetTextures )
	{
		if ( renderTarget->GetName() == name )
		{
			return renderTarget;
		}
	}
	return nullptr;
}

void RenderTargetController::CreateRenderTarget( std::string name, UINT width, UINT height, ID3D11Device* pDevice )
{
	m_vTargetTextures.push_back( new RenderTargetTexture( name, pDevice, width, height ) );
}

void RenderTargetController::CleanUp()
{
	for ( RenderTargetTexture* renderTarget : m_vTargetTextures )
	{
		delete renderTarget;
	}

	m_vTargetTextures.clear();
}