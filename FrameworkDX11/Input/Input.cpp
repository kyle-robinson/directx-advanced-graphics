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
    if ( m_pCameraControl->GetCurrentCam()->GetName() == "Spot Camera" )
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
                    m_pCameraControl->GetCurrentCam()->AdjustRotation(
                        static_cast<float>( me.GetPosY() ) * 0.005f,
                        static_cast<float>( me.GetPosX() ) * 0.005f,
                        0.0f
                    );
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
                XMFLOAT3(
                    XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetUpVector() ),
                    XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetUpVector() ),
                    XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetUpVector() ) )
                :
                XMFLOAT3(
                    XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetForwardVector() ),
                    XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetForwardVector() ),
                    XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetForwardVector() ) );
            m_pCameraControl->GetCurrentCam()->AdjustPosition( {
                moveVec.x* m_pCameraControl->GetCurrentCam()->GetSpeed(),
                moveVec.y* m_pCameraControl->GetCurrentCam()->GetSpeed(),
                moveVec.z * m_pCameraControl->GetCurrentCam()->GetSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( 'S' ) )
        {
            XMFLOAT3 moveVec = m_bUsingSpotCamera ?
                XMFLOAT3(
                    XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetDownVector() ),
                    XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetDownVector() ),
                    XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetDownVector() ) )
                :
                XMFLOAT3(
                    XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetBackwardVector() ),
                    XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetBackwardVector() ),
                    XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetBackwardVector() ) );

            m_pCameraControl->GetCurrentCam()->AdjustPosition( {
                moveVec.x* m_pCameraControl->GetCurrentCam()->GetSpeed(),
                moveVec.y* m_pCameraControl->GetCurrentCam()->GetSpeed(),
                moveVec.z * m_pCameraControl->GetCurrentCam()->GetSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( 'D' ) )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPosition( {
                XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetRightVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetRightVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetRightVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( 'A' ) )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPosition( {
                XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetLeftVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetLeftVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetLeftVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( VK_CONTROL ) && !m_bUsingSpotCamera )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPosition( {
                XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetDownVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetDownVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetDownVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed()
            } );
        }
        if ( m_keyboard.KeyIsPressed( VK_SPACE ) && !m_bUsingSpotCamera )
        {
            m_pCameraControl->GetCurrentCam()->AdjustPosition( {
                XMVectorGetX( m_pCameraControl->GetCurrentCam()->GetUpVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetY( m_pCameraControl->GetCurrentCam()->GetUpVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed(),
                XMVectorGetZ( m_pCameraControl->GetCurrentCam()->GetUpVector() ) * m_pCameraControl->GetCurrentCam()->GetSpeed()
            } );
        }
    }
}