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

    try
    {
        // Create instance vertex buffer
        HRESULT hr = m_billboardInstanceVB.Initialize( pDevice, &m_vPositions[0], m_iNumberOfBillBoards );
        COM_ERROR_IF_FAILED( hr, "Failed to create BILLBOARD INSTANCE vertex buffer!" );

        // Create vertex buffer
        SimpleVertexBillboard sharedData;
        sharedData.Pos = XMFLOAT3( 0.0f, 0.0f, 0.0f );
        hr = m_billboardVB.Initialize( pDevice, &sharedData, 1 );
        COM_ERROR_IF_FAILED( hr, "Failed to create BILLBOARD vertex buffer!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}

void BillboardObject::Draw( ID3D11DeviceContext* pContext, ShaderController::ShaderData shaderData, ConstantBuffer<MatrixBuffer>& buffer )
{
    pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
    pContext->IASetInputLayout( shaderData.m_pVertexLayout );

    UINT strides[2] = { m_billboardVB.Stride(), m_billboardInstanceVB.Stride() };
    UINT offsets[2] = { 0, 0 };

    buffer.data.mWorld = XMMatrixIdentity();
    if ( !buffer.ApplyChanges() )
        return;

    // Load vertex buffers
    ID3D11Buffer* vertBillInstBuffers[2] = { m_billboardVB.Get(), m_billboardInstanceVB.Get() };
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
    try
    {
        std::wstring tex = std::wstring( texName.begin(), texName.end() );
        const wchar_t* widecstr = tex.c_str();
        HRESULT hr = CreateDDSTextureFromFile( pDevice, widecstr, nullptr, &m_pDiffuseResourceView );
        COM_ERROR_IF_FAILED( hr, "Failed to create DIFFUSE texture for BILLBOARD object!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}

void BillboardObject::UpdatePositions( ID3D11DeviceContext* pContext )
{
    m_billboardInstanceVB.SetVertexPositions( m_vPositions );
}

void BillboardObject::CleanUp()
{
    if ( m_pDiffuseResourceView )
    {
        m_pDiffuseResourceView->Release();
    }
}