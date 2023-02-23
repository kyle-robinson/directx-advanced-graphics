#include "stdafx.h"
#include "RasterizerController.h"

RasterizerController::RasterizerController() {}

RasterizerController::~RasterizerController() {}

void RasterizerController::CreateState( std::string name, Bind::Rasterizer::Type type, ID3D11Device* pDevice )
{
    m_vRasterStates.emplace( name, std::make_shared<Bind::Rasterizer>( pDevice, type ) );
}

void RasterizerController::SetState( std::string name, ID3D11DeviceContext* pContext )
{
    m_vRasterStates.at( name )->Bind( pContext );
}

void RasterizerController::SetOverrideState( ID3D11DeviceContext* pContext )
{
    if ( m_pCurrentState )
    {
        pContext->RSSetState( m_pCurrentState.Get() );
    }
    else
    {
        pContext->RSSetState( m_vRasterStates.begin()->second->Get() );
    }
}

void RasterizerController::SetState( std::string name )
{
    m_pCurrentState = m_vRasterStates.at( name )->Get();
}

std::shared_ptr<Bind::Rasterizer> RasterizerController::GetState( std::string name )
{
    return m_vRasterStates.at( name );
}

std::vector<std::string> RasterizerController::GetStateNames()
{
    std::vector<std::string> names;
    for ( auto state : m_vRasterStates )
    {
        names.push_back( state.first );
    }

    return names;
}