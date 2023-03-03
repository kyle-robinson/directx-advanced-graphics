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
#include "Input.h"

#include "implot.h"
#include "imgui_internal.h"
#include "fileDialog/ImGuiFileDialog.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "misc/cpp/imgui_stdlib.h"

#include <dxtk/WICTextureLoader.h>
#include <format>

#define SLIDER_HINT_TEXT "CTRL + click the slider to input a value directly."
#define DRAG_HINT_TEXT "Double click the drag slider to input a value directly."
#define COLOR_PICKER_HINT_TEXT "Click the color image to open a colour picker."

extern void HelpMarker( const char* desc )
{
    ImGui::TextDisabled( "(?)" );
    if ( ImGui::IsItemHovered( ImGuiHoveredFlags_DelayShort ) )
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
        ImGui::TextUnformatted( desc );
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

ImGuiManager::ImGuiManager()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    SetUbuntuTheme();
}

ImGuiManager::~ImGuiManager()
{
    ImPlot::DestroyContext();
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
    ImGuizmo::BeginFrame();
    ImGui::DockSpaceOverViewport( ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode );

    // Show demo windows
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
    ImGui::ShowMetricsWindow();
}

void ImGuiManager::EndRender()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
}

void ImGuiManager::SceneWindow( UINT width, UINT height, ID3D11ShaderResourceView* pTexture, Input* pInput )
{
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
    if ( ImGui::Begin( "Scene Window", FALSE ) )
    {
        // Prevent key/mouse inputs sticking when the window is not focused
        static bool updateInputState = false;
        if ( ImGui::IsWindowFocused() && !updateInputState )
        {
            updateInputState = true;
            pInput->UnblockKeyInputs();
            pInput->UnblockMouseInputs();
        }
        else if ( !ImGui::IsWindowFocused() && updateInputState )
        {
            updateInputState = false;
            pInput->BlockKeyInputs();
            pInput->BlockMouseInputs();
        }

        // Handle the activation of the ImGuizmo manipulator
        if ( !pInput->GetIsMovingCursor() )
        {
            if ( m_bUsingTranslation )
            {
                if ( ImGui::IsWindowFocused() )
                {
                    pInput->DisableImGuiMouse();
                }
                else
                {
                    pInput->EnableImGuiMouse();
                }
            }
            else
            {
                if ( m_bUsingManipulator )
                {
                    pInput->DisableImGuiMouse();
                }
                else
                {
                    pInput->EnableImGuiMouse();
                }
            }
        }

        // Add the scene texture to an imgui window and render
        ImVec2 vRegionMax = ImGui::GetWindowContentRegionMax();
        ImVec2 vImageMax = ImVec2(
            vRegionMax.x + ImGui::GetWindowPos().x,
            vRegionMax.y + ImGui::GetWindowPos().y );

        ImVec2 vRatio =
        {
            width / ImGui::GetWindowSize().x,
            height / ImGui::GetWindowSize().y
        };

        bool bIsFitToWidth = vRatio.x < vRatio.y ? true : false;
        ImVec2 ivMax =
        {
            bIsFitToWidth ? width / vRatio.y : vRegionMax.x,
            bIsFitToWidth ? vRegionMax.y : height / vRatio.x
        };

        ImVec2 pos = ImGui::GetCursorScreenPos();
        XMFLOAT2 half = { ( ivMax.x - vRegionMax.x ) / 2, ( ivMax.y - vRegionMax.y ) / 2 };
        ImVec2 vHalfPos = { pos.x - half.x, pos.y - half.y };

        ImVec2 ivMaxPos =
        {
            ivMax.x + ImGui::GetWindowPos().x - half.x,
            ivMax.y + ImGui::GetWindowPos().y - half.y
        };

        m_vSceneWindowPos = XMFLOAT2( (float)ImGui::GetWindowPos().x, (float)ImGui::GetWindowPos().y );
        m_vSceneWindowSize = XMFLOAT2( (float)ImGui::GetWindowWidth(), (float)ImGui::GetWindowHeight() );

        ImGui::GetWindowDrawList()->AddImage( (void*)pTexture, vHalfPos, ivMaxPos );
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void ImGuiManager::CameraMenu( CameraController* cameraControl, bool usingVoxels )
{
    static std::string sName = cameraControl->GetCurentCam()->GetCamName();
    static const char* cCurrentItem = sName.c_str();

    if ( ImGui::Begin( "Cameras", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        float movementSpeed = cameraControl->GetCurentCam()->GetCamSpeed();
        float nearPlane = cameraControl->GetCurentCam()->GetNear();
        float farPlane = cameraControl->GetCurentCam()->GetFar();

        ImGui::Text( "Current Camera" );
        if ( ImGui::BeginCombo( "##Combo", cCurrentItem ) )
        {
            for ( int i = 0; i < cameraControl->GetCamList().size(); i++ )
            {
                bool is_selected = ( cCurrentItem == cameraControl->GetCamList()[i]->GetCamName().c_str() );

                if ( ImGui::Selectable( cameraControl->GetCamList()[i]->GetCamName().c_str(), is_selected ) )
                {
                    sName = cameraControl->GetCamList()[i]->GetCamName();
                    cCurrentItem = sName.c_str();
                    cameraControl->SetCam( i );
                }

                if ( is_selected )
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemOpen( true, ImGuiCond_Once );
        if ( ImGui::TreeNode( "Instructions" ) )
        {
            if ( ImGui::BeginTable( "Controls", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame ) )
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "W" );

                ImGui::TableNextColumn();
                ImGui::Text( "Move Forward" );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "A" );

                ImGui::TableNextColumn();
                ImGui::Text( "Move Left" );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "S" );

                ImGui::TableNextColumn();
                ImGui::Text( "Move Back" );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "D" );

                ImGui::TableNextColumn();
                ImGui::Text( "Move Right" );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "CTRL" );

                ImGui::TableNextColumn();
                ImGui::Text( "Move Down" );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "SPACE" );

                ImGui::TableNextColumn();
                ImGui::Text( "Move Up" );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "Right Mouse" );

                ImGui::TableNextColumn();
                ImGui::Text( "Look Around" );

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text( "ESC" );

                ImGui::TableNextColumn();
                ImGui::Text( "Close Application" );

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        ImGui::Separator();

        if ( ImGui::TreeNode( "Info" ) )
        {
            for ( int n = 0; n < cameraControl->GetCamList().size(); n++ )
            {
                if ( ImGui::TreeNode( cameraControl->GetCamList()[n]->GetCamName().append( "##" ).append( std::to_string( n ) ).c_str() ) )
                {
                    if ( ImGui::BeginTable( "Camera Data", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame ) )
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
					    ImGui::Text( "Position" );

                        ImGui::TableNextColumn();
                        std::string xPos = std::format( "{:.2f}", cameraControl->GetCamList()[n]->GetPosition().x );
                        std::string yPos = std::format( "{:.2f}", cameraControl->GetCamList()[n]->GetPosition().y );
                        std::string zPos = std::format( "{:.2f}", cameraControl->GetCamList()[n]->GetPosition().z );
                        std::string sPos = xPos + ", " + yPos + ", " + zPos;
					    ImGui::Text( sPos.c_str() );

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text( "Rotation" );

                        ImGui::TableNextColumn();
                        std::string xRot = std::format( "{:.2f}", cameraControl->GetCamList()[n]->GetRot().x );
                        std::string yRot = std::format( "{:.2f}", cameraControl->GetCamList()[n]->GetRot().y );
                        std::string zRot = std::format( "{:.2f}", cameraControl->GetCamList()[n]->GetRot().z );
                        std::string sRot = xRot + ", " + yRot + ", " + zRot;
                        ImGui::Text( sRot.c_str() );

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text( "Speed" );

                        ImGui::TableNextColumn();
                        std::string sSpeed = std::format( "{:.2f}", cameraControl->GetCurentCam()->GetCamSpeed() );
                        ImGui::Text( sSpeed.c_str() );

                        ImGui::EndTable();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::Separator();

        if ( ImGui::TreeNode( "Controls" ) )
        {
            ImGui::Text( "Near Plane" );
            ImGui::SameLine();
            HelpMarker( DRAG_HINT_TEXT );
            if ( ImGui::DragFloat( "##Near Plane", &nearPlane, 0.01f, 0.01f, 10.0f ) )
                cameraControl->GetCurentCam()->SetNear( nearPlane );

            ImGui::Text( "Far Plane" );
            ImGui::SameLine();
            HelpMarker( DRAG_HINT_TEXT );
            if ( usingVoxels ) ImGui::TextColored( ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ), "Can't adjust far plane while drawing voxel terrain!" );
            ImGui::PushItemFlag( ImGuiItemFlags_Disabled, usingVoxels );
            if ( ImGui::DragFloat( "##Far Plane", &farPlane, 0.1f, 100.0f, 200.0f ) )
                cameraControl->GetCurentCam()->SetFar( farPlane );
            ImGui::PopItemFlag();

            ImGui::Text( "Movement Speed" );
            ImGui::SameLine();
            HelpMarker( SLIDER_HINT_TEXT );
            if ( ImGui::SliderFloat( "##Movement Speed", &movementSpeed, 0.1f, 1.0f, "%.1f" ) )
                cameraControl->GetCurentCam()->SetCamSpeed( movementSpeed );

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ImGuiManager::ShaderMenu( ShaderController* shaderControl, PostProcessingCB* postSettings, RasterizerController* rasterControl )
{
    static std::string sCurrentRasterState = "Back";
    static std::string sShaderName = shaderControl->GetShaderData().m_sName;
    static const char* cCurrentShader = sShaderName.c_str();

    if ( ImGui::Begin( "Shaders", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
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
            ImGui::SameLine();
            HelpMarker( SLIDER_HINT_TEXT );
            ImGui::SliderFloat( "##Fade Amount", &postSettings->FadeAmount, 0.0f, 1.0f, "%.1f" );

            bool useBloom = postSettings->UseBloom;
            bool useBlur = postSettings->UseBlur;
            bool useColour = postSettings->UseColour;
            bool useDOF = postSettings->UseDepthOfField;

            if ( ImGui::BeginTable( "##Post-Processing Options", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchSame ) )
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
                ImGui::SameLine();
                HelpMarker( COLOR_PICKER_HINT_TEXT );
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
                    ImGui::InputFloat( "##Attenuation", &postSettings->BlurAttenuation );
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ImGuiManager::ObjectMenu( ID3D11Device* pDevice, Camera* pCamera, std::vector<DrawableGameObject*>& gameObjects )
{
    static int iSelectedIdx = 0;
    static std::string sNameO = gameObjects[0]->GetName();
    static DrawableGameObject* currObject = gameObjects[0];
    static const char* cCurrentItemO = sNameO.c_str();
    static XMFLOAT4X4 world = currObject->GetTransform()->GetWorldMatrix();

    static std::vector<ImGuizmoData> guizmoData;
    static bool bLoadO = false;
    if ( !bLoadO )
    {
        for ( unsigned int i = 0; i < gameObjects.size(); i++ )
        {
            guizmoData.push_back( ImGuizmoData() );
            guizmoData[i].ID = i;
            m_iGizmoID++;
        }
        bLoadO = true;
    }

    if ( ImGui::Begin( "Objects", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        ImGui::Text( "Current Object" );
        if ( ImGui::BeginCombo( "##Combo", cCurrentItemO ) )
        {
            for ( int i = 0; i < gameObjects.size(); i++ )
            {
                bool is_selected = ( cCurrentItemO == gameObjects[i]->GetName().c_str() );
                if ( ImGui::Selectable( gameObjects[i]->GetName().c_str(), is_selected ) )
                {
                    iSelectedIdx = i;
                    sNameO = gameObjects[i]->GetName();
                    cCurrentItemO = sNameO.c_str();
                    currObject = gameObjects[i];
                    world = currObject->GetTransform()->GetWorldMatrix();
                }

                if ( is_selected )
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        static bool bDrawObject = currObject->GetAppearance()->IsVisible();
        ImGui::Checkbox( "Draw Object?", &bDrawObject );
        if ( bDrawObject )
        {
			currObject->GetAppearance()->Show();

            if ( ImGui::TreeNode( "Transform Controls" ) )
            {
                static XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
                static XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
                if ( ImGuizmo::IsUsing() )
                {
                    position = currObject->GetTransform()->GetPosition();
                    rotation = currObject->GetTransform()->GetRotation();
                }

                ImGui::Text( "Position" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                if ( ImGui::DragFloat3( std::string( "##Position" ).append( std::to_string( iSelectedIdx ) ).c_str(), &position.x, 0.01f ) )
                    currObject->GetTransform()->SetPosition( position );

                ImGui::Text( "Rotation" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                if ( ImGui::DragFloat3( std::string( "##Rotation" ).append( std::to_string( iSelectedIdx ) ).c_str(), &rotation.x, 1.0f ) )
                    currObject->GetTransform()->SetRotation( rotation );

                if ( ImGui::Button( "Reset" ) )
                {
                    rotation = { 0.0f, 0.0f, 0.0f };
                    position = { 0.0f, 0.0f, 0.0f };
                }

                ImGui::TreePop();
            }
            ImGui::Separator();

            if ( ImGui::TreeNode( "Texture Controls" ) )
            {
                MaterialPropertiesCB materialData = currObject->GetAppearance()->GetMaterialData();

                bool useTexture = materialData.Material.UseTexture;
                if ( ImGui::Checkbox( "Use Texture?", &useTexture ) )
                    materialData.Material.UseTexture = useTexture;

                if ( useTexture )
                {
                    if ( ImGui::BeginTable( "Texture Table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp ) )
                    {
                        ImGui::TableSetupColumn( "Type" );
                        ImGui::TableSetupColumn( "File" );
                        ImGui::TableSetupColumn( "Change" );
                        ImGui::TableHeadersRow();

                        // Change diffuse texture
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text( "Diffuse" );

                        ImGui::TableNextColumn();
                        static std::string sDiffuseTex = currObject->GetAppearance()->GetDiffuseName();
                        ImGui::Text( sDiffuseTex.c_str() );

                        ImGui::TableNextColumn();
                        if ( ImGui::Button( "Load##DiffuseButton" ) )
                        {
                            ImGuiFileDialog::Instance()->OpenDialog( "Load Diffuse##Dialog", "Load Diffuse Map", ".dds,.jpg,.png", "." );
                            ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once );
                            ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once, ImVec2( 0.5f, 0.5f ) );
                        }

                        if ( ImGuiFileDialog::Instance()->Display( "Load Diffuse##Dialog" ) )
                        {
                            if ( ImGuiFileDialog::Instance()->IsOk() )
                            {
                                try
                                {
                                    std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                                    sDiffuseTex = ImGuiFileDialog::Instance()->GetCurrentFileName();
                                    ID3D11ShaderResourceView* pTexture = nullptr;
                                    HRESULT hr = S_OK;
                                    if ( filePath.find( ".dds" ) != std::string::npos )
                                    {
                                        hr = CreateDDSTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    else if ( filePath.find( ".jpg" ) != std::string::npos )
                                    {
                                        hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    else if ( filePath.find( ".png" ) != std::string::npos )
                                    {
                                        hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    COM_ERROR_IF_FAILED( hr, "Failed to load DIFFUSE texture!" );
                                    currObject->GetAppearance()->SetTextureRV( sDiffuseTex, pTexture );
                                }
                                catch ( COMException& exception )
                                {
                                    ErrorLogger::Log( exception );
                                }
                            }
                            ImGuiFileDialog::Instance()->Close();
                        }

                        // Change normal texture
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text( "Normal" );

                        ImGui::TableNextColumn();
                        static std::string sNormalTex = currObject->GetAppearance()->GetNormalName();
                        ImGui::Text( sNormalTex.c_str() );

                        ImGui::TableNextColumn();
                        if ( ImGui::Button( "Load##NormalButton" ) )
                        {
                            ImGuiFileDialog::Instance()->OpenDialog( "Load Normal##Dialog", "Load Normal Map", ".dds,.jpg,.png", "." );
                            ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once );
                            ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once, ImVec2( 0.5f, 0.5f ) );
                        }

                        if ( ImGuiFileDialog::Instance()->Display( "Load Normal##Dialog" ) )
                        {
                            if ( ImGuiFileDialog::Instance()->IsOk() )
                            {
                                try
                                {
                                    std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                                    sNormalTex = ImGuiFileDialog::Instance()->GetCurrentFileName();
                                    ID3D11ShaderResourceView* pTexture = nullptr;
                                    HRESULT hr = S_OK;
                                    if ( filePath.find( ".dds" ) != std::string::npos )
                                    {
                                        hr = CreateDDSTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    else if ( filePath.find( ".jpg" ) != std::string::npos )
                                    {
                                        hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    else if ( filePath.find( ".png" ) != std::string::npos )
                                    {
                                        hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    COM_ERROR_IF_FAILED( hr, "Failed to load NORMAL texture!" );
                                    currObject->GetAppearance()->SetNormalRV( sNormalTex, pTexture );
                                }
                                catch ( COMException& exception )
                                {
                                    ErrorLogger::Log( exception );
                                }
                            }
                            ImGuiFileDialog::Instance()->Close();
                        }

                        // Change parallax texture
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text( "Parallax" );

                        ImGui::TableNextColumn();
                        static std::string sParallaxTex = currObject->GetAppearance()->GetParallaxName();
                        ImGui::Text( sParallaxTex.c_str() );

                        ImGui::TableNextColumn();
                        if ( ImGui::Button( "Load##ParallaxButton" ) )
                        {
                            ImGuiFileDialog::Instance()->OpenDialog( "Load Parallax##Dialog", "Load Parallax Map", ".dds,.jpg,.png", "." );
                            ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once );
                            ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once, ImVec2( 0.5f, 0.5f ) );
                        }

                        if ( ImGuiFileDialog::Instance()->Display( "Load Parallax##Dialog" ) )
                        {
                            if ( ImGuiFileDialog::Instance()->IsOk() )
                            {
                                try
                                {
                                    std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                                    sParallaxTex = ImGuiFileDialog::Instance()->GetCurrentFileName();
                                    ID3D11ShaderResourceView* pTexture = nullptr;
                                    HRESULT hr = S_OK;
                                    if ( filePath.find( ".dds" ) != std::string::npos )
                                    {
                                        hr = CreateDDSTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    else if ( filePath.find( ".jpg" ) != std::string::npos )
                                    {
                                        hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    else if ( filePath.find( ".png" ) != std::string::npos )
                                    {
                                        hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                    }
                                    COM_ERROR_IF_FAILED( hr, "Failed to load PARALLAX texture!" );
                                    currObject->GetAppearance()->SetParallaxRV( sParallaxTex, pTexture );
                                }
                                catch ( COMException& exception )
                                {
                                    ErrorLogger::Log( exception );
                                }
                            }
                            ImGuiFileDialog::Instance()->Close();
                        }

                        ImGui::EndTable();
                    }

                    if ( ImGui::TreeNode( "Colour Controls" ) )
                    {
                        ImGui::Text( "Ambient" );
                        ImGui::SameLine();
                        HelpMarker( COLOR_PICKER_HINT_TEXT );
                        ImGui::ColorEdit3( "##Ambient", &materialData.Material.Ambient.x );

                        ImGui::Text( "Diffuse" );
                        ImGui::SameLine();
                        HelpMarker( COLOR_PICKER_HINT_TEXT );
                        ImGui::ColorEdit3( "##Diffuse", &materialData.Material.Diffuse.x );

                        ImGui::Text( "Emissive" );
                        ImGui::SameLine();
                        HelpMarker( COLOR_PICKER_HINT_TEXT );
                        ImGui::ColorEdit3( "##Emissive", &materialData.Material.Emissive.x );

                        ImGui::Text( "Specular" );
                        ImGui::SameLine();
                        HelpMarker( COLOR_PICKER_HINT_TEXT );
                        ImGui::ColorEdit3( "##Specular", &materialData.Material.Specular.x );

                        ImGui::Text( "Power" );
                        ImGui::SameLine();
                        HelpMarker( DRAG_HINT_TEXT );
                        ImGui::DragFloat( "##Power", &materialData.Material.SpecularPower, 1.0f, 1.0f, 32.0f );
                        ImGui::TreePop();
                    }

                    if ( ImGui::TreeNode( "Parallax Controls" ) )
                    {
                        ImGui::Text( "Height Scale" );
                        ImGui::SameLine();
                        HelpMarker( DRAG_HINT_TEXT );
                        ImGui::DragFloat( "##Height Scale", &materialData.Material.HeightScale, 0.1f, 0.0f, 100.0f );

                        ImGui::Text( "Min Layers" );
                        ImGui::SameLine();
                        HelpMarker( DRAG_HINT_TEXT );
                        ImGui::DragFloat( "##Min Layers", &materialData.Material.MinLayers, 1.0f, 1.0f, 30.0f );

                        ImGui::Text( "Max Layers" );
                        ImGui::SameLine();
                        HelpMarker( DRAG_HINT_TEXT );
                        ImGui::DragFloat( "##Max Layers", &materialData.Material.MaxLayers, 1.0f, 1.0f, 30.0f );
                        ImGui::TreePop();
                    }
                }

                currObject->GetAppearance()->SetMaterialData( materialData );
                ImGui::TreePop();
            }
            ImGui::Separator();

            SpawnImGuizmo( currObject->GetTransform(), pCamera, world, guizmoData[iSelectedIdx] );
        }
        else
        {
			currObject->GetAppearance()->Hide();
        }
    }
    ImGui::End();
}

void ImGuiManager::LightMenu( LightController* lightControl )
{
    static std::string nameL = lightControl->GetLight( 0 )->GetName();
    static Light currLightData = lightControl->GetLightList()[0]->GetLightData();
    static const char* cCurrentItemL = nameL.c_str();

    if ( ImGui::Begin( "Lights", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
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

        bool enable = currLightData.Enabled;
        if ( ImGui::Checkbox( "Enabled?", &enable ) )
            currLightData.Enabled = enable;
        if ( enable )
        {
            if ( ImGui::TreeNode( "Light Data" ) )
            {
                ImGui::Text( "Position" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                ImGui::DragFloat3( "##Position", &currLightData.Position.x, 1.0f, -10.0f, 10.0f );

                ImGui::Text( "Colour" );
                ImGui::SameLine();
                HelpMarker( COLOR_PICKER_HINT_TEXT );
                float colour[] = { currLightData.Color.x, currLightData.Color.y, currLightData.Color.z, currLightData.Color.w };
                if ( ImGui::ColorEdit4( "##Colour", colour, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayRGB ) )
                    currLightData.Color = { colour[0], colour[1], colour[2], colour[3] };

                ImGui::TreePop();
            }
            ImGui::Separator();

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
            ImGui::Separator();

            if ( ImGui::TreeNode( ( currLightData.LightType == LightType::DirectionalLight ) ? "Direction" : "Attenuation" ) )
            {
                switch ( currLightData.LightType )
                {
                case LightType::PointLight:
                    ImGui::Text( "Constant" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    ImGui::DragFloat( "##Constant", &currLightData.ConstantAttenuation, 0.1f, 1.0f, 10.0f, "%.2f" );

                    ImGui::Text( "Linear" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    ImGui::DragFloat( "##Linear", &currLightData.LinearAttenuation, 0.1f, 0.0f, 5.0f, "%.4f" );

                    ImGui::Text( "Quadratic" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    ImGui::DragFloat( "##Quadratic", &currLightData.QuadraticAttenuation, 0.1f, 0.0f, 2.0f, "%.7f" );
                    break;

                case LightType::SpotLight:
                {
                    ImGui::Text( "Constant" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    ImGui::DragFloat( "##Constant", &currLightData.ConstantAttenuation, 0.1f, 1.0f, 10.0f, "%.2f" );

                    ImGui::Text( "Linear" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    ImGui::DragFloat( "##Linear", &currLightData.LinearAttenuation, 0.01f, 0.0f, 5.0f, "%.4f" );

                    ImGui::Text( "Quadratic" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    ImGui::DragFloat( "##Quadratic", &currLightData.QuadraticAttenuation, 0.001f, 0.0f, 2.0f, "%.7f" );

                    ImGui::Text( "Spot Angle" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    float SpotAngle = XMConvertToDegrees( currLightData.SpotAngle );
                    ImGui::DragFloat( "##Spot Angle", &SpotAngle, 0.1f );
                    currLightData.SpotAngle = XMConvertToRadians( SpotAngle );
                }
                break;

                case LightType::DirectionalLight:
                {
                    ImGui::Text( "Direction" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    ImGui::DragFloat3( "##Direction", &currLightData.Direction.x );
                }
                break;
                }
                ImGui::TreePop();
            }
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
    vBbVerts = billboardObject->GetPositions();

    if ( ImGui::Begin( "Billboard Control", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        if ( ImGui::BeginCombo( "##Combo", cCurrentItemBB ) )
        {
            for ( int n = 0; n < billboardObject->GetPositions().size(); n++ )
            {
                std::string name = "Billboard ";
                name += std::to_string( n );
                bool is_selected = ( cCurrentItemBB == name.c_str() );
                if ( ImGui::Selectable( name.c_str(), is_selected ) )
                {
                    iPicked = n;
                    nameBB = name;
                    cCurrentItemBB = nameBB.c_str();
                    vBbVerts = billboardObject->GetPositions();
                }

                if ( is_selected )
                {
                    ImGui::SetItemDefaultFocus();
                    vBbVerts = billboardObject->GetPositions();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text( "Position" );
        ImGui::SameLine();
        HelpMarker( DRAG_HINT_TEXT );
        ImGui::DragFloat3( "##Position", &vBbVerts[iPicked].Pos.x );
        billboardObject->SetPositions( vBbVerts );
    }
    ImGui::End();
}

std::vector<float> CubicBezierBasis( float u )
{
    float compla = 1.0f - u; // complement of u
    // compute value of basis functions for given value of u
    float BF0 = compla * compla * compla;
    float BF1 = 3.0f * u * compla * compla;
    float BF2 = 3.0f * u * u * compla;
    float BF3 = u * u * u;

    std::vector<float> bfArray = { BF0, BF1, BF2, BF3 };
    return bfArray;
}

std::vector<XMFLOAT2> CubicBezierCurve( std::vector<XMFLOAT2> controlPoints )
{
    std::vector<XMFLOAT2> points;
    for ( float i = 0.0f; i < 1.0f; i += 0.1f )
    {
        // Calculate value of each basis function for current u
        std::vector<float> basisFnValues = CubicBezierBasis( i );

        XMFLOAT2 sum = { 0.0f, 0.0f };
        for ( int cPointIndex = 0; cPointIndex <= 3; cPointIndex++ )
        {
            // Calculate weighted sum (weightx * CPx)
            sum.x += controlPoints[cPointIndex].x * basisFnValues[cPointIndex];
            sum.y += controlPoints[cPointIndex].y * basisFnValues[cPointIndex];
        }

        XMFLOAT2 point = sum; // point for current u on cubic Bezier curve
        points.push_back( point );
    }
    return points;
}

void ImGuiManager::BezierSplineMenu()
{
    // Setup Bezier spline
    static std::vector<XMFLOAT2> points;
    static bool bLoadSpline = false;
    if ( !bLoadSpline )
    {
        points = {
            XMFLOAT2{ 0.0f, 0.0f },
            XMFLOAT2{ 2.0f, 1.0f },
            XMFLOAT2{ 5.0f, 0.6f },
            XMFLOAT2{ 6.0f, 0.0f } };
        m_vPoints = CubicBezierCurve( points );
        bLoadSpline = true;
    }

    if ( ImGui::Begin( "Bezier Curve", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        if ( ImGui::TreeNode( "Spline Points" ) )
        {
            for ( unsigned int i = 0; i < points.size(); i++ )
            {
                if ( ImGui::DragFloat2( std::string( "##" ).append( std::to_string( i ) ).c_str(), &points[i].x, 0.1f ) )
                {
                    m_vPoints = CubicBezierCurve( points );
                }
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
            }
            ImGui::TreePop();
        }
        ImGui::Separator();

        static float thickness = 1.0f;
        static bool drawShaded = false;
        static bool showMarkers = false;
		static ImPlotMarker eMarkerType = ImPlotMarker_Square;
        static ImVec4 barColor = ImVec4( 1.0f, 0.4f, 0.6f, 1.0f );
        static ImVec4 lineColor = ImVec4( 1.0f, 0.7f, 0.3f, 1.0f );

        if ( ImGui::TreeNode( "Line Options" ) )
        {
            ImGui::Text( "Line Colour" );
            ImGui::SameLine();
            HelpMarker( COLOR_PICKER_HINT_TEXT );
            ImGui::ColorEdit3( "##Line Colour", &lineColor.x );

            ImGui::Text( "Thickness" );
            ImGui::SameLine();
            HelpMarker( SLIDER_HINT_TEXT );
            ImGui::SliderFloat( "##Thickness", &thickness, 0.0f, 5.0f );

            ImGui::Checkbox( "Markers", &showMarkers );
            ImGui::SameLine();
            ImGui::Checkbox( "Shaded", &drawShaded );

            if ( showMarkers )
            {
                static int activeMarker = 2;
                static const char* cCurrentItemC = "Square";
				static const char* cCurrentItemList[] = { "None", "Circle", "Square", "Diamond", "Up", "Down", "Left", "Right", "Cross", "Plus", "Asterisk" };

                ImGui::Text( "Marker Type" );
                if ( ImGui::BeginCombo( "##Combo", cCurrentItemC ) )
                {
                    for ( int i = 0; i < IM_ARRAYSIZE( cCurrentItemList ); i++ )
                    {
                        const bool is_selected = i == activeMarker;
                        if ( ImGui::Selectable( cCurrentItemList[i], is_selected ) )
                        {
                            activeMarker = i;
                            cCurrentItemC = cCurrentItemList[i];
                        }

                        if ( is_selected )
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    switch ( activeMarker )
                    {
                    case 0: eMarkerType = ImPlotMarker_None; break;
                    case 1: eMarkerType = ImPlotMarker_Circle; break;
                    case 2: eMarkerType = ImPlotMarker_Square; break;
                    case 3: eMarkerType = ImPlotMarker_Diamond; break;
                    case 4: eMarkerType = ImPlotMarker_Up; break;
                    case 5: eMarkerType = ImPlotMarker_Down; break;
                    case 6: eMarkerType = ImPlotMarker_Left; break;
                    case 7: eMarkerType = ImPlotMarker_Right; break;
                    case 8: eMarkerType = ImPlotMarker_Cross; break;
                    case 9: eMarkerType = ImPlotMarker_Plus; break;
                    case 10: eMarkerType = ImPlotMarker_Asterisk; break;
                    }

                    ImGui::EndCombo();
                }
            }

            ImGui::TreePop();
        }
        ImGui::Separator();

        if ( ImGui::TreeNode( "Bar Options" ) )
        {
            ImGui::Text( "Bar Colour" );
            ImGui::SameLine();
            HelpMarker( COLOR_PICKER_HINT_TEXT );
            ImGui::ColorEdit3( "##Bar Colour", &barColor.x );
            ImGui::TreePop();
        }

        float linesX[120];
        float linesY[120];
        for ( int n = 0; n < m_vPoints.size(); n++ )
        {
            linesX[n] = m_vPoints[n].x;
            linesY[n] = m_vPoints[n].y;
        }

        ImGui::NewLine();
        if ( ImPlot::BeginPlot( "Spline Plot" ) )
        {
            ImPlot::SetNextFillStyle( barColor );
            ImPlot::PlotBars( "Bar Plot", linesX, linesY, m_vPoints.size(), 0.5f );

            if ( showMarkers ) ImPlot::SetNextMarkerStyle( eMarkerType );
            ImPlot::SetNextLineStyle( lineColor, thickness );
            ImPlot::PlotLine( "Line Plot", linesX, linesY, m_vPoints.size(), ( drawShaded ? ImPlotLineFlags_Shaded : 0 ) );

            ImPlot::EndPlot();
        }
    }
    ImGui::End();
}

void ImGuiManager::TerrainMenu( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, Terrain* terrain, TerrainVoxel* voxelTerrain )
{
    if ( ImGui::Begin( "Terrain", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        if ( ImGui::TreeNode( "Terrain##Normal" ) )
        {
            ImGui::Checkbox( "Draw Terrain?", terrain->GetIsDraw() );
            if ( *terrain->GetIsDraw() )
            {
                if ( ImGui::TreeNode( "Terrain Info" ) )
                {
                    ImGui::Text( "Width: %i", terrain->GetHeightMapWidth() );
                    ImGui::Text( "Height: %i", terrain->GetHeightMapHeight() );
                    ImGui::Text( "Cell Spacing: %f", terrain->GetCellSpacing() );

                    switch ( terrain->GetGenType() )
                    {
                    case TerrainGenType::HeightMapLoad:
                        ImGui::Text( "GenType: HeightMap" );
                        ImGui::Text( terrain->GetHeightMapName().c_str() );
                        ImGui::Text( "Height Scale: %i", terrain->GetHeightScale() );
                        break;

                    case TerrainGenType::FaultLine:
                        ImGui::Text( "GenType: FaultLine" );
                        ImGui::Text( "Seed: %i", terrain->GetSeed() );
                        ImGui::Text( "Displacement: %f", terrain->GetDisplacement() );
                        ImGui::Text( "Iteration Count: %i", terrain->GetNumOfIterations() );
                        break;

                    case TerrainGenType::Noise:
                        ImGui::Text( "GenType: Noise" );
                        ImGui::Text( "Seed: %i", terrain->GetSeed() );
                        ImGui::Text( "Frequency: %f", terrain->GetFrequency() );
                        ImGui::Text( "Octave Count: %i", terrain->GetNumOfOctaves() );
                        ImGui::Text( "Height Scale: %i", terrain->GetHeightScale() );
                        break;

                    case TerrainGenType::DiamondSquare:
                        ImGui::Text( "GenType: Diamond Square" );
                        ImGui::Text( "Seed: %i", terrain->GetSeed() );
                        ImGui::Text( "Range: %i", terrain->GetRange() );
                        ImGui::Text( "Height Scale: %i", terrain->GetHeightScale() );
                        break;
                    }
                    ImGui::TreePop();
                }

                if ( ImGui::TreeNode( "Re-Build Options##Terrain" ) )
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
                                {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }

                    static int width = 514;
                    ImGui::Text( "Width" );
                    ImGui::InputInt( "##Width", &width );

                    static int length = 514;
                    ImGui::Text( "Length" );
                    ImGui::InputInt( "##Length", &length );

                    static float cellSpacing = 1.0f;
                    ImGui::Text( "Cell Spacing" );
                    ImGui::InputFloat( "##Cell Spacing", &cellSpacing );

                    static float heightScale = 50.0f;
                    static float frequency = 0.01f;
                    static float displacement = 0.01f;
                    static int seed = 1234;
                    static int range = 196;
                    static int numberOfOctaves = 3;

                    switch ( mode )
                    {
                    case TerrainGenType::HeightMapLoad:
                        ImGui::Text( "Height Scale" );
                        ImGui::InputFloat( "##HeightScale", &heightScale );
                        break;

                    case TerrainGenType::FaultLine:
                        ImGui::Text( "Seed" );
                        ImGui::InputInt( "##Seed", &seed );
                        ImGui::Text( "Interaction Count" );
                        ImGui::InputInt( "##NumberOfIteration", &range );
                        ImGui::Text( "Displacement" );
                        ImGui::InputFloat( "##Displacement", &displacement );
                        break;

                    case TerrainGenType::Noise:
                        ImGui::Text( "Seed" );
                        ImGui::InputInt( "##Seed", &seed );
                        ImGui::Text( "Frequency" );
                        ImGui::InputFloat( "##Frequency", &frequency );
                        ImGui::Text( "Octave Count" );
                        ImGui::InputInt( "##NumberOfOctaves", &numberOfOctaves );
                        ImGui::Text( "Height Scale" );
                        ImGui::InputFloat( "##HeightScale", &heightScale );
                        break;

                    case TerrainGenType::DiamondSquare:
                        ImGui::Text( "Seed" );
                        ImGui::InputInt( "##Seed", &seed );
                        ImGui::Text( "Range" );
                        ImGui::InputInt( "##Range", &range );
                        ImGui::Text( "Height Scale" );
                        ImGui::InputFloat( "##HeightScale", &heightScale );
                        break;
                    }

                    if ( ImGui::Button( "Re-Build Terrain##Normal" ) )
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

                        terrain->ReBuildTerrain( XMFLOAT2( width, length ), heightScale, cellSpacing, mode, pDevice );
                    }
                    ImGui::NewLine();
                    ImGui::Separator();
                    ImGui::NewLine();

                    ImGui::Text( "Height Map Override" );
                    ImGui::Text( terrain->GetHeightMapName().c_str() );

                    // Open file dialog to load height map
                    if ( ImGui::Button( "Load Height Map##Button" ) )
                    {
                        ImGuiFileDialog::Instance()->OpenDialog( "Load Height Map##Dialog", "Load Height Map", ".json", "." );
                        ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once );
                        ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once, ImVec2( 0.5f, 0.5f ) );
                    }

                    // Display file dialog window
                    if ( ImGuiFileDialog::Instance()->Display( "Load Height Map##Dialog" ) )
                    {
                        if ( ImGuiFileDialog::Instance()->IsOk() )
                        {
                            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
                            int position = fileName.find( ".json" );
                            std::string fileNameNoExt = fileName.erase( position );

                            TerrainData terrainData;
                            TerrainJsonLoad::LoadData( fileNameNoExt, terrainData );
                            TerrainGenType genType = (TerrainGenType)terrainData.Mode;
                            double heightScale = 0;

                            switch ( genType )
                            {
                            case TerrainGenType::HeightMapLoad:
                                heightScale = terrainData.HeightMapSettings.HeightScale;
                                break;

                            case TerrainGenType::FaultLine:
                                terrain->SetFaultLineData( terrainData.FaultLineSettings.Seed, terrainData.FaultLineSettings.IterationCount, terrainData.FaultLineSettings.Displacement );
                                heightScale = terrain->GetHeightScale();
                                break;

                            case TerrainGenType::Noise:
                                terrain->SetNoiseData( terrainData.NoiseSettings.Seed, terrainData.NoiseSettings.Frequency, terrainData.NoiseSettings.NumOfOctaves );
                                heightScale = terrainData.NoiseSettings.HeightScale;
                                break;

                            case TerrainGenType::DiamondSquare:
                                terrain->SetDiamondSquareData( terrainData.DiamondSquareSettings.Seed, terrainData.DiamondSquareSettings.Range );
                                heightScale = terrainData.DiamondSquareSettings.HeightScale;
                                break;
                            }

                            terrain->ReBuildTerrain( XMFLOAT2( terrainData.Width, terrainData.Depth ), heightScale, terrainData.CellSpacing, genType, pDevice );
                        }
                        ImGuiFileDialog::Instance()->Close();
                    }

                    ImGui::SameLine();
                    // Open file dialog to save height map
                    if ( ImGui::Button( "Save Height Map##Button" ) )
                    {
                        ImGuiFileDialog::Instance()->OpenDialog( "Save Height Map##Dialog", "Save Height Map", ".json", ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite );
                        ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once );
                        ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once, ImVec2( 0.5f, 0.5f ) );
                    }

                     // Display file dialog window
                    if ( ImGuiFileDialog::Instance()->Display( "Save Height Map##Dialog" ) )
                    {
                        if ( ImGuiFileDialog::Instance()->IsOk() )
                        {
                            TerrainData terrainData;
                            terrainData.Width = terrain->GetHeightMapWidth();
                            terrainData.Depth = terrain->GetHeightMapHeight();
                            terrainData.CellSpacing = terrain->GetCellSpacing();
                            terrainData.Mode = (int)terrain->GetGenType();

                            switch ( terrain->GetGenType() )
                            {
                            case TerrainGenType::HeightMapLoad:
                                terrainData.HeightMapSettings.HeightMapFile = terrain->GetHeightMapName();
                                terrainData.HeightMapSettings.HeightScale = terrain->GetHeightScale();
                                break;

                            case TerrainGenType::FaultLine:
                                terrainData.FaultLineSettings.Seed = terrain->GetSeed();
                                terrainData.FaultLineSettings.IterationCount = terrain->GetNumOfIterations();
                                terrainData.FaultLineSettings.Displacement = terrain->GetDisplacement();
                                break;

                            case TerrainGenType::Noise:
                                terrainData.NoiseSettings.Seed = terrain->GetSeed();
                                terrainData.NoiseSettings.HeightScale = terrain->GetHeightScale();
                                terrainData.NoiseSettings.Frequency = terrain->GetFrequency();
                                terrainData.NoiseSettings.NumOfOctaves = terrain->GetNumOfOctaves();
                                break;

                            case TerrainGenType::DiamondSquare:
                                terrainData.DiamondSquareSettings.Seed = terrain->GetSeed();
                                terrainData.DiamondSquareSettings.HeightScale = terrain->GetHeightScale();
                                terrainData.DiamondSquareSettings.Range = terrain->GetRange();
                                break;
                            }

                            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentFileName();
                            int position = filePath.find( ".json" );
                            std::string fileName = filePath.erase( position );
                            TerrainJsonLoad::StoreData( fileName, terrainData );
                        }

                        ImGuiFileDialog::Instance()->Close();
                    }

                    ImGui::TreePop();
                    ImGui::NewLine();
                }

                if ( ImGui::TreeNode( "Transform Controls" ) )
                {
                    ImGui::Text( "Position" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    XMFLOAT3 posTerrain = terrain->GetTransform()->GetPosition();
                    ImGui::DragFloat3( "##Position", &posTerrain.x );
                    terrain->GetTransform()->SetPosition( posTerrain );

                    ImGui::Text( "Rotation" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    XMFLOAT3 rotationTerrain = terrain->GetTransform()->GetRotation();
                    ImGui::DragFloat3( "##Rotation", &rotationTerrain.x, 1.0f, 0.0f, 360.0f );
                    terrain->GetTransform()->SetRotation( rotationTerrain );

                    ImGui::Text( "Scale" );
                    ImGui::SameLine();
                    HelpMarker( DRAG_HINT_TEXT );
                    XMFLOAT3 scaleTerrain = terrain->GetTransform()->GetScale();
                    ImGui::DragFloat3( "##Scale", &scaleTerrain.x );
                    terrain->GetTransform()->SetScale( scaleTerrain );

                    ImGui::TreePop();
                }

                if ( ImGui::TreeNode( "Texture Controls" ) )
                {
                    if ( ImGui::BeginTable( "Texture Name", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp ) )
                    {
                        ImGui::TableSetupColumn( "Name" );
                        ImGui::TableSetupColumn( "Texture" );
                        ImGui::TableHeadersRow();

                        for ( size_t i = 0; i < terrain->GetTexNames().size(); i++ )
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            std::string texType = "";
                            switch ( i )
                            {
                            case 0: texType = "Grass"; break;
                            case 1: texType = "Dark Dirt"; break;
                            case 2: texType = "Light Dirt"; break;
                            case 3: texType = "Stone"; break;
                            case 4: texType = "Snow"; break;
                            }
                            ImGui::Text( texType.c_str() );

                            ImGui::TableNextColumn();
                            ImGui::Text( terrain->GetTexNames()[i].c_str() );

                            ImGui::TableNextColumn();
                            if ( ImGui::Button( "Load##Button" ) )
                            {
                                ImGuiFileDialog::Instance()->OpenDialog( std::string( "Load##" ).append( texType ).c_str(), "Load Terrain Texture", ".dds,.jpg,.png", "." );
                                ImGui::SetNextWindowSize( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once );
                                ImGui::SetNextWindowPos( ImVec2( ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f ), ImGuiCond_Once, ImVec2( 0.5f, 0.5f ) );
                            }

                            if ( ImGuiFileDialog::Instance()->Display( std::string( "Load##" ).append( texType ).c_str() ) )
                            {
                                if ( ImGuiFileDialog::Instance()->IsOk() )
                                {
                                    try
                                    {
                                        std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                                        std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
                                        ID3D11ShaderResourceView* pTexture = nullptr;
                                        HRESULT hr = S_OK;
                                        if ( filePath.find( ".dds" ) != std::string::npos )
                                        {
                                            hr = CreateDDSTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                        }
                                        else if ( filePath.find( ".jpg" ) != std::string::npos )
                                        {
                                            hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                        }
                                        else if ( filePath.find( ".png" ) != std::string::npos )
                                        {
                                            hr = CreateWICTextureFromFile( pDevice, StringHelper::ToWide( filePath ).c_str(), nullptr, &pTexture );
                                        }
                                        COM_ERROR_IF_FAILED( hr, "Failed to load TERRAIN texture!" );
                                        terrain->SetTexture( i, fileName, pTexture );
                                    }
                                    catch ( COMException& exception )
                                    {
                                        ErrorLogger::Log( exception );
                                    }
                                }
                                ImGuiFileDialog::Instance()->Close();
                            }
                        }

                        ImGui::EndTable();
                    }

                    float layer1MaxHeight = terrain->GetTerrainData().Layer1MaxHeight;
                    float layer2MaxHeight = terrain->GetTerrainData().Layer2MaxHeight;
                    float layer3MaxHeight = terrain->GetTerrainData().Layer3MaxHeight;
                    float layer4MaxHeight = terrain->GetTerrainData().Layer4MaxHeight;
                    float layer5MaxHeight = terrain->GetTerrainData().Layer5MaxHeight;

                    ImGui::Text( "Layer 1 Max Height" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Layer1MaxHeight", &layer1MaxHeight, 0.0f, layer2MaxHeight );

                    ImGui::Text( "Layer 2 Max Height" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Layer2MaxHeight", &layer2MaxHeight, layer1MaxHeight, layer3MaxHeight );

                    ImGui::Text( "Layer 3 Max Height" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Layer3MaxHeight", &layer3MaxHeight, layer2MaxHeight, layer4MaxHeight );

                    ImGui::Text( "Layer 4 Max Height" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Layer4MaxHeight", &layer4MaxHeight, layer3MaxHeight, layer5MaxHeight );

                    ImGui::Text( "Layer 5 Max Height" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Layer5MaxHeight", &layer5MaxHeight, layer4MaxHeight, 100000.0f );

                    terrain->SetTexHeights( layer1MaxHeight, layer2MaxHeight, layer3MaxHeight, layer4MaxHeight, layer5MaxHeight );
                    ImGui::TreePop();
                }

                if ( ImGui::TreeNode( "LOD Controls" ) )
                {
                    float floatMinTess = terrain->GetTerrainData().MinTess;
                    ImGui::Text( "Min Tess" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Min Tess", &floatMinTess, 0.0f, 6.0f );

                    float floatMaxTess = terrain->GetTerrainData().MaxTess;
                    ImGui::Text( "Max Tess" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Max Tess", &floatMaxTess, 0.0f, 6.0f );

                    if ( floatMaxTess < floatMaxTess )
                    {
                        floatMaxTess = floatMinTess;
                    }
                    terrain->SetMaxTess( floatMaxTess );
                    terrain->SetMinTess( floatMinTess );

                    float minTessDist = terrain->GetTerrainData().MinDist;
                    ImGui::Text( "Min Tess Distance" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Min Tess Distance", &minTessDist, 1.0f, 1000.0f );

                    float maxTessDist = terrain->GetTerrainData().MaxDist;
                    ImGui::Text( "Max Tess Distance" );
                    ImGui::SameLine();
                    HelpMarker( SLIDER_HINT_TEXT );
                    ImGui::SliderFloat( "##Max Tess Distance", &maxTessDist, 1.0f, 1000.0f );

                    if ( maxTessDist < minTessDist )
                    {
                        maxTessDist = minTessDist;
                    }
                    terrain->SetMaxTessDist( maxTessDist );
                    terrain->SetMinTessDist( minTessDist );

                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::Separator();

        if ( ImGui::TreeNode( "Voxel Terrain" ) )
        {
            ImGui::Checkbox( "Draw Voxels?", voxelTerrain->GetIsDraw() );

            if ( *voxelTerrain->GetIsDraw() )
            {
                if ( ImGui::TreeNode( "Voxel Info" ) )
                {
                    ImGui::Text( "Chunk Count: %i", voxelTerrain->GetNumberOfChunks() );
                    ImGui::Text( "Octave Count: %i", voxelTerrain->GetOctaves() );
                    ImGui::Text( "Frequency: %f", voxelTerrain->GetFrequency() );
                    ImGui::Text( "Seed: %i", voxelTerrain->GetSeed() );
                    ImGui::TreePop();
                }

                if ( ImGui::TreeNode( "Rebuild Options##Voxel" ) )
                {
                    static int seed;
                    ImGui::Text( "Seed" );
                    ImGui::InputInt( "#Seed Voxel", &seed );

                    static float frequency;
                    ImGui::Text( "Frequency" );
                    ImGui::InputFloat( "##Frequency Voxel", &frequency );

                    static int octave;
                    ImGui::Text( "Octaves" );
                    ImGui::InputInt( "##Octaves Voxel", &octave );

                    static int numberOfChunksX = 0;
                    ImGui::Text( "Chunk Count X" );
                    ImGui::InputInt( "##Number of Chunk X", &numberOfChunksX );

                    static int numberOfChunksZ = 0;
                    ImGui::Text( "Chunk Count Z" );
                    ImGui::InputInt( "##Number of Chunk Z", &numberOfChunksZ );

                    if ( ImGui::Button( "Re-Build Terrain##Voxel" ) )
                    {
                        voxelTerrain->RebuildMap( pDevice, pContext, seed, numberOfChunksX, numberOfChunksZ, frequency, octave );
                    }
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ImGuiManager::AnimationMenu( AnimatedModel* animModel, Camera* pCamera )
{
    if ( ImGui::Begin( "Animation", FALSE, ImGuiWindowFlags_AlwaysAutoResize ) )
    {
        static bool bDraw = animModel->IsVisible();
        ImGui::Checkbox( "Draw Animated Model?", &bDraw );
		if ( bDraw )
		{
			animModel->Show();

            if ( ImGui::TreeNode( "Model Data" ) )
            {
                std::string modelName = "Model Name: ";
                modelName += animModel->GetModelName().c_str();
                ImGui::Text( modelName.c_str() );
                if ( ImGui::BeginTable( "Mesh Data", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp ) )
                {
                    ImGui::TableSetupColumn( "ID" );
                    ImGui::TableSetupColumn( "Face Count" );
                    ImGui::TableSetupColumn( "Vertex Count" );
                    ImGui::TableSetupColumn( "Diffuse Tex" );
                    ImGui::TableHeadersRow();

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
                        ImGui::Text( StringHelper::ToNarrow( diffuseMap ).c_str() );
                    }
                    ImGui::EndTable();
                }

                static bool bRequiresUpdate = true;
                if ( ImGuizmo::IsUsing() )
                    bRequiresUpdate = true;

                ImGui::Text( "Position" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                static float modelPos[3];
                if ( bRequiresUpdate )
                {
                    modelPos[0] = animModel->GetTransform()->GetPosition().x;
                    modelPos[1] = animModel->GetTransform()->GetPosition().y;
                    modelPos[2] = animModel->GetTransform()->GetPosition().z;
                }
                if ( ImGui::DragFloat3( "##PositionModel", modelPos, 0.01f ) )
                    animModel->GetTransform()->SetPosition( XMFLOAT3( modelPos ) );

                ImGui::Text( "Rotation" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                static float modelRotation[3];
                if ( bRequiresUpdate )
                {
                    modelRotation[0] = animModel->GetTransform()->GetRotation().x;
                    modelRotation[1] = animModel->GetTransform()->GetRotation().y;
                    modelRotation[2] = animModel->GetTransform()->GetRotation().z;
                }
                if ( ImGui::DragFloat3( "##RotationModel", modelRotation ) )
                    animModel->GetTransform()->SetRotation( XMFLOAT3( modelRotation ) );

                ImGui::Text( "Scale" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                static float modelScale[3];
                if ( bRequiresUpdate )
                {
                    modelScale[0] = animModel->GetTransform()->GetScale().x;
                    modelScale[1] = animModel->GetTransform()->GetScale().y;
                    modelScale[2] = animModel->GetTransform()->GetScale().z;
                }
                if ( ImGui::DragFloat3( "##ScaleModel", modelScale, 0.01f ) )
                    animModel->GetTransform()->SetScale( XMFLOAT3( modelScale ) );

                bRequiresUpdate = false;
                ImGui::TreePop();
            }
            ImGui::Separator();

            if ( ImGui::TreeNode( "Bone Data##Node" ) )
            {
                if ( ImGui::BeginChild( "Bone List", ImVec2( 0, 250 ), true ) )
                {
                    if ( ImGui::BeginTable( "Bone Data##Table", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchSame ) )
                    {
                        ImGui::TableSetupColumn( "ID" );
                        ImGui::TableSetupColumn( "Parent" );
                        ImGui::TableSetupColumn( "Child" );
					    ImGui::TableHeadersRow();

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
                }
                ImGui::EndChild();

                static int boneNum = 0;
                static int boneNumPrev = -1;
                static float bonePos[3];
                static float boneScale[3];
                ImGui::Text( "Bone Number" );
                ImGui::InputInt( "##Bone Num", &boneNum );
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

                ImGui::Text( "Position" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                ImGui::DragFloat3( "##Bone Position", bonePos, 0.01f );

                ImGui::Text( "Rotation" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                static float boneRotation[3];
                ImGui::DragFloat3( "##Bone Rotation", boneRotation );

                ImGui::Text( "Scale" );
                ImGui::SameLine();
                HelpMarker( DRAG_HINT_TEXT );
                ImGui::DragFloat3( "##Bone Scale", boneScale, 0.01f );

                if ( ImGui::Button( "Apply Changes" ) )
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

                ImGui::TreePop();
            }
            ImGui::Separator();

            if ( ImGui::TreeNode( "Animation Data" ) )
            {
                // Set animation
                static std::vector<std::string> animationName = animModel->GetSkeleton()->AnimationClips();
                static const char* cCurrentItem = animationName[0].c_str();
                static int selectedIdx = 0;

                ImGui::Text( "Animation Clip" );
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
                                animModel->SetAnimation( cCurrentItem );
                                ImGui::SetItemDefaultFocus();
                                selectedIdx = n;
                            }
                        }
                    }
                    ImGui::EndCombo();
                }

                if ( animationName[selectedIdx] == "Walk" )
                {
                    static bool isLoop = animModel->GetIsLoop();
                    if ( ImGui::Checkbox( "Loop Animation?", &isLoop ) )
                        animModel->SetIsLoop( isLoop );

                    if ( !isLoop )
                    {
                        ImGui::Text( "Animation Time Position" );
                        ImGui::SameLine();
                        HelpMarker( DRAG_HINT_TEXT );
                        float timePos = animModel->GetTimePos();
                        if ( ImGui::DragFloat( "##Animation Time Position", &timePos, 0.001f ) )
                        animModel->SetTimePos( timePos );
                    }

                    // Current animation data
                    if ( ImGui::TreeNode( "Animation Info" ) )
                    {
                        if ( ImGui::BeginTable( "##Animation Info", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame ) )
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text( "Animation Start" );

                            ImGui::TableNextColumn();
                            std::string sStart = std::format( "{:.3f}", animModel->GetSkeleton()->GetClipStartTime( animModel->GetClipName() ) );
                            ImGui::Text( sStart.c_str() );

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text( "Animation End" );

                            ImGui::TableNextColumn();
                            std::string sEnd = std::format( "{:.3f}", animModel->GetSkeleton()->GetClipEndTime( animModel->GetClipName() ) );
                            ImGui::Text( sEnd.c_str() );

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text( "Animation Time" );

                            ImGui::TableNextColumn();
                            std::string sTime = std::format( "{:.3f}", animModel->GetTimePos() );
                            ImGui::Text( sTime.c_str() );

                            ImGui::EndTable();
                        }
                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }
            ImGui::Separator();

            static bool bLoad = false;
            static ImGuizmoData imguizmoData = ImGuizmoData();
            if ( !bLoad )
            {
                imguizmoData.ID = m_iGizmoID;
                m_iGizmoID++;
                bLoad = true;
            }
            static XMFLOAT4X4 world = animModel->GetTransform()->GetWorldMatrix();
            SpawnImGuizmo( animModel->GetTransform(), pCamera, world, imguizmoData );
		}
		else
		{
			animModel->Hide();
		}
    }
    ImGui::End();
}

void ImGuiManager::SpawnImGuizmo( Transform* pTransform, Camera* pCamera, XMFLOAT4X4& worldMat, ImGuizmoData& imguizmoData )
{
    // Setup the ImGuizmo
    static int idOffset = 100;
    ImGuizmo::SetID( imguizmoData.ID );
    ImGuizmo::SetRect( m_vSceneWindowPos.x, m_vSceneWindowPos.y, m_vSceneWindowSize.x, m_vSceneWindowSize.y );

    if ( ImGui::TreeNode( std::string( "Gizmo Controls###" ).append( std::to_string( imguizmoData.ID ) ).c_str() ) )
    {
        ImGui::PushID( imguizmoData.ID + idOffset * 6 );
        ImGui::Checkbox( std::string( "Visible?###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), &imguizmoData.IsVisible );
        ImGui::PopID();

        if ( imguizmoData.IsVisible )
        {
            ImGui::Text( "Gizmo Operation" );
            if ( ImGui::BeginTable( "Gizmo Operation###Table", 3, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchSame ) )
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID( imguizmoData.ID + idOffset );
                if ( ImGui::RadioButton( std::string( "Translate###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), imguizmoData.CurrentGizmoOperation == ImGuizmo::TRANSLATE ) )
                    imguizmoData.CurrentGizmoOperation = ImGuizmo::TRANSLATE;
                ImGui::PopID();

                ImGui::TableNextColumn();
                ImGui::PushID( imguizmoData.ID + idOffset * 2 );
                if ( ImGui::RadioButton( std::string( "Rotate###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), imguizmoData.CurrentGizmoOperation == ImGuizmo::ROTATE ) )
				    imguizmoData.CurrentGizmoOperation = ImGuizmo::ROTATE;
                ImGui::PopID();

                ImGui::TableNextColumn();
                ImGui::PushID( imguizmoData.ID + idOffset * 3 );
                if ( ImGui::RadioButton( std::string( "Scale###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), imguizmoData.CurrentGizmoOperation == ImGuizmo::SCALE ) )
                    imguizmoData.CurrentGizmoOperation = ImGuizmo::SCALE;
                ImGui::PopID();

                ImGui::EndTable();
            }

            if ( imguizmoData.CurrentGizmoOperation != ImGuizmo::SCALE )
            {
                ImGui::Text( "Gizmo Mode" );
                if ( ImGui::BeginTable( "Gizmo Mode###Table", 3, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchSame ) )
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::PushID( imguizmoData.ID + idOffset * 4 );
                    if ( ImGui::RadioButton( std::string( "World###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), imguizmoData.CurrentGizmoMode == ImGuizmo::WORLD ) )
                        imguizmoData.CurrentGizmoMode = ImGuizmo::WORLD;
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    ImGui::PushID( imguizmoData.ID + idOffset * 5 );
                    if ( ImGui::RadioButton( std::string( "Local###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), imguizmoData.CurrentGizmoMode == ImGuizmo::LOCAL ) )
                        imguizmoData.CurrentGizmoMode = ImGuizmo::LOCAL;
                    ImGui::PopID();

                    ImGui::NextColumn();
                    // Extra column to better fit radio buttons

                    ImGui::EndTable();
                }
            }

            ImGui::Text( "Gizmo Options" );
            if ( ImGui::BeginTable( "Gizmo Options###", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp ) )
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID( imguizmoData.ID + idOffset * 7 );
                if ( ImGui::Checkbox( std::string( "Enable?###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), &imguizmoData.Enable ) )
                    ImGuizmo::Enable( imguizmoData.Enable );
                ImGui::PopID();

                ImGui::TableNextColumn();
                ImGui::PushID( imguizmoData.ID + idOffset * 8 );
                ImGui::Checkbox( std::string( "Use Snapping?###" ).append( std::to_string( imguizmoData.ID ) ).c_str(), &imguizmoData.UseSnap );
                ImGui::PopID();

                ImGui::EndTable();
            }

            if ( imguizmoData.UseSnap )
            {
                ImGui::Text( "Snap Amount" );
                ImGui::SameLine();
                HelpMarker( SLIDER_HINT_TEXT );
                ImGui::PushID( imguizmoData.ID + idOffset * 9 );
                ImGui::SliderFloat( std::string( "###Snap" ).append( std::to_string( imguizmoData.ID ) ).c_str(), &imguizmoData.SnapAmount.x, 0.0f, 2.5f );
                ImGui::PopID();
            }
        }

        ImGui::TreePop();
    }

    if ( imguizmoData.IsVisible )
    {
        // Decompose/recompose matrix
        float* worldPtr = (float*)&worldMat;
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents( worldPtr, matrixTranslation, matrixRotation, matrixScale );
        ImGuizmo::RecomposeMatrixFromComponents( matrixTranslation, matrixRotation, matrixScale, worldPtr );

        // Handle manipulation of the gizmo
        XMFLOAT4X4 view = pCamera->GetView();
        float* viewPtr = (float*)&view;
        XMFLOAT4X4 projection = pCamera->GetProjection();
        float* projectionPtr = (float*)&projection;

        if ( ImGuizmo::Manipulate( viewPtr, projectionPtr, imguizmoData.CurrentGizmoOperation, imguizmoData.CurrentGizmoMode, worldPtr, NULL, imguizmoData.UseSnap ? &imguizmoData.SnapAmount.x : NULL ) )
        {
            // Update object parameters
            XMFLOAT3 pos = XMFLOAT3( matrixTranslation[0], matrixTranslation[1], matrixTranslation[2] );
            pTransform->SetPosition( pos );

            XMFLOAT3 rot = XMFLOAT3( matrixRotation[0], matrixRotation[1], matrixRotation[2] );
            pTransform->SetRotation( rot );

            XMFLOAT3 scale = XMFLOAT3( matrixScale[0], matrixScale[1], matrixScale[2] );
            pTransform->SetScale( scale );
        }
    }

    // Handle ImGuizmo operation for when using multiple manipulators
    if ( ImGuizmo::IsUsing() )
        m_iActiveGizmoID = imguizmoData.ID;

    if ( m_iActiveGizmoID == imguizmoData.ID )
    {
        if ( imguizmoData.CurrentGizmoOperation == ImGuizmo::TRANSLATE )
        {
            m_bUsingTranslation = true;
            m_bUsingManipulator = false;
        }
        else if ( imguizmoData.CurrentGizmoOperation == ImGuizmo::ROTATE )
        {
            m_bUsingTranslation = false;
            m_bUsingManipulator = true;
            if ( !ImGuizmo::IsUsing() )
            {
                m_bUsingTranslation = true;
                m_bUsingManipulator = false;
            }
        }
        else if ( imguizmoData.CurrentGizmoOperation == ImGuizmo::SCALE )
        {
            m_bUsingTranslation = false;
            m_bUsingManipulator = true;
            if ( !ImGuizmo::IsUsing() )
            {
                m_bUsingTranslation = true;
                m_bUsingManipulator = false;
            }
        }
    }
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