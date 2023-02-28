#include "stdafx.h"
#include "ImGuiManager.h"
#include "LightController.h"
#include "DrawableGameObject.h"
#include "RasterizerController.h"
#include "ShaderController.h"
#include "BillboradObject.h"
#include "AnimatedModel.h"
#include "TerrainVoxel.h"
#include "Terrain.h"

ImGuiManager::ImGuiManager()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    SetUbuntuTheme();
}

ImGuiManager::~ImGuiManager()
{
    ImGui::DestroyContext();
}

bool ImGuiManager::Initialize( HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    if ( !ImGui_ImplWin32_Init( hWnd ) || !ImGui_ImplDX11_Init( pDevice, pContext ) )
        return false;
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    return true;
}

void ImGuiManager::BeginRender()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport( ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode );
}

void ImGuiManager::EndRender()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
}

void ImGuiManager::CameraMenu( CameraController* cameraControl )
{
    static const char* cCurrentItem = NULL;
    static bool bLoad = false;
    static std::string sName;

    ImGui::ShowMetricsWindow();
    if ( !bLoad )
    {
        sName = cameraControl->GetCurentCam()->GetCamName();
        cCurrentItem = sName.c_str();
        bLoad = true;
    }

    if ( ImGui::Begin( "Camera Controls", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        ImGui::SetNextItemOpen( true, ImGuiCond_Once );
        if ( ImGui::TreeNode( "Instructions" ) )
        {
            ImGui::Text( "WASD      Move" );
            ImGui::Text( "R-Mouse   Look" );
            ImGui::TreePop();
        }
        ImGui::Separator();

        ImGui::SetNextItemOpen( true, ImGuiCond_Once );
        if ( ImGui::TreeNode( "Info" ) )
        {
            for ( int n = 0; n < cameraControl->GetCamList().size(); n++ )
            {
                if ( ImGui::TreeNode( cameraControl->GetCamList()[n]->GetCamName().append( "##" ).append( std::to_string( n ) ).c_str() ) )
                {
					std::string sPos = "Position : " +
                        std::to_string( cameraControl->GetCamList()[n]->GetPosition().x ) + ", " +
                        std::to_string( cameraControl->GetCamList()[n]->GetPosition().y ) + ", " +
                        std::to_string( cameraControl->GetCamList()[n]->GetPosition().z );
					ImGui::Text( sPos.c_str() );

                    std::string sRot = "Rotation : " +
                        std::to_string( cameraControl->GetCamList()[n]->GetRot().x ) + ", " +
                        std::to_string( cameraControl->GetCamList()[n]->GetRot().y ) + ", " +
                        std::to_string( cameraControl->GetCamList()[n]->GetRot().z );
                    ImGui::Text( sRot.c_str() );

                    std::string sSpeed = "Speed: " + std::to_string( cameraControl->GetCurentCam()->GetCamSpeed() );
                    ImGui::Text( sSpeed.c_str() );
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::Separator();

        ImGui::SetNextItemOpen( true, ImGuiCond_Once );
        if ( ImGui::TreeNode( "Controls" ) )
        {
            ImGui::Text( "Currrent Camera" );
            static float movementSpeed = cameraControl->GetCurentCam()->GetCamSpeed();
            if ( ImGui::BeginCombo( "##Combo", cCurrentItem ) )
            {
                for ( int n = 0; n < cameraControl->GetCamList().size(); n++ )
                {
                    bool is_selected = ( cCurrentItem == cameraControl->GetCamList()[n]->GetCamName().c_str() );
                    if ( ImGui::Selectable( cameraControl->GetCamList()[n]->GetCamName().c_str(), is_selected ) )
                    {
                        sName = cameraControl->GetCamList()[n]->GetCamName();
                        cameraControl->SetCam( n );
                        cCurrentItem = sName.c_str();
                        movementSpeed = cameraControl->GetCurentCam()->GetCamSpeed();
                    }

                    if ( is_selected )
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Text( "Movement Speed" );
            if ( ImGui::SliderFloat( "##Movement Speed", &movementSpeed, 0.1f, 1.0f, "%.1f" ) )
                cameraControl->GetCurentCam()->SetCamSpeed( movementSpeed );

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ImGuiManager::ShaderMenu( ShaderController* shaderControl, PostProcessingCB* postSettings, RasterizerController* rasterControl )
{
    static std::string sCurrentRasterState = "";
    static const char* cCurrentShader = NULL;
    static bool bLoadShader = false;
    static std::string sShaderName;

    if ( !bLoadShader )
    {
        sShaderName = shaderControl->GetShaderData().m_sName;
        cCurrentShader = sShaderName.c_str();
        bLoadShader = true;
    }

    if ( ImGui::Begin( "Shader Controls", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        ImGui::Text( "Current Shader" );
        if ( ImGui::BeginCombo( "##ShaderCombo", cCurrentShader ) )
        {
            for ( int n = 0; n < shaderControl->GetShaderList().size(); n++ )
            {
                bool is_selected = ( cCurrentShader == shaderControl->GetShaderList()[n].m_sName.c_str() );
                if ( ImGui::Selectable( shaderControl->GetShaderList()[n].m_sName.c_str(), is_selected ) )
                {
                    sShaderName = shaderControl->GetShaderList()[n].m_sName;
                    shaderControl->SetShaderData( n );
                    cCurrentShader = sShaderName.c_str();
                }

                if ( is_selected )
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text( "Current Rasterizer" );
        if ( ImGui::BeginCombo( "##RasterizerCombo", sCurrentRasterState.c_str() ) )
        {
            for ( int n = 0; n < rasterControl->GetStateNames().size(); n++ )
            {
                bool is_selected = ( sCurrentRasterState == rasterControl->GetStateNames()[n].c_str() );
                if ( ImGui::Selectable( rasterControl->GetStateNames()[n].c_str(), is_selected ) )
                {
                    rasterControl->SetState( rasterControl->GetStateNames()[n].c_str() );
                    sCurrentRasterState = rasterControl->GetStateNames()[n].c_str();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemOpen( true, ImGuiCond_Once );
        if ( ImGui::TreeNode( "Post-Processing" ) )
        {
            ImGui::Text( "Fade Amount" );
            ImGui::SliderFloat( "##Fade Amount", &postSettings->FadeAmount, 0.0f, 1.0f, "%.1f" );

            bool useBloom = postSettings->UseBloom;
            bool useBlur = postSettings->UseBlur;
            bool useColour = postSettings->UseColour;
            bool useDOF = postSettings->UseDepthOfField;

            if ( ImGui::BeginTable( "##Post-Processing Options", 2, ImGuiTableFlags_NoBordersInBody ) )
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if ( ImGui::Checkbox( "Bloom", &useBloom ) )
                    postSettings->UseBloom = useBloom;

                ImGui::TableNextColumn();
                if ( ImGui::Checkbox( "Gaussian Blur", &useBlur ) )
                    postSettings->UseBlur = useBlur;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if ( ImGui::Checkbox( "Colour Change", &useColour ) )
                    postSettings->UseColour = useColour;

                ImGui::TableNextColumn();
                if( ImGui::Checkbox( "Depth Of Field", &useDOF ) )
                    postSettings->UseDepthOfField = useDOF;

                ImGui::EndTable();
            }

            if ( useColour )
            {
                ImGui::Separator();
                ImGui::Text( "Colour Overlay" );
                float colour[] = { postSettings->Color.x , postSettings->Color.y, postSettings->Color.z, postSettings->Color.w };
                if ( ImGui::ColorEdit4( "##Colour", colour, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB ) )
                    postSettings->Color = { colour[0], colour[1], colour[2], colour[3] };
            }

            if ( useDOF )
            {
                ImGui::Separator();
                if ( ImGui::TreeNode( "DOF Settings" ) )
                {
                    ImGui::Text( "Far Plane" );
                    ImGui::InputFloat( "##Far Plane", &postSettings->FarPlane );
                    if ( postSettings->FarPlane < 0 )
                        postSettings->FarPlane = 0;

                    ImGui::Text( "Focal Width" );
                    ImGui::InputFloat( "##Focal Width", &postSettings->FocalWidth );

                    ImGui::Text( "Focal Distance" );
                    ImGui::InputFloat( "##Focal Distance", &postSettings->FocalDistance );

                    ImGui::Text( "Attenuation" );
                    ImGui::InputFloat( "##Attuenation", &postSettings->BlurAttenuation );
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ImGuiManager::ObjectMenu( std::vector<DrawableGameObject*>& gameObjects )
{
	static XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
    static XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };

    static const char* cCurrentItemO = NULL;
    static DrawableGameObject* currObject;
    static bool bLoadO = false;
    static std::string sNameO;

    if ( !bLoadO )
    {
        sNameO = gameObjects[0]->GetName();
        cCurrentItemO = sNameO.c_str();
        currObject = gameObjects[0];
        bLoadO = true;
    }

    if ( ImGui::Begin( "Object Controls", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        ImGui::Text( "Current Object" );
        if ( ImGui::BeginCombo( "##Combo", cCurrentItemO ) )
        {
            for ( int i = 0; i < gameObjects.size(); i++ )
            {
                bool is_selected = ( cCurrentItemO == gameObjects[i]->GetName().c_str() );
                if ( ImGui::Selectable( gameObjects[i]->GetName().c_str(), is_selected ) )
                {
                    sNameO = gameObjects[i]->GetName();
                    cCurrentItemO = sNameO.c_str();
                    currObject = gameObjects[i];
                }

                if ( is_selected )
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if ( ImGui::TreeNode( "Transform Controls" ) )
        {
            ImGui::Text( "Position" );
            if ( ImGui::DragFloat3( "##Position", &position.x, 1.0f, -10.0f, 10.0f ) )
                currObject->GetTransfrom()->SetPosition( position );

            ImGui::Text( "Rotation" );
            if ( ImGui::DragFloat3( "##Rotation", &rotation.x, 1.0f, 0.0f, 360.0f ) )
                currObject->GetTransfrom()->SetRotation( rotation );

            if ( ImGui::Button( "Reset" ) )
            {
                rotation = { 0.0f, 0.0f, 0.0f };
                position = { 0.0f, 0.0f, 0.0f };
            }

            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen( true, ImGuiCond_Once );
        if ( ImGui::TreeNode( "Texture Controls" ) )
        {
            MaterialPropertiesCB materialData = currObject->GetAppearance()->GetMaterialData();

            bool useTexture = materialData.Material.UseTexture;
            if ( ImGui::Checkbox( "Use Texture?", &useTexture ) )
                materialData.Material.UseTexture = useTexture;

            if ( useTexture )
            {
                if ( ImGui::TreeNode( "Colour Controls" ) )
                {
                    ImGui::Text( "Ambient" );
                    ImGui::ColorEdit3( "##Ambient", &materialData.Material.Ambient.x );

                    ImGui::Text( "Diffuse" );
                    ImGui::ColorEdit3( "##Diffuse", &materialData.Material.Diffuse.x );

                    ImGui::Text( "Emissive" );
                    ImGui::ColorEdit3( "##Emissive", &materialData.Material.Emissive.x );

                    ImGui::Text( "Specular" );
                    ImGui::ColorEdit3( "##Specular", &materialData.Material.Specular.x );

                    ImGui::Text( "Power" );
                    ImGui::DragFloat( "##Power", &materialData.Material.SpecularPower, 1.0f, 1.0f, 32.0f );
                    ImGui::TreePop();
                }

                if ( ImGui::TreeNode( "Parallax Controls" ) )
                {
                    ImGui::Text( "Height Scale" );
                    ImGui::DragFloat( "##Height Scale", &materialData.Material.HeightScale, 1.0f, 0.0f, 100.0f, "%1.f" );

                    ImGui::Text( "Min Layers" );
                    ImGui::DragFloat( "##Min Layers", &materialData.Material.MinLayers, 1.0f, 1.0f, 30.0f );

                    ImGui::Text( "Max Layers" );
                    ImGui::DragFloat( "##Max Layers", &materialData.Material.MaxLayers, 1.0f, 1.0f, 30.0f );
                    ImGui::TreePop();
                }
            }

            currObject->GetAppearance()->SetMaterialData( materialData );
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ImGuiManager::LightMenu( LightController* lightControl )
{
    static const char* cCurrentItemL = NULL;
    static Light currLightData;
    static bool bLoadL = false;
    static std::string nameL;

    if ( !bLoadL )
    {
        nameL = lightControl->GetLight( 0 )->GetName();
        currLightData = lightControl->GetLightList()[0]->GetLightData();
        cCurrentItemL = nameL.c_str();
        bLoadL = true;
    }

    if ( ImGui::Begin( "Light Controls", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        ImGui::Text( "Current Light" );
        if ( ImGui::BeginCombo( "##Combo", cCurrentItemL ) )
        {
            for ( int n = 0; n < lightControl->GetLightList().size(); n++ )
            {
                bool is_selected = ( cCurrentItemL == lightControl->GetLightList()[n]->GetName().c_str() );
                if ( ImGui::Selectable( lightControl->GetLightList()[n]->GetName().c_str(), is_selected ) )
                {
                    nameL = lightControl->GetLightList()[n]->GetName().c_str();
                    currLightData = lightControl->GetLightList()[n]->GetLightData();
                    cCurrentItemL = nameL.c_str();
                    currLightData = lightControl->GetLight( nameL )->GetLightData();
                }

                if ( is_selected )
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text( "Position" );
        ImGui::DragFloat3( "##Position", &currLightData.Position.x, 1.0f, -10.0f, 10.0f );

        ImGui::Text( "Colour" );
        float colour[] = { currLightData.Color.x, currLightData.Color.y, currLightData.Color.z, currLightData.Color.w };
        if ( ImGui::ColorEdit4( "##Colour", colour, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB ) )
            currLightData.Color = { colour[0], colour[1], colour[2], colour[3] };

        bool enable = currLightData.Enabled;
        if ( ImGui::Checkbox( "Enabled?", &enable ) )
            currLightData.Enabled = enable;

        if ( ImGui::TreeNode( "Shadow Direction" ) )
        {
            XMFLOAT3 lightDirection = lightControl->GetLight( nameL )->GetCamera()->GetRot();

            ImGui::Text( "Pitch" );
            ImGui::SliderAngle( "##Pitch", &lightDirection.x, 0.995f * -90.0f, 0.995f * 90.0f );

            ImGui::Text( "Yaw" );
            ImGui::SliderAngle( "##Yaw", &lightDirection.y, -180.0f, 180.0f );

            lightControl->GetLight( nameL )->GetCamera()->SetRot( lightDirection );
            ImGui::TreePop();
        }

        if ( ImGui::TreeNode( ( currLightData.LightType == LightType::DirectionalLight ) ? "Direction" : "Attenuation" ) )
        {
            switch ( currLightData.LightType )
            {
            case LightType::PointLight:
                ImGui::Text( "Constant" );
                ImGui::DragFloat( "##Constant", &currLightData.ConstantAttenuation, 0.1f, 1.0f, 10.0f, "%.2f" );

                ImGui::Text( "Linear" );
                ImGui::DragFloat( "##Linear", &currLightData.LinearAttenuation, 0.1f, 0.0f, 5.0f, "%.4f" );

                ImGui::Text( "Quadratic" );
                ImGui::DragFloat( "##Quadratic", &currLightData.QuadraticAttenuation, 0.1f, 0.0f, 2.0f, "%.7f" );
                break;

            case LightType::SpotLight:
            {
                ImGui::Text( "Constant" );
                ImGui::DragFloat( "##Constant", &currLightData.ConstantAttenuation, 0.1f, 1.0f, 10.0f, "%.2f" );

                ImGui::Text( "Linear" );
                ImGui::DragFloat( "##Linear", &currLightData.LinearAttenuation, 0.01f, 0.0f, 5.0f, "%.4f" );

                ImGui::Text( "Quadratic" );
                ImGui::DragFloat( "##Quadratic", &currLightData.QuadraticAttenuation, 0.001f, 0.0f, 2.0f, "%.7f" );

                ImGui::Text( "Spot Angle" );
                float SpotAngle = XMConvertToDegrees( currLightData.SpotAngle );
                ImGui::InputFloat( "##Spot Angle", &SpotAngle );
                currLightData.SpotAngle = XMConvertToRadians( SpotAngle );
            }
            break;

            case LightType::DirectionalLight:
            {
                ImGui::DragFloat3( "##Direction", &currLightData.Direction.x );
            }
            break;
            }
            ImGui::TreePop();
        }

        lightControl->GetLight( nameL )->SetLightData( currLightData );
    }
    ImGui::End();
}

void ImGuiManager::BillboardMenu( BillboardObject* billboardObject )
{
    static int iPicked = 0;
    static std::string nameBB;
    static const char* cCurrentItemBB = NULL;
    static std::vector<SimpleVertexBillboard> vBbVerts;
    vBbVerts = billboardObject->GetPosistions();

    if ( ImGui::Begin( "Billboard Control", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        if ( ImGui::BeginCombo( "##Combo", cCurrentItemBB ) )
        {
            for ( int n = 0; n < billboardObject->GetPosistions().size(); n++ )
            {
                std::string name = "Billboard ";
                name += std::to_string( n );
                bool is_selected = ( cCurrentItemBB == name.c_str() );
                if ( ImGui::Selectable( name.c_str(), is_selected ) )
                {
                    iPicked = n;
                    nameBB = name;
                    cCurrentItemBB = nameBB.c_str();
                    vBbVerts = billboardObject->GetPosistions();
                }

                if ( is_selected )
                {
                    ImGui::SetItemDefaultFocus();
                    vBbVerts = billboardObject->GetPosistions();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text( "Position" );
        ImGui::InputFloat( "X", &vBbVerts[iPicked].Pos.x );
        ImGui::InputFloat( "Y", &vBbVerts[iPicked].Pos.y );
        ImGui::InputFloat( "Z", &vBbVerts[iPicked].Pos.z );
        billboardObject->SetPositions( vBbVerts );
    }
    ImGui::End();
}

void ImGuiManager::BezierSplineMenu()
{
    if ( ImGui::Begin( "Bezier Curve Spline", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        float lines[120];
        for ( int n = 0; n < m_vPoints.size(); n++ )
            lines[n] = m_vPoints[n].y;
        ImGui::PlotLines( "G", lines, m_vPoints.size() );
    }
    ImGui::End();
}

void ImGuiManager::TerrainMenu( Terrain* terrain, TerrainVoxel* voxelTerrain, ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    if ( ImGui::Begin( "Terrain Control", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        if ( ImGui::CollapsingHeader( "Terrain" ) )
        {
            ImGui::Checkbox( "Draw Terrain", terrain->GetIsDraw() );
            ImGui::Text( "Terrain Data" );
            ImGui::Text( "Width = %i", terrain->GetHeightMapWidth() );
            ImGui::Text( "Height = %i", terrain->GetHeightMapHeight() );
            ImGui::Text( "Cell Spacing = %f", terrain->GetCellSpacing() );

            switch ( terrain->GetGenType() )
            {
            case TerrainGenType::HeightMapLoad:
                ImGui::Text( "GenType: HeightMap" );
                ImGui::Text( terrain->GetHeightMapName().c_str() );
                ImGui::Text( "Height Scale = %i", terrain->GetHeightScale() );
                break;

            case TerrainGenType::FaultLine:
                ImGui::Text( "GenType: FaultLine" );
                ImGui::Text( "Seed:  %i", terrain->GetSeed() );
                ImGui::Text( "Displacment:  %f", terrain->GetDisplacement() );
                ImGui::Text( "Number Of iterations:  %i", terrain->GetNumOfIterations() );
                break;

            case TerrainGenType::Noise:
                ImGui::Text( "GenType: Noise" );
                ImGui::Text( "Height Scale = %i", terrain->GetHeightScale() );
                ImGui::Text( "Seed:  %i", terrain->GetSeed() );
                ImGui::Text( "Frequency:  %f", terrain->GetFrequency() );
                ImGui::Text( "Number of Octaves:  %i", terrain->GetNumOfOctaves() );
                break;

            case TerrainGenType::DiamondSquare:
                ImGui::Text( "GenType: Diamond Square" );
                ImGui::Text( "Height Scale = %i", terrain->GetHeightScale() );
                ImGui::Text( "Seed:  %i", terrain->GetSeed() );
                ImGui::Text( "Range:  %i", terrain->GetRange() );
                break;
            }

            if ( ImGui::CollapsingHeader( "Build Data" ) )
            {
                static TerrainGenType mode = (TerrainGenType)0;
                const char* items[] = { "HeightMapLoad", "FaultLine", "Noise", "DiamondSquare" };
                static const char* current_item = "HeightMapLoad";

                if ( ImGui::BeginCombo( "##Combo", current_item ) )
                {
                    for ( int n = 0; n < IM_ARRAYSIZE( items ); n++ )
                    {
                        bool is_selected = ( current_item == items[n] );
                        if ( ImGui::Selectable( items[n], is_selected ) )
                        {
                            current_item = items[n];
                            mode = (TerrainGenType)n;
                            if ( is_selected )
                                ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                static int width = 514;
                ImGui::InputInt( "Width", &width );
                static int Length = 514;
                ImGui::InputInt( "Length", &Length );

                static float cellSpacing = 1.0f;
                ImGui::InputFloat( "CellSpacing", &cellSpacing );

                static float heightScale = 50.0f;
                static float frequency = 0.01f;
                static float displacement = 0.01f;
                static int seed = 1234;
                static int range = 196;
                static int numberOfOctaves = 3;

                switch ( mode )
                {
                case TerrainGenType::HeightMapLoad:
                    ImGui::InputFloat( "HeightScale", &heightScale );
                    break;

                case TerrainGenType::FaultLine:
                    ImGui::InputInt( "Seed", &seed );
                    ImGui::InputInt( "NumberOfIteration", &range );
                    ImGui::InputFloat( "Displacement", &displacement );
                    break;

                case TerrainGenType::Noise:
                    ImGui::InputInt( "Seed", &seed );
                    ImGui::InputFloat( "Frequency", &frequency );
                    ImGui::InputInt( "NumberOfOctaves", &numberOfOctaves );
                    ImGui::InputFloat( "HeightScale", &heightScale );
                    break;

                case TerrainGenType::DiamondSquare:
                    ImGui::InputInt( "Seed", &seed );
                    ImGui::InputInt( "Range", &range );
                    ImGui::InputFloat( "HeightScale", &heightScale );
                    break;
                }

                if ( ImGui::Button( "Build Terrain" ) )
                {
                    switch ( mode )
                    {
                    case TerrainGenType::HeightMapLoad:
                        break;

                    case TerrainGenType::FaultLine:
                        terrain->SetFaultLineData( seed, range, displacement );
                        break;

                    case TerrainGenType::Noise:
                        terrain->SetNoiseData( seed, frequency, numberOfOctaves );
                        break;

                    case TerrainGenType::DiamondSquare:
                        terrain->SetDiamondSquareData( seed, range );
                        break;
                    }

                    terrain->ReBuildTerrain( XMFLOAT2( width, Length ), heightScale, cellSpacing, mode, pDevice );
                }
            }

            if ( ImGui::CollapsingHeader( "Texture Data" ) )
            {
                if ( ImGui::BeginTable( "Texture Name", 3 ) )
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text( "Name" );

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text( "Water" );

                    for ( size_t i = 0; i < terrain->GetTexNames().size(); i++ )
                    {
                        std::string data = terrain->GetTexNames()[i];
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text( data.c_str() );
                    }
                    ImGui::EndTable();
                }

                ImGui::Text( "Texture Height Level" );
                float Layer1MaxHeight = terrain->GetTerrainData().Layer1MaxHeight;
                float Layer2MaxHeight = terrain->GetTerrainData().Layer2MaxHeight;
                float Layer3MaxHeight = terrain->GetTerrainData().Layer3MaxHeight;
                float Layer4MaxHeight = terrain->GetTerrainData().Layer4MaxHeight;
                float Layer5MaxHeight = terrain->GetTerrainData().Layer5MaxHeight;

                ImGui::SliderFloat( "Layer1MaxHeight", &Layer1MaxHeight, 0.0f, Layer2MaxHeight );
                ImGui::SliderFloat( "Layer2MaxHeight", &Layer2MaxHeight, Layer1MaxHeight, Layer3MaxHeight );
                ImGui::SliderFloat( "Layer3MaxHeight", &Layer3MaxHeight, Layer2MaxHeight, Layer4MaxHeight );
                ImGui::SliderFloat( "Layer4MaxHeight", &Layer4MaxHeight, Layer3MaxHeight, Layer5MaxHeight );
                ImGui::SliderFloat( "Layer5MaxHeight", &Layer5MaxHeight, Layer4MaxHeight, 100000.0f );
                terrain->SetTexHeights( Layer1MaxHeight, Layer2MaxHeight, Layer3MaxHeight, Layer4MaxHeight, Layer5MaxHeight );
            }

            if ( ImGui::CollapsingHeader( "LOD Data" ) )
            {
                float floatMinTess = terrain->GetTerrainData().MinTess;
                float floatMaxTess = terrain->GetTerrainData().MaxTess;
                ImGui::SliderFloat( "Min Tess", &floatMinTess, 0.0f, 6.0f );
                ImGui::SliderFloat( "Max Tess", &floatMaxTess, 0.0f, 6.0f );
                if ( floatMaxTess < floatMaxTess )
                {
                    floatMaxTess = floatMinTess;
                }
                terrain->SetMaxTess( floatMaxTess );
                terrain->SetMinTess( floatMinTess );

                float maxTessDist = terrain->GetTerrainData().MaxDist;
                float minTessDist = terrain->GetTerrainData().MinDist;
                ImGui::SliderFloat( "Min Tess Dist", &minTessDist, 1.0f, 1000.0f );
                ImGui::SliderFloat( "Max Tess Dist", &maxTessDist, 1.0f, 1000.0f );
                if ( maxTessDist < minTessDist )
                {
                    maxTessDist = minTessDist;
                }
                terrain->SetMaxTessDist( maxTessDist );
                terrain->SetMinTessDist( minTessDist );
            }

            if ( ImGui::CollapsingHeader( "Transfrom Data" ) )
            {
                ImGui::Text( "Rotation" );
                XMFLOAT3 RotationTerrain = terrain->GetTransfrom()->GetRotation();
                ImGui::SliderFloat( "X##Rotation", &RotationTerrain.x, 0, 360 );
                ImGui::SliderFloat( "Y##Rotation", &RotationTerrain.y, 0, 360 );
                ImGui::SliderFloat( "Z##Rotation", &RotationTerrain.z, 0, 360 );
                terrain->GetTransfrom()->SetRotation( RotationTerrain );

                ImGui::Text( "Position" );
                XMFLOAT3 posTerrain = terrain->GetTransfrom()->GetPosition();
                ImGui::InputFloat( "X##Position", &posTerrain.x );
                ImGui::InputFloat( "Y##Position", &posTerrain.y );
                ImGui::InputFloat( "Z##Position", &posTerrain.z );
                terrain->GetTransfrom()->SetPosition( posTerrain );

                ImGui::Text( "Scale" );
                XMFLOAT3 scaleTerrain = terrain->GetTransfrom()->GetScale();
                ImGui::InputFloat( "X##Scale", &scaleTerrain.x );
                ImGui::InputFloat( "Y##Scale", &scaleTerrain.y );
                ImGui::InputFloat( "Z##Scale", &scaleTerrain.z );
                terrain->GetTransfrom()->SetScale( scaleTerrain );
            }

            static char buf[32] = "";
            static bool modifiedName = false;
            ImGui::InputText( "File Name", buf, IM_ARRAYSIZE( buf ) );
            if ( ImGui::Button( "Load" ) )
            {
                TerrainData Data;
                TerrainJsonLoad::LoadData( buf, Data );
                TerrainGenType GenMode = (TerrainGenType)Data.Mode;
                double HeightScale = 0;

                switch ( GenMode )
                {
                case TerrainGenType::HeightMapLoad:
                    HeightScale = Data.HeightMapSettings.HeightScale;
                    break;

                case TerrainGenType::FaultLine:
                    terrain->SetFaultLineData( Data.FaultLineSettings.Seed, Data.FaultLineSettings.IterationCount, Data.FaultLineSettings.Displacement );
                    HeightScale = terrain->GetHeightScale();
                    break;

                case TerrainGenType::Noise:
                    terrain->SetNoiseData( Data.NoiseSettings.Seed, Data.NoiseSettings.Frequency, Data.NoiseSettings.NumOfOctaves );
                    HeightScale = Data.NoiseSettings.HeightScale;
                    break;

                case TerrainGenType::DiamondSquare:
                    terrain->SetDiamondSquareData( Data.DiamondSquareSettings.Seed, Data.DiamondSquareSettings.Range );
                    HeightScale = Data.DiamondSquareSettings.HeightScale;
                    break;
                }

                terrain->ReBuildTerrain( XMFLOAT2( Data.Width, Data.Depth ), HeightScale, Data.CellSpacing, GenMode, pDevice );
            }

            ImGui::SameLine();
            if ( ImGui::Button( "Save" ) )
            {
                TerrainData Data;
                Data.Width = terrain->GetHeightMapWidth();
                Data.Depth = terrain->GetHeightMapHeight();
                Data.CellSpacing = terrain->GetCellSpacing();
                Data.Mode = (int)terrain->GetGenType();

                switch ( terrain->GetGenType() )
                {
                case TerrainGenType::HeightMapLoad:
                    Data.HeightMapSettings.HeightMapFile = terrain->GetHeightMapName();
                    Data.HeightMapSettings.HeightScale = terrain->GetHeightScale();
                    break;

                case TerrainGenType::FaultLine:
                    Data.FaultLineSettings.Seed = terrain->GetSeed();
                    Data.FaultLineSettings.IterationCount = terrain->GetNumOfIterations();
                    Data.FaultLineSettings.Displacement = terrain->GetDisplacement();
                    break;

                case TerrainGenType::Noise:
                    Data.NoiseSettings.Seed = terrain->GetSeed();
                    Data.NoiseSettings.HeightScale = terrain->GetHeightScale();
                    Data.NoiseSettings.Frequency = terrain->GetFrequency();
                    Data.NoiseSettings.NumOfOctaves = terrain->GetNumOfOctaves();
                    break;

                case TerrainGenType::DiamondSquare:
                    Data.DiamondSquareSettings.Seed = terrain->GetSeed();
                    Data.DiamondSquareSettings.HeightScale = terrain->GetHeightScale();
                    Data.DiamondSquareSettings.Range = terrain->GetRange();
                    break;
                }

                TerrainJsonLoad::StoreData( buf, Data );
            }
        }

        if ( ImGui::CollapsingHeader( "Voxel Terrain" ) )
        {
            ImGui::Checkbox( "Draw Voxel", voxelTerrain->GetIsDraw() );
            ImGui::Text( "Number Of Chunks: %i", voxelTerrain->GetNumberOfChunks() );

            if ( ImGui::CollapsingHeader( "Rebuild World" ) )
            {
                static int seed;
                ImGui::InputInt( "Seed Vox", &seed );
                static float Frequancy;
                ImGui::InputFloat( "Frequency Vox", &Frequancy );
                static int Octave;
                ImGui::InputInt( "Octave Vox", &Octave );
                static int NumberOfChuncksX = 0;
                ImGui::InputInt( "Number of Chunk X", &NumberOfChuncksX );
                static int NumberOfChuncksZ = 0;
                ImGui::InputInt( "Number of Chunk Z", &NumberOfChuncksZ );
                if ( ImGui::Button( "Build" ) )
                {
                    voxelTerrain->RebuildMap( pDevice, pContext, seed, NumberOfChuncksX, NumberOfChuncksZ, Frequancy, Octave );
                }
            }
        }
    }
    ImGui::End();
}

void ImGuiManager::AnimationMenu( AnimatedModel* animModel )
{
    if ( ImGui::Begin( "Animation", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        if ( ImGui::CollapsingHeader( "Model Data" ) )
        {
            std::string modelName = "Model Name: ";
            modelName += animModel->GetModelName().c_str();
            ImGui::Text( modelName.c_str() );
            if ( ImGui::BeginTable( "SubSetData", 5 ) )
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "ID" );
                ImGui::TableNextColumn();
                ImGui::Text( "Face Count" );
                ImGui::TableNextColumn();
                ImGui::Text( "Vertex Count" );
                ImGui::TableNextColumn();
                ImGui::Text( "Diffuse Tex" );
                ImGui::TableNextColumn();
                ImGui::Text( "Normal Map" );
                for ( size_t i = 0; i < animModel->GetSubsets().size(); i++ )
                {
                    Subset data = animModel->GetSubsets()[i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text( "%i", data.m_uId );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%i", data.m_uFaceCount );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%i", data.m_uVertexCount );
                    ImGui::TableNextColumn();
                    std::wstring diffuseMap = animModel->GetMaterialData()[i].DiffuseMapName.c_str();
                    ImGui::Text( StringHelpers::ws2s( diffuseMap ).c_str() );
                    ImGui::TableNextColumn();
                    std::wstring normalMap = animModel->GetMaterialData()[i].NormalMapName.c_str();
                    ImGui::Text( StringHelpers::ws2s( normalMap ).c_str() );
                }
                ImGui::EndTable();
            }

            ImGui::Text( "Model Place" );
            static float modelPos[3] = {
                animModel->GetTransformData()->GetPosition().x,
                animModel->GetTransformData()->GetPosition().y,
                animModel->GetTransformData()->GetPosition().z
            };
            ImGui::InputFloat3( "Model Position", modelPos );
            animModel->GetTransformData()->SetPosition( XMFLOAT3( modelPos ) );

            static float modelScale[3] = {
                animModel->GetTransformData()->GetScale().x,
                animModel->GetTransformData()->GetScale().y,
                animModel->GetTransformData()->GetScale().z
            };
            ImGui::InputFloat3( "Model Scale", modelScale );
            animModel->GetTransformData()->SetScale( XMFLOAT3( modelScale ) );

            static float modelRotation[3] = {
                animModel->GetTransformData()->GetRotation().x,
                animModel->GetTransformData()->GetRotation().y,
                animModel->GetTransformData()->GetRotation().z
            };
            ImGui::InputFloat3( "Model Rotation", modelRotation );
            animModel->GetTransformData()->SetRotation( XMFLOAT3( modelRotation ) );
        }

        if ( ImGui::CollapsingHeader( "Bone Data" ) )
        {
            if ( ImGui::BeginChild( "Bone List", ImVec2( 500, 100 ), true, ImGuiWindowFlags_HorizontalScrollbar ) )
            {
                ImGui::BeginTable( "Bone Info", 3 );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "ID" );
                ImGui::TableNextColumn();
                ImGui::Text( "Parent" );
                ImGui::TableNextColumn();
                ImGui::Text( "Child" );

                for ( size_t i = 0; i < animModel->GetSkeleton()->GetBoneData().size(); i++ )
                {
                    Bone* data = animModel->GetSkeleton()->GetBoneData()[i];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text( "%i", i );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%i", data->Getparent() );
                    ImGui::TableNextColumn();
                    std::string childData = "";
                    for ( size_t i = 0; i < data->GetChild().size(); i++ )
                    {
                        if ( data->GetChild()[i] == -1 )
                        {
                            childData = "No Child";
                        }
                        else if ( i == data->GetChild().size() - 1 )
                        {
                            childData += std::to_string( data->GetChild()[i] );
                        }
                        else
                        {
                            childData += std::to_string( data->GetChild()[i] ) + ", ";
                        }
                    }
                    ImGui::Text( childData.c_str() );

                }
                ImGui::EndTable();
            }
            ImGui::EndChild();

            ImGui::Text( "Edit Bone" );
            static int boneNum = 0;
            static int boneNumPrev = -1;
            static float bonePos[3];
            static float boneScale[3];
            ImGui::InputInt( "Bone No:", &boneNum );
            if ( boneNum > animModel->GetSkeleton()->BoneCount() )
            {
                boneNum = animModel->GetSkeleton()->BoneCount() - 1;
            }
            else if ( boneNum < 0 )
            {
                boneNum = 0;
            }

            if ( boneNum != boneNumPrev )
            {
                bonePos[0] = animModel->GetSkeleton()->GetBoneData()[boneNum]->GetWorldPos().x;
                bonePos[1] = animModel->GetSkeleton()->GetBoneData()[boneNum]->GetWorldPos().y;
                bonePos[2] = animModel->GetSkeleton()->GetBoneData()[boneNum]->GetWorldPos().z;
                boneScale[0] = animModel->GetSkeleton()->GetBoneData()[boneNum]->GetWorldScale().x;
                boneScale[1] = animModel->GetSkeleton()->GetBoneData()[boneNum]->GetWorldScale().y;
                boneScale[2] = animModel->GetSkeleton()->GetBoneData()[boneNum]->GetWorldScale().z;
                boneNumPrev = boneNum;
            }

            ImGui::InputFloat3( "Bone Position", bonePos );
            ImGui::InputFloat3( "Bone Scale", boneScale );
            static float boneRotation[3];
            ImGui::InputFloat3( "Bone Rotation", boneRotation );

            if ( ImGui::Button( "SetBone" ) )
            {
                animModel->GetSkeleton()->SetBonePosition( boneNum, XMFLOAT3( bonePos ) );
                XMMATRIX rotation = XMMatrixRotationX(
                    XMConvertToRadians( boneRotation[0] ) ) *
                    XMMatrixRotationY( XMConvertToRadians( boneRotation[1] ) ) *
                    XMMatrixRotationZ( XMConvertToRadians( boneRotation[2] ) );
                XMFLOAT4 orientation;
                XMStoreFloat4( &orientation, XMQuaternionRotationMatrix( rotation ) );
                animModel->GetSkeleton()->SetBoneRotQuat( boneNum, XMFLOAT4( orientation ) );
                animModel->GetSkeleton()->SetBoneScale( boneNum, XMFLOAT3( boneScale ) );
            }
        }

        if ( ImGui::CollapsingHeader( "Animation Data" ) )
        {
            // Set animation
            static std::vector<std::string> animationName = animModel->GetSkeleton()->AnimationClips();
            static const char* cCurrentItem = animationName[0].c_str();

            if ( ImGui::BeginCombo( "##Combo", cCurrentItem ) )
            {
                for ( int n = 0; n < animationName.size(); n++ )
                {
                    bool is_selected = ( cCurrentItem == animationName[n].c_str() );
                    if ( ImGui::Selectable( animationName[n].c_str(), is_selected ) )
                    {
                        cCurrentItem = animationName[n].c_str();
                        if ( is_selected )
                        {
                            ImGui::SetItemDefaultFocus();
                            animModel->SetAnimation( animationName[n] );
                        }
                    }
                }
                ImGui::EndCombo();
            }

            animModel->SetAnimation( cCurrentItem );
            static bool isLoop = animModel->GetIsLoop();
            ImGui::Checkbox( "Loop Animation", &isLoop );
            animModel->SetIsLoop( isLoop );
            float TimePos = animModel->GetTimePos();

            if ( !isLoop )
            {
                ImGui::InputFloat( "Animation Time Position", &TimePos );
                animModel->SetTimePos( TimePos );
            }

            // Current animation data
            ImGui::Text( "Current Animation: %s", animModel->GetClipName().c_str() );
            ImGui::Text( "Current Animation Start: %f", animModel->GetSkeleton()->GetClipStartTime( animModel->GetClipName() ) );
            ImGui::Text( "Current Animation End: %f", animModel->GetSkeleton()->GetClipEndTime( animModel->GetClipName() ) );
            ImGui::Text( "Current Time: %f", animModel->GetTimePos() );
        }
    }
    ImGui::End();
}

void ImGuiManager::SetBlackGoldTheme()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] = ImVec4( 0.92f, 0.92f, 0.92f, 1.00f );
    colors[ImGuiCol_TextDisabled] = ImVec4( 0.44f, 0.44f, 0.44f, 1.00f );
    colors[ImGuiCol_WindowBg] = ImVec4( 0.06f, 0.06f, 0.06f, 1.00f );
    colors[ImGuiCol_ChildBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
    colors[ImGuiCol_PopupBg] = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
    colors[ImGuiCol_Border] = ImVec4( 0.51f, 0.36f, 0.15f, 1.00f );
    colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
    colors[ImGuiCol_FrameBg] = ImVec4( 0.11f, 0.11f, 0.11f, 1.00f );
    colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.51f, 0.36f, 0.15f, 1.00f );
    colors[ImGuiCol_FrameBgActive] = ImVec4( 0.78f, 0.55f, 0.21f, 1.00f );
    colors[ImGuiCol_TitleBg] = ImVec4( 0.51f, 0.36f, 0.15f, 1.00f );
    colors[ImGuiCol_TitleBgActive] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
    colors[ImGuiCol_MenuBarBg] = ImVec4( 0.11f, 0.11f, 0.11f, 1.00f );
    colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.06f, 0.06f, 0.06f, 0.53f );
    colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.21f, 0.21f, 0.21f, 1.00f );
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.47f, 0.47f, 0.47f, 1.00f );
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.81f, 0.83f, 0.81f, 1.00f );
    colors[ImGuiCol_CheckMark] = ImVec4( 0.78f, 0.55f, 0.21f, 1.00f );
    colors[ImGuiCol_SliderGrab] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_Button] = ImVec4( 0.51f, 0.36f, 0.15f, 1.00f );
    colors[ImGuiCol_ButtonHovered] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_ButtonActive] = ImVec4( 0.78f, 0.55f, 0.21f, 1.00f );
    colors[ImGuiCol_Header] = ImVec4( 0.51f, 0.36f, 0.15f, 1.00f );
    colors[ImGuiCol_HeaderHovered] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_HeaderActive] = ImVec4( 0.93f, 0.65f, 0.14f, 1.00f );
    colors[ImGuiCol_Separator] = ImVec4( 0.21f, 0.21f, 0.21f, 1.00f );
    colors[ImGuiCol_SeparatorHovered] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_SeparatorActive] = ImVec4( 0.78f, 0.55f, 0.21f, 1.00f );
    colors[ImGuiCol_ResizeGrip] = ImVec4( 0.21f, 0.21f, 0.21f, 1.00f );
    colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.78f, 0.55f, 0.21f, 1.00f );
    colors[ImGuiCol_Tab] = ImVec4( 0.51f, 0.36f, 0.15f, 1.00f );
    colors[ImGuiCol_TabHovered] = ImVec4( 0.91f, 0.64f, 0.13f, 1.00f );
    colors[ImGuiCol_TabActive] = ImVec4( 0.78f, 0.55f, 0.21f, 1.00f );
    colors[ImGuiCol_TabUnfocused] = ImVec4( 0.07f, 0.10f, 0.15f, 0.97f );
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.14f, 0.26f, 0.42f, 1.00f );
    colors[ImGuiCol_PlotLines] = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
    colors[ImGuiCol_PlotLinesHovered] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
    colors[ImGuiCol_PlotHistogram] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
    colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
    colors[ImGuiCol_DragDropTarget] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
    colors[ImGuiCol_NavHighlight] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );

    style->FramePadding = ImVec2( 4, 2 );
    style->ItemSpacing = ImVec2( 10, 2 );
    style->IndentSpacing = 12;
    style->ScrollbarSize = 10;
    style->WindowRounding = 4;
    style->FrameRounding = 4;
    style->PopupRounding = 4;
    style->ScrollbarRounding = 6;
    style->GrabRounding = 4;
    style->TabRounding = 4;
    style->WindowTitleAlign = ImVec2( 1.0f, 0.5f );
    style->WindowMenuButtonPosition = ImGuiDir_Right;
    style->DisplaySafeAreaPadding = ImVec2( 4, 4 );
}

void ImGuiManager::SetWhiteTheme()
{
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Light.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Regular.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Bold.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Italic.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-BoldItalic.ttf", 16 );
    io.Fonts->Build();

    ImGuiStyle* style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2( 15, 15 );
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2( 5, 5 );
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2( 12, 8 );
    style->ItemInnerSpacing = ImVec2( 8, 6 );
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->Colors[ImGuiCol_Text] = ImVec4( 0.40f, 0.39f, 0.38f, 1.00f );
    style->Colors[ImGuiCol_TextDisabled] = ImVec4( 0.40f, 0.39f, 0.38f, 0.77f );
    style->Colors[ImGuiCol_WindowBg] = ImVec4( 0.92f, 0.91f, 0.88f, 0.70f );
    style->Colors[ImGuiCol_PopupBg] = ImVec4( 0.92f, 0.91f, 0.88f, 0.92f );
    style->Colors[ImGuiCol_Border] = ImVec4( 0.84f, 0.83f, 0.80f, 0.65f );
    style->Colors[ImGuiCol_BorderShadow] = ImVec4( 0.92f, 0.91f, 0.88f, 0.00f );
    style->Colors[ImGuiCol_FrameBg] = ImVec4( 1.00f, 0.98f, 0.95f, 1.00f );
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.99f, 1.00f, 0.40f, 0.78f );
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4( 0.26f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_TitleBg] = ImVec4( 1.00f, 0.98f, 0.95f, 1.00f );
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 1.00f, 0.98f, 0.95f, 0.75f );
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4( 1.00f, 0.98f, 0.95f, 0.47f );
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4( 1.00f, 0.98f, 0.95f, 1.00f );
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.00f, 0.00f, 0.00f, 0.21f );
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.90f, 0.91f, 0.00f, 0.78f );
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_CheckMark] = ImVec4( 0.25f, 1.00f, 0.00f, 0.80f );
    style->Colors[ImGuiCol_SliderGrab] = ImVec4( 0.00f, 0.00f, 0.00f, 0.14f );
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_Button] = ImVec4( 0.00f, 0.00f, 0.00f, 0.14f );
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4( 0.99f, 1.00f, 0.22f, 0.86f );
    style->Colors[ImGuiCol_ButtonActive] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_Header] = ImVec4( 0.25f, 1.00f, 0.00f, 0.76f );
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4( 0.25f, 1.00f, 0.00f, 0.86f );
    style->Colors[ImGuiCol_HeaderActive] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4( 0.00f, 0.00f, 0.00f, 0.04f );
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.25f, 1.00f, 0.00f, 0.78f );
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_PlotLines] = ImVec4( 0.40f, 0.39f, 0.38f, 0.63f );
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4( 0.40f, 0.39f, 0.38f, 0.63f );
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 0.25f, 1.00f, 0.00f, 1.00f );
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.25f, 1.00f, 0.00f, 0.43f );
}

void ImGuiManager::SetUbuntuTheme()
{
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Light.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Regular.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Bold.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-Italic.ttf", 16 );
    io.Fonts->AddFontFromFileTTF( "Resources/Fonts/OpenSans-BoldItalic.ttf", 16 );
    io.Fonts->Build();

    // Borders
    ImGui::GetStyle().WindowBorderSize = 0.0f;
    ImGui::GetStyle().FrameBorderSize = 0.0f;
    ImGui::GetStyle().PopupBorderSize = 0.0f;

    // Rounding
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().LogSliderDeadzone = 0.0f;
    ImGui::GetStyle().TabRounding = 0.0f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
    colors[ImGuiCol_TextDisabled] = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f );
    colors[ImGuiCol_WindowBg] = ImVec4( 0.07f, 0.07f, 0.07f, 1.00f );
    colors[ImGuiCol_ChildBg] = ImVec4( 0.07f, 0.07f, 0.07f, 1.00f );
    colors[ImGuiCol_PopupBg] = ImVec4( 0.07f, 0.07f, 0.07f, 1.00f );
    colors[ImGuiCol_Border] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
    colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
    colors[ImGuiCol_FrameBg] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_FrameBgActive] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_TitleBg] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_TitleBgActive] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 0.00f, 0.00f, 0.00f, 0.51f );
    colors[ImGuiCol_MenuBarBg] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.02f, 0.02f, 0.02f, 0.53f );
    colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_CheckMark] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_SliderGrab] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_Button] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_ButtonHovered] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_ButtonActive] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_Header] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_HeaderHovered] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_HeaderActive] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_Separator] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_SeparatorHovered] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_SeparatorActive] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_ResizeGrip] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_Tab] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_TabHovered] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_TabActive] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_TabUnfocused] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_PlotLines] = ImVec4( 1.00f, 1.00f, 1.00f, 1.00f );
    colors[ImGuiCol_PlotLinesHovered] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_PlotHistogram] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_TableHeaderBg] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_TableBorderStrong] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_TableBorderLight] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
    colors[ImGuiCol_TableRowBg] = ImVec4( 0.07f, 0.07f, 0.07f, 1.00f );
    colors[ImGuiCol_TableRowBgAlt] = ImVec4( 1.00f, 1.00f, 1.00f, 0.06f );
    colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_DragDropTarget] = ImVec4( 0.91f, 0.33f, 0.13f, 1.00f );
    colors[ImGuiCol_NavHighlight] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4( 0.05f, 0.52f, 0.13f, 1.00f );
}