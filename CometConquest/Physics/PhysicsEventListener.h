#pragma once
//========================================================================
// PhysicsEventListener.cpp : implements the events sent FROM the phsyics system
//
// Part of the GameCode3 Application
//
// GameCode3 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 3rd Edition" by Mike McShaffry, published by
// Charles River Media. ISBN-10: 1-58450-680-6   ISBN-13: 978-1-58450-680-5
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the author a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1584506806?ie=UTF8&tag=gamecodecompl-20&linkCode=as2&camp=1789&creative=390957&creativeASIN=1584506806
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: http://gamecode3.googlecode.com/svn/trunk/
//
// (c) Copyright 2009 Michael L. McShaffry
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License v2
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================


//========================================================================
//  Content References in Game Coding Complete 3rd Edition
// 

#include "../EventManager/EventManager.h"
#include "../GameApp.h"

//-- physics trigger notification
//
//( novodex apparently supports both enter and leave,
//  unclear if both are required )

enum NxShapeFlag;
class NxShape;
typedef NxShapeFlag NxTriggerFlag;

struct EvtData_PhysTrigger_Enter : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_PhysTrigger_Enter( int triggerID,
							      optional<ActorId> other )
		: m_triggerID(triggerID),
		  m_other(other)
	{}

	IEventDataPtr VCopy() const 
	{
		return IEventDataPtr(GCC_NEW EvtData_PhysTrigger_Enter(m_triggerID, m_other));
	}

	

	int m_triggerID;
	optional<ActorId> m_other;

};

struct EvtData_PhysTrigger_Leave : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}


	explicit EvtData_PhysTrigger_Leave( int triggerID,
							      optional<ActorId> other )
		: m_triggerID(triggerID),
		  m_other(other)
	{}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_PhysTrigger_Leave(m_triggerID, m_other) );
	}

		int m_triggerID;
	optional<ActorId> m_other;

};

struct EvtData_PhysCollision : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_PhysCollision(ActorId actorA,
								ActorId actorB,
								Vec3 sumNormalForce,
								Vec3 sumFrictionForce,
								Vec3List collisionPoints )
		: m_ActorA(actorA),
		m_ActorB(actorB),
		m_SumNormalForce(sumNormalForce),
		m_SumFrictionForce(sumFrictionForce),
		m_CollisionPoints(collisionPoints)
	{}


	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_PhysCollision(m_ActorA, m_ActorB, m_SumNormalForce, m_SumFrictionForce, m_CollisionPoints));
	}

		ActorId m_ActorA;
	ActorId  m_ActorB;
	Vec3 m_SumNormalForce;
	Vec3 m_SumFrictionForce;
	Vec3List m_CollisionPoints;

};

struct EvtData_PhysSeparation : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_PhysSeparation(ActorId actorA,
								ActorId actorB
								)
		: m_ActorA(actorA)
		, m_ActorB(actorB)
	{}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_PhysSeparation(m_ActorA, m_ActorB) );
	}

		ActorId m_ActorA;
	ActorId m_ActorB;

};




// Event listener used for watching and reacting to physics
// based events

class IGamePhysics;

class PhysicsEventListener : public IEventListener
{
public:
	
	explicit PhysicsEventListener( IGamePhysics *pPhysics );
	virtual ~PhysicsEventListener();
	char const * GetName(void) { return "PhysicsEventListener"; }
	bool HandleEvent( IEventDataPtr const & event );

private:

	IGamePhysics *m_pPhysics;
};
