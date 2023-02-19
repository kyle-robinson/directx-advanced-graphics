#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include<string>
#include<vector>
#include "structures.h"
#include "DDSTextureLoader.h"
#include"ShaderController.h"
using namespace DirectX;
using namespace std;

//vetex data that would be used to creat billboard
struct SimpleVertexBill
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
};

/// <summary>
/// controlls a set of instances of a billboard use a gemory shader to draw the vertex data
/// </summary>
class BillboardObject
{

public:
	BillboardObject(string TexName,int numberToCreate, ID3D11Device* _pd3dDevice);
	~BillboardObject();

	void CreatBillboard(int Number, ID3D11Device* _pd3dDevice);

	void Draw(ID3D11DeviceContext* pContext,ShaderData Shader, ConstantBuffer* cb, ID3D11Buffer* _pConstantBuffer);
	void SetTexture(string TexName, ID3D11Device* _pd3dDevice);

	void UpdatePositions(ID3D11DeviceContext* pContext);
	void SetPositions(vector<SimpleVertexBill> pos) { Positions = pos; }
	vector<SimpleVertexBill> GetPosistions() { return Positions; }

private:
	void CleanUp();

	int NumberOfBillBoards=0;

	ID3D11Buffer* BillboardInstanceBuff;
	ID3D11Buffer* BillboardVertBuff;
	vector<SimpleVertexBill> Positions;
	ID3D11ShaderResourceView* m_pDiffuseResourceView;
};

