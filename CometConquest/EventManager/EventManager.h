#pragma once

#include "../MiscStuff/String.h"
#include "../Multicore/CriticalSection.h"
#include <strstream>

char const * const kpWildcardEventType = "*";

typedef HashedString EventType;

class IEventData;
typedef boost::shared_ptr<IEventData> IEventDataPtr;

class IEventData
{
public:
	virtual const EventType & VGetEventType( void ) const = 0;
	virtual float VGetTimeStamp() const = 0;
	virtual void VSerialize(std::ostrstream &out) const = 0;

	// added for the Multicore chapter
	virtual IEventDataPtr VCopy() const = 0;
};

class BaseEventData : public IEventData
{
public:
	explicit BaseEventData( const float timeStamp = 0.0f )
		: m_TimeStamp( timeStamp )	
	{
	}

	virtual ~BaseEventData()	{ }

	//Returns the type of the event
	virtual const EventType & VGetEventType( void ) const = 0;

	float VGetTimeStamp( void ) const
	{
		return m_TimeStamp;
	}

	//Serializing for network out.
	virtual void VSerialize(std::ostrstream &out) const
	{
	};

protected:
	const float m_TimeStamp;
};

// Helper for events that require no data (still need to derive for the EventType, tho).
class EmptyEventData : public BaseEventData
{
public:
	explicit EmptyEventData( const float timeStamp = 0.0f )
		: BaseEventData( timeStamp )
	{
	}

	virtual ~EmptyEventData()	{ }

};


// IEventListener Description
//
// Base type for all event handler objects, note tht this is a
// C++ interface in the classic sense in that it defines the call
// sequence but does not provide any functionality nor aggregate
// any data.
//
// Thus, this is suitable for both single-inheritance use and MAY
// be used safely in a mutiple-inheritance construct, neither is
// imposed or assumed.

class IEventListener
{
public:

	explicit IEventListener()
	{}

	virtual ~IEventListener()
	{}

	// Returns ascii-text name for this listener, used mostly for
	// debugging

	virtual char const * GetName(void) = 0;

	// Return 'false' to indicate that this listener did NOT
	// consume the event, ( and it should continue to be
	// propogated )
	// 					
	// return 'true' to indicate that this listener consumed the
	//event, ( and it should NOT continue to be propgated )

	virtual bool HandleEvent( IEventData const & event ) = 0
	{		
		// Note: while HandleEvent() MUST be implemented in all
		// derivative classes, (as this function is pure-virtual
		// and thus the hook for IEventListener being an
		// interface definition) a base implementation is
		// provided here to make it easier to wire up do-nothing
		// stubs that can easily be wired to log the
		// unhandled-event (once logging is available)

		// HandleEvent() functioning should be kept as brief as
		// possible as multiple events will need to be evaluated
		// per-frame in many cases.
		return true;
	}
};

// IEventManager Description
//
// This is the object which maintains the list of registered
// events and their listeners
//
// This is a many-to-many relationship, as both one listener can
// be configured to process multiple event types and of course
// multiple listeners can be registered to each event type.
//
// The interface to this contruct uses smart pointer wrapped
// objects, the purpose being to ensure that no object that the
// registry is referring to is destoyed before it is removed from
// the registry AND to allow for the registry to be the only
// place where this list is kept ... the application code does
// not need to maintain a second list.
//
// Simply tearing down the registry (e.g.: destroying it) will
// automatically clean up all pointed-to objects ( so long as
// there are no other oustanding references, of course ).

typedef boost::shared_ptr<IEventListener>   EventListenerPtr;
//typedef concurrent_queue<IEventDataPtr> ThreadSafeEventQueue;

class IEventManager
{
public:

	enum eConstants
	{
		kINFINITE = 0xffffffff
	};

	explicit IEventManager( char const * const pName,
									   bool setAsGlobal );
	
	virtual ~IEventManager();

	// Register a handler for a specific event type, implicitly
	// the event type will be added to the known event types if
	// not already known.
	//
	// The function will return false on failure for any
	// reason. The only really anticipated failure reason is if
	// the input event type is bad ( e.g.: known-ident number
	// with different signature text, or signature text is empty
	// )
	
	virtual bool VAddListener ( EventListenerPtr const & inHandler,
							   EventType const & inType ) = 0;

	// Remove a listener/type pairing from the internal tables
	//
	// Returns false if the pairing was not found.
	
	virtual bool VDelListener ( EventListenerPtr const & inHandler,
							   EventType const & inType ) = 0;

	// Fire off event - synchronous - do it NOW kind of thing -
	// analogous to Win32 SendMessage() API.
	//
	// returns true if the event was consumed, false if not. Note
	// that it is acceptable for all event listeners to act on an
	// event and not consume it, this return signature exists to
	// allow complete propogation of that shred of information
	// from the internals of this system to outside uesrs.

	virtual bool VTrigger ( IEventData const & inEvent ) const = 0;

	// Fire off event - asynchronous - do it WHEN the event
	// system tick() method is called, normally at a judicious
	// time during game-loop processing.
	//
	// returns true if the message was added to the processing
	// queue, false otherwise.

	virtual bool VQueueEvent ( IEventDataPtr const & inEvent ) = 0;

	//virtual bool VThreadSafeQueueEvent ( IEventDataPtr const & inEvent ) = 0;

	// Find the next-available instance of the named event type
	// and remove it from the processing queue.
	//
	// This may be done up to the point that it is actively being
	// processed ...  e.g.: is safe to happen during event
	// processing itself.
	//
	// if 'allOfType' is input true, then all events of that type
	// are cleared from the input queue.
	//
	// returns true if the event was found and removed, false
	// otherwise

	virtual bool VAbortEvent ( EventType const & inType,
							  bool allOfType = false ) = 0;

	// Allow for processing of any queued messages, optionally
	// specify a processing time limit so that the event
	// processing does not take too long. Note the danger of
	// using this artificial limiter is that all messages may not
	// in fact get processed.
	//
	// returns true if all messages ready for processing were
	// completed, false otherwise (e.g. timeout )

	virtual bool VTick ( unsigned long maxMillis = kINFINITE ) = 0;

	// --- information lookup functions ---
	
	// Validate an event type, this does NOT add it to the
	// internal registry, only verifies that it is legal (
	// e.g. either the ident number is not yet assigned, or it is
	// assigned to matching signature text, and the signature
	// text is not empty ).

	virtual bool VValidateType( EventType const & inType ) const = 0;

private:

	// internal use only accessor for the static methods in the
	// helper to use to get the active global instance.

	static IEventManager * Get();

	// These methods are declared friends in order to get access to the
	// Get() method. Since there is no other private entity declared
	// in this class this does not break encapsulation, but does allow
	// us to do this without requiring macros or other older-style
	// mechanims.

	friend bool safeAddListener( EventListenerPtr const & inHandler,
								 EventType const & inType );
	
	friend bool safeDelListener( EventListenerPtr const & inHandler,
								 EventType const & inType );

	friend bool safeTriggerEvent( IEventData const & inEvent );

	friend bool safeQueEvent( IEventDataPtr const & inEvent );
	friend bool threadSafeQueEvent( IEventDataPtr const & inEvent );

	friend bool safeAbortEvent( EventType const & inType,
								bool allOfType = false );

	friend bool safeTickEventManager( unsigned long maxMillis =
								IEventManager::kINFINITE );

	friend bool safeValidateEventType( EventType const & inType );


};

// Lastly, these methods are used for easy-access methods to carry
// out basic operations without needing to pass around a pointer-to
// ( or some other construct ) for sharing a global event manager.

bool safeAddListener( EventListenerPtr const & inHandler,
					  EventType const & inType );
	
bool safeDelListener( EventListenerPtr const & inHandler,
					  EventType const & inType );

bool safeTriggerEvent( IEventData const & inEvent );

bool safeQueEvent( IEventDataPtr const & inEvent );
bool threadSafeQueEvent( IEventDataPtr const & inEvent );

bool safeAbortEvent( EventType const & inType,
					 bool allOfType /* = false */ );

bool safeTickEventManager( unsigned long maxMillis
					/* = IEventManager::kINFINITE */ );

bool safeValidateEventType( EventType const & inType );

