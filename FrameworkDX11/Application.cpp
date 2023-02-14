#include "stdafx.h"
#include "Application.h"
#include "TextRenderer.h"
#include <imgui/imgui.h>

#include "Level1.h"
#include "Level2.h"
#include "Level3.h"

bool Application::Initialize( HINSTANCE hInstance, int width, int height )
{
    try
    {
        // Initialize window
        if ( !renderWindow.Initialize( &m_input, hInstance, "DirectX 11 Advanced Graphics & Rendering", "TutorialWindowClass", width, height ) )
		    return false;

        // Initialize graphics
        if ( !graphics.Initialize( renderWindow.GetHWND(), width, height ) )
		    return false;

        // Initialize input
        m_camera.Initialize( XMFLOAT3( 0.0f, 0.0f, -3.0f ), width, height );
        m_input.Initialize( renderWindow, m_camera );
        m_imgui.Initialize( renderWindow.GetHWND(), graphics.GetDevice(), graphics.GetContext() );

        // Initialize systems
        TextRenderer::GetInstance()->Initialize( "open_sans_ms_16_bold.spritefont", graphics.GetDevice(), graphics.GetContext() );

        // Create levels
        std::shared_ptr<Level1> level1 = std::make_shared<Level1>( "Level 1" );
        level1->Initialize( &graphics, &m_camera, &m_imgui );
        m_pLevels.push_back( std::move( level1 ) );

        std::shared_ptr<Level2> level2 = std::make_shared<Level2>( "Level 2" );
        level2->Initialize( &graphics, &m_camera, &m_imgui );
        m_pLevels.push_back( std::move( level2 ) );

        std::shared_ptr<Level3> level3 = std::make_shared<Level3>( "Level 3" );
        level3->Initialize( &graphics, &m_camera, &m_imgui );
        m_pLevels.push_back( std::move( level3 ) );

        for ( unsigned int i = 0; i < m_pLevels.size(); i++ )
            m_sLevelNames.push_back( m_stateMachine.Add( m_pLevels[i] ) );

        m_stateMachine.SwitchTo( m_sLevelNames[0] );
        m_sCurrentLevelName = m_sLevelNames[0];
    }
    catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}

    return true;
}

void Application::CleanupDevice()
{
#ifdef _DEBUG
    // Useful for finding dx memory leaks
    ID3D11Debug* debugDevice = nullptr;
    graphics.GetDevice()->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &debugDevice ) );
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
    if ( debugDevice ) debugDevice->Release();
#endif
}

bool Application::ProcessMessages() noexcept
{
    // Process messages sent to the window
	return renderWindow.ProcessMessages();
}

void Application::Update()
{
    // Update delta time
    float dt = m_timer.GetDeltaTime(); // capped at 60 fps
    if ( dt == 0.0f ) return;

    // Update input
    m_input.Update( dt );
    if ( windowResized )
    {
        m_imgui.Initialize( renderWindow.GetHWND(), graphics.GetDevice(), graphics.GetContext() );
        m_camera.SetProjectionValues( 75.0f,
            static_cast<float>( graphics.GetWidth() ) /
            static_cast<float>( graphics.GetHeight() ),
            0.01f, 100.0f );
    }

    m_stateMachine.Update( dt );
}

void Application::Render()
{
    m_stateMachine.Render_Start();

    if ( ImGui::Begin( "Level Selection", FALSE, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove ) )
    {
        static int levelIndex = 0;
        static bool shouldSwitchLevel = false;

        ImGui::Text( "Active Level: " );
        ImGui::SameLine();
        ImGui::TextColored( ImVec4( 1.0f, 0.6f, 0.0f, 1.0f ), m_stateMachine.GetCurrentLevel()->GetLevelName().c_str() );
        ImGui::NewLine();

        ImGui::Text( "Level List" );
        if ( ImGui::BeginListBox( "##Level List", ImVec2( -FLT_MIN, m_pLevels.size() * ImGui::GetTextLineHeightWithSpacing() * 1.1f ) ) )
        {
            int index = 0;
            for ( unsigned int i = 0; i < m_pLevels.size(); i++ )
            {
                const bool isSelected = ( m_sCurrentLevelName == m_pLevels[index]->GetLevelName() );
                if ( ImGui::Selectable( m_pLevels[i]->GetLevelName().c_str(), isSelected ) )
                {
                    levelIndex = index;
                    m_sCurrentLevelName = m_pLevels[index]->GetLevelName();
                    if ( m_sCurrentLevelName == m_sLevelNames[index] )
                    {
                        shouldSwitchLevel = true;
                        break;
                    }
                }

                if ( isSelected )
                    ImGui::SetItemDefaultFocus();

                index++;
            }
            ImGui::EndListBox();
        }

        ImGui::NewLine();
        if ( ImGui::Button( "Switch To" ) && shouldSwitchLevel )
        {
            m_stateMachine.SwitchTo( m_sCurrentLevelName );
            shouldSwitchLevel = false;
        }
    }
    ImGui::End();

    m_stateMachine.Render_End();
}