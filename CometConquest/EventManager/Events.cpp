#include "std.h"

#include "Events.h"

const EventType EvtData_New_Game::sk_EventType( "new_game" );
const EventType EvtData_New_Actor::sk_EventType( "new_actor" );
const EventType EvtData_Move_Actor::sk_EventType( "move_actor" );
const EventType EvtData_Destroy_Actor::sk_EventType( "destroy_actor" );
const EventType EvtData_Game_State::sk_EventType( "game_state" );
const EventType EvtData_Request_Start_Game::sk_EventType( "game_request_start" );
const EventType EvtData_Remote_Client::sk_EventType( "remote_client" );
const EventType EvtData_Network_Player_Actor_Assignment::sk_EventType ( "netplayer_actor_assign" );
const EventType EvtData_Update_Tick::sk_EventType( "update_tick" );
const EventType EvtData_Request_New_Actor::sk_EventType( "request_new_actor" );

// Note: these are game specific events, which really should be in TeapotEvents.cpp.
// They aren't because of an annoying dependancy in Network.cpp, which creates events
// from a bitstream. Once we get "real" streaming code in we can move these back to TeapotEvents.cpp.
const EventType EvtData_Fire_Weapon::sk_EventType( "fire_weapon" );
const EventType EvtData_Thrust::sk_EventType( "thrust" );
const EventType EvtData_Steer::sk_EventType( "steer" );

ActorParams *ActorParams::CreateFromStream(std::istrstream &in)
{
	int actorType;
	in >> actorType;

	ActorParams *actor = NULL;
	switch (actorType)
	{
		case AT_Ship:
			actor = GCC_NEW ShipParams;
			break;
		case AT_Wall:
			actor = GCC_NEW WallParams;
			break;
		case AT_Bullet:
			actor = GCC_NEW BulletParams;
			break;
		case AT_Floor:
			actor = GCC_NEW FloorParams;
			break;
		case AT_BoundryWall:
			actor = GCC_NEW BoundryWallParams;
			break;
		case AT_Comet:
			actor = GCC_NEW CometParams;
			break;
		case AT_Ring:
			actor = GCC_NEW RingParams;
			break;
		case AT_Goal:
			actor = GCC_NEW GoalParams;
			break;
		default:
			assert(0 && _T("Unimplemented actor type in stream"));
			return 0;
	}

	if (! actor->VInit(in))
	{
		// something went wrong with the serialization...
		assert(0 && _T("Error in Actor stream initialization"));
		SAFE_DELETE(actor);
	}

	return actor;

}