#pragma once
#ifndef SAMPLERCONTROLLER_H
#define SAMPLERCONTROLLER_H

#include "Sampler.h"

class SamplerController
{
public:
	SamplerController();
	~SamplerController();

	void CreateState( std::string name, Bind::Sampler::Type type, ID3D11Device* pDevice );
	void SetState( std::string name, UINT slot, ID3D11DeviceContext* pContext );

	inline std::map<std::string, std::shared_ptr<Bind::Sampler>> GetStates() const noexcept { return m_pSamplerStates; }
	std::shared_ptr<Bind::Sampler> GetState( std::string name );
	std::vector<std::string> GetStateNames();

private:
	std::map<std::string, std::shared_ptr<Bind::Sampler>> m_pSamplerStates;
};

#endif