#include "RenderTargetControll.h"

RenderTargetControll::RenderTargetControll()
{
}

RenderTargetControll::~RenderTargetControll()
{
	CleanUP();
}

RenderTargetTextureClass* RenderTargetControll::GetRenderTarget(int Number)
{
	return TargetTexture[Number];
}

RenderTargetTextureClass* RenderTargetControll::GetRenderTarget(string Name)
{

	for (RenderTargetTextureClass* rendertarget : TargetTexture)
	{
		if (rendertarget->GetName() == Name) {
			return rendertarget;
		}
	}
	return nullptr;
}

void RenderTargetControll::CreatRenderTarget(string name, UINT width, UINT Hight, ID3D11Device* _pd3dDevice)
{

	TargetTexture.push_back(new RenderTargetTextureClass(name,_pd3dDevice, width, Hight));

}

void RenderTargetControll::CleanUP()
{
	for (RenderTargetTextureClass* rendertarget : TargetTexture)
	{
		delete rendertarget;
	}

	TargetTexture.clear();
}
