//--------------------------------------------------------------------------------------
// File: main.cpp
//
// This application demonstrates animation using matrix transformations
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729722.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
//#define _XM_NO_INTRINSICS_

#include "main.h"
#include"Application.h"



//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------




//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
Application* theApp;
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );


    theApp = new Application();

    if (FAILED(theApp->Initialise(hInstance, nCmdShow)))
    {
        return -1;
    }



    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {

            bool handled = false;

            if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
            {
                handled = theApp->InputControll(msg);
            }
            if (msg.message >= WM_MOUSEFIRST && msg.message < WM_MOUSELAST) {
                handled = theApp->InputControll(msg);
            }
            if (msg.message == WM_INPUT) {
                handled = theApp->InputControll(msg);
            }


            else if (WM_QUIT == msg.message)
                break;

            if (!handled)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            theApp->Update();
            theApp->Draw();
        }
    }

    delete theApp;
    theApp = nullptr;

    return ( int )msg.wParam;
}




