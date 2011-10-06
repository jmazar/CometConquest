#include "std.h"
#include "GameApp.h"

INT WINAPI GameMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR	  lpCmdLine,
					int		  nCmdShow)
{
	DXUTSetCallbackFrameMove( GameApp::OnFrameMove );
	
    DXUTSetCallbackD3D11DeviceCreated( GameApp::OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( GameApp::OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( GameApp::OnD3D11FrameRender );
    
    DXUTSetCallbackD3D11DeviceDestroyed( GameApp::OnD3D11DestroyDevice );
	DXUTSetCallbackMsgProc( GameApp::MsgProc );
	 DXUTSetCursorSettings( true, true );
    
	// Perform application initialization
	if (!g_pApp->InitInstance (hInstance, lpCmdLine)) 
	{
		return FALSE;
	}

    // Pass control to the sample framework for handling the message pump and 
    // dispatching render calls. The sample framework will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.

	DXUTMainLoop();
	DXUTShutdown();

    return g_pApp->GetExitCode();	
}