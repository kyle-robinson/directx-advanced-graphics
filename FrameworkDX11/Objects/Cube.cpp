#include "stdafx.h"
#include "Cube.h"
#include "Camera.h"
#include <imgui/imgui.h>

#define NUM_VERTICES 36

//Vertex vertices[] =
//{
//	{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },
//	{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },
//	{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },
//	{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } },
//
//	{ { -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },
//	{ {  1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },
//	{ {  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },
//	{ { -1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } },
//
//	{ { -1.0f, -1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ { -1.0f, -1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ { -1.0f,  1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ { -1.0f,  1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } },
//
//	{ {  1.0f, -1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ {  1.0f, -1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ {  1.0f,  1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ {  1.0f,  1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } },
//
//	{ { -1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ {  1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//
//	{ { -1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ {  1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//	{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } },
//};

//WORD indices[] =
//{
//	3,1,0,
//	2,1,3,
//
//	6,4,5,
//	7,4,6,
//
//	11,9,8,
//	10,9,11,
//
//	14,12,13,
//	15,12,14,
//
//	19,17,16,
//	18,17,19,
//
//	22,20,21,
//	23,20,22
//};

bool Cube::InitializeMesh( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	try
	{
		// Set position to world origin
		XMStoreFloat4x4( &m_World, XMMatrixIdentity() );
		m_position = { 0.0f, 0.0f, 0.0f };
		m_rotation = { 0.0f, 0.0f, 0.0f };
		m_scale = { 1.0f, 1.0f, 1.0f };

		Vertex vertices[] =
		{
			//top
			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//3
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//1
			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) },//0

			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) },//2
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//1
			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//3

			//bot
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//6
			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//4
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) },//5

			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) },//7
			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//4
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//6

			//left

			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//11
			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//9
			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) },//8


			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) },//10
			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//9
			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//11

			//right
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//14
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//12
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) },//13

			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) },//15
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) },//12
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },//14

			//front
			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },//19
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ) , XMFLOAT2( 1.0f, 1.0f ) },//17
			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },//16

			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },//18
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ) , XMFLOAT2( 1.0f, 1.0f ) },//17
			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },//19

			//back
			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },//22
			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },//20
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },//21

			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },//23
			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },//20
			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },//22
		};

		WORD indices[] =
		{
			0,1,2,
			3,4,5,

			6,7,8,
			9,10,11,

			12,13,14,
			15,16,17,

			18,19,20,
			21,22,23,

			24,25,26,
			27,28,29,

			30,31,32,
			33,34,35
		};

		// Create vertex buffer
		CalculateModelVectors( vertices );
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
		UpdateCB(); // Update constant buffer with default values
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
	UpdateCB();
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

void Cube::CalculateTangentBinormalLH( Vertex v0, Vertex v1, Vertex v2, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal )
{
	XMFLOAT3 edge1( v1.Position.x - v0.Position.x, v1.Position.y - v0.Position.y, v1.Position.z - v0.Position.z );
	XMFLOAT3 edge2( v2.Position.x - v0.Position.x, v2.Position.y - v0.Position.y, v2.Position.z - v0.Position.z );

	XMFLOAT2 deltaUV1( v1.TexCoord.x - v0.TexCoord.x, v1.TexCoord.y - v0.TexCoord.y );
	XMFLOAT2 deltaUV2( v2.TexCoord.x - v0.TexCoord.x, v2.TexCoord.y - v0.TexCoord.y );

	float f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );

	tangent.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
	tangent.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
	tangent.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
	XMVECTOR tn = XMLoadFloat3( &tangent );
	tn = XMVector3Normalize( tn );
	XMStoreFloat3( &tangent, tn );

	binormal.x = f * ( deltaUV1.x * edge2.x - deltaUV2.x * edge1.x );
	binormal.y = f * ( deltaUV1.x * edge2.y - deltaUV2.x * edge1.y );
	binormal.z = f * ( deltaUV1.x * edge2.z - deltaUV2.x * edge1.z );

	tn = XMLoadFloat3( &binormal );
	tn = XMVector3Normalize( tn );
	XMStoreFloat3( &binormal, tn );

	XMVECTOR vv0 = XMLoadFloat3( &v0.Position );
	XMVECTOR vv1 = XMLoadFloat3( &v1.Position );
	XMVECTOR vv2 = XMLoadFloat3( &v2.Position );

	XMVECTOR e0 = vv1 - vv0;
	XMVECTOR e1 = vv2 - vv0;

	XMVECTOR e01cross = XMVector3Cross( e0, e1 );
	e01cross = XMVector3Normalize( e01cross );
	XMFLOAT3 normalOut;
	XMStoreFloat3( &normalOut, e01cross );
	normal = normalOut;
}

void Cube::CalculateModelVectors( Vertex* vertices )
{
	int faceCount, i, index;
	Vertex vertex1, vertex2, vertex3;
	XMFLOAT3 tangent, binormal, normal;

	// Calculate the number of faces in the model.
	faceCount = NUM_VERTICES / 3;

	// Initialize the index to the model data.
	index = 0;

	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for ( i = 0; i < faceCount; i++ )
	{
		// Get the three vertices for this face from the model.
		vertex1.Position.x = vertices[index].Position.x;
		vertex1.Position.y = vertices[index].Position.y;
		vertex1.Position.z = vertices[index].Position.z;
		vertex1.TexCoord.x = vertices[index].TexCoord.x;
		vertex1.TexCoord.y = vertices[index].TexCoord.y;
		vertex1.Normal.x = vertices[index].Normal.x;
		vertex1.Normal.y = vertices[index].Normal.y;
		vertex1.Normal.z = vertices[index].Normal.z;
		index++;

		vertex2.Position.x = vertices[index].Position.x;
		vertex2.Position.y = vertices[index].Position.y;
		vertex2.Position.z = vertices[index].Position.z;
		vertex2.TexCoord.x = vertices[index].TexCoord.x;
		vertex2.TexCoord.y = vertices[index].TexCoord.y;
		vertex2.Normal.x = vertices[index].Normal.x;
		vertex2.Normal.y = vertices[index].Normal.y;
		vertex2.Normal.z = vertices[index].Normal.z;
		index++;

		vertex3.Position.x = vertices[index].Position.x;
		vertex3.Position.y = vertices[index].Position.y;
		vertex3.Position.z = vertices[index].Position.z;
		vertex3.TexCoord.x = vertices[index].TexCoord.x;
		vertex3.TexCoord.y = vertices[index].TexCoord.y;
		vertex3.Normal.x = vertices[index].Normal.x;
		vertex3.Normal.y = vertices[index].Normal.y;
		vertex3.Normal.z = vertices[index].Normal.z;
		index++;

		// Calculate the Tangent and binormal of that face.
		CalculateTangentBinormalLH( vertex1, vertex2, vertex3, normal, tangent, binormal );

		// Store the normal, Tangent, and binormal for this face back in the model structure.
		vertices[index - 1].Normal.x = normal.x;
		vertices[index - 1].Normal.y = normal.y;
		vertices[index - 1].Normal.z = normal.z;
		vertices[index - 1].Tangent.x = tangent.x;
		vertices[index - 1].Tangent.y = tangent.y;
		vertices[index - 1].Tangent.z = tangent.z;
		vertices[index - 1].Binormal.x = binormal.x;
		vertices[index - 1].Binormal.y = binormal.y;
		vertices[index - 1].Binormal.z = binormal.z;

		vertices[index - 2].Normal.x = normal.x;
		vertices[index - 2].Normal.y = normal.y;
		vertices[index - 2].Normal.z = normal.z;
		vertices[index - 2].Tangent.x = tangent.x;
		vertices[index - 2].Tangent.y = tangent.y;
		vertices[index - 2].Tangent.z = tangent.z;
		vertices[index - 2].Binormal.x = binormal.x;
		vertices[index - 2].Binormal.y = binormal.y;
		vertices[index - 2].Binormal.z = binormal.z;

		vertices[index - 3].Normal.x = normal.x;
		vertices[index - 3].Normal.y = normal.y;
		vertices[index - 3].Normal.z = normal.z;
		vertices[index - 3].Tangent.x = tangent.x;
		vertices[index - 3].Tangent.y = tangent.y;
		vertices[index - 3].Tangent.z = tangent.z;
		vertices[index - 3].Binormal.x = binormal.x;
		vertices[index - 3].Binormal.y = binormal.y;
		vertices[index - 3].Binormal.z = binormal.z;
	}
}