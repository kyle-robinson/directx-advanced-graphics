#include "stdafx.h"
#include "Cube.h"
#include "Camera.h"
#include <imgui/imgui.h>

Vertex vertices[] =
{
	{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },
	{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },
	{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },
	{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },

	{ { -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },
	{ {  1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },
	{ {  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },
	{ { -1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },

	{ { -1.0f, -1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },
	{ { -1.0f, -1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },
	{ { -1.0f,  1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },
	{ { -1.0f,  1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },

	{ {  1.0f, -1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },
	{ {  1.0f, -1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },
	{ {  1.0f,  1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },
	{ {  1.0f,  1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },

	{ { -1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
	{ {  1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
	{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
	{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },

	{ { -1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
	{ {  1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
	{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
	{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
};

WORD indices[] =
{
	3,1,0,
	2,1,3,

	6,4,5,
	7,4,6,

	11,9,8,
	10,9,11,

	14,12,13,
	15,12,14,

	19,17,16,
	18,17,19,

	22,20,21,
	23,20,22
};

bool Cube::InitializeMesh( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	try
	{
		// Set position to world origin
		XMStoreFloat4x4( &m_World, XMMatrixIdentity() );
		m_position = { 0.0f, 0.0f, 0.0f };
		m_rotation = { 0.0f, 0.0f, 0.0f };
		m_scale = { 1.0f, 1.0f, 1.0f };

		// Create vertex buffer
		HRESULT hr = m_vertexBuffer.Initialize( pDevice, vertices, ARRAYSIZE( vertices ) );
        COM_ERROR_IF_FAILED( hr, "Failed to create cube vertex buffer!" );
		
		// Create index buffer
        hr = m_indexBuffer.Initialize( pDevice, indices, ARRAYSIZE( indices ) );
        COM_ERROR_IF_FAILED( hr, "Failed to create cube index buffer!" );

		// Load and setup textures
		m_pTexturesDiffuse.resize( 3u );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\bricks_TEX.dds", nullptr, m_pTexturesDiffuse[0].GetAddressOf() );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\crate_TEX.dds", nullptr, m_pTexturesDiffuse[1].GetAddressOf() );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\rocks_TEX.dds", nullptr, m_pTexturesDiffuse[2].GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'diffuse' texture!" );

		m_pTexturesNormal.resize( 3u );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\bricks_NORM.dds", nullptr, m_pTexturesNormal[0].GetAddressOf() );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\crate_NORM.dds", nullptr, m_pTexturesNormal[1].GetAddressOf() );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\rocks_NORM.dds", nullptr, m_pTexturesNormal[2].GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'normal' texture!" );

		m_pTexturesDisplacement.resize( 3u );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\bricks_DISP.dds", nullptr, m_pTexturesDisplacement[0].GetAddressOf() );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\crate_DISP.dds", nullptr, m_pTexturesDisplacement[1].GetAddressOf() );
		hr = DirectX::CreateDDSTextureFromFile( pDevice, L"Resources\\Textures\\rocks_DISP.dds", nullptr, m_pTexturesDisplacement[2].GetAddressOf() );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'displacement' texture!" );

		// Setup constant buffer
		hr = m_cbMaterial.Initialize( pDevice, pContext );
		COM_ERROR_IF_FAILED( hr, "Failed to create 'Material' constant buffer!" );
	}
	catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}

	return true;
}

void Cube::Update( float dt )
{
	if ( m_bEnableSpin )
	{
		if ( m_bReverseSpin )
			m_rotation.y -= dt;
		else
			m_rotation.y += dt;
	}

	UpdateMatrix();
}

void Cube::UpdateCB()
{
	// Setup material data
	MaterialData materialData;
	materialData.Emissive = m_fEmissive;
	materialData.Ambient = m_fAmbient;
	materialData.Diffuse = m_fDiffuse;
	materialData.Specular = m_fSpecular;
	materialData.SpecularPower = m_fSpecularPower;
	materialData.UseTexture = m_bUseTexture;

	// Add to constant buffer
	m_cbMaterial.data.Material = materialData;
	if ( !m_cbMaterial.ApplyChanges() ) return;
}

void Cube::UpdateMatrix()
{
	XMMATRIX world = XMMatrixScaling( m_scale.x, m_scale.y, m_scale.z ) *
		XMMatrixRotationRollPitchYaw( m_rotation.x, m_rotation.y, m_rotation.z ) *
		XMMatrixTranslation( m_position.x, m_position.y, m_position.z );
	XMStoreFloat4x4( &m_World, world );
}

void Cube::UpdateBuffers( ConstantBuffer<Matrices>& cb_vs_matrices, const Camera& pCamera )
{
	// Get the game object world transform
    XMMATRIX mGO = XMLoadFloat4x4( &m_World );
	cb_vs_matrices.data.mWorld = XMMatrixTranspose( mGO );
    
    // Store the view / projection in a constant buffer for the vertex shader to use
	cb_vs_matrices.data.mView = XMMatrixTranspose( pCamera.GetViewMatrix() );
	cb_vs_matrices.data.mProjection = XMMatrixTranspose( pCamera.GetProjectionMatrix() );
	if ( !cb_vs_matrices.ApplyChanges() ) return;
}

void Cube::Draw( ID3D11DeviceContext* pContext )
{
	UINT offset = 0;
	pContext->IASetVertexBuffers( 0u, 1u, m_vertexBuffer.GetAddressOf(), m_vertexBuffer.StridePtr(), &offset );
	pContext->IASetIndexBuffer( m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0 );

	pContext->PSSetShaderResources( 0u, 1u, m_pTexturesDiffuse[m_textureIndex].GetAddressOf() );
	pContext->PSSetShaderResources( 1u, 1u, m_pTexturesNormal[m_textureIndex].GetAddressOf() );
	pContext->PSSetShaderResources( 2u, 1u, m_pTexturesDisplacement[m_textureIndex].GetAddressOf() );

	pContext->DrawIndexed( m_indexBuffer.IndexCount(), 0u, 0u );
}

void Cube::DrawDeferred(
	ID3D11DeviceContext* pContext,
	ID3D11ShaderResourceView** position,
	ID3D11ShaderResourceView** albedo,
	ID3D11ShaderResourceView** normal )
{
	UINT offset = 0;
	pContext->IASetVertexBuffers( 0u, 1u, m_vertexBuffer.GetAddressOf(), m_vertexBuffer.StridePtr(), &offset );
	pContext->IASetIndexBuffer( m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0 );

	pContext->PSSetShaderResources( 0u, 1u, position );
	pContext->PSSetShaderResources( 1u, 1u, albedo );
	pContext->PSSetShaderResources( 2u, 1u, normal );

	pContext->DrawIndexed( m_indexBuffer.IndexCount(), 0u, 0u );
}

void Cube::SpawnControlWindows()
{
	if ( ImGui::Begin( "Texture Data", FALSE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
	{
		static int activeTexture = 0;
		static bool selectedTexture[3];
		static std::string previewValueTexture = "Red Bricks";
		static const char* textureList[]{ "Red Bricks", "Wooden Crate", "Cobblestone" };
		ImGui::Text( "Active Texture" );
		if ( ImGui::BeginCombo( "##Active Texture", previewValueTexture.c_str() ) )
		{
			for ( uint32_t i = 0; i < IM_ARRAYSIZE( textureList ); i++ )
			{
				const bool isSelected = i == activeTexture;
				if ( ImGui::Selectable( textureList[i], isSelected ) )
				{
					activeTexture = i;
					previewValueTexture = textureList[i];
				}
			}

			switch ( activeTexture )
			{
			case 0: m_textureIndex = 0; break;
			case 1: m_textureIndex = 1; break;
			case 2: m_textureIndex = 2; break;
			}

			ImGui::EndCombo();
		}
	}
	ImGui::End();

	if ( ImGui::Begin( "Cube Data", FALSE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
	{
		ImGui::Checkbox( "Rotate Cube?", &m_bEnableSpin );
		ImGui::Checkbox( "Reverse?", &m_bReverseSpin );
		if ( ImGui::Checkbox( "Reset?", &m_bResetSpin ) )
		{
			m_rotation.y = 0.0f;
			m_bResetSpin = false;
			m_bEnableSpin = false;
			m_bReverseSpin = false;
		}
	}
	ImGui::End();

	if ( ImGui::Begin( "Material Data", FALSE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
	{
		ImGui::Text( "Emission Color" );
		ImGui::SliderFloat4( "##Emission", &m_fEmissive.x, 0.0f, 1.0f, "%.1f" );
		ImGui::NewLine();

		ImGui::Text( "Ambient Color" );
		ImGui::SliderFloat4( "##Ambient", &m_fAmbient.x, 0.0f, 1.0f, "%.1f" );
		ImGui::NewLine();

		ImGui::Text( "Diffuse Color" );
		ImGui::SliderFloat4( "##Diffuse", &m_fDiffuse.x, 0.0f, 1.0f, "%.1f" );
		ImGui::NewLine();

		ImGui::Text( "Specular Color" );
		ImGui::SliderFloat4( "##Specular", &m_fSpecular.x, 0.0f, 1.0f, "%.1f" );
		ImGui::NewLine();

		ImGui::Text( "Specular Power" );
		ImGui::SliderFloat( "##Spec Power", &m_fSpecularPower, 0.0f, 256.0f, "%1.f" );
		ImGui::NewLine();

		static bool useTexture = m_bUseTexture;
		ImGui::Checkbox( "Use Texture?", &useTexture );
		m_bUseTexture = useTexture;
	}
	ImGui::End();
}