#include"Application.h"

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
            TranslateMessage(&msg);
            DispatchMessage(&msg);
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