#include "stdafx.h"
#include "ShaderController.h"

#define SHADER_PATH L"Resources/Shaders/"

ShaderController::ShaderController()
{
    m_uCurrentShader = 0u;
}

ShaderController::~ShaderController()
{
    CleanUp();
}

ShaderController::ShaderData ShaderController::GetFullScreenShaderByName( std::string name )
{
    for ( auto fullShader : m_vFullScreenShaderData )
    {
        if ( fullShader.m_sName == name )
        {
            return fullShader;
        }
    }

    assert( false && "Shader does not exsit in fullscreen shader list" );
    return m_vFullScreenShaderData[0];
}

ShaderController::ShaderData ShaderController::GetFullScreenShaderByNumber( int num )
{
    return m_vFullScreenShaderData[num];
}

ShaderController::ShaderData ShaderController::GetShaderByName( std::string name )
{
    for ( auto shader : m_vShaderData )
    {
        if ( shader.m_sName == name )
        {
            return shader;
        }
    }

    assert( false && "Shader does not exist in the shader list!" );
    return m_vShaderData[0];
}

ShaderController::ShaderData ShaderController::GetShaderData()
{
    return m_vShaderData[m_uCurrentShader];
}

void ShaderController::SetShaderData( UINT shaderNum )
{
    m_uCurrentShader = shaderNum;
}

#pragma region CREATE-SHADERS
bool ShaderController::NewShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    try
    {
        fileName = SHADER_PATH + fileName;
        HRESULT hr = NewVertexShader( fileName, pDevice, pContext, Layout::Default );
        COM_ERROR_IF_FAILED( hr, "Failed to create a VERTEX shader for " + name );

        hr = NewPixelShader( fileName, pDevice );
        COM_ERROR_IF_FAILED( hr, "Failed to create a PIXEL shader for " + name );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    ShaderData newShaderData = ShaderData( name, m_pVertexShader, m_pPixelShader, m_pVertexLayout );
    m_vShaderData.push_back( newShaderData );

    return true;
}

bool ShaderController::NewFullScreenShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    ID3DBlob* pVSBlob = nullptr;
    ID3DBlob* pPSBlob = nullptr;

    try
    {
        fileName = SHADER_PATH + fileName;
        std::wstring filePathCopy = fileName;

        std::wstring shaderTag = L"_VS";
        int position = fileName.find( L".hlsl" );
        fileName = fileName.substr( 0, position ) + shaderTag + fileName.substr( position );

        // Compile the vertex shader
        HRESULT hr = CompileShaderFromFile( fileName, "QuadVS", "vs_5_0", &pVSBlob );
        COM_ERROR_IF_FAILED( hr, "Failed to compile the FULLSCREEN VERTEX shader!" );

        // Create the vertex shader
        hr = pDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pQuadVS );
        COM_ERROR_IF_FAILED( hr, "Failed to create the FULLSCREEN VERTEX shader!" );

        // Define the input layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        // Create the input layout
        UINT numElements = ARRAYSIZE( layout );
        hr = pDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &m_pQuadLayout );
        COM_ERROR_IF_FAILED( hr, "Failed to create the INPUT LAYOUT for a FULLSCREEN vertex shader!" );

        shaderTag = L"_PS";
        position = filePathCopy.find( L".hlsl" );
        filePathCopy = filePathCopy.substr( 0, position ) + shaderTag + filePathCopy.substr( position );

        // Compile the pixel shader
        ID3DBlob* pPSBlob = nullptr;
        hr = CompileShaderFromFile( filePathCopy, "QuadPS", "ps_5_0", &pPSBlob );
        COM_ERROR_IF_FAILED( hr, "Failed to compile the FULLSCREEN PIXEL shader!" );

        // Create the pixel shader
        hr = pDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pQuadPS );
        COM_ERROR_IF_FAILED( hr, "Failed to create the FULLSCREEN PIXEL shader!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        pVSBlob->Release();
        pPSBlob->Release();
        return false;
    }

    ShaderData newShaderData = ShaderData( name, m_pQuadVS, m_pQuadPS, m_pQuadLayout );
    m_vFullScreenShaderData.push_back( newShaderData );

    return true;
}

bool ShaderController::NewGeometryShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    try
    {
        fileName = SHADER_PATH + fileName;
        HRESULT hr = NewVertexShader( fileName, pDevice, pContext, Layout::Instance );
        COM_ERROR_IF_FAILED( hr, "Failed to create a VERTEX shader for " + name );

        hr = NewPixelShader( fileName, pDevice );
        COM_ERROR_IF_FAILED( hr, "Failed to create a PIXEL shader for " + name );

        hr = NewGeometryShader( fileName, pDevice );
        COM_ERROR_IF_FAILED( hr, "Failed to create a GEOMETRY shader for " + name );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    m_geometryData = ShaderData( name, m_pVertexShader, m_pPixelShader, m_pVertexLayout, m_pGeometryShader );
    return true;
}

bool ShaderController::NewTessellationShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    try
    {
        fileName = SHADER_PATH + fileName;
        HRESULT hr = NewVertexShader( fileName, pDevice, pContext, Layout::Terrain );
        COM_ERROR_IF_FAILED( hr, "Failed to create a VERTEX shader for " + name );

        hr = NewHullShader( fileName, pDevice );
        COM_ERROR_IF_FAILED( hr, "Failed to create a HULL shader for " + name );

        hr = NewDomainShader( fileName, pDevice );
        COM_ERROR_IF_FAILED( hr, "Failed to create a DOMAIN shader for " + name );

        hr = NewPixelShader( fileName, pDevice );
        COM_ERROR_IF_FAILED( hr, "Failed to create a PIXEL shader for " + name );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    ShaderData newShaderData = ShaderData( name, m_pVertexShader, m_pPixelShader, m_pVertexLayout, nullptr, m_pHullShader, m_pDomainShader );
    m_vShaderData.push_back( newShaderData );

    return true;
}

bool ShaderController::NewAnimationShader( std::string name, std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    try
    {
        fileName = SHADER_PATH + fileName;
        HRESULT hr = NewVertexShader( fileName, pDevice, pContext, Layout::Animation );
        COM_ERROR_IF_FAILED( hr, "Failed to create a VERTEX shader for " + name );

        hr = NewPixelShader( fileName, pDevice );
        COM_ERROR_IF_FAILED( hr, "Failed to create a PIXEL shader for " + name );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    ShaderData newShaderData = ShaderData( name, m_pVertexShader, m_pPixelShader, m_pVertexLayout );
    m_vShaderData.push_back( newShaderData );

    return true;
}
#pragma endregion

#pragma region COMPILE-SHADERS
HRESULT ShaderController::CompileShaderFromFile( std::wstring fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut )
{
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    dwShaderFlags |= D3DCOMPILE_DEBUG;
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile( fileName.c_str(), nullptr, nullptr, entryPoint,
        shaderModel, dwShaderFlags, 0, blobOut, &pErrorBlob );

    if ( pErrorBlob )
        pErrorBlob->Release();

    if ( FAILED( hr ) )
    {
        if ( pErrorBlob != nullptr ) OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        ErrorLogger::Log( hr, "Failed to compile shader from file!" );
        return hr;
    }

    return hr;
}

bool ShaderController::NewVertexShader( std::wstring fileName, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, Layout layout )
{
    ID3DBlob* pVSBlob = nullptr;

    try
    {
        std::wstring shaderTag = L"_VS";
        int position = fileName.find( L".hlsl" );
        fileName = fileName.substr( 0, position ) + shaderTag + fileName.substr( position );

        // Compile the vertex shader
        HRESULT hr = CompileShaderFromFile( fileName.c_str(), "VS", "vs_5_0", &pVSBlob );
        COM_ERROR_IF_FAILED( hr, "Failed to compile the VERTEX shader!" );

        // Create the vertex shader
        hr = pDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader );
        COM_ERROR_IF_FAILED( hr, "Failed to create the VERTEX shader!" );

        switch ( layout )
        {
        case Layout::Default:
        {
            // Define the input layout
            D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "biTangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            // Create the input layout
            UINT numElements = ARRAYSIZE( layout );
            hr = pDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                pVSBlob->GetBufferSize(), &m_pVertexLayout );
            COM_ERROR_IF_FAILED( hr, "Failed to create the INPUT LAYOUT for a DEFAULT vertex shader!" );
        }
        break;
        case Layout::Instance:
        {
            // Define the input layout
            D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0 , D3D11_INPUT_PER_INSTANCE_DATA, 1 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "biTangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

            };

            // Create the input layout
            UINT numElements = ARRAYSIZE( layout );
            hr = pDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                pVSBlob->GetBufferSize(), &m_pVertexLayout );
            COM_ERROR_IF_FAILED( hr, "Failed to create the INPUT LAYOUT for an INSTANCE vertex shader!" );
        }
        break;
        case Layout::Terrain:
        {
            // Define the input layout
            D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "BoundsY", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            // Create the input layout
            UINT numElements = ARRAYSIZE( layout );
            hr = pDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                pVSBlob->GetBufferSize(), &m_pVertexLayout );
            COM_ERROR_IF_FAILED( hr, "Failed to create the INPUT LAYOUT for a TERRAIN vertex shader!" );
        }
        break;
        case Layout::Animation:
        {
            // Define the input layout
            D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "WEIGHTS",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "BONEINDICES",  0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };

            // Create the input layout
            UINT numElements = ARRAYSIZE( layout );
            hr = pDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                pVSBlob->GetBufferSize(), &m_pVertexLayout );
            COM_ERROR_IF_FAILED( hr, "Failed to create the INPUT LAYOUT for an ANIMATION vertex shader!" );
        }
        break;
        }

        // Set the input layout
        pContext->IASetInputLayout( m_pVertexLayout );

    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        pVSBlob->Release();
        return false;
    }

    return true;
}

bool ShaderController::NewPixelShader( std::wstring fileName, ID3D11Device* pDevice )
{
    ID3DBlob* pPSBlob = nullptr;

    try
    {
        std::wstring shaderTag = L"_PS";
        int position = fileName.find( L".hlsl" );
        fileName = fileName.substr( 0, position ) + shaderTag + fileName.substr( position );

        // Compile the pixel shader
        HRESULT hr = CompileShaderFromFile( fileName, "PS", "ps_5_0", &pPSBlob );
        COM_ERROR_IF_FAILED( hr, "Failed to compile the PIXEL shader!" );

        // Create the pixel shader
        hr = pDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader );
        COM_ERROR_IF_FAILED( hr, "Failed to create the PIXEL shader!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        pPSBlob->Release();
        return false;
    }

    return true;
}

bool ShaderController::NewGeometryShader( std::wstring fileName, ID3D11Device* pDevice )
{
    ID3DBlob* pGSBlob = nullptr;

    try
    {
        std::wstring shaderTag = L"_GS";
        int position = fileName.find( L".hlsl" );
        fileName = fileName.substr( 0, position ) + shaderTag + fileName.substr( position );

        // Compile the geometry shader
        HRESULT hr = CompileShaderFromFile( fileName, "GS", "gs_5_0", &pGSBlob );
        COM_ERROR_IF_FAILED( hr, "Failed to compile the GEOMETRY shader!" );

        // Create the geometry shader
        hr = pDevice->CreateGeometryShader( pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_pGeometryShader );
        COM_ERROR_IF_FAILED( hr, "Failed to create the GEOMETRY shader!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        pGSBlob->Release();
        return false;
    }

    return true;
}

bool ShaderController::NewHullShader( std::wstring fileName, ID3D11Device* pDevice )
{
    ID3DBlob* pHSBlob = nullptr;

    try
    {
        std::wstring shaderTag = L"_HS";
        int position = fileName.find( L".hlsl" );
        fileName = fileName.substr( 0, position ) + shaderTag + fileName.substr( position );

        // Compile the hull shader
        HRESULT hr = CompileShaderFromFile( fileName, "HS", "hs_5_0", &pHSBlob );
        COM_ERROR_IF_FAILED( hr, "Failed to compile the HULL shader!" );

        // Create the hull shader
        hr = pDevice->CreateHullShader( pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &m_pHullShader );
        COM_ERROR_IF_FAILED( hr, "Failed to create the HULL shader!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        pHSBlob->Release();
        return false;
    }

    return true;
}

bool ShaderController::NewDomainShader( std::wstring fileName, ID3D11Device* pDevice )
{
    ID3DBlob* pDSBlob = nullptr;

    try
    {
        std::wstring shaderTag = L"_DS";
        int position = fileName.find( L".hlsl" );
        fileName = fileName.substr( 0, position ) + shaderTag + fileName.substr( position );

        // Compile the domain shader
        HRESULT hr = CompileShaderFromFile( fileName, "DS", "ds_5_0", &pDSBlob );
        COM_ERROR_IF_FAILED( hr, "Failed to compile the DOMAIN shader!" );

        // Create the domain shader
        hr = pDevice->CreateDomainShader( pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &m_pDomainShader );
        COM_ERROR_IF_FAILED( hr, "Failed to create the DOMAIN shader!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        pDSBlob->Release();
        return false;
    }

    return true;
}
#pragma endregion

void ShaderController::CleanUp()
{
    m_pVertexShader = nullptr;
    m_pPixelShader = nullptr;
    m_pVertexLayout = nullptr;

    m_pQuadLayout = nullptr;
    m_pQuadVS = nullptr;
    m_pQuadPS = nullptr;

    for ( auto shader : m_vShaderData )
        shader.CleanUp();

    for ( auto shader : m_vFullScreenShaderData )
        shader.CleanUp();

    m_geometryData.CleanUp();
}