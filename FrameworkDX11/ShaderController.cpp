#include "ShaderController.h"

ShaderController::ShaderController()
{
    CurrentShader = 0;
    
}

ShaderController::~ShaderController()
{
    CleanUp();
}

HRESULT ShaderController::NewShader(string Name,const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{

    HRESULT hr = NewVertexShader(szFileName, pd3dDevice, pImmediateContext, Layout::Defualt);
    if (FAILED(hr))
        return hr;
    
    hr = NewPixleShader(szFileName, pd3dDevice);
    if (FAILED(hr))
        return hr;


    ShaderData NewShaderData= ShaderData(Name,_pVertexShader, _pPixelShader, _pVertexLayout);

    _ShaderData.push_back(NewShaderData);

    return S_OK;
}

HRESULT ShaderController::NewVertexShader(const WCHAR* szFileName,ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext , Layout layout)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(szFileName, "VS", "vs_5_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    ID3D11VertexShader* pVertexShader;
    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }
    _pVertexShader = pVertexShader;
   
    switch (layout)
    {
    case Layout::Defualt:
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
        UINT numElements = ARRAYSIZE(layout);

        // Create the input layout
        hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &_pVertexLayout);
        pVSBlob->Release();
        if (FAILED(hr))
            return hr;
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
        UINT numElements = ARRAYSIZE(layout);
        // Create the input layout
        hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &_pVertexLayout);
        pVSBlob->Release();
        if (FAILED(hr))
            return hr;
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
        UINT numElements = ARRAYSIZE(layout);

        // Create the input layout
        hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &_pVertexLayout);
        pVSBlob->Release();
        if (FAILED(hr))
            return hr;
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
            {"WEIGHTS",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"BONEINDICES",  0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        UINT numElements = ARRAYSIZE(layout);

        // Create the input layout
        hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &_pVertexLayout);
        pVSBlob->Release();
        if (FAILED(hr))
            return hr;
    }
    break;
    default:
        break;
    }
  
    // Set the input layout
    pImmediateContext->IASetInputLayout(_pVertexLayout);

    return hr;
}


HRESULT ShaderController::NewPixleShader(const WCHAR* szFileName,ID3D11Device* pd3dDevice)
{
    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(szFileName, "PS", "ps_5_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }
    
    // Create the pixel shader
    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

  
    return hr;
}



ShaderData ShaderController::GetShaderData()
{
    return _ShaderData[CurrentShader];
}

void ShaderController::SetShaderData(UINT ShaderSet)
{
    CurrentShader = ShaderSet;
}

ShaderData ShaderController::GetShaderByName(string Name)
{
    for (auto Shader : _ShaderData) {
        if (Shader.Name == Name) {
            return Shader;
        }
    }
    assert(false && "Shader does not exsit in list");
    return _ShaderData[0];
}

HRESULT ShaderController::NewFullScreenShader(string Name, const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{




    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(szFileName, "QuadVS", "vs_5_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

  
    // Create the vertex shader
    hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pQuadVS);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }
    

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pQuadLayout);

    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

 
    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
     hr = CompileShaderFromFile(szFileName, "QuadPS", "ps_5_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pQuadPS);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;


    ShaderData NewShaderData = ShaderData(Name, g_pQuadVS, g_pQuadPS, g_pQuadLayout);

    _FullScreenShaderData.push_back(NewShaderData);



    return hr;

    
}

ShaderData ShaderController::GetFullScreenShaderByName(string Name)
{

    for (auto FullShader : _FullScreenShaderData) {
        if (FullShader.Name == Name) {
            return FullShader;
        }
    }
    assert(false && "Shader does not exsit in list");
    return _FullScreenShaderData[0];
}

ShaderData ShaderController::GetFullScreenShaderByNumber(int No)
{
    return _FullScreenShaderData[No];
}

HRESULT ShaderController::NewGeoShader(string Name, const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{

    HRESULT hr = NewVertexShader(szFileName, pd3dDevice, pImmediateContext, Layout::Instance);
    if (FAILED(hr))
        return hr;

    hr = NewPixleShader(szFileName, pd3dDevice);
    if (FAILED(hr))
        return hr;

    hr = NewGeometryShader(szFileName, pd3dDevice, pImmediateContext);
    if (FAILED(hr))
        return hr;

    GeoShader = ShaderData(Name, _pVertexShader, _pPixelShader, _pVertexLayout, _pGeometryShader);
    return S_OK;
}

HRESULT ShaderController::NewTessShader(string Name, const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
    HRESULT hr;
   

    hr = NewVertexShader(szFileName, pd3dDevice, pImmediateContext, Layout::Terrain);
    if (FAILED(hr))
        return hr;
     hr = NewHullShader(szFileName, pd3dDevice, pImmediateContext);
        if (FAILED(hr))
            return hr;
     hr = NewDomianShader(szFileName, pd3dDevice, pImmediateContext);
        if (FAILED(hr))
            return hr;
    hr = NewPixleShader(szFileName, pd3dDevice);
    if (FAILED(hr))
        return hr;

     
    

    ShaderData NewShaderData = ShaderData(Name, _pVertexShader, _pPixelShader, _pVertexLayout,nullptr, _pHullShader, _pDomainShader);
    _ShaderData.push_back(NewShaderData);
    return hr;
}

HRESULT ShaderController::NewAnimationShader(string Name, const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
    HRESULT hr = NewVertexShader(szFileName, pd3dDevice, pImmediateContext, Layout::Animation);
    if (FAILED(hr))
        return hr;

    hr = NewPixleShader(szFileName, pd3dDevice);
    if (FAILED(hr))
        return hr;

    ShaderData NewShaderData = ShaderData(Name, _pVertexShader, _pPixelShader, _pVertexLayout);

    _ShaderData.push_back(NewShaderData);

    return S_OK;
}

HRESULT ShaderController::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

void ShaderController::CleanUp()
{
     _pVertexShader = nullptr;
     _pPixelShader = nullptr;
     _pVertexLayout = nullptr;

     for (auto Shader : _ShaderData) {

         Shader.CleanUp();
     }


     g_pQuadLayout = nullptr;
     g_pQuadVS = nullptr;
     g_pQuadPS = nullptr;

     for (auto Shader : _FullScreenShaderData) {

         Shader.CleanUp();
     }

     GeoShader.CleanUp();
}


HRESULT ShaderController::NewGeometryShader(const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(szFileName, "GS", "gs_5_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    ID3D11GeometryShader* pGeoShader;
    // Create the vertex shader
    hr = pd3dDevice->CreateGeometryShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pGeoShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }
    _pGeometryShader = pGeoShader;

    

    return hr;
}

HRESULT ShaderController::NewHullShader(const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(szFileName, "HS", "hs_5_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    ID3D11HullShader* pHullShader;
    // Create the vertex shader
    hr = pd3dDevice->CreateHullShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pHullShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }
    _pHullShader = pHullShader;



    return hr;
}

HRESULT ShaderController::NewDomianShader(const WCHAR* szFileName, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(szFileName, "DS", "ds_5_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    ID3D11DomainShader* pDomainShader;
    // Create the vertex shader
    hr = pd3dDevice->CreateDomainShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pDomainShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }
    _pDomainShader = pDomainShader;



    return hr;
}
