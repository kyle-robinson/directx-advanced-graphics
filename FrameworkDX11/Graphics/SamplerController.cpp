#include "stdafx.h"
#include "SamplerController.h"

SamplerController::SamplerController() {}

SamplerController::~SamplerController() {}

void SamplerController::CreateState( std::string name, Bind::Sampler::Type type, ID3D11Device* pDevice )
{
	m_pSamplerStates[name] = std::make_shared<Bind::Sampler>( pDevice, type );
}

void SamplerController::SetState( std::string name, UINT slot, ID3D11DeviceContext* pContext )
{
	m_pSamplerStates[name]->Bind( pContext, slot );
}

std::shared_ptr<Bind::Sampler> SamplerController::GetState( std::string name )
{
	return m_pSamplerStates[name];
}

std::vector<std::string> SamplerController::GetStateNames()
{
	std::vector<std::string> names;
	for ( auto& state : m_pSamplerStates )
	{
		names.push_back( state.first );
	}
	return names;
}