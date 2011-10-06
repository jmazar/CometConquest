
#include "std.h"

#include <cstring>
#include "EventManager.h"

static IEventManager * g_pEventMgr = NULL;

IEventManager * IEventManager::Get()
{
	return g_pEventMgr;
}

IEventManager::IEventManager(
	char const * const pName,
	bool setAsGlobal )
{
	if ( setAsGlobal )
		g_pEventMgr = this;
}

IEventManager::~IEventManager()
{
	if ( g_pEventMgr == this )
		g_pEventMgr = NULL;
}

//--

// lastly, some macros that make it less difficult to safely use
// a possibly not-ready system ...

bool safeAddListener( EventListenerPtr const & inHandler, EventType const & inType )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VAddListener( inHandler, inType );
}

bool safeDelListener( EventListenerPtr const & inHandler, EventType const & inType )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VDelListener( inHandler, inType );
}

bool safeTriggerEvent( IEventData const & inEvent )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VTrigger( inEvent );
}

bool safeQueEvent( IEventDataPtr const & inEvent )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VQueueEvent( inEvent );
}

//bool threadSafeQueEvent ( IEventDataPtr const & inEvent )
//{
//	assert(IEventManager::Get() && _T("No event manager!"));
//	return IEventManager::Get()->VThreadSafeQueueEvent( inEvent );
//}

bool safeAbortEvent( EventType const & inType,
					 bool allOfType /*= false*/ )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VAbortEvent( inType, allOfType );
}

bool safeTickEventManager( unsigned long maxMillis /*= kINFINITE*/ )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VTick( maxMillis );
}

bool safeValidateEventType( EventType const & inType )
{
	assert(IEventManager::Get() && _T("No event manager!"));
	return IEventManager::Get()->VValidateType( inType );
}
