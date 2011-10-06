#pragma once

#pragma warning( disable : 4996 ) // 'function' declared deprecated - gets rid of all those 2005 warnings....

class CometConquestBaseGame;

class CometConquestGameApp : public GameApp
{
private:
	void RegisterGameSpecificEvents( void );	//Registers game-specific events for later usage.

protected:
	virtual BaseGameLogic *VCreateGameAndView();
	virtual bool VLoadGame();

public:
	CometConquestGameApp() : GameApp() { }
	inline CometConquestBaseGame const * const GetGame() const;

	virtual TCHAR *VGetGameTitle() { return _T("Comet Conquest"); }
	virtual TCHAR *VGetGameAppDirectory() { return _T("CometConquest"); }
	virtual HICON VGetIcon();
};

class IEventManager;
typedef std::map<ActorId, int> ActorScoreMap;

class CometConquestGameData 
{
public:
	int redTeamScore;
	int blueTeamScore;
	unsigned int m_lastCometTime;
};

class CometConquestBaseGame : public BaseGameLogic
{
protected:
	// TODO: This should really be private
	CometConquestGameData m_data;


public: 
	CometConquestBaseGame(GameOptions const &options)
		: BaseGameLogic(options) 
	{ 
		m_data.blueTeamScore = 0;
		m_data.redTeamScore = 0;
	}

	bool VLoadGame(std::string gameName);
	void VRemoveActor(ActorId id);
	void VAddActor(shared_ptr<IActor> actor, ActorParams *p);
	void VSetProxy() { BaseGameLogic::VSetProxy(); }
	const int getBlueTeamScore () const { return m_data.blueTeamScore; }
	const int getRedTeamScore () const { return m_data.redTeamScore; }
	void blueTeamScore () { m_data.blueTeamScore++; }
	void redTeamScore () { m_data.redTeamScore++; }
	// overloadable

	const CometConquestGameData& GetData() const { return m_data; }
};

class CometConquestGame : public CometConquestBaseGame
{
	friend class CometConquestEventListener;

protected:
	float m_Lifetime;	//indicates how long this game has been in session
	Vec3 m_StartPosition;
	int m_HumanPlayersAttached;
	EventListenerPtr m_cometConquestEventListener;
public:
	CometConquestGame(struct GameOptions const &options);

	~CometConquestGame();


	// Update
	virtual void VOnUpdate(float time, float elapsedTime);
	virtual void VSetProxy();

	// Overloads
	virtual void VRegisterHit(const ActorId sphere, const ActorId ship);
	virtual void VBuildInitialScene();
	virtual void VChangeState(BaseGameState newState);
	virtual void VAddView(shared_ptr<IGameView> pView, optional<ActorId> actor=optional_empty());
	virtual shared_ptr<IGamePhysics> VGetGamePhysics(void) { return m_pPhysics; }
	
};

inline CometConquestBaseGame const * const  CometConquestGameApp::GetGame() const
{ 
	return dynamic_cast<CometConquestBaseGame*>(m_pGame); 
}