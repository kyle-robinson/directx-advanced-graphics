#pragma once
#ifndef QUAD_H
#define QUAD_H

#include "IndexBuffer.h"
#include "VertexBuffer.h"

struct VertexPP
{
	XMFLOAT2 Position;
};

class Quad
{
public:
	bool Initialize( ID3D11Device* device );
	void SetupBuffers( ID3D11DeviceContext* context ) noexcept;
	//inline IndexBuffer& GetIndexBuffer() noexcept { return m_indexBuffer; }
	IndexBuffer m_indexBuffer;
private:
	VertexBuffer<VertexPP> m_vertexBuffer;
};

#endif