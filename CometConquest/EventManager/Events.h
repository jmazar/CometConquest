#pragma once

#include "EventManager.h"
#include "../GameApp.h"
#include "../Actors.h"

// This event is sent out when an actor is
// *actually* created.
struct EvtData_New_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_New_Actor( ActorId id,
							  ActorParams *pCreateParams)
	{
		m_id = id;
		m_pActorParams = reinterpret_cast<ActorParams *>(GCC_NEW char[pCreateParams->m_Size]);
		memcpy(m_pActorParams, pCreateParams, pCreateParams->m_Size);
		m_pActorParams->m_Id = id;
	}

	explicit EvtData_New_Actor( std::istrstream & in )
	{
		in >> m_id;
		m_pActorParams = ActorParams::CreateFromStream( in );
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_New_Actor( m_id, m_pActorParams ) );
	}

	virtual ~EvtData_New_Actor() 
	{ 
		SAFE_DELETE(m_pActorParams); 
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id << " ";
		m_pActorParams->VSerialize(out);
	}

	ActorId m_id;	//ID of actor created
	ActorParams *m_pActorParams;	//Parameters for actor

};

// struct EvtData_Destroy_Actor				- Chapter 10, 279

struct EvtData_Destroy_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Destroy_Actor( ActorId id )
		: m_id( id )
	{
	}

	explicit EvtData_Destroy_Actor( std::istrstream & in )
	{
		in >> m_id;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Destroy_Actor ( m_id ) );
	}

	virtual ~EvtData_Destroy_Actor() {}

	

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id;
	}

	ActorId m_id;

};

struct EvtData_Move_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Move_Actor( ActorId id, const Mat4x4 & mat)
	{
		m_Id = id;
		m_Mat = mat;
	}

	explicit EvtData_Move_Actor( std::istrstream & in )
	{
		in >> m_Id;
		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				in >> m_Mat.m[i][j];
			}
		}
	}

		virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_Id << " ";
		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				out << m_Mat.m[i][j] << " ";
			}
		}
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr(GCC_NEW EvtData_Move_Actor(m_Id, m_Mat));
	}

	ActorId m_Id;
	Mat4x4 m_Mat;
};

class EvtData_New_Game : public EmptyEventData
{
public:
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_New_Game()
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_New_Game( ) );
	}

	EvtData_New_Game( std::istrstream &in )
	{
	}
};

class EvtData_Request_Start_Game : public EmptyEventData
{
public:
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_Request_Start_Game()
	{
	}


	EvtData_Request_Start_Game( std::istrstream &in )
	{
	}


	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Request_Start_Game( ) );
	}
};

struct EvtData_Game_State : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Game_State( const BaseGameState gameState )
		: m_gameState( gameState )
	{
	}

	EvtData_Game_State( std::istrstream &in )
	{
		int tempVal;
		in >> tempVal;
		m_gameState = static_cast<BaseGameState>( tempVal );
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Game_State( m_gameState ) );
	}

	virtual ~EvtData_Game_State() {}

	

	virtual void VSerialize(std::ostrstream &out) const
	{
		const int tempVal = static_cast< int >( m_gameState );
		out << tempVal;
	}

	BaseGameState m_gameState;
};

struct EvtData_Remote_Client : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Remote_Client( const int socketid, const int ipaddress )
		: m_socketId( socketid ), m_ipAddress(ipaddress)
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Remote_Client( m_socketId, m_ipAddress ) );
	}

	EvtData_Remote_Client( std::istrstream &in )
	{
		in >> m_socketId;
		in >> m_ipAddress;
	}

	virtual ~EvtData_Remote_Client() {}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_socketId << " ";
		out << m_ipAddress;
	}

	int m_socketId;
	int m_ipAddress;

};

struct EvtData_Update_Tick : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Update_Tick( const int deltaMilliseconds )
		: m_DeltaMilliseconds( deltaMilliseconds )
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_Update_Tick ( m_DeltaMilliseconds ) );
	}

	
	virtual void VSerialize( std::ostrstream & out )
	{
		assert( 0 && "You should not be serializing update ticks!" );
	}

	int m_DeltaMilliseconds;
};

struct EvtData_Network_Player_Actor_Assignment : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Network_Player_Actor_Assignment( const int actorId, const int remotePlayerId )
		: m_actorId( actorId ), m_remotePlayerId(remotePlayerId)
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		 return IEventDataPtr( GCC_NEW EvtData_Network_Player_Actor_Assignment( m_actorId, m_remotePlayerId ) ) ;
	}

	EvtData_Network_Player_Actor_Assignment( std::istrstream &in )
	{
		in >> m_actorId;
		in >> m_remotePlayerId;
	}


	virtual ~EvtData_Network_Player_Actor_Assignment() {}

	
	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_actorId << " ";
		out << m_remotePlayerId;
	}

	int m_actorId;
	int m_remotePlayerId;

};

struct EvtData_Request_New_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	

	explicit EvtData_Request_New_Actor( const ActorParams *pCreateParams )
		: m_pActorParams( NULL )
	{
		m_pActorParams = reinterpret_cast<ActorParams *>(GCC_NEW char[pCreateParams->m_Size]);
		memcpy(m_pActorParams, pCreateParams, pCreateParams->m_Size);
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Request_New_Actor ( m_pActorParams ) );
	}

	virtual ~EvtData_Request_New_Actor()
	{
		SAFE_DELETE( m_pActorParams );
	}


	ActorParams * m_pActorParams;
};

class EvtData_Fire_Weapon : public BaseEventData
{
public:
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType() const
	{
		return sk_EventType;
	}

	explicit EvtData_Fire_Weapon( ActorId id )
		: m_id( id )
	{
	}

	explicit EvtData_Fire_Weapon( std::istrstream & in )
	{
		in >> m_id;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Fire_Weapon (m_id) );
	}

	virtual ~EvtData_Fire_Weapon() {}

	virtual void VSerialize( std::ostrstream & out ) const
	{
		out << m_id << " ";
	}

	ActorId m_id;

};

struct EvtData_Thrust : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Thrust( ActorId id, float throttle )
		: m_id(id), 
		  m_throttle(throttle)
	{}

	explicit EvtData_Thrust( std::istrstream & in )
	{
		in >> m_id;
		in >> m_throttle;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Thrust (m_id, m_throttle) );
	}

	virtual ~EvtData_Thrust()
	{
	}

	ActorId m_id;
	float m_throttle;


	virtual void VSerialize( std::ostrstream & out ) const
	{
		out << m_id << " ";
		out << m_throttle << " ";
	}

};

struct EvtData_Steer : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Steer( ActorId id, float dir)
		: m_id(id), 
		  m_dir(dir)
	{}

	explicit EvtData_Steer( std::istrstream & in )
	{
		in >> m_id;
		in >> m_dir;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Steer (m_id, m_dir) );
	}

	virtual ~EvtData_Steer()
	{
	}

	virtual void VSerialize( std::ostrstream & out ) const
	{
		out << m_id << " ";
		out << m_dir << " ";
	}

	ActorId m_id;
	float m_dir;		// -1.0 is all the way left, 0 is straight, 1.0 is right

};