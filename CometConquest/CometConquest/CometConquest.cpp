#include "CometConquestStd.h"

#include "GameApp.h"
#include "CometConquest.h"
#include "CometConquestEvents.h"
#include "CometConquestResources.h"
#include "CometConquestNetwork.h"
#include "CometConquestView.h"

#include "MainLoop\Initialization.h"

#include "EventManager\Events.h"
#include "EventManager\EventManagerImpl.h"

#include "Physics\Physics.h"
#include "Physics\PhysicsEventListener.h"

CometConquestGameApp g_CometConquestApp;

INT WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	return GameMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

void CometConquestGameApp::RegisterGameSpecificEvents( void )
{
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Fire_Weapon::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Thrust::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Steer::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_New_Game::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Request_Start_Game::sk_EventType );
	m_pEventManager->RegisterCodeOnlyEvent( EvtData_Request_New_Actor::sk_EventType );
}

BaseGameLogic *CometConquestGameApp::VCreateGameAndView()
{
	BaseGameLogic *game = NULL;
	assert(m_pOptions && _T("The game options object is uninitialized."));

	// Register any game-specific events here.
	RegisterGameSpecificEvents();
	// Need to know if you are client or server before this point?
	if (m_pOptions->m_gameHost.empty())
	{
		game = GCC_NEW CometConquestGame(*m_pOptions);
	}
	else
	{
		game = GCC_NEW CometConquestGame(*m_pOptions);

		//EventListenerPtr listener ( GCC_NEW NetworkEventForwarder( 0 ) );
		//extern void ListenForCometConquestGameCommands(EventListenerPtr listener);
		//ListenForCometConquestGameCommands(listener);

	}

	shared_ptr<IGameView> menuView(GCC_NEW MainMenuView());
	game->VAddView(menuView);

	return game;
}




//
// TeapotWarsGameApp::VLoadGame				- Chapter 19, page 703
//
bool CometConquestGameApp::VLoadGame()
{
	// Ordinarilly you'd read the game options and see what the current game
	// needs to be - or perhaps pop up a dialog box and ask which game
	// needed loading. All of the game graphics are initialized by now, too...
	return m_pGame->VLoadGame("NewGame");
}

HICON CometConquestGameApp::VGetIcon()
{
	return LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_ICON1));
}

class CometConquestEventListener : public IEventListener
{
public:

	explicit CometConquestEventListener( CometConquestGame *cometConquest ) : m_CometConquest(cometConquest) { };
	virtual ~CometConquestEventListener() { };
	char const * GetName(void) { return "CometConquestEventListener"; }
	bool HandleEvent( IEventData const & event );

private:
	CometConquestGame *m_CometConquest;
};

const float g_WeaponForce = 1.0f;
bool CometConquestEventListener::HandleEvent( IEventData const & event )
{
	if ( EvtData_Request_Start_Game::sk_EventType == event.VGetEventType() )
	{
		m_CometConquest->VChangeState(BGS_WaitingForPlayers);
	}
	else if ( EvtData_Game_State::sk_EventType == event.VGetEventType() )
	{
		const EvtData_Game_State & castEvent = static_cast< const EvtData_Game_State & >( event );
		m_CometConquest->VChangeState(castEvent.m_gameState);
	}
	else if ( EvtData_Remote_Client::sk_EventType == event.VGetEventType() )
	{
		// This event is always sent from clients to the game server.

		const EvtData_Remote_Client & castEvent = static_cast< const EvtData_Remote_Client & >( event );
		const int sockID = castEvent.m_socketId;
		const int ipAddress = castEvent.m_ipAddress;

		// The teapot has already been created - we need to go find it.
		//ActorMap::iterator i = m_CometConquest->m_ActorList.begin();
		//ActorMap::iterator end = m_CometConquest->m_ActorList.end();
		//shared_ptr<IActor> actor = shared_ptr<BaseActor>(); 
		//while (i != end)
		//{
		//	actor = (*i).second;
		//	if (actor->VGetType() == AT_Ship)
		//	{
		//		shared_ptr<ActorParams> params = actor->VGetParams();
		//		shared_ptr<ShipParams> teapotParams = boost::static_pointer_cast<ShipParams>(params);
		//		if (teapotParams->m_ViewId == VIEWID_NO_VIEW_ATTACHED)
		//		{
		//			break;
		//		}
		//	}
		//	++i;
		//}

		//if (actor != shared_ptr<BaseActor>())
		//{
		NetworkGameView *netGameView = GCC_NEW NetworkGameView( sockID );

		shared_ptr<IGameView> gameView(netGameView);
		m_CometConquest->VAddView(gameView, sockID);

		extern void ListenForCometConquestViewEvents(EventListenerPtr listener);

		EventListenerPtr listener ( GCC_NEW NetworkEventForwarder( sockID ) );
		ListenForCometConquestViewEvents( listener );
		//}
	}

	else if ( EvtData_Network_Player_Actor_Assignment::sk_EventType == event.VGetEventType() )
	{
		// we're a remote client getting an actor assignment.
		// the server assigned us a playerId when we first attached (the server's socketId, actually)
		const EvtData_Network_Player_Actor_Assignment & castEvent =
			static_cast< const EvtData_Network_Player_Actor_Assignment & >( event );

		shared_ptr<IGameView> playersView(GCC_NEW CometConquestGameView(true));
		playersView.get()->VOnAttach(castEvent.m_remotePlayerId, castEvent.m_actorId);
		m_CometConquest->VAddView(playersView, castEvent.m_actorId);	
	}

	else if ( EvtData_PhysCollision::sk_EventType == event.VGetEventType() )
	{
		const EvtData_PhysCollision & castEvent = static_cast< const EvtData_PhysCollision & >( event );
		shared_ptr<IActor> pGameActorA = m_CometConquest->VGetActor(castEvent.m_ActorA);
		shared_ptr<IActor> pGameActorB = m_CometConquest->VGetActor(castEvent.m_ActorB);
		if (!pGameActorA || !pGameActorB)
			return false;

		int typeA = pGameActorA->VGetType();
		int typeB = pGameActorB->VGetType();

		//Bullets hitting things
		if(AT_Bullet == typeA && AT_Ship != typeB && AT_Floor != typeB)
		{
			m_CometConquest->VRemoveActor(pGameActorA->VGetID());
		}
		if(AT_Bullet == typeB && AT_Ship != typeA && AT_Floor != typeA)
		{
			m_CometConquest->VRemoveActor(pGameActorB->VGetID());
		}
		//Comets hitting boundry wall
		if(AT_Comet == typeB && AT_BoundryWall == typeA)
		{
			m_CometConquest->VRemoveActor(pGameActorB->VGetID());
		}
		if(AT_Comet == typeA && AT_BoundryWall == typeB)
		{
			m_CometConquest->VRemoveActor(pGameActorA->VGetID());
		}
		//Ship getting hit by a comet
		if(AT_Ship == typeA && AT_Comet == typeB)
		{
			ShipParams tp;
			tp.m_StartPosition = static_cast<ShipParams *>(pGameActorA->VGetParams().get())->m_StartPosition;
			tp.m_Mat = tp.m_StartPosition;
			tp.m_Length = 2.5;
			tp.m_ViewId = static_cast<ShipParams *>(pGameActorA->VGetParams().get())->m_ViewId;
			tp.m_Team = static_cast<ShipParams *>(pGameActorA->VGetParams().get())->m_Team;
			const EvtData_Request_New_Actor requestShip( &tp );
			m_CometConquest->VRemoveActor(pGameActorA->VGetID());
			safeTriggerEvent( requestShip );
		}
		if(AT_Ship == typeB && AT_Comet == typeA)
		{
			ShipParams tp;
			tp.m_StartPosition = static_cast<ShipParams *>(pGameActorB->VGetParams().get())->m_StartPosition;
			tp.m_Mat = tp.m_StartPosition;
			tp.m_Length = 2.5;
			tp.m_ViewId = static_cast<ShipParams *>(pGameActorB->VGetParams().get())->m_ViewId;
			tp.m_Team = static_cast<ShipParams *>(pGameActorB->VGetParams().get())->m_Team;
			const EvtData_Request_New_Actor requestShip( &tp );
			m_CometConquest->VRemoveActor(pGameActorB->VGetID());
			safeTriggerEvent( requestShip );
		}
		//Shooting players
		if(AT_Ship == typeA && AT_Bullet == typeB)
		{

			ShipParams tp;
			tp.m_StartPosition = static_cast<ShipParams *>(pGameActorA->VGetParams().get())->m_StartPosition;
			tp.m_Mat = tp.m_StartPosition;
			tp.m_Length = 2.5;
			//tp.m_Id = pGameActorA->VGetID();
			tp.m_ViewId = static_cast<ShipParams *>(pGameActorA->VGetParams().get())->m_ViewId;
			tp.m_Team = static_cast<ShipParams *>(pGameActorA->VGetParams().get())->m_Team;
			const EvtData_Request_New_Actor requestShip( &tp );
			//m_CometConquest->VRemoveActor(pGameActorB->VGetID());

			safeTriggerEvent( requestShip );
			m_CometConquest->VRemoveActor(pGameActorA->VGetID());

		}
		if(AT_Ship == typeB && AT_Bullet == typeA)
		{

			ShipParams tp;
			tp.m_StartPosition = static_cast<ShipParams *>(pGameActorB->VGetParams().get())->m_StartPosition;
			tp.m_Mat = tp.m_StartPosition;
			tp.m_Length = 2.5;
			//tp.m_Id = pGameActorB->VGetID();
			tp.m_ViewId = static_cast<ShipParams *>(pGameActorB->VGetParams().get())->m_ViewId;
			const EvtData_Request_New_Actor requestShip( &tp );

			safeTriggerEvent( requestShip );
			m_CometConquest->VRemoveActor(pGameActorB->VGetID());
		}
		//Shooting Comets
		if(AT_Bullet == typeA && AT_Comet == typeB)
		{
			m_CometConquest->VRemoveActor(pGameActorB->VGetID());
			//m_CometConquest->VRemoveActor(pGameActorA->VGetID());
		}
		if(AT_Bullet == typeB && AT_Comet == typeA)
		{
			//m_CometConquest->VRemoveActor(pGameActorB->VGetID());
			m_CometConquest->VRemoveActor(pGameActorA->VGetID());
		}

		//Goal & Ring
		if(AT_Ring == typeA && AT_Goal == typeB)
		{
			RingParams rp;
			rp.m_StartPosition == static_cast<RingParams *>(pGameActorA->VGetParams().get())->m_StartPosition;
			rp.m_Mat = rp.m_StartPosition;
			const EvtData_Request_New_Actor requestRing (&rp);
			safeTriggerEvent (requestRing);
			m_CometConquest->VRemoveActor(pGameActorA->VGetID());
						if(0 == static_cast<GoalParams*>(pGameActorB->VGetParams().get())->m_Team)
			{
				m_CometConquest->blueTeamScore();
			}
			else
			{
				m_CometConquest->redTeamScore();
			}
		}
		if(AT_Ring == typeB && AT_Goal == typeA)
		{
			RingParams rp;
			rp.m_StartPosition == static_cast<RingParams *>(pGameActorB->VGetParams().get())->m_StartPosition;
			rp.m_Mat = rp.m_StartPosition;
			const EvtData_Request_New_Actor requestRing (&rp);
			safeTriggerEvent (requestRing);
			m_CometConquest->VRemoveActor(pGameActorB->VGetID());
			if(0 == static_cast<GoalParams*>(pGameActorA->VGetParams().get())->m_Team)
			{
				m_CometConquest->blueTeamScore();
			}
			else
			{
				m_CometConquest->redTeamScore();
			}

		}

	}
	else if ( EvtData_Thrust::sk_EventType == event.VGetEventType() )
	{
		const EvtData_Thrust & castEvent = static_cast< const EvtData_Thrust & >( event );
		shared_ptr<IActor> pActor = m_CometConquest->VGetActor(castEvent.m_id);
		if( pActor )
		{
			static const float newtonForce = 1.f;
			float thrustForce = castEvent.m_throttle * newtonForce;

			Mat4x4 rotation = pActor->VGetMat();
			rotation.SetPosition(Vec3(0,0,0));
			Vec3 dir = rotation.Xform(g_Forward);
			dir.Normalize();
			m_CometConquest->m_pPhysics->VApplyForce(dir, thrustForce, castEvent.m_id);
		}
	}
	else if ( EvtData_Steer::sk_EventType == event.VGetEventType() )
	{
		static const float newtonForce = -.25 * 1.8f;

		const EvtData_Steer & castEvent = static_cast< const EvtData_Steer & >( event );
		float steerForce = -castEvent.m_dir * newtonForce;
		m_CometConquest->m_pPhysics->VApplyTorque(Vec3(0,1,0), steerForce, castEvent.m_id);
	}
	else if ( EvtData_Fire_Weapon::sk_EventType == event.VGetEventType() )
	{
		if(!this->m_CometConquest->m_bProxy)
		{
			const EvtData_Fire_Weapon & castEvent = static_cast< const EvtData_Fire_Weapon & >( event );
			ActorId gunnerId = castEvent.m_id;

			shared_ptr<IActor> pGunner = m_CometConquest->VGetActor(gunnerId);
			if (pGunner)
			{

				//Calculate depth offset from the controller
				Vec4 at = g_Forward4 * 3.0f;
				Vec4 atWorld = pGunner->VGetMat().Xform(at);

				Vec3 normalDir(atWorld);
				normalDir.Normalize();

				BulletParams sp;
				sp.m_Pos = pGunner->VGetMat().GetPosition() + Vec3(atWorld) * 3;
				sp.m_Radius = 0.25;
				sp.m_Segments = 16;
				sp.m_Color = g_Cyan;
				sp.m_NormalDir = normalDir;
				sp.m_Force = g_WeaponForce;
				sp.m_TeamFiredBy = static_cast<ShipParams *>(pGunner->VGetParams().get())->m_Team;
				//Request creation of this actor.
				const EvtData_Request_New_Actor cannonBallEvt( &sp );
				safeTriggerEvent( cannonBallEvt );
				return true;
			}
		}

	}
	else if ( EvtData_Move_Actor::sk_EventType == event.VGetEventType() )
	{
		const EvtData_Move_Actor & castEvent = static_cast< const EvtData_Move_Actor & >( event );
		m_CometConquest->VMoveActor(castEvent.m_Id, castEvent.m_Mat);
	}
	else if ( EvtData_Request_New_Actor::sk_EventType == event.VGetEventType() )
	{
		const EvtData_Request_New_Actor & castEvent = static_cast< const EvtData_Request_New_Actor & >( event );

		ActorParams * pActorParams = NULL;

		pActorParams = castEvent.m_pActorParams;


		//Did we get valid actor params?
		if ( NULL == pActorParams )
		{
			assert( 0 && "Invalid parameters specified for actor!" );
			return false;
		}

		//Valid params.
		const ActorId actorID = m_CometConquest->GetNewActorID();
		pActorParams->m_Id = actorID;
		//Package as a new actor event.
		/*
		IEventDataPtr actorEvent( IEventDataPtr( GCC_NEW EvtData_New_Actor( actorID, pActorParams ) ) );
		const bool bSuccess = safeQueEvent( actorEvent );
		*/
		const EvtData_New_Actor actorEvent( actorID, pActorParams );
		const bool bSuccess = safeTriggerEvent( actorEvent );

		return bSuccess;
	}
	else if ( EvtData_New_Actor::sk_EventType == event.VGetEventType() )
	{
		const EvtData_New_Actor & castEvent = static_cast< const EvtData_New_Actor & >( event );
		ActorParams * pActorParams = castEvent.m_pActorParams;

		if ( NULL == pActorParams )
		{
			assert( 0 && "Received a new actor event with NULL actor parameters!" );
			return false;
		}

		pActorParams->VCreate(m_CometConquest);


		if ( false == castEvent.m_id )
		{
			assert( 0 && "Unable to construct desired actor type!" );
			return false;
		}
	}

	return false;
}

bool CometConquestBaseGame::VLoadGame(std::string gameName)
{
	if (gameName=="NewGame")
	{
		VBuildInitialScene();
		safeTriggerEvent( EvtData_New_Game() );
	}

	return true;
}




//
// TeapotWarsBaseGame::VRemoveActor			- TODO
//
void CometConquestBaseGame::VRemoveActor(ActorId aid) 
{ 
	shared_ptr<IActor> actor = VGetActor( aid );
	assert(actor && "No such actor!");
	if (!actor)
	{
		return;
	}

	//Stuff about calling function when actor is destroyed

	BaseGameLogic::VRemoveActor(aid); 

	//Now remove the actor from the system.
	safeQueEvent( IEventDataPtr( GCC_NEW EvtData_Destroy_Actor( aid ) ) );
}

//
// TeapotWarsBaseGame::VAddActor			- Chapter 19, page 705
//
void CometConquestBaseGame::VAddActor(shared_ptr<IActor> actor, ActorParams *p)
{
	BaseGameLogic::VAddActor(actor, p);


	////Ensure script knows about this actor, too.
	//LuaPlus::LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
	//LuaPlus::LuaObject globalActorTable = g_pApp->m_pLuaStateManager->GetGlobalActorTable();
	//assert( globalActorTable.IsTable() && "Global actor table is NOT a table!" );
	//LuaPlus::LuaObject addedActorData = globalActorTable.CreateTable( *p->m_Id );	//The actor ID is the key.
	//addedActorData.SetInteger( "ActorID", *p->m_Id );

	//if ( 0 != p->m_OnCreateLuaFunctionName[0] ) 
	//{
	//	addedActorData.SetString( "OnCreateFunc", p->m_OnCreateLuaFunctionName );
	//}

	//if ( 0 != p->m_OnDestroyLuaFunctionName[0] ) 
	//{
	//	addedActorData.SetString( "OnDestroyFunc", p->m_OnDestroyLuaFunctionName );
	//}

	////If this actor has any script-specific functions to call, do so now.
	//if ( 0 != strlen( p->m_OnCreateLuaFunctionName ) )
	//{
	//	//First attempt to FIND the function specified.
	//	LuaPlus::LuaObject foundObj = g_pApp->m_pLuaStateManager->GetGlobalState()->GetGlobal( p->m_OnCreateLuaFunctionName );
	//	if ( foundObj.IsNil() )
	//	{
	//		assert( 0 && "Unable to find specified OnCreateFunc function!" );
	//	}
	//	else
	//	{
	//		//Make sure it actually *IS* a function.
	//		if ( false == foundObj.IsFunction() )
	//		{
	//			assert( 0 && "Specified OnCreateFunc doesn't exist!" );
	//		}
	//		else
	//		{
	//			//Attempt to call the function.
	//			LuaPlus::LuaFunction< void > onCreateFunc( foundObj );
	//			onCreateFunc( *p->m_Id, addedActorData );	//Pass in the actor ID and this actor's user-owned data table.
	//		}
	//	}
	//}
}


//========================================================================
//
// CometConquestWarsGame Implementation
//
//========================================================================

CometConquestGame::CometConquestGame(GameOptions const &options)
	: CometConquestBaseGame(options)
	, m_Lifetime(0)
	, m_StartPosition(6.0f, 1.5f, 3.0f)
	, m_HumanPlayersAttached(0)
{
	m_pPhysics.reset(CreateGamePhysics());
	m_data.m_lastCometTime = timeGetTime();
	m_cometConquestEventListener = shared_ptr<CometConquestEventListener> (GCC_NEW CometConquestEventListener ( this ) );
	safeAddListener( m_cometConquestEventListener, EvtData_Remote_Client::sk_EventType );
	safeAddListener( m_cometConquestEventListener, EvtData_PhysCollision::sk_EventType );
	safeAddListener( m_cometConquestEventListener, EvtData_New_Actor::sk_EventType );
	safeAddListener( m_cometConquestEventListener, EvtData_Destroy_Actor::sk_EventType );
	safeAddListener( m_cometConquestEventListener, EvtData_Move_Actor::sk_EventType );
	safeAddListener( m_cometConquestEventListener, EvtData_Request_New_Actor::sk_EventType );
	safeAddListener( m_cometConquestEventListener, EvtData_Request_Start_Game::sk_EventType );
	safeAddListener( m_cometConquestEventListener, EvtData_Network_Player_Actor_Assignment::sk_EventType );

	extern void ListenForCometConquestGameCommands(EventListenerPtr listener);
	ListenForCometConquestGameCommands(m_cometConquestEventListener);
}





//
// TeapotWarsGame::~TeapotWarsGame			- added post press
//
// This destructor must be called to remove all the actors from the actor list.
// If we rely on the BaseGame class to do that, the virtual chain is broken and the
// VRemoveActor method that gets called is BaseGame::VRemoveActor, which doesn't 
// remove the actor from the game data or physics, causing memory leaks.
//
CometConquestGame::~CometConquestGame()
{
	while (!m_ActorList.empty())
	{
		ActorMap::iterator i = m_ActorList.begin();
		ActorId aid = (*i).first;
		VRemoveActor(aid);
	}
}


//
// TeapotWarsGame::VOnUpdate			- Chapter 19, page 709
//
void CometConquestGame::VOnUpdate(float time, float elapsedTime)
{
	int deltaMilliseconds = int(elapsedTime * 1000.0f);
	m_Lifetime += elapsedTime;
	unsigned int currentTime = timeGetTime();
	BaseGameLogic::VOnUpdate(time, elapsedTime);

	if (m_bProxy)
		return;

	switch(m_State)
	{
	case BGS_LoadingGameEnvironment:
		break;

	case BGS_MainMenu:
		break;

	case BGS_WaitingForPlayers:
		if (m_ExpectedPlayers + m_ExpectedRemotePlayers == m_HumanPlayersAttached ) 
		{
			VChangeState(BGS_LoadingGameEnvironment);
		}
		break;

	case BGS_Running:
		if(currentTime > (m_data.m_lastCometTime + 5000))
		{
			Vec4 at = -g_Right4 * 2.0f;
			Vec4 atWorld = Mat4x4::g_Identity.Xform(at);
			int randVertical = m_random.Random(115) + 1 - 60;
			Vec3 normalDir(atWorld);
			normalDir.Normalize();
			Mat4x4 temp = Mat4x4::g_Identity;
			temp.SetPosition(Vec3(110,10,randVertical));
			CometParams cp;
			cp.m_Pos = temp.GetPosition() + Vec3(atWorld);
			cp.m_Radius = 6.0f;
			cp.m_Color = g_Cyan;
			cp.m_NormalDir = normalDir;
			cp.m_Force = 40000.0f;

			const EvtData_Request_New_Actor cannonBallEvt( &cp );
			safeTriggerEvent( cannonBallEvt );
			m_data.m_lastCometTime = currentTime;
		}
		break;
	default:
		assert(0 && _T("Unrecognized state."));
	}

	// look in Chapter 15, page 563 for more on this bit of code
	if(m_pPhysics)
	{
		m_pPhysics->VOnUpdate(elapsedTime);
		m_pPhysics->VSyncVisibleScene();
	}
}



void CometConquestGame::VSetProxy()
{
	CometConquestBaseGame::VSetProxy();
	safeAddListener( m_cometConquestEventListener, EvtData_Game_State::sk_EventType );
}

//
// TeapotWarsGame::VChangeState					- Chapter 19, page 712
//
void CometConquestGame::VChangeState(BaseGameState newState)
{
	CometConquestBaseGame::VChangeState(newState);

	switch(newState)
	{
	case BGS_WaitingForPlayers:
		if (m_bProxy)
			break;

		shared_ptr<IGameView> playersView(GCC_NEW CometConquestGameView(false));
		VAddView(playersView);			
		break;
	}
}

//
// TeapotWarsGame::VAddView					- Chapter 19, page 713
//
void CometConquestGame::VAddView(shared_ptr<IGameView> pView, optional<ActorId> actor)
{
	CometConquestBaseGame::VAddView(pView, actor);
	if (boost::dynamic_pointer_cast<NetworkGameView>(pView))
	{
		m_HumanPlayersAttached++;
	}
	else if (boost::dynamic_pointer_cast<CometConquestGameView>(pView))
	{
		m_HumanPlayersAttached++;
	}
}








//
// TeapotWarsGame::VRegisterHit				- Chapter 19, page 714
//
void CometConquestGame::VRegisterHit(const ActorId sphere, const ActorId teapot)
{
	//if (m_bProxy)
	//	return;

	////Make the teapot dizzy if it hits

	//const float hitForce = 80.f;

	//m_pPhysics->VApplyTorque(Vec3(0,1,0), hitForce, teapot);

	////everyone else gets a point, hahaha
	//for( ActorScoreMap::iterator i=m_data.m_actorScores.begin()
	//	;i!=GetData().GetActorScores().end(); ++i )
	//{
	//	if( i->first!=teapot )
	//	{
	//		++(i->second);
	//	}
	//}
}


// VBuildInitialScene was added post-press.
void CometConquestGame::VBuildInitialScene()
{
	CometConquestBaseGame::VBuildInitialScene();
	Vec3 redSpawns[3];
	Vec3 blueSpawns[3];

	redSpawns[0] = Vec3(-25,0,-90);
	redSpawns[1] = Vec3(-25,0,-102.5);
	redSpawns[2] = Vec3(-25,0,-115);

	blueSpawns[0] = Vec3(-25,0,90);
	blueSpawns[1] = Vec3(-25,0,102.5);
	blueSpawns[2] = Vec3(-25,0,115);
	if (m_bProxy)
		return;

	FloorParams fp;
	fp.m_Mat = Mat4x4::g_Identity;
	fp.m_Mat.BuildRotationY(0);
	fp.m_Mat.SetPosition(Vec3(0, -1, 0));
	fp.m_ViewId = -1;
	fp.m_Color = g_Green;
	const EvtData_Request_New_Actor requestActorFloor( &fp );
	safeTriggerEvent( requestActorFloor );

	int count = 0;
	int redSpawnCount = 0;
	int blueSpawnCount = 0;
	ShipParams tp;

	for(GameViewList::iterator i=++m_gameViews.begin(),
		end=m_gameViews.end(); i!=end; ++i)
	{
		tp.m_Mat = Mat4x4::g_Identity;
		tp.m_Mat.BuildRotationY(-D3DX_PI / 2.0f);
		tp.m_StartPosition = Mat4x4::g_Identity;
		tp.m_StartPosition.BuildRotationY(-D3DX_PI / 2.0f);
		if(count % 2 == 0)
		{
			tp.m_Mat.SetPosition(redSpawns[redSpawnCount]);

			tp.m_StartPosition.SetPosition(redSpawns[redSpawnCount++]);
			tp.m_Team = 1;
		}
		else
		{
			tp.m_Mat.SetPosition(blueSpawns[blueSpawnCount]);
			tp.m_StartPosition.SetPosition(blueSpawns[blueSpawnCount++]);
			tp.m_Team = 0;
		}
		tp.m_Length = 2.5;
		tp.m_ViewId = (*i)->VGetId();
		const EvtData_Request_New_Actor requestShip( &tp );
		safeTriggerEvent( requestShip );
		m_StartPosition += Vec3(15, 0, 15);
		count++;
	}

	/*Vec4 at = -g_Right4 * 2.0f;
	Vec4 atWorld = Mat4x4::g_Identity.Xform(at);

	Vec3 normalDir(atWorld);
	normalDir.Normalize();
	Mat4x4 temp = Mat4x4::g_Identity;
	temp.SetPosition(Vec3(110,10,-60));
	CometParams cp;
	cp.m_Pos = temp.GetPosition() + Vec3(atWorld);
	cp.m_Radius = 10.0f;
	cp.m_Color = g_Cyan;
	cp.m_NormalDir = normalDir;
	cp.m_Force = 1000000.0f;

	const EvtData_Request_New_Actor cannonBallEvt( &cp );
	safeTriggerEvent( cannonBallEvt );
	*/

	//Building the boundries
	BoundryWallParams bwp;
	bwp.m_Mat = Mat4x4::g_Identity;
	bwp.m_Mat.SetPosition(Vec3(0, 0, 140));
	bwp.m_Length = 180;
	bwp.m_ViewId = -1;
	bwp.m_Color = g_Green;
	const EvtData_Request_New_Actor requestActorBoundryWall( &bwp );
	safeTriggerEvent( requestActorBoundryWall );

	bwp.m_Mat.SetPosition(Vec3(0, 0, -140));
	const EvtData_Request_New_Actor requestActorBoundryWall2( &bwp );
	safeTriggerEvent( requestActorBoundryWall2 );

	bwp.m_Mat.BuildRotationY(-D3DX_PI / 2.0);
	bwp.m_Mat.SetPosition(Vec3(140, 0, 0));
	const EvtData_Request_New_Actor requestActorBoundryWall3( &bwp );
	safeTriggerEvent( requestActorBoundryWall3 );

	bwp.m_Mat.BuildRotationY(-D3DX_PI / 2.0);
	bwp.m_Mat.SetPosition(Vec3(-140, 0, 0));
	const EvtData_Request_New_Actor requestActorBoundryWall4( &bwp );
	safeTriggerEvent( requestActorBoundryWall4 );


	//Build the inner walls
	WallParams wp;
	wp.m_Mat = Mat4x4::g_Identity;
	wp.m_Mat.SetPosition(Vec3(-50, 0, 75));
	wp.m_Length = 42.5;
	wp.m_ViewId = -1;
	wp.m_Color = g_Green;
	const EvtData_Request_New_Actor requestActor( &wp );
	safeTriggerEvent( requestActor );

	wp.m_Mat.SetPosition(Vec3(-50, 0, -75));
	const EvtData_Request_New_Actor requestActor2( &wp );
	safeTriggerEvent( requestActor2 );

	wp.m_Mat.SetPosition(Vec3(75, 0, 75));
	const EvtData_Request_New_Actor requestActor3( &wp );
	safeTriggerEvent( requestActor3 );

	wp.m_Mat.SetPosition(Vec3(75, 0, -75));
	const EvtData_Request_New_Actor requestActor4( &wp );
	safeTriggerEvent( requestActor4 );

	wp.m_Mat.BuildRotationY(-D3DX_PI / 2.0);
	wp.m_Mat.SetPosition(Vec3(-10, 0, 110));
	const EvtData_Request_New_Actor requestActor5( &wp );
	safeTriggerEvent( requestActor5 );

	wp.m_Mat.BuildRotationY(-D3DX_PI / 2.0);
	wp.m_Mat.SetPosition(Vec3(-10, 0, -115));
	const EvtData_Request_New_Actor requestActor6( &wp );
	safeTriggerEvent( requestActor6 );

	RingParams rp;
	rp.m_StartPosition = Mat4x4::g_Identity;
	rp.m_StartPosition.SetPosition(Vec3(100,10,0));
	rp.m_Mat = rp.m_StartPosition;
	rp.m_Length = 9;
	rp.m_ViewId = -1;
	const EvtData_Request_New_Actor requestRing( &rp );
	safeTriggerEvent( requestRing );

	GoalParams gp;
	gp.m_Mat = Mat4x4::g_Identity;
	gp.m_Mat.SetPosition(Vec3(110,0,100));
	gp.m_Length = 15;
	gp.m_Team = 0;
	gp.m_ViewId = -1;
	const EvtData_Request_New_Actor requestGoalBlue( &gp );
	safeTriggerEvent( requestGoalBlue );

	gp.m_Mat.SetPosition(Vec3(110,0,-100));
	gp.m_Team = 1;
	gp.m_ViewId = -1;
	const EvtData_Request_New_Actor requestGoalRed( &gp );
	safeTriggerEvent( requestGoalRed );
}