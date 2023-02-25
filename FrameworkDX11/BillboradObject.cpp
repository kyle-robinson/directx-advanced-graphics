#include "stdafx.h"
#include "BillboradObject.h"

BillboardObject::BillboardObject( std::string texName, int numToCreate, ID3D11Device* pDevice )
{
    SetTexture( texName, pDevice );
    CreateBillboard( numToCreate, pDevice );
}

BillboardObject::~BillboardObject()
{
    CleanUp();
}

void BillboardObject::CreateBillboard( int num, ID3D11Device* pDevice )
{
    m_iNumberOfBillBoards = num;

    // Creates 2 start points
    m_vPositions.resize( m_iNumberOfBillBoards );
    m_vPositions[0].Pos = XMFLOAT3( 0, 0, 0 );
    m_vPositions[1].Pos = XMFLOAT3( 0, 10, 0 );

    // Create an instance buffer against the number of instances
    D3D11_BUFFER_DESC instBuffDesc;
    ZeroMemory( &instBuffDesc, sizeof( instBuffDesc ) );
    instBuffDesc.Usage = D3D11_USAGE_DEFAULT;
    instBuffDesc.ByteWidth = sizeof( SimpleVertexBillboard ) * m_iNumberOfBillBoards;
    instBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instBuffDesc.CPUAccessFlags = 0;
    instBuffDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA instData;
    ZeroMemory( &instData, sizeof( instData ) );
    instData.pSysMem = &m_vPositions[0];
    instData.SysMemPitch = 0;
    instData.SysMemSlicePitch = 0;
    HRESULT hr = pDevice->CreateBuffer( &instBuffDesc, &instData, &m_pBillboardInstanceBuffer );
    if ( FAILED( hr ) )
        return;

    // Create the vertex buffer
    D3D11_BUFFER_DESC billboardBufferDesc;
    ZeroMemory( &billboardBufferDesc, sizeof( billboardBufferDesc ) );
    billboardBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    billboardBufferDesc.ByteWidth = sizeof( SimpleVertexBillboard );
    billboardBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    billboardBufferDesc.CPUAccessFlags = 0;
    billboardBufferDesc.MiscFlags = 0;

    // Vertex data to be used for all items
    D3D11_SUBRESOURCE_DATA initData;
    SimpleVertexBillboard sharedData;
    sharedData.Pos = XMFLOAT3( 0.0f, 0.0f, 0.0f );
    initData.pSysMem = &sharedData;
    pDevice->CreateBuffer( &billboardBufferDesc, &initData, &m_pBillboardVertexBuffer );
}

void BillboardObject::Draw( ID3D11DeviceContext* pContext, ShaderController::ShaderData shaderData, ConstantBuffer<MatrixBuffer>& buffer )
{
    pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
    pContext->IASetInputLayout( shaderData.m_pVertexLayout );

    UINT strides[2] = { sizeof( SimpleVertexBillboard ), sizeof( SimpleVertexBillboard ) };
    UINT offsets[2] = { 0, 0 };

    buffer.data.mWorld = XMMatrixIdentity();
    if ( !buffer.ApplyChanges() )
        return;

    // Load vertex buffers
    ID3D11Buffer* vertBillInstBuffers[2] = { m_pBillboardVertexBuffer, m_pBillboardInstanceBuffer };
    pContext->IASetVertexBuffers( 0, 2, vertBillInstBuffers, strides, offsets );

	// Set shaders
    pContext->VSSetShader( shaderData.m_pVertexShader, nullptr, 0 );
    pContext->VSSetConstantBuffers( 0, 1, buffer.GetAddressOf() );

    // Geometry shader is used to create the plane of the billboard
    pContext->GSSetShader( shaderData.m_pGeometryShader, nullptr, 0 );
    pContext->GSSetConstantBuffers( 0, 1, buffer.GetAddressOf() );
    pContext->PSSetShaderResources( 0, 1, &m_pDiffuseResourceView );
    pContext->PSSetShader( shaderData.m_pPixelShader, nullptr, 0 );
    pContext->DrawInstanced( 1, m_iNumberOfBillBoards, 0, 0 );

    // Reset buffers
    pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    pContext->GSSetShader( nullptr, nullptr, 0 );
}

void BillboardObject::SetTexture( std::string texName, ID3D11Device* pDevice )
{
    std::wstring tex = std::wstring( texName.begin(), texName.end() );
    const wchar_t* widecstr = tex.c_str();
    HRESULT hr = CreateDDSTextureFromFile( pDevice, widecstr, nullptr, &m_pDiffuseResourceView );
    if ( FAILED( hr ) )
        return;
}

void BillboardObject::UpdatePositions( ID3D11DeviceContext* pContext )
{
    pContext->UpdateSubresource( m_pBillboardInstanceBuffer, 0, NULL, &m_vPositions[0], 0, 0 );
}

void BillboardObject::CleanUp()
{
    if ( m_pBillboardInstanceBuffer )
    {
        m_pBillboardInstanceBuffer->Release();
    }
    if ( m_pBillboardVertexBuffer )
    {
        m_pBillboardVertexBuffer->Release();
    }
    if ( m_pDiffuseResourceView )
    {
        m_pDiffuseResourceView->Release();
    }
}