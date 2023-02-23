#pragma once
#ifndef RENDERTARGETCONTROLLER_H
#define RENDERTARGETCONTROLLER_H

#include"RenderTargetTexture.h"

class RenderTargetController
{
public:
	RenderTargetController();
	~RenderTargetController();

	RenderTargetTexture* GetRenderTarget( int num );
	RenderTargetTexture* GetRenderTarget( std::string name );
	void CreateRenderTarget( std::string name, UINT width, UINT height, ID3D11Device* pDevice );

private:
	void CleanUp();
	std::vector<RenderTargetTexture*> m_vTargetTextures;
};

#endif