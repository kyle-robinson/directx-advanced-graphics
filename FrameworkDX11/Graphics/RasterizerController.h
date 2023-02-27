#pragma once
#ifndef RASTERIZERCONTROLLER_H
#define RASTERIZERCONTROLLER_H

#include "Rasterizer.h"

class RasterizerController
{
public:
    RasterizerController();
    ~RasterizerController();

	void CreateState( std::string name, Bind::Rasterizer::Type type, ID3D11Device* pDevice );
    void SetState( std::string name, ID3D11DeviceContext* pContext );
    void SetOverrideState( ID3D11DeviceContext* pContext );
    void SetState( std::string name );

    inline std::map<std::string, std::shared_ptr<Bind::Rasterizer>> GetStates() const noexcept { return m_vRasterStates; }
    std::shared_ptr<Bind::Rasterizer> GetState( std::string name );
    std::vector<std::string> GetStateNames();

private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_pCurrentState;
    std::map<std::string, std::shared_ptr<Bind::Rasterizer>> m_vRasterStates;
};

#endif