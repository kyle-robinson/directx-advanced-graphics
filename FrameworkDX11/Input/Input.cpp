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
    // Update control scheme if using the spot camera
    if ( m_pCameraControl->GetCurrentCam()->GetCamName() == "Spot Camera" )
    {
		m_bUsingSpotCamera = true;
	}
    else
    {
		m_bUsingSpotCamera = false;
	}

    UpdateMouse( dt );
    UpdateKeyboard( dt );
}

void Input::UpdateMouse( float dt )
{
    // Update camera orientation
    while ( !m_mouse.EventBufferIsEmpty() )
    {
        Mouse::MouseEvent me = m_mouse.ReadEvent();
        if ( m_bAllowMouseInputs && !m_bUsingSpotCamera )
        {
            if ( m_mouse.IsRightDown() || !m_bCursorEnabled )
            {
                if ( me.GetType() == Mouse::MouseEvent::EventType::RawMove )
                {
                    m_pCameraControl->GetCurrentCam()->AdjustRot( XMFLOAT3(
                        static_cast<float>( me.GetPosY() ) * 0.005f,
                        static_cast<float>( me.GetPosX() ) * 0.005f,
                        0.0f
                    ) );
                }
                HideCursor();
                DisableImGuiMouse();
                m_bMovingCursor = true;
            }
            else if ( m_bMovingCursor )
            {
                ShowCursor();
                EnableImGuiMouse();
                m_bMovingCursor = false;
            }

            if ( me.GetType() == Mouse::MouseEvent::EventType::WheelUp )
            {
                if ( m_pCameraControl->GetCurrentCam()->GetFov() > 50.0f )
                {
                    m_pCameraControl->GetCurrentCam()->AdjustFov( -1.0f );
                }
            }
            else if ( me.GetType() == Mouse::MouseEvent::EventType::WheelDown )
            {
                if ( m_pCameraControl->GetCurrentCam()->GetFov() < 110.0f )
                {
				    m_pCameraControl->GetCurrentCam()->AdjustFov( 1.0f );
                }
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
        if ( m_bAllowKeyboardInputs )
        {

        }
    }

    if ( m_bAllowKeyboardInputs )
    {
        // Camera movement
        if ( m_keyboard.KeyIsPressed( 'W' ) )
        {
            XMFLOAT3 moveVec = m_bUsingSpotCamera ?
                m_pCameraControl->GetCurrentCam()->GetVecUp() :
                m_pCameraControl->GetCurrentCam()->GetVecForward();
            m_pCameraControl->GetCurrentCam()->AdjustPos( {
                moveVec.x* m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                moveVec.y* m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                moveVec.z * m_pCameraControl->GetCurrentCam()->GetCamSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( 'S' ) )
        {
            XMFLOAT3 moveVec = m_bUsingSpotCamera ?
                m_pCameraControl->GetCurrentCam()->GetVecDown() :
                m_pCameraControl->GetCurrentCam()->GetVecBack();
            m_pCameraControl->GetCurrentCam()->AdjustPos( {
                moveVec.x* m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                moveVec.y* m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                moveVec.z * m_pCameraControl->GetCurrentCam()->GetCamSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( 'D' ) )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPos( {
                m_pCameraControl->GetCurrentCam()->GetVecRight().x * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecRight().y * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecRight().z * m_pCameraControl->GetCurrentCam()->GetCamSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( 'A' ) )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPos( {
                m_pCameraControl->GetCurrentCam()->GetVecLeft().x * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecLeft().y * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecLeft().z * m_pCameraControl->GetCurrentCam()->GetCamSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( VK_CONTROL ) && !m_bUsingSpotCamera )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPos( {
                m_pCameraControl->GetCurrentCam()->GetVecDown().x * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecDown().y * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecDown().z * m_pCameraControl->GetCurrentCam()->GetCamSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( VK_SPACE ) && !m_bUsingSpotCamera )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPos( {
                m_pCameraControl->GetCurrentCam()->GetVecUp().x * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecUp().y * m_pCameraControl->GetCurrentCam()->GetCamSpeed(),
                m_pCameraControl->GetCurrentCam()->GetVecUp().z * m_pCameraControl->GetCurrentCam()->GetCamSpeed()
            } );
        }
    }
}