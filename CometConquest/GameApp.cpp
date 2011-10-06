#include "std.h"
#include "GameApp.h"

#include "EventManager\EventManagerImpl.h"
#include "Network\Network.h"
#include "MiscStuff\CMath.h"
#include "MiscStuff\String.h"
#include "MainLoop\Initialization.h"
#include "Network\Network.h"
#include "EventManager\Events.h"
#include "Physics\PhysicsEventListener.h"


GameApp *g_pApp = NULL;

CDXUTDialogResourceManager GameApp::g_DialogResourceManager;

GameApp::GameApp()
{
	g_pApp = this;

	m_rcWindow.bottom = m_rcWindow.left = m_rcWindow.right = m_rcWindow.top = 0;
	m_rcDesktop.bottom = m_rcDesktop.left = m_rcDesktop.right = m_rcDesktop.top = 0;
	m_iColorDepth = 32;

	m_pEventManager = NULL;

	m_bQuitRequested = false;
	m_bQuitting = false;
}

HWND GameApp::GetHwnd()
{

	return DXUTGetHWND();
}

bool GameApp::InitInstance(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hWnd, int screenWidth, int screenHeight)
{
	SetCursor( NULL );

	m_hInstance = hInstance;

	m_pOptions = GCC_NEW GameOptions("m_pOptions->ini");
	ParseCommandLine(lpCmdLine);

	m_pEventManager = GCC_NEW EventManager("CometConquest Event Mgr", true );
	if (!m_pEventManager)
	{
		return false;
	}

	RegisterBaseGameEvents();

	DXUTInit( true, true, NULL );

	if(hWnd==NULL)
	{
		DXUTCreateWindow( VGetGameTitle(), hInstance, VGetIcon() );
	}
	else
	{
		DXUTSetWindow( hWnd, hWnd, hWnd);
	}

	if(!GetHwnd())
	{
		return false;
	}
	SetWindowText(GetHwnd(), VGetGameTitle());

	DXUTCreateDevice (D3D_FEATURE_LEVEL_9_2, true, 800, 600 );

	m_pGame = VCreateGameAndView();
	if (!m_pGame)
		return false;

	m_bIsRunning = true;

	return true;

}

void GameApp::ParseCommandLine(LPWSTR lpCmdLine)
{
	WCHAR *playersCmd = _T("-players");
	WCHAR *gameHostCmd = _T("-gamehost");
	WCHAR *listenCmd = _T("-listen");
	WCHAR *c;
	WCHAR temp[_MAX_PATH];

	if (c=_tcsstr(lpCmdLine, playersCmd))
	{
		_tcscpy(temp, c);
		c = temp;
		c = _tcstok(temp, _T(" "));		// get -players command
		c = _tcstok(NULL, _T(" "));
		int players = _tstoi(c);
		if (players>6 || players<1)
		{
			assert(0 && "Invalid number of players on the command line");
			players = 1;
		}

		m_pOptions->m_expectedPlayers = players;
	}

	if (c=_tcsstr(lpCmdLine, gameHostCmd))
	{
		_tcscpy(temp, c);
		c = temp;
		c = _tcstok(temp, _T(" "));		// get -hostname command
		c = _tcstok(NULL, _T(" "));		// get to hostname
		if (c)
		{
			CHAR gameHost[256];
			GenericToAnsiCch(gameHost, c, static_cast<int>(_tcslen(c)+1));
			m_pOptions->m_gameHost = gameHost;
		}
		c = _tcstok(NULL, _T(" "));		// get to port number
		if (c)
		{
			int listenPort = _tstoi(c);
			if (listenPort>0 && listenPort<65535)
			{
				// the game options already has the default port set...
				m_pOptions->m_listenPort = listenPort;
			}
		}		
		assert(m_pOptions->m_listenPort!=-1 && _T("Must set server port in command line"));
	}

	if (c=_tcsstr(lpCmdLine, listenCmd))
	{
		_tcscpy(temp, c);
		c = temp;
		c = _tcstok(temp, _T(" "));		// get -listed command
		c = _tcstok(NULL, _T(" "));		// get to port number
		int listenPort = _tstoi(c);
		if (listenPort>0 && listenPort<65535)
		{
			m_pOptions->m_listenPort = listenPort;
		}
	}


}

LRESULT CALLBACK GameApp::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext )
{
	*pbNoFurtherProcessing = g_pApp->g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
	LRESULT result = 0;

	switch (uMsg)
	{

	case WM_POWERBROADCAST:
		{
			int event = (int)wParam;
			result = g_pApp->OnPowerBroadcast(event);
			break;
		}

	case WM_DISPLAYCHANGE:
		{
			int colorDepth = (int)wParam;
			int width = (int)(short) LOWORD(lParam);
			int height = (int)(short) HIWORD(lParam);

			result = g_pApp->OnDisplayChange(colorDepth, width, height);
			break;
		}

	case WM_SYSCOMMAND: 
		{
			result = g_pApp->OnSysCommand(wParam, lParam);
			if (result)
			{
				*pbNoFurtherProcessing = true;
			}
			break;
		}

	case WM_SYSKEYDOWN:
		{
			if (wParam == VK_RETURN)
			{
				*pbNoFurtherProcessing = true;
				return g_pApp->OnAltEnter();
			}
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

	case WM_CLOSE:
		{
			// DXUT apps choose ESC key as a default exit command.
			// GameCode3 doesn't like this so we disable it by checking 
			// the m_bQuitting bool, and if we're not really quitting
			// set the "no further processing" parameter to true.
			if (g_pApp->m_bQuitting)
			{
				result = g_pApp->OnClose();
			}
			else
			{
				*pbNoFurtherProcessing = true;
			}
			break;
		}
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case MM_JOY1BUTTONDOWN:
	case MM_JOY1BUTTONUP:
	case MM_JOY1MOVE:
	case MM_JOY1ZMOVE:
	case MM_JOY2BUTTONDOWN:
	case MM_JOY2BUTTONUP:
	case MM_JOY2MOVE:
	case MM_JOY2ZMOVE:
		{
			//
			// See Chapter 9, page 265 for more explanation of this code.
			//
			if (g_pApp->m_pGame)
			{
				BaseGameLogic *pGame = g_pApp->m_pGame;
				// Note the reverse order! User input is grabbed first from the view that is on top, 
				// which is the last one in the list.
				AppMsg msg;
				msg.m_hWnd = hWnd;
				msg.m_uMsg = uMsg;
				msg.m_wParam = wParam;
				msg.m_lParam = lParam;
				for(GameViewList::reverse_iterator i=pGame->m_gameViews.rbegin(); i!=pGame->m_gameViews.rend(); ++i)
				{
					if ( (*i)->VOnMsgProc( msg ) )
					{
						result = true;
						break;				// WARNING! This breaks out of the for loop.
					}
				}
			}
			break;
		}
	}

	return result;
}

LRESULT GameApp::OnDisplayChange(int colorDepth, int width, int height)
{
	m_rcDesktop.left = 0;
	m_rcDesktop.top = 0; 
	m_rcDesktop.right = width;
	m_rcDesktop.bottom = height;
	m_iColorDepth = colorDepth;
	return 0;
}

LRESULT GameApp::OnPowerBroadcast(int event)
{
	// Don't allow the game to go into sleep mode
	if ( event == PBT_APMQUERYSUSPEND )
		return BROADCAST_QUERY_DENY;
	else if ( event == PBT_APMBATTERYLOW )
	{
		AbortGame();
	}

	return true;
}

LRESULT GameApp::OnSysCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case SC_MAXIMIZE :
		{
			// If windowed and ready...
			if ( m_bWindowedMode && IsRunning() )
			{
				// Make maximize into FULLSCREEN toggle
				OnAltEnter();
			}
		}
		return 0;

	case SC_CLOSE :
		{
			// The quit dialog confirmation would appear once for
			// every SC_CLOSE we get - which happens multiple times
			// if modal dialogs are up.  This now uses the QUIT_NO_PROMPT
			// flag to only prompt when receiving a SC_CLOSE that isn't
			// generated by us (identified by QUIT_NO_PROMPT).

			// If closing, prompt to close if this isn't a forced quit
			if ( lParam != QUIT_NO_PROMPT )
			{
				// ET - 05/21/01 - Bug #1916 - Begin
				// We were receiving multiple close dialogs
				// when closing again ALT-F4 while the close
				// confirmation dialog was up.
				// Eat if already servicing a close
				if ( m_bQuitRequested )
					return true;

				// Wait for the application to be restored
				// before going any further with the new 
				// screen.  Flash until the person selects
				// that they want to restore the game, then
				// reinit the display if fullscreen.  
				// The reinit is necessary otherwise the game
				// will switch to windowed mode.

				// TODO MrMike: Need a message eater, message saver
				//if (postableMessageBacklog.valid())
				//{
				//	postableMessageBacklog->Add( PostableMessage(WM_SYSCOMMAND, wParam, MAKELPARAM(0, 0) ) );
				//	return true;
				//}

				// Quit requested
				m_bQuitRequested = true;

			}

			m_bQuitting = true;


			// Reset the quit after any other dialogs have popped up from this close
			m_bQuitRequested = false;
		}
		return 0;

	default:
		// return non-zero of we didn't process the SYSCOMMAND message
		return DefWindowProc(GetHwnd(), WM_SYSCOMMAND, wParam, lParam);
	}

	return 0;
}

LRESULT GameApp::OnClose()
{
	// release all the game systems in reverse order from which they were created

	SAFE_DELETE(m_pGame);

	DestroyWindow(GetHwnd());

	SAFE_DELETE(m_pBaseSocketManager);

	SAFE_DELETE(m_pEventManager);

	SAFE_DELETE(m_pOptions);



	return 0;
}

LRESULT GameApp::OnAltEnter()
{
	DXUTToggleFullScreen();
	return 0;
}

void CALLBACK GameApp::OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	if (g_pApp->m_bQuitting)
	{
		PostMessage(g_pApp->GetHwnd(), WM_CLOSE, 0, 0);		
	}

	if (g_pApp->m_pGame)
	{
		safeTickEventManager( 20 ); // allow event queue to process for up to 20 ms

		if (g_pApp->m_pBaseSocketManager)
			g_pApp->m_pBaseSocketManager->DoSelect(0);	// pause 0 microseconds

		g_pApp->m_pGame->VOnUpdate(fTime, fElapsedTime);
	}
}

void CALLBACK GameApp::OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext )
{
	BaseGameLogic *pGame = g_pApp->m_pGame;

	for(GameViewList::iterator i=pGame->m_gameViews.begin(),
		end=pGame->m_gameViews.end(); i!=end; ++i)
	{
		(*i)->VOnRender(fTime, fElapsedTime);
	}

}

HRESULT CALLBACK GameApp::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext )
{
	HRESULT hr;
	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_pApp->g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	return S_OK;
}

HRESULT CALLBACK GameApp::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

    V_RETURN( g_pApp->g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

	return S_OK;
}

void CALLBACK GameApp::OnD3D11DestroyDevice( void* pUserContext )
{
	g_pApp->g_DialogResourceManager.OnD3D11DestroyDevice();
}

void GameApp::RegisterBaseGameEvents( void )
{
	if ( NULL == m_pEventManager )
	{
		assert( 0 && "Cannot register base game events until the event manager exists!" );
		return;
	}

	//Physics Events
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_PhysTrigger_Enter::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_PhysTrigger_Leave::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_PhysCollision::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_PhysSeparation::sk_EventType );

	//Actor Events
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_New_Actor::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Destroy_Actor::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Move_Actor::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Game_State::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Remote_Client::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Network_Player_Actor_Assignment::sk_EventType );

	//General Game Events


}

////////////
// BaseGameLogic
///////////

#include <MMSystem.h>

const unsigned int SCREEN_REFRESH_RATE(1000/60);


BaseGameLogic::BaseGameLogic(struct GameOptions const &options)
{
	m_LastActorId = 0;
	m_pProcessManager = GCC_NEW CProcessManager;
	m_random.Randomize();
	//should be BGS_Initializing
	m_State = BGS_Initializing;
	m_bProxy = false;
	//m_RenderDiagnostics = false;
	m_ExpectedPlayers = 0;
	m_ExpectedRemotePlayers = 0;
}

BaseGameLogic::~BaseGameLogic()
{
	// MrMike: 12-Apr-2009 
	//   Added this to explicity remove views from the game logic list.

	while (!m_gameViews.empty())
		m_gameViews.pop_front();

	SAFE_DELETE(m_pProcessManager);

	assert (m_ActorList.empty() && _T("You should destroy the actor list in the inherited class!") );
}

ActorId BaseGameLogic::GetRandomActor(optional<ActorId> ignoreMe)
{
	// [rez] This function is very lame, but it's past 5am and I have a deadline.  Welcome to gaming industry.  ;)

	// choose a random element
	unsigned int count = (unsigned int)m_ActorList.size();
	unsigned int choice = m_random.Random(count);

	// since this is a map, we have to walk the tree to find the item in question
	ActorMap::iterator it = m_ActorList.begin();
	for (unsigned int i = 0; i < choice; i++)
		++it;

	// If we're ignoring an actor, choose the next one.  incidentally, this makes the next actor the most likely 
	// choice to be chosen, although the player won't notice in a game of a dozen or so actors.
	if (ignoreMe.valid() && it->first == *ignoreMe)
	{
		++it;
		if (it == m_ActorList.end())
			it = m_ActorList.begin();
	}

	// found someone
	return it->first;
}

void BaseGameLogic::VAddActor(shared_ptr<IActor> actor, ActorParams *p)
{
	optional< ActorId > destActorID = p->m_Id;
	assert( destActorID.valid() && "Attempted to add an actor with no actor ID!" );
	m_ActorList[ *destActorID ] = actor;
	actor->VSetID( *destActorID );
}

void BaseGameLogic::VRemoveActor(ActorId aid)
{
	if (!m_bProxy)
	{
		if( NULL!=VGetActor(aid).get() )
		{
			m_pPhysics->VRemoveActor(aid);
			m_ActorList.erase(aid);
		}
		else
		{
			assert(0 && _T("Unknown actor!"));
		}
	}



	if( NULL!=VGetActor(aid).get() )
	{
		m_ActorList.erase(aid);
	}
	else
	{
		assert(0 && _T("Unknown actor!"));
	}


}


void BaseGameLogic::VMoveActor(const ActorId id, Mat4x4 const &mat)
{
	shared_ptr<IActor> pActor = VGetActor(id);
	if (pActor)
	{
		pActor->VSetMat(mat);
	}
}

shared_ptr<IActor> BaseGameLogic::VGetActor(const ActorId id)
{
	ActorMap::iterator i = m_ActorList.find(id);
	if (i==m_ActorList.end())
	{
		shared_ptr<IActor> null;
		return null;
	}

	return (*i).second;
}

void BaseGameLogic::VOnUpdate(float time, float elapsedTime)
{
	int deltaMilliseconds = int(elapsedTime * 1000.0f);

	switch(m_State)
	{
	case BGS_Initializing:
		// If we get to here we're ready to attach players
		//todo should be BGS_MainMenu but i dont have one!
		//VChangeState(BGS_WaitingForPlayers);
		VChangeState(BGS_MainMenu);
		break;

	case BGS_LoadingGameEnvironment:
		if (g_pApp->VLoadGame())
		{	
			VChangeState(BGS_Running);
		}
		else
		{
			assert(0 && _T("The game failed to load."));
			g_pApp->AbortGame();
		}
		break;

	case BGS_MainMenu:
		// do nothing
		break;

	case BGS_WaitingForPlayers:
		// do nothing - the game class should handle this one.
		break;

	case BGS_Running:
		m_pProcessManager->UpdateProcesses(deltaMilliseconds);
		break;

	default:
		assert(0 && _T("Unrecognized state."));
		// Not a bad idea to throw an exception here to 
		// catch this in a release build...
	}

	GameViewList::iterator i=m_gameViews.begin();
	GameViewList::iterator end=m_gameViews.end();
	while (i != end)
	{
		(*i)->VOnUpdate( deltaMilliseconds );
		++i;
	}

	//Trigger an update event for anybody else (script functions, etc.).

}


void BaseGameLogic::VChangeState(BaseGameState newState)
{
	if (newState==BGS_WaitingForPlayers)
	{
		// Get rid of the Main Menu...
		m_gameViews.pop_front();
		//todo
		// Note: Split screen support would require this to change!
		m_ExpectedPlayers = 1;
		m_ExpectedRemotePlayers = g_pApp->m_pOptions->m_expectedPlayers;

		if (!g_pApp->m_pOptions->m_gameHost.empty())
		{
			VSetProxy();
			m_ExpectedRemotePlayers = 0;	// the server will create these
			ClientSocketManager *pClient = GCC_NEW ClientSocketManager(g_pApp->m_pOptions->m_gameHost, g_pApp->m_pOptions->m_listenPort);
			if (!pClient->Connect())
			{
				// Throw up a main menu
				VChangeState(BGS_MainMenu);
				return;
			}
			g_pApp->m_pBaseSocketManager = pClient;
		}
		else if (m_ExpectedRemotePlayers > 0)
		{
			BaseSocketManager *pServer = GCC_NEW BaseSocketManager();
			if (!pServer->Init())
			{
				// Throw up a main menu
				VChangeState(BGS_MainMenu);	
				return;
			}

			pServer->AddSocket(new GameServerListenSocket(g_pApp->m_pOptions->m_listenPort));
			g_pApp->m_pBaseSocketManager = pServer;
		}
	}

	m_State = newState;
	if (!m_bProxy)
	{
		safeQueEvent( IEventDataPtr(GCC_NEW EvtData_Game_State(m_State)) );
	}
}

// Chapter 19/20 refactor work
//  -- refactor VBuildInitialScene into BaseGameLogic class
void BaseGameLogic::VBuildInitialScene()
{
	//Execute our startup script file first.
	//const bool bStartupScriptSuccess = g_pApp->m_pLuaStateManager->DoFile( "data\\Scripts\\startup.lua" );
	//if ( false == bStartupScriptSuccess )
	//{
	//	assert( 0 && "Unable to execute startup.lua!" );
	//}
}

void BaseGameLogic::VAddView(shared_ptr<IGameView> pView, optional<ActorId> actorId)
{
	if(m_bProxy)
	{
		m_gameViews.push_back(pView);
		pView->VOnRestore();
	}
	else
	{
		// This makes sure that all views have a non-zero view id.
		int viewId = static_cast<int>(m_gameViews.size());
		m_gameViews.push_back(pView);
		pView->VOnAttach(viewId, actorId);
		pView->VOnRestore();
	}
}


void BaseGameLogic::VRemoveView(shared_ptr<IGameView> pView)
{
	m_gameViews.remove(pView);
}

//
// HumanView::HumanView - Chapter 9
//
HumanView::HumanView()
{
	m_pProcessManager = GCC_NEW CProcessManager;

	m_pFont = NULL;         // Font for drawing text
	m_pTextSprite = NULL;   // Sprite for batching draw text calls
}

HumanView::~HumanView()
{
	while (!m_ScreenElements.empty())
	{
		m_ScreenElements.pop_front();		
	}

	SAFE_RELEASE( m_pTextSprite );
	SAFE_RELEASE( m_pFont );

	SAFE_DELETE(m_pProcessManager);

	// post press - I changed the g_pAudio pointer to a shared_ptr
	// since there can be multiple human views
}

void HumanView::VOnRender(double fTime, float fElapsedTime )
{
	m_currTick = timeGetTime();
	if (m_currTick == m_lastDraw)
		return;

	// It is time to draw ?
	if( (m_currTick - m_lastDraw) > SCREEN_REFRESH_RATE )
	{
		ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
		pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
		ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
		pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

		m_ScreenElements.sort(SortBy_SharedPtr_Content<IScreenElement>());

		for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
		{
			(*i)->VOnRender(fTime, fElapsedTime);
		}


		// record the last successful paint
		m_lastDraw = m_currTick;

	}
}

//
// HumanView::VOnRestore - Chapter 9, page 238
//
HRESULT HumanView::VOnRestore()
{
	HRESULT hr = S_OK;

	//   if( !m_pFont )
	//{
	//    // Initialize the font
	//    D3DXCreateFont( DXUTGetD3D9Device(), 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
	//                        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
	//                        L"Arial", &m_pFont );
	//}
	//else
	//{
	//       V_RETURN( m_pFont->OnResetDevice() );
	//}

	//if (!m_pTextSprite)
	//{
	//	// Create a sprite to help batch calls when drawing many lines of text
	//	V_RETURN( D3DXCreateSprite( DXUTGetD3D9Device(), &m_pTextSprite ) );
	//}
	//else
	//{
	//       V_RETURN( m_pTextSprite->OnResetDevice() );		
	//}

	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		V_RETURN ( (*i)->VOnRestore() );
	}

	return hr;
}


//
// HumanView::VOnLostDevice - Chapter 9, page 239
//
void HumanView::VOnLostDevice() 
{

	//if( m_pFont )
	//       m_pFont->OnLostDevice();
	//   SAFE_RELEASE( m_pTextSprite );
}

//
// HumanView::VOnMsgProc - Chapter 9, page 242
//
LRESULT CALLBACK HumanView::VOnMsgProc( AppMsg msg )
{
	// Iterate through the screen layers first
	// In reverse order since we'll send input messages to the 
	// screen on top
	for(ScreenElementList::reverse_iterator i=m_ScreenElements.rbegin(); i!=m_ScreenElements.rend(); ++i)
	{
		if ( (*i)->VIsVisible() )
		{
			if ( (*i)->VOnMsgProc( msg ) )
			{
				return 1;
			}
		}
	}

	LRESULT result = 0;
	switch (msg.m_uMsg) 
	{
	case WM_KEYDOWN:
		if (m_KeyboardHandler)
		{
			result = m_KeyboardHandler->VOnKeyDown(static_cast<const BYTE>(msg.m_wParam));
		}
		break;

	case WM_KEYUP:
		if (m_KeyboardHandler)
		{
			result = m_KeyboardHandler->VOnKeyUp(static_cast<const BYTE>(msg.m_wParam));
		}
		break;

	case WM_MOUSEMOVE:
		if (m_MouseHandler)
			result = m_MouseHandler->VOnMouseMove(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
		break;

	case WM_LBUTTONDOWN:
		if (m_MouseHandler)
		{
			SetCapture(msg.m_hWnd);
			result = m_MouseHandler->VOnLButtonDown(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
		}	
		break;

	case WM_LBUTTONUP:
		if (m_MouseHandler)
		{
			SetCapture(NULL);
			result = m_MouseHandler->VOnLButtonUp(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
		}
		break;

	case WM_RBUTTONDOWN:
		if (m_MouseHandler)
		{
			SetCapture(msg.m_hWnd);
			result = m_MouseHandler->VOnRButtonDown(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
		}
		break;

	case WM_RBUTTONUP:
		if (m_MouseHandler)
		{
			SetCapture(NULL);
			result = m_MouseHandler->VOnRButtonUp(CPoint(LOWORD(msg.m_lParam), HIWORD(msg.m_lParam)));
		}
		break;
	default:
		return 0;
	}

	return 0;
}

//
// HumanView::VOnRestore - Chapter 9, page 239
//
void HumanView::VOnUpdate( int deltaMilliseconds )
{
	m_pProcessManager->UpdateProcesses(deltaMilliseconds);

	// This section of code was added post-press. It runs through the screenlist
	// and calls VOnUpdate. Some screen elements need to update every frame, one 
	// example of this is a 3D scene attached to the human view.
	//
	for(ScreenElementList::iterator i=m_ScreenElements.begin(); i!=m_ScreenElements.end(); ++i)
	{
		(*i)->VOnUpdate(deltaMilliseconds);
	}
}

//
// HumanView::VPushElement - Chapter 9
// (note - this was renamed from HumanView::VPushScreen)
//
void HumanView::VPushElement(shared_ptr<IScreenElement> pElement)
{
	m_ScreenElements.push_front(pElement);
}

//
// HumanView::VPopElement - Chapter 9
// (note - this was renamed from HumanView::VPopScreen)
//    MrMike: 11-Apr-2009 - changed to solve a bug with modal dialogs.
void HumanView::VPopElement(shared_ptr<IScreenElement> pElement)
{
	// m_ScreenElements.pop_front();
	m_ScreenElements.remove(pElement);
}