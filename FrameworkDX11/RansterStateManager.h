#pragma once
#include<map>
#include<vector>
#include<string>
#include <d3d11_1.h>




class RasterStateManager
{
public:
    RasterStateManager();
    ~RasterStateManager();

    void SetCurrentState(std::string State);
    void CreatRasterizerState(ID3D11Device* pDevice, D3D11_RASTERIZER_DESC SateSetting,std::string Name);
    void AddRasterizerState(std::string Name,ID3D11RasterizerState* state);
    ID3D11RasterizerState* GetRasterizerState(std::string state);
    void SetRasterizerState(std::string state, ID3D11DeviceContext* context);
    void SetRasterizerState(ID3D11DeviceContext* context);
    std::map<std::string, ID3D11RasterizerState*> GetStates() { return RasterStates; }
    std::vector<std::string> GetStateNames();
private:
    void CleanUp();
    ID3D11RasterizerState* CurrentState= nullptr;
    std::map<std::string, ID3D11RasterizerState*> RasterStates;
};

