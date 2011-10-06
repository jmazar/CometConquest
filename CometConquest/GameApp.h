#pragma once


#include "std.h"
#include "Actors.h"
#include "MiscStuff\CMath.h"
#include "MainLoop\CProcess.h"
#include "SceneGraph\SceneNodes.h"
#include "EventManager\EventManager.h"

#define QUIT_NO_PROMPT MAKELPARAM(-1,-1)


class BaseGameLogic;
class EventManager;
class BaseSocketManager;


class GameApp
{
protected:
	HINSTANCE m_hInstance;
	bool m_bWindowedMode;
	bool m_bIsRunning;
	bool m_bQuitRequested;
	bool m_bQuitting;
	CRect m_rcWindow, m_rcDesktop;
	int m_iColorDepth;

public:
	GameApp();

	virtual TCHAR *VGetGameTitle()=0;
	virtual TCHAR *VGetGameAppDirectory()=0;
	virtual HICON VGetIcon() = 0;

	LRESULT OnDisplayChange(int colorDepth, int width, int height);
	LRESULT OnPowerBroadcast(int event);
	LRESULT OnSysCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnClose();


	LRESULT OnAltEnter();

	HWND GetHwnd();
	HINSTANCE GetInstance() { return m_hInstance; }
	virtual bool InitInstance(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hWnd = NULL, int screenWidth = SCREEN_WIDTH, int screenHeight = SCREEN_HEIGHT);
	void ParseCommandLine(LPWSTR lpCmdLine);

	static LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void *pUserContext );

	
	static HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
										  void* pUserContext );
	static HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
	static void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
	static void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
									  float fElapsedTime, void* pUserContext );
	static void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );

	static CDXUTDialogResourceManager g_DialogResourceManager;

		// GameCode Specific Stuff
	BaseGameLogic *m_pGame;
	struct GameOptions *m_pOptions;

	virtual BaseGameLogic *VCreateGameAndView()=0;
	virtual bool VLoadGame()=0;

	EventManager *m_pEventManager;
	void RegisterBaseGameEvents( void );

	BaseSocketManager *m_pBaseSocketManager;

	void AbortGame() { m_bQuitting = true; }
	int GetExitCode() { return DXUTGetExitCode(); }
	bool IsRunning() { return m_bIsRunning; }
	void SetQuitting(bool quitting) { m_bQuitting = quitting; }
};

enum BaseGameState
{
	BGS_Initializing,
	BGS_LoadingGameEnvironment,
	BGS_MainMenu,
	BGS_WaitingForPlayers,
	BGS_Running
};

typedef std::map<ActorId, shared_ptr<IActor> > ActorMap;

class BaseGameLogic : public IGameLogic
{
	friend class GameApp;						// This is only to gain access to the view list

protected:
	CProcessManager *m_pProcessManager;				// a game logic entity
	CRandom m_random;								// our RNG
	ActorMap m_ActorList;
	ActorId m_LastActorId;
	BaseGameState m_State;							// game state: loading, running, etc.
	int m_ExpectedPlayers;							// how many local human players
	int m_ExpectedRemotePlayers;					// expected remote human players
	GameViewList m_gameViews;						// views that are attached to our game
    

	bool m_bProxy;									// set if this is a proxy game logic, not a real one
	int m_remotePlayerId;							// if we are a remote player - what is out socket number on the server

	// Refactor Chapter 19/20 work.
	//bool m_RenderDiagnostics;						// Are we rendering diagnostics?
	shared_ptr<IGamePhysics> m_pPhysics;

public:

	BaseGameLogic(struct GameOptions const &optionss);
	virtual ~BaseGameLogic();

	ActorId GetNewActorID( void )
	{
		return ++m_LastActorId;
	}
	
	ActorId GetRandomActor(optional<ActorId> ignoreMe);
	
	CRandom& GetRNG(void) { return m_random; }

	virtual void VBuildInitialScene();


	virtual void VAddView(shared_ptr<IGameView> pView, optional<ActorId> actorId=optional_empty());
	virtual void VRemoveView(shared_ptr<IGameView> pView);

	virtual void VAddActor(shared_ptr<IActor> actor, ActorParams *p);
	virtual shared_ptr<IActor> VGetActor(const ActorId id);				// TODO: Make Protected...
	virtual void VRemoveActor(ActorId id);
	virtual void VMoveActor(const ActorId id, Mat4x4 const &mat);

	virtual bool VLoadGame(std::string gameName) { return true; }
	virtual void VSetProxy() { m_bProxy = true; }

	// Logic Update
	virtual void VOnUpdate(float time, float elapsedTime);

	// Changing Game Logic State
	virtual void VChangeState(BaseGameState newState);

	// Render Diagnostics
	//virtual void VRenderDiagnostics();
	virtual shared_ptr<IGamePhysics> VGetGamePhysics(void) { return m_pPhysics; }
	
	void AttachProcess(shared_ptr<CProcess> pProcess) { if (m_pProcessManager) {m_pProcessManager->Attach(pProcess);} }
};

class CBaseUI : public IScreenElement
{
protected:
	int					m_PosX, m_PosY;
	int					m_Width, m_Height;
	optional<int>		m_Result;
	bool				m_bIsVisible;
public:
	CBaseUI() { m_bIsVisible = true; m_PosX = m_PosY = 0; m_Width = 100; m_Height = 100; }
	virtual void VOnUpdate(int) { };
	virtual bool VIsVisible() const { return m_bIsVisible; }
	virtual void VSetVisible(bool visible) { m_bIsVisible = visible; }
};



class ScreenElementScene : public IScreenElement, public Scene
{
public:
	ScreenElementScene() : Scene() { }

	// IScreenElement Implementation
	virtual void VOnUpdate(int deltaMS) { OnUpdate(deltaMS); };
	virtual HRESULT VOnRestore() 
		{ OnRestore(); return S_OK; }
	virtual HRESULT VOnRender(double fTime, float fElapsedTime)
		{ OnRender(); return S_OK; }
	virtual int VGetZOrder() const { return 0; }
	virtual void VSetZOrder(int const zOrder) { }

	// Don't handle any messages
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg ) { return 0; }

	virtual bool VIsVisible() const { return true; }
	virtual void VSetVisible(bool visible) { }
	virtual bool VAddChild(optional<ActorId> id, shared_ptr<ISceneNode> kid) { return Scene::AddChild(id, kid); }
};


//
// class HumanView - Chapter 9, page 235
//
// This is a view onto the game - which includes anything visual or audible.
// It gets messages about the changes in game state.
//

class HumanView : public IGameView
{
protected:
	GameViewId		m_ViewId;
	optional<ActorId> m_ActorId;

	CProcessManager *m_pProcessManager;				// strictly for things like button animations, etc.

	DWORD m_currTick;		// time right now
	DWORD m_lastDraw;		// last time the game rendered
	bool m_runFullSpeed;	// set to true if you want to run full speed

	ID3DXFont*			        m_pFont;
	ID3DXSprite*				m_pTextSprite;



public:
	// Implement the IGameView interface
	virtual HRESULT VOnRestore();
	virtual void VOnRender(double fTime, float fElapsedTime );
	virtual void VOnLostDevice();
	virtual GameViewType VGetType() { return GameView_Human; }
	virtual GameViewId VGetId() const { return m_ViewId; }

	virtual void VOnAttach(GameViewId vid, optional<ActorId> aid)
	{
		m_ViewId = vid; 
		m_ActorId = aid;
	}
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );
	virtual void VOnUpdate( int deltaMilliseconds );

	// Virtual methods to control the layering of interface elements
	virtual void VPushElement(shared_ptr<IScreenElement> pScreen);
	virtual void VPopElement(shared_ptr<IScreenElement> pElement);


	virtual ~HumanView();
	HumanView();

	ScreenElementList m_ScreenElements;						// a game screen entity

	// Interface sensitive objects
	shared_ptr<IMouseHandler> m_MouseHandler;
	shared_ptr<IKeyboardHandler> m_KeyboardHandler;

	//Camera adjustments.
	virtual void VSetCameraOffset(const Vec4 & camOffset ) { }
};


extern GameApp *g_pApp;