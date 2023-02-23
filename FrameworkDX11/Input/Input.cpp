#include "stdafx.h"
#include "Input.h"

Input::Input()
{
    // Update m_keyboard processing
    m_keyboard.DisableAutoRepeatKeys();
    m_keyboard.DisableAutoRepeatChars();
}

Input::~Input() { }

void Input::AddCamControl( CameraController* cam )
{
    m_pCameraControl = cam;
}

void Input::Update( float dt )
{
    UpdateMouse( dt );
    UpdateKeyboard( dt );
}

void Input::UpdateMouse( float dt )
{
    // update camera orientation
    while ( !m_mouse.EventBufferIsEmpty() )
    {
        Mouse::MouseEvent me = m_mouse.ReadEvent();
        if ( m_mouse.IsRightDown() || !m_bCursorEnabled )
        {
            if ( me.GetType() == Mouse::MouseEvent::EventType::RawMove )
            {
                m_pCameraControl->GetCurentCam()->AgustRot( XMFLOAT3(
                    static_cast<float>( me.GetPosY() ) * 0.005f,
                    static_cast<float>( me.GetPosX() ) * 0.005f,
                    0.0f
                ) );
            }
        }
    }
}

void Input::UpdateKeyboard( float dt )
{
    // Handle input for single key presses
    while ( !m_keyboard.KeyBufferIsEmpty() )
    {
        Keyboard::KeyboardEvent kbe = m_keyboard.ReadKey();
        unsigned char keycode = kbe.GetKeyCode();

        // Set cursor enabled/disabled
        if ( keycode == VK_HOME )
            EnableCursor();
        else if ( keycode == VK_END )
            DisableCursor();
    }

    // Camera movement
    if ( m_keyboard.KeyIsPressed( 'W' ) )
    {
        m_pCameraControl->GetCurentCam()->AgustPos( {
            m_pCameraControl->GetCurentCam()->GetVecFord().x * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecFord().y * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecFord().z * m_pCameraControl->GetCurentCam()->GetCamSpeed()
        } );
    }
    if ( m_keyboard.KeyIsPressed( 'S' ) )
    {
        m_pCameraControl->GetCurentCam()->AgustPos( {
            m_pCameraControl->GetCurentCam()->GetVecBack().x * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecBack().y * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecBack().z * m_pCameraControl->GetCurentCam()->GetCamSpeed()
        } );
    }
    if ( m_keyboard.KeyIsPressed( 'D' ) )
    {
        m_pCameraControl->GetCurentCam()->AgustPos( {
            m_pCameraControl->GetCurentCam()->GetVecRight().x * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecRight().y * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecRight().z * m_pCameraControl->GetCurentCam()->GetCamSpeed()
        } );
    }
    if ( m_keyboard.KeyIsPressed( 'A' ) )
    {
        m_pCameraControl->GetCurentCam()->AgustPos( {
            m_pCameraControl->GetCurentCam()->GetVecLeft().x * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecLeft().y * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecLeft().z * m_pCameraControl->GetCurentCam()->GetCamSpeed()
        } );
    }
    if ( m_keyboard.KeyIsPressed( VK_CONTROL ) )
    {
        m_pCameraControl->GetCurentCam()->AgustPos( {
            m_pCameraControl->GetCurentCam()->GetVecDown().x * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecDown().y * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecDown().z * m_pCameraControl->GetCurentCam()->GetCamSpeed()
        } );
    }
    if ( m_keyboard.KeyIsPressed( VK_SPACE ) )
    {
        m_pCameraControl->GetCurentCam()->AgustPos( {
            m_pCameraControl->GetCurentCam()->GetVecUp().x * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecUp().y * m_pCameraControl->GetCurentCam()->GetCamSpeed(),
            m_pCameraControl->GetCurentCam()->GetVecUp().z * m_pCameraControl->GetCurentCam()->GetCamSpeed()
        } );
    }
}