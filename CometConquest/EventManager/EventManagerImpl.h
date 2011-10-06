#pragma once

#include "EventManager.h"

#include <vector>
#include <list>
#include <map>
#include <set>


typedef std::vector<EventListenerPtr> EventListenerList;

typedef std::vector<EventType>      EventTypeList;

class EventManager : public IEventManager
{
public:

	explicit EventManager( char const * const pName,
									   bool setAsGlobal );

	virtual ~EventManager();

	// --- primary use functions --- make it work ---
	
	// Register a listener for a specific event type, implicitly
	// the event type will be added to the known event types if
	// not already known.
	//
	// The function will return false on failure for any
	// reason. The only really anticipated failure reason is if
	// the input event type is bad ( e.g.: known-ident number
	// with different signature text, or signature text is empty
	// )
	
	virtual bool VAddListener ( EventListenerPtr const & inListener,
					   EventType const & inType );

	// Remove a listener/type pairing from the internal tables
	//
	// Returns false if the pairing was not found.
	
	virtual bool VDelListener ( EventListenerPtr const & inListener,
					   EventType const & inType );

	// Fire off event - synchronous - do it NOW kind of thing -
	// analogous to Win32 SendMessage() API.
	//
	// returns true if the event was consumed, false if not. Note
	// that it is acceptable for all event listeners to act on an
	// event and not consume it, this return signature exists to
	// allow complete propogation of that shred of information
	// from the internals of this system to outside uesrs.

	virtual bool VTrigger ( IEventData const & inEvent ) const;

	// Fire off event - asynchronous - do it WHEN the event
	// system tick() method is called, normally at a judicious
	// time during game-loop processing.
	//
	// returns true if the message was added to the processing
	// queue, false otherwise.

	virtual bool VQueueEvent ( IEventDataPtr const & inEvent );

	// This was added to allow real time events to be sent into the 
	// event manager.
	//virtual bool VThreadSafeQueueEvent ( IEventDataPtr const & inEvent );

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
					  bool allOfType );

	// Allow for processing of any queued messages, optionally
	// specify a processing time limit so that the event
	// processing does not take too long. Note the danger of
	// using this artificial limiter is that all messages may not
	// in fact get processed.
	//
	// returns true if all messages ready for processing were
	// completed, false otherwise (e.g. timeout )

	virtual bool VTick ( unsigned long maxMillis );

	// --- information lookup functions ---
	
	// Validate an event type, this does NOT add it to the
	// internal registry, only verifies that it is legal (
	// e.g. either the ident number is not yet assigned, or it is
	// assigned to matching signature text, and the signature
	// text is not empty ).

	virtual bool VValidateType( EventType const & inType ) const;

	// Get the list of listeners associated with a specific event
	// type

	EventListenerList GetListenerList ( EventType const & eventType ) const;

	// Get the list of known event types

	EventTypeList GetTypeList ( void ) const;

	void RegisterCodeOnlyEvent( const EventType & eventType );


private:
	
	// This class holds meta data for each event type, and allows 
	// (or disallows!) creation of code-defined events from script.
	
	// class IRegisteredEvent				- Chapter 11, page 324
	class IRegisteredEvent
	{
	};


	// class CodeOnlyDefinedEvent					- Chapter 11, page 327
	//   Code defined, but script is NOT allowed to create this event type.
	class CodeOnlyDefinedEvent : public IRegisteredEvent
	{
	public:
		explicit CodeOnlyDefinedEvent( void )
		{
		}

		
	};


	typedef boost::shared_ptr< IRegisteredEvent > IRegisteredEventPtr;

	//Verifies that such an event does not already exist, then registers it.
	void AddRegisteredEventType( const EventType & eventType, IRegisteredEventPtr metaData );

	// one global instance
	typedef std::map< EventType, IRegisteredEventPtr >		EventTypeSet;

	// insert result into event type set
	typedef std::pair< EventTypeSet::iterator, bool >		EventTypeSetIRes;

	// one list per event type ( stored in the map )
	typedef std::list< EventListenerPtr >					EventListenerTable;

	// mapping of event ident to listener list
	typedef std::map< unsigned int, EventListenerTable >	EventListenerMap;

	// entry in the event listener map
	typedef std::pair< unsigned int, EventListenerTable >	EventListenerMapEnt;

	// insert result into listener map
	typedef std::pair< EventListenerMap::iterator, bool >	EventListenerMapIRes;

	// queue of pending- or processing-events
	typedef std::list< IEventDataPtr >						EventQueue;

	enum eConstants
	{
		kNumQueues = 2
	};
	
	EventTypeSet     m_typeList;           // list of registered
											// event types

	EventListenerMap m_registry;           // mapping of event types
											// to listeners

	EventQueue       m_queues[kNumQueues]; // event processing queue,
											// double buffered to
											// prevent infinite cycles

	int               m_activeQueue;        // valid denoting which
											// queue is actively
											// processing, en-queing
											// events goes to the
											// opposing queue

	//ThreadSafeEventQueue m_RealtimeEventQueue;

private:
	
};



// Event listener used for snoooping ... simply emits event stats
// for all processed & known events to runtime log ( actually, in
// this case, debugger output )

class EventSnooper : public IEventListener
{
public:
	
	explicit EventSnooper( char const * const kpLogFileName = NULL);
	virtual ~EventSnooper();
	char const * GetName(void) { return "Snoop"; }
	bool HandleEvent( IEventData const & event );

private:

	FILE *m_OutFile;
	char m_msgBuf[4090];

};