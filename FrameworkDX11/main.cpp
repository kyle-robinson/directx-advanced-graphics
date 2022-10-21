#include "stdafx.h"
#include "Application.h"

// Functions
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Application theApp;
    theApp.InitWindow(hInstance, nCmdShow);

    if (FAILED(theApp.InitDevice()))
    {
        theApp.CleanupDevice();
        return 0;
    }

    // Main message loop
    while ( theApp.ProcessMessages() == true )
    {
        theApp.Update();
        theApp.Render();
    }
    theApp.CleanupDevice();

    return 0;
}