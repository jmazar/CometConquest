#include "CometConquestStd.h"

#include "ShipController.h"
#include "../SceneGraph/SceneNodes.h"
#include "CometConquestEvents.h"

ShipController::ShipController(shared_ptr<SceneNode> object)
	: m_object(object)
{
	memset(m_bKey, 0x00, sizeof(m_bKey));
	m_LastShotTime = timeGetTime();
}

void ShipController::OnUpdate(DWORD const deltaMilliseconds)
{
	unsigned int timeNow = timeGetTime();
	if (m_bKey['W'] || m_bKey['S'])
	{
		const ActorId actorID = *m_object->VGet()->ActorId();
		safeQueEvent( IEventDataPtr( GCC_NEW EvtData_Thrust( actorID, m_bKey['W']? 0.04f : -0.04f ) ) );
	}
	if (m_bKey['A'] || m_bKey['D'])
	{
		const ActorId actorID = *m_object->VGet()->ActorId();
		safeQueEvent( IEventDataPtr( GCC_NEW EvtData_Steer( actorID, m_bKey['A']? -0.04f : 0.04 ) ) );
	}
	if(m_bKey['F'])
	{
		if(timeNow > m_LastShotTime + 1000)
		{
			optional<ActorId> aid = m_object->VGet()->ActorId();
			safeQueEvent( IEventDataPtr( GCC_NEW EvtData_Fire_Weapon( *aid ) ) );
			m_LastShotTime = timeNow;
			m_bKey['F'] = false;
		}
	}

}