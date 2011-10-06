#pragma once

#include "SceneGraph\SceneNodes.h"
#include "EventManager\EventManager.h"

class MainMenuUI : public CBaseUI
{
protected:
	CDXUTDialog m_SampleUI;					// dialog for sample specific controls
	void Set();
	bool m_bCreatingGame;
	int m_NumAIs;
	int m_NumPlayers;

	std::string m_HostName;
	std::string m_HostListenPort;
	std::string m_ClientAttachPort;
	CDXUTTextHelper*            m_pTxtHelper;
public:
	MainMenuUI();
	virtual ~MainMenuUI();

	// IScreenElement Implementation
	virtual HRESULT VOnRestore();
	virtual HRESULT VOnRender(double fTime, float fElapsedTime);
	virtual int VGetZOrder() const { return 1; }
	virtual void VSetZOrder(int const zOrder) { }

	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );
	static void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext );
	void CALLBACK _OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void *pUserContext );
};

class MainMenuView : public HumanView
{
protected:
	shared_ptr<MainMenuUI> m_MainMenuUI; 
public:

	MainMenuView(); 
	~MainMenuView(); 
	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );
	virtual void VOnUpdate(int deltaMilliseconds);
};

class ShipController;

class CometConquestGameView : public HumanView
{
	friend class CometConquestGameViewListener;

protected:
	bool  m_bShowUI;					// If true, it renders the UI control text
	BaseGameState m_BaseGameState;	// what is the current game state
	CDXUTDialog m_ScoreOverlay;
	shared_ptr<ScreenElementScene> m_pScene;
	shared_ptr<ShipController> m_pShipController;
	//shared_ptr<CameraNode> m_pCamera;
	shared_ptr<SceneNode> m_pShip;
	//shared_ptr<StandardHUD> m_StandardHUD; 
	bool m_bRender;
	void BuildInitialScene();

	void MoveActor(ActorId id, Mat4x4 const &mat);
	void HandleGameState(BaseGameState newState);
public:

	CometConquestGameView(bool bRender); 
	~CometConquestGameView();

	virtual LRESULT CALLBACK VOnMsgProc( AppMsg msg );	
	virtual void VOnUpdate(int deltaMilliseconds);
	virtual void VOnAttach(GameViewId vid, optional<ActorId> aid);
	virtual void VOnRender(double fTime, float fElapsedTime);
	//virtual void VSetCameraOffset(const Vec4 & camOffset );
};


class CometConquestGameViewListener : public IEventListener
{
	CometConquestGameView *m_pView;
public:
	explicit CometConquestGameViewListener( CometConquestGameView *view);
	virtual char const * GetName(void) { return "TeapotWarsGameViewListener"; }
	virtual bool HandleEvent( IEventData const & event );
};
