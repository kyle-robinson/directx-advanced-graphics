#include "stdafx.h"
#include "Deferred.h"
#include <imgui/imgui.h>

bool Deferred::Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	try
	{
		HRESULT hr = m_cbDeferred.Initialize( pDevice, pContext );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'Deferred' constant buffer!" );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}
	return true;
}

void Deferred::UpdateCB()
{
	// Setup mapping data
	DeferredData drData;
	drData.UseDeferredShading = m_bUseDeferredShading;
	drData.OnlyPositions = m_bOnlyPositions;
	drData.OnlyAlbedo = m_bOnlyAlbedo;
	drData.OnlyNormals = m_bOnlyNormals;

	// Add to constant buffer
	m_cbDeferred.data.Deferred = drData;
    if ( !m_cbDeferred.ApplyChanges() ) return;
}

void Deferred::SpawnControlWindow()
{
	if ( ImGui::Begin( "Deferred Rendering", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		static bool useDeferred = m_bUseDeferredShading;
	    ImGui::Checkbox( "Use Deferred Shading?", &useDeferred );
		m_bUseDeferredShading = useDeferred;

		if ( m_bUseDeferredShading )
		{
			// normal & parallax mapping
			static int activeRenderTechnique = 0;
			static bool selectedRenderTechnique[4];
			static std::string previewValueRenderTechnique = "None";
			static const char* renderList[]{ "None", "Only Positions", "Only Albedo", "Only Normals" };
			ImGui::Text( "Rendering Technique" );
			if ( ImGui::BeginCombo( "##Rendering Technique", previewValueRenderTechnique.c_str() ) )
			{
				for ( uint32_t i = 0; i < IM_ARRAYSIZE( renderList ); i++ )
				{
					const bool isSelected = i == activeRenderTechnique;
					if ( ImGui::Selectable( renderList[i], isSelected ) )
					{
						activeRenderTechnique = i;
						previewValueRenderTechnique = renderList[i];
					}
				}

				switch ( activeRenderTechnique )
				{
				case 0: m_bOnlyPositions = false; m_bOnlyAlbedo = false; m_bOnlyNormals = false; break;
				case 1: m_bOnlyPositions = true; m_bOnlyAlbedo = false; m_bOnlyNormals = false; break;
				case 2: m_bOnlyPositions = false; m_bOnlyAlbedo = true; m_bOnlyNormals = false; break;
				case 3: m_bOnlyPositions = false; m_bOnlyAlbedo = false; m_bOnlyNormals = true; break;
				}

				ImGui::EndCombo();
			}
		}
	}
	ImGui::End();
}