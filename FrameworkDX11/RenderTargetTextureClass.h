#pragma once
#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include<string>
using namespace DirectX;
using namespace std;
/// <summary>
/// stores the render target data
/// </summary>
class RenderTargetTextureClass
{
public:
	RenderTargetTextureClass();
	RenderTargetTextureClass(ID3D11Device* _pd3dDevice, UINT width, UINT height);
	RenderTargetTextureClass(string Name,ID3D11Device* _pd3dDevice, UINT width, UINT height);
	~RenderTargetTextureClass();

	HRESULT Inizalize(ID3D11Device* _pd3dDevice, UINT width, UINT height);
	HRESULT SetRenderTarget(ID3D11DeviceContext* _pImmediateContext);


	ID3D11ShaderResourceView* GetTexture() { return _pShaderResourceView; }
	string GetName() { return Name; }
private:
	//  front buffer
	ID3D11Texture2D* _pRrenderTargetTexture;
	ID3D11RenderTargetView* _pRenderTargetView;
	// Depth buffer
	ID3D11Texture2D* _pDepthStencil = nullptr;
	ID3D11DepthStencilView* _pDepthStencilView;

	
	ID3D11ShaderResourceView* _pShaderResourceView;
	string Name;

	void CleanUp();
};

