#include "RansterStateManager.h"

RasterStateManager::RasterStateManager()
{
}

RasterStateManager::~RasterStateManager()
{
    CleanUp();
}

void RasterStateManager::SetCurrentState(std::string State)
{
    CurrentState = RasterStates.at(State);
}

void RasterStateManager::CreatRasterizerState(ID3D11Device* pDevice, D3D11_RASTERIZER_DESC SateSetting, std::string Name)
{
    pDevice->CreateRasterizerState(&SateSetting, &RasterStates[Name]);
}

void RasterStateManager::AddRasterizerState(std::string Name, ID3D11RasterizerState* state)
{
    RasterStates[Name] = state;
}

ID3D11RasterizerState* RasterStateManager::GetRasterizerState(std::string state)
{
    return RasterStates.at(state);
}

void RasterStateManager::SetRasterizerState(std::string state, ID3D11DeviceContext* context)
{
    context->RSSetState(RasterStates.at(state));
}

void RasterStateManager::SetRasterizerState(ID3D11DeviceContext* context)
{
    if (CurrentState) {
        context->RSSetState(CurrentState);
    }
    else
    {
        context->RSSetState(RasterStates.begin()->second);
    }
}

std::vector<std::string> RasterStateManager::GetStateNames()
{
    std::vector<std::string> Names;
    for (auto RS : RasterStates)
    {
        Names.push_back(RS.first);
    }

    return Names;
}

void RasterStateManager::CleanUp()
{
    for (auto RS: RasterStates)
    {
        if (RS.second) {
             RS.second->Release();
        }
        RS.second = nullptr;
    }

    RasterStates.empty();
}
