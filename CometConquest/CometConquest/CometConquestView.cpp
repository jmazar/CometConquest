#include "CometConquestStd.h"

#include "GameApp.h"
#include "Actors.h"
#include "SceneGraph\SceneNodes.h"
#include "SceneGraph\Geometry.h"
#include "CometConquestNetwork.h"
#include "CometConquestEvents.h"
#include "CometConquestView.h"
#include "ShipController.h"
#include "CometConquest.h"
#include "MainLoop\Initialization.h"
#include "Physics\PhysicsEventListener.h"

extern CometConquestGameApp g_CometConquestApp;

//========================================================================
//
// MainMenuUI & MainMenuView implementation
//
//
//========================================================================

#define CID_CREATE_GAME_RADIO	2
#define CID_JOIN_GAME_RADIO		3
#define CID_NUM_AI_SLIDER		4
#define CID_NUM_PLAYER_SLIDER	5
#define CID_HOST_LISTEN_PORT	6
#define CID_CLIENT_ATTACH_PORT	7
#define CID_START_BUTTON		8
#define CID_HOST_NAME			9
#define CID_NUM_AI_LABEL		10
#define CID_NUM_PLAYER_LABEL	11
#define CID_HOST_LISTEN_PORT_LABEL 12
#define CID_CLIENT_ATTACH_PORT_LABEL 13
#define CID_HOST_NAME_LABEL		14
#define CID_BLUE_SCORE 15
#define CID_RED_SCORE 16


const int g_SampleUIWidth = 500;
const int g_SampleUIHeight = 450;

MainMenuUI::MainMenuUI()
{

	m_SampleUI.Init( &g_CometConquestApp.g_DialogResourceManager );
    m_SampleUI.SetCallback( OnGUIEvent, this ); 
	int iY = 10; 
	int iX = 35;
	int iX2 = g_SampleUIWidth / 2;
	int width = (g_SampleUIWidth/2)-10;
	int height = 20;
	int lineHeight = height + 2;

	// grab defaults from the game options.
	m_NumPlayers = g_pApp->m_pOptions->m_expectedPlayers;
	m_HostName = g_pApp->m_pOptions->m_gameHost;
	m_HostListenPort = g_pApp->m_pOptions->m_listenPort;
	m_ClientAttachPort = g_pApp->m_pOptions->m_listenPort;


	m_bCreatingGame = true;

	m_SampleUI.SetFont( 2, L"Courier New", 16, FW_NORMAL );
	m_SampleUI.SetRefreshTime(0);
	m_SampleUI.SetBackgroundColors(g_Gray40);

	m_SampleUI.AddStatic(0, L"Comet Conquest Main Menu", iX, iY, g_SampleUIWidth, height * 2);
	m_SampleUI.GetStatic(0)->SetTextColor( g_Black );
	iY += (lineHeight * 3);

    m_SampleUI.AddRadioButton( CID_CREATE_GAME_RADIO, 1, L"Create Game", iX, iY, g_SampleUIWidth, height);
	m_SampleUI.GetRadioButton (CID_CREATE_GAME_RADIO) ->SetTextColor( g_Black);
	iY += lineHeight;
	
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_NUM_PLAYER_LABEL, L"Number of Players", iX, iY, width, height);
	m_SampleUI.GetStatic(CID_NUM_PLAYER_LABEL)->SetTextColor(g_Black);
	m_SampleUI.AddSlider( CID_NUM_PLAYER_SLIDER, iX2, iY, width, height);
	m_SampleUI.GetSlider( CID_NUM_PLAYER_SLIDER )->SetRange(1, g_pApp->m_pOptions->m_maxPlayers);
	m_SampleUI.GetSlider( CID_NUM_PLAYER_SLIDER )->SetValue(m_NumPlayers);  // should be player options default
	m_SampleUI.GetSlider( CID_NUM_PLAYER_SLIDER )->SetTextColor(g_Black);
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_HOST_LISTEN_PORT_LABEL, L"Host Listen Port", iX, iY, width, height);
	m_SampleUI.GetStatic(CID_HOST_LISTEN_PORT_LABEL)->SetTextColor(g_Black);
	m_SampleUI.AddEditBox( CID_HOST_LISTEN_PORT, L"3000", iX2, iY, width, height * 2);
	m_SampleUI.GetEditBox(CID_HOST_LISTEN_PORT)->SetTextColor(g_Black);


	CDXUTEditBox *eb = m_SampleUI.GetEditBox( CID_HOST_LISTEN_PORT );
	eb->SetVisible(false);
	iY += lineHeight * 3;

    m_SampleUI.AddRadioButton( CID_JOIN_GAME_RADIO, 1, L"Join Game", iX, iY, width, height);
    m_SampleUI.GetRadioButton( CID_JOIN_GAME_RADIO )->SetChecked( true ); 
	m_SampleUI.GetRadioButton( CID_JOIN_GAME_RADIO )->SetTextColor (g_Black);
	iY += lineHeight;

	m_SampleUI.AddStatic(CID_CLIENT_ATTACH_PORT_LABEL, L"Host Attach Port", iX, iY, width, height);
	m_SampleUI.GetStatic(CID_CLIENT_ATTACH_PORT_LABEL)->SetTextColor(g_Black);
	m_SampleUI.AddEditBox( CID_CLIENT_ATTACH_PORT, L"3000", iX2, iY, width, height * 2);
	m_SampleUI.GetEditBox(CID_CLIENT_ATTACH_PORT)->SetTextColor(g_Black);
	iY += lineHeight * 3;


	m_SampleUI.AddStatic(CID_HOST_NAME_LABEL, L"Host Name", iX, iY, width, height);
	m_SampleUI.GetStatic(CID_HOST_NAME_LABEL)->SetTextColor(g_Black);
	m_SampleUI.AddEditBox( CID_HOST_NAME, L"ssd", iX2, iY, width, height * 2);
	m_SampleUI.GetEditBox(CID_HOST_NAME)->SetTextColor(g_Black);
	iY += lineHeight * 3;

	m_SampleUI.AddButton(CID_START_BUTTON, L"Start Game",  (g_SampleUIWidth-(width/2))/2, iY += lineHeight, width/2, height);
	m_SampleUI.SetFont(0, L"Arial", height, 0);

    m_SampleUI.GetRadioButton( CID_CREATE_GAME_RADIO )->SetChecked(true);

	Set();

}

void MainMenuUI::Set()
{
	WCHAR buffer[256];
	CHAR ansiBuffer[256];
	
	m_NumPlayers = m_SampleUI.GetSlider( CID_NUM_PLAYER_SLIDER )->GetValue(); 
	m_SampleUI.GetSlider(CID_NUM_PLAYER_SLIDER)->SetVisible(m_bCreatingGame);
	wsprintf( buffer, _T("%s: %d\n"), L"Number of Players: ", m_NumPlayers );
	m_SampleUI.GetStatic(CID_NUM_PLAYER_LABEL)->SetText(buffer);
	m_SampleUI.GetStatic(CID_NUM_PLAYER_LABEL)->SetVisible(m_bCreatingGame);

	m_SampleUI.GetStatic( CID_HOST_LISTEN_PORT_LABEL )->SetVisible( m_bCreatingGame);
	m_SampleUI.GetEditBox( CID_HOST_LISTEN_PORT )->SetVisible( m_bCreatingGame);
	if (m_bCreatingGame)
	{
		WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox( CID_HOST_LISTEN_PORT )->GetText(), 256);
		m_HostListenPort = ansiBuffer;
	}

	m_SampleUI.GetStatic( CID_HOST_NAME_LABEL )->SetVisible(!m_bCreatingGame);
	m_SampleUI.GetEditBox( CID_HOST_NAME )->SetVisible(!m_bCreatingGame);

	WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox( CID_HOST_NAME )->GetText(), 256);
	m_HostName = ansiBuffer;

	m_SampleUI.GetStatic( CID_CLIENT_ATTACH_PORT_LABEL )->SetVisible(!m_bCreatingGame);
	m_SampleUI.GetEditBox( CID_CLIENT_ATTACH_PORT )->SetVisible(!m_bCreatingGame);
	if (!m_bCreatingGame)
	{
		WideToAnsiCch(ansiBuffer, m_SampleUI.GetEditBox( CID_CLIENT_ATTACH_PORT )->GetText(), 256);
		m_ClientAttachPort = ansiBuffer;
	}
}

MainMenuUI::~MainMenuUI() 
{ 

}


HRESULT MainMenuUI::VOnRestore()
{
    m_SampleUI.SetLocation( (g_pApp->g_DialogResourceManager.m_nBackBufferWidth-g_SampleUIWidth)/2, (g_pApp->g_DialogResourceManager.m_nBackBufferHeight-g_SampleUIHeight) / 2  );
	m_SampleUI.SetSize( g_SampleUIWidth, g_SampleUIHeight );
	return S_OK;
}

HRESULT MainMenuUI::VOnRender(double fTime, float fElapsedTime)
{
	HRESULT hr;
	//DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"Main Menu" );
	V( m_SampleUI.OnRender( fElapsedTime ) );

	//DXUT_EndPerfEvent();
	return S_OK;
};

LRESULT CALLBACK MainMenuUI::VOnMsgProc( AppMsg msg )
{
	return m_SampleUI.MsgProc( msg.m_hWnd, msg.m_uMsg, msg.m_wParam, msg.m_lParam );
}



//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
//    Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
void CALLBACK MainMenuUI::_OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext )
{
    switch( nControlID )
    {
		case CID_CREATE_GAME_RADIO:
			m_bCreatingGame = true;
			break;

		case CID_JOIN_GAME_RADIO:
			m_bCreatingGame = false;
			break;

		case CID_NUM_AI_SLIDER:
		case CID_NUM_PLAYER_SLIDER:
		case CID_HOST_LISTEN_PORT:
		case CID_CLIENT_ATTACH_PORT:
		case CID_HOST_NAME:
			break;

		case CID_START_BUTTON:	
			// grab defaults from the game options.
			g_pApp->m_pOptions->m_expectedPlayers = m_NumPlayers;
			if (m_bCreatingGame)
			{
				g_pApp->m_pOptions->m_gameHost = "";
				g_pApp->m_pOptions->m_listenPort = atoi(m_HostListenPort.c_str());
			}
			else
			{
				g_pApp->m_pOptions->m_gameHost = m_HostName;
				g_pApp->m_pOptions->m_listenPort = atoi(m_ClientAttachPort.c_str());
			}

			VSetVisible(false);

			if (!g_pApp->m_pOptions->m_gameHost.empty())
			{

				EventListenerPtr listener ( GCC_NEW NetworkEventForwarder( 0 ) );
				extern void ListenForCometConquestGameCommands(EventListenerPtr listener);
				ListenForCometConquestGameCommands(listener);
			}
			safeQueEvent( IEventDataPtr( GCC_NEW EvtData_Request_Start_Game( ) ) );

			break;


		default:
			assert(0 && "Unknown control.");
    }

	Set();
}

void CALLBACK MainMenuUI::OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext )
{
	MainMenuUI *ui = static_cast<MainMenuUI *>(pUserContext);
	ui->_OnGUIEvent(nEvent, nControlID, pControl, pUserContext);
}


MainMenuView::MainMenuView() : HumanView()
{
	m_MainMenuUI.reset(GCC_NEW MainMenuUI); 
	VPushElement(m_MainMenuUI);
}


MainMenuView::~MainMenuView()
{
	assert(1);
	// does nothing...
}




void MainMenuView::VOnUpdate( int deltaMilliseconds )
{
	HumanView::VOnUpdate( deltaMilliseconds );
}

LRESULT CALLBACK MainMenuView::VOnMsgProc( AppMsg msg )
{
	if (m_MainMenuUI->VIsVisible() )
	{
		if (HumanView::VOnMsgProc(msg))
			return 1;
	}
	return 0;
}


//========================================================================
//
// CometconquestGameView Implementation
//
//========================================================================

void ListenForCometConquestViewEvents(EventListenerPtr listener)
{
	// hook in the physics event listener
	safeAddListener( listener, EvtData_PhysCollision::sk_EventType );
	safeAddListener( listener, EvtData_Destroy_Actor::sk_EventType );
	safeAddListener( listener, EvtData_Fire_Weapon::sk_EventType );
	safeAddListener( listener, EvtData_New_Game::sk_EventType );
	safeAddListener( listener, EvtData_New_Actor::sk_EventType );
	safeAddListener( listener, EvtData_Move_Actor::sk_EventType );
	safeAddListener( listener, EvtData_Game_State::sk_EventType );
	safeAddListener( listener, EvtData_Request_New_Actor::sk_EventType );


}

//
// ListenForTeapotGameCommands				- Chapter 19
//
void ListenForCometConquestGameCommands(EventListenerPtr listener)
{
	safeAddListener( listener, EvtData_Fire_Weapon::sk_EventType );
	safeAddListener( listener, EvtData_Thrust::sk_EventType );
	safeAddListener( listener, EvtData_Steer::sk_EventType );
}

CometConquestGameView::CometConquestGameView(bool bRender) 
{ 
	m_bRender = bRender;
	m_BaseGameState = BGS_Initializing;
	m_bShowUI = true;
	m_pScene.reset(GCC_NEW ScreenElementScene());
	m_ScoreOverlay.Init( &g_CometConquestApp.g_DialogResourceManager );


	m_ScoreOverlay.AddStatic(CID_BLUE_SCORE, L"0", g_CometConquestApp.g_DialogResourceManager.m_nBackBufferWidth - 30, 10, 125, 22);
	m_ScoreOverlay.GetStatic(CID_BLUE_SCORE)->SetTextColor(g_Blue);

	m_ScoreOverlay.AddStatic(CID_RED_SCORE, L"0", g_CometConquestApp.g_DialogResourceManager.m_nBackBufferWidth - 20, g_CometConquestApp.g_DialogResourceManager.m_nBackBufferHeight - 30, 125, 22);
	m_ScoreOverlay.GetStatic(CID_RED_SCORE)->SetTextColor(g_Red);

	EventListenerPtr listener ( GCC_NEW CometConquestGameViewListener( this ) );
	ListenForCometConquestViewEvents(listener);
}

CometConquestGameView::~CometConquestGameView() 
{
	assert(1);
}

LRESULT CALLBACK CometConquestGameView::VOnMsgProc( AppMsg msg )
{
	if (HumanView::VOnMsgProc(msg))
		return 1;

	//if (msg.m_uMsg==WM_KEYDOWN)
	//{
	//	if (msg.m_wParam==VK_F1)
	//	{
	//		m_bShowUI = !m_bShowUI;
	//		m_StandardHUD->VSetVisible(m_bShowUI);
	//		return 1;
	//	}
	//	else if (msg.m_wParam==VK_F2)
	//	{
	//		// test the picking API

	//		CPoint ptCursor;
	//		GetCursorPos( &ptCursor );
	//		ScreenToClient( g_pApp->GetHwnd(), &ptCursor );

	//		RayCast rayCast(ptCursor);
	//		m_pScene->Pick(&rayCast);
	//		rayCast.Sort();

	//		if (rayCast.m_NumIntersections)
	//		{
	//			// You can iterate through the intersections on the raycast.
	//			int a = 0;
	//		}
	//	}
	//	else if (msg.m_wParam==VK_F3)
	//	{
	//		//extern void CreateThreads();
	//		//CreateThreads();

	//		extern void testThreading(CProcessManager *procMgr);
	//		//testThreading(m_pProcessManager);

	//		extern void testStreaming(CProcessManager *procMgr);
	//		//testStreaming(m_pProcessManager);

	//		extern void testRealtimeEvents(CProcessManager *procMgr);
	//		//testRealtimeEvents(m_pProcessManager);

	//		extern void testRealtimeDecompression(CProcessManager *procMgr);
	//		testRealtimeDecompression(m_pProcessManager);
	//	}
	//	else if (msg.m_wParam==VK_F8)
	//	{
	//		TeapotWarsGame *twg = static_cast<TeapotWarsGame *>(g_pApp->m_pGame);
	//		twg->ToggleRenderDiagnostics();
	//	}
	//	else if (msg.m_wParam==VK_F9)
	//	{
	//		m_KeyboardHandler = m_pTeapotController;
	//		m_MouseHandler = m_pTeapotController;
	//		m_pCamera->SetTarget(m_pTeapot);
	//		m_pTeapot->SetAlpha(0.8f);
	//		ReleaseCapture();
	//		return 1;
	//	}
	//	else if (msg.m_wParam==VK_F11)
	//	{
	//		m_KeyboardHandler = m_pFreeCameraController;
	//		m_MouseHandler = m_pFreeCameraController;
	//		m_pCamera->ClearTarget();
	//		m_pTeapot->SetAlpha(fOPAQUE);
	//		SetCapture(g_pApp->GetHwnd());
	//		return 1;
	//	}
	//	else if (msg.m_wParam=='Q')
	//	{
	//		if (GameCodeApp::Ask(QUESTION_QUIT_GAME)==IDYES)
	//		{
	//			g_pApp->SetQuitting(true);
	//		}
	//		return 1;
	//	}
	//}

	return 0;
}


void CometConquestGameView::VOnUpdate( int deltaMilliseconds )
{
	HumanView::VOnUpdate( deltaMilliseconds );

	m_ScoreOverlay.SetLocation( 0, 0  );
	m_ScoreOverlay.SetSize( g_pApp->g_DialogResourceManager.m_nBackBufferWidth, g_pApp->g_DialogResourceManager.m_nBackBufferHeight );

	int blueScore = static_cast<CometConquestGameApp *>(g_pApp)->GetGame()->getBlueTeamScore();
	WCHAR wszText[10];
	swprintf_s(wszText, L"%d", blueScore);
	m_ScoreOverlay.GetStatic(CID_BLUE_SCORE)->SetText(wszText);

	int redScore = static_cast<CometConquestGameApp *>(g_pApp)->GetGame()->getRedTeamScore();
	swprintf_s(wszText, L"%d", redScore);
	m_ScoreOverlay.GetStatic(CID_RED_SCORE)->SetText(wszText);
	if (m_pShipController)
	{
		m_pShipController->OnUpdate(deltaMilliseconds);
	}


}

void CometConquestGameView::VOnAttach(GameViewId vid, optional<ActorId> aid)
{
	HumanView::VOnAttach(vid, aid);
	BuildInitialScene();
}

void CometConquestGameView::MoveActor(ActorId id, Mat4x4 const &mat)
{
    shared_ptr<ISceneNode> node = m_pScene->FindActor(id);
	if (node)
	{
		node->VSetTransform(&mat);
	}
}

void CometConquestGameView::HandleGameState(BaseGameState newState)
{
	m_BaseGameState = newState;
}

void CometConquestGameView::BuildInitialScene()
{




	// Here's our sky node


	VPushElement(m_pScene);

	// A movement controller is going to control the camera, 
	// but it could be constructed with any of the objects you see in this
	// function. You can have your very own remote controlled sphere. What fun...


	// Jan 2011 - mlm - Fix the D3D device has a non-zero reference count bug! 
	//                  Solution posted by shallway
	//VOnRestore();

}


void CometConquestGameView::VOnRender(double fTime, float fElapsedTime)
{
	if(m_bRender)
	{
		HumanView::VOnRender(fTime, fElapsedTime);
		m_ScoreOverlay.OnRender( fElapsedTime );
	}
}


//
// TeapotWarsGameViewListener::TeapotWarsGameViewListener	- Chapter 19, page 766
//
CometConquestGameViewListener::CometConquestGameViewListener( CometConquestGameView *view)
{
	m_pView = view;
}

bool CometConquestGameViewListener::HandleEvent( IEventData const & event )
{
	// [rez] removing event spew
	//if ( EvtData_Move_Actor::sk_EventType.getHashValue() != event.VGetEventType().getHashValue() )
	//{
	//	OutputDebugStringA( event.VGetEventType().getStr().c_str() );
	//	OutputDebugStringA("\n");
	//}

	if ( EvtData_PhysCollision::sk_EventType == event.VGetEventType() )
	{
		//EvtData_PhysCollision const & ed = static_cast< const EvtData_PhysCollision & >( event );
		//shared_ptr<IActor> pGameActorA = g_pApp->m_pGame->VGetActor(ed.m_ActorA);
		//shared_ptr<IActor> pGameActorB = g_pApp->m_pGame->VGetActor(ed.m_ActorB);
		//if (!pGameActorA || !pGameActorB)
		//	return false;

		//int typeA = pGameActorA->VGetType();
		//int typeB = pGameActorB->VGetType();

		////if(	(AT_Teapot==typeA && AT_Sphere==typeB)
		////	|| (AT_Sphere==typeA && AT_Teapot==typeB) )
		////{
		////	// play the sound a bullet makes when it hits a teapot

		////	SoundResource resource("computerbeep3.wav");
		////	shared_ptr<SoundResHandle> srh = boost::static_pointer_cast<SoundResHandle>(g_pApp->m_ResCache->GetHandle(&resource));
		////	shared_ptr<SoundProcess> sfx(GCC_NEW SoundProcess(srh, PROC_SOUNDFX, 100, false));
		////	m_pView->m_pProcessManager->Attach(sfx);
		////}
	}
	else
		if ( EvtData_Destroy_Actor::sk_EventType == event.VGetEventType() )
		{
			const EvtData_Destroy_Actor & castEvent = static_cast< const EvtData_Destroy_Actor & >( event );
			ActorId aid = castEvent.m_id;
			m_pView->m_pScene->RemoveChild(aid);
		}
		else if ( EvtData_Move_Actor::sk_EventType == event.VGetEventType() )
		{
			const EvtData_Move_Actor & ed = static_cast< const EvtData_Move_Actor & >( event );
			m_pView->MoveActor(ed.m_Id, ed.m_Mat);
		}
		else if ( EvtData_New_Actor::sk_EventType == event.VGetEventType() )
		{
			const EvtData_New_Actor & ed = static_cast< const EvtData_New_Actor & >( event );

			shared_ptr<SceneNode> node = ed.m_pActorParams->VCreateSceneNode(m_pView->m_pScene);
			m_pView->m_pScene->VAddChild(ed.m_pActorParams->m_Id, node);
			//TODO: Add real error handling here.		
			if(FAILED(node->VOnRestore(&(*(m_pView->m_pScene)))))
				return false;

			if (ed.m_pActorParams->m_Type == AT_Ship)
			{
				ShipParams *p = static_cast<ShipParams *>(ed.m_pActorParams);
		
				if (p->m_ViewId == m_pView->m_ViewId)
				{
					m_pView->m_pShip = node;
					m_pView->m_pShipController.reset(GCC_NEW ShipController(m_pView->m_pShip));
					m_pView->m_KeyboardHandler = m_pView->m_pShipController;
					//m_pView->m_MouseHandler = m_pView->m_pShipController;
				}
			}
		}
		else if ( EvtData_Game_State::sk_EventType == event.VGetEventType() )
		{
			const EvtData_Game_State & ed = static_cast< const EvtData_Game_State & >( event );
			BaseGameState gameState = ed.m_gameState;
			m_pView->HandleGameState(gameState);
		}
		
		
/**	else if ( EvtData_Decompression_Progress::sk_EventType == event.VGetEventType() )
	{
		const EvtData_Decompression_Progress & castEvent = static_cast< const EvtData_Decompression_Progress & >( event );
		if (castEvent.m_buffer != NULL)
		{
			const void *buffer = castEvent.m_buffer;
			SAFE_DELETE_ARRAY(buffer);
		}
	}
**/
	return false;
}