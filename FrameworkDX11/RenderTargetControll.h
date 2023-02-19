#pragma once

#include<vector>
#include<string>

#include"RenderTargetTextureClass.h"
using namespace std;
/// <summary>
/// controlls the render targets
/// </summary>
class RenderTargetControll
{
public:
	RenderTargetControll();
	~RenderTargetControll();

	//get a render target
	RenderTargetTextureClass* GetRenderTarget(int Number);
	RenderTargetTextureClass* GetRenderTarget(string Name);

	//creat a render target
	void CreatRenderTarget(string name, UINT width, UINT Hight, ID3D11Device* _pd3dDevice);

	
private:
	vector<RenderTargetTextureClass*> TargetTexture;

	void CleanUP();
};

