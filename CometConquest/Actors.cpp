
#pragma once
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
//  struct ActorParams				- Chapter 19, page 683
//  struct SphereParams				- Chapter 19, page 738
//  struct TeapotParams				- Chapter 19, page 690




/****************************************

IMPORTANT GROUND RULES FOR DEFINING ACTOR PARAMETERS!!!!

1. No destructors.
2. No pointers.
3. Never break rules one, two, or three.

Why? These parameters are marshalled over god knows what. 
Could be a function call - or it could be the internet.

****************************************/

#include "std.h"

#include <strstream>
#include "GameApp.h"
#include "Actors.h"
#include "Physics/Physics.h"
#include "SceneGraph/SceneNodes.h"




ActorParams::ActorParams() 
{
	m_Pos=Vec3(0,0,0); 
	m_Type=AT_Unknown; 
	m_Color = g_White;
	m_Size=sizeof(ActorParams); 
}

//
// ActorParams::VInit					- Chapter 19, page 684
//
bool ActorParams::VInit(std::istrstream &in)
{ 
	int hasActorId = 0;

	in >> m_Size;
	in >> hasActorId;
	if (hasActorId)
	{
		in >> hasActorId;
		m_Id = hasActorId;
	}
	in >> m_Pos.x >> m_Pos.y >> m_Pos.z;
	in >> m_Color.r >> m_Color.g >> m_Color.b >> m_Color.a;
	return true;
}


//
// ActorParams::VInit						- Chpater 19, page 685
//
bool ActorParams::VInit( optional<ActorId> id, const Vec3& pos, const Color& color  )
{
	//ActorId, Position, And Color
	m_Id = id;
	m_Pos = pos;
	m_Color = color;

	return true;
}


//
// ActorParams::VSerialize					- Chapter 19, 685
//
void ActorParams::VSerialize(std::ostrstream &out) const
{
	out << m_Type << " ";
	out << m_Size << " ";
	out << static_cast<int>(m_Id.valid()) << " ";
	if (m_Id.valid())
	{
		out << *m_Id << " ";
	}
	out << m_Pos.x << " " << m_Pos.y << " " << m_Pos.z << " ";
	out << m_Color.r << " " << m_Color.g << " " << m_Color.b << " " << m_Color.a << " ";
}

ShipParams::ShipParams() 
	: ActorParams()
{ 
	m_Type=AT_Ship; 
	m_Length=1.0f; 
	m_ViewId = VIEWID_NO_VIEW_ATTACHED;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(ShipParams); 
}

bool ShipParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Ship; 
		in >> m_Length;
		in >> m_ViewId;
		in >> m_Team;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_StartPosition.m[i][j];
		return true;
	}
	return false;
}

//
// TeapotParams::VInit					- Chapter 19, page 691
//
bool ShipParams::VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color, Mat4x4 startPosition, int team )
{
	m_Type = AT_Ship;
	if( ActorParams::VInit( id, pos, color ) )
	{
		m_Length = length;
		m_ViewId = viewId;
		m_Mat = mat;
		m_StartPosition = startPosition;
		m_Team = team;
	}


	return true;
}

void ShipParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Length << " ";
	out << m_ViewId << " ";
	out << m_Team << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_StartPosition.m[i][j] << " ";
}

//
// TeapotParams::VCreate				- Chapter 19, page 693
//
shared_ptr<IActor> ShipParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pShip(new BaseActor(mat, AT_Ship, shared_ptr<ShipParams>(GCC_NEW ShipParams(*this))));
	logic->VAddActor(pShip, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Length, m_Length/3, m_Length), &*pShip, SpecificGravity(PhysDens_Water), PhysMat_Normal);
	return pShip;
}



WallParams::WallParams() 
	: ActorParams()
{ 
	m_Type=AT_Wall; 
	m_Length=1.0f; 
	m_ViewId = VIEWID_NO_VIEW_ATTACHED;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(ShipParams); 
}

bool WallParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Wall; 
		in >> m_Length;
		in >> m_ViewId;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];

		return true;
	}
	return false;
}

//
// TeapotParams::VInit					- Chapter 19, page 691
//
bool WallParams::VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color )
{
	m_Type = AT_Wall;
	if( ActorParams::VInit( id, pos, color ) )
	{
		m_Length = length;
		m_ViewId = viewId;
		m_Mat = mat;
	}


	return true;
}

void WallParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Length << " ";
	out << m_ViewId << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";


}

//
// TeapotParams::VCreate				- Chapter 19, page 693
//
shared_ptr<IActor> WallParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pWall(new BaseActor(mat, AT_Wall, shared_ptr<WallParams>(GCC_NEW WallParams(*this))));
	logic->VAddActor(pWall, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Length, m_Length/10, m_Length/10), &*pWall, SpecificGravity(PhysDens_Infinite), PhysMat_Normal);
	return pWall;
}



BoundryWallParams::BoundryWallParams() 
	: ActorParams()
{ 
	m_Type=AT_BoundryWall; 
	m_Length=1.0f; 
	m_ViewId = VIEWID_NO_VIEW_ATTACHED;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(ShipParams); 
}

bool BoundryWallParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Wall; 
		in >> m_Length;
		in >> m_ViewId;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];

		return true;
	}
	return false;
}


bool BoundryWallParams::VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color )
{
	m_Type = AT_Wall;
	if( ActorParams::VInit( id, pos, color ) )
	{
		m_Length = length;
		m_ViewId = viewId;
		m_Mat = mat;
	}


	return true;
}

void BoundryWallParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Length << " ";
	out << m_ViewId << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";

}

//
// TeapotParams::VCreate				- Chapter 19, page 693
//
shared_ptr<IActor> BoundryWallParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pWall(new BaseActor(mat, AT_BoundryWall, shared_ptr<BoundryWallParams>(GCC_NEW BoundryWallParams(*this))));
	logic->VAddActor(pWall, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Length, m_Length*100, m_Length/10), &*pWall, SpecificGravity(PhysDens_Infinite), PhysMat_Normal);
	return pWall;
}

BulletParams::BulletParams() 
	: ActorParams()
{ 
	m_Type=AT_Bullet; 
	m_Radius=1.0f; 
	m_Segments=16; 
	m_Size=sizeof(BulletParams);
	m_NormalDir= Vec3( 0.0f, 0.0f, 0.0f );
	m_Force=0.0f;
}

bool BulletParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Bullet;
		in >> m_Radius;
		in >> m_Segments;
		in >> m_NormalDir.x >> m_NormalDir.y >> m_NormalDir.y;
		in >> m_Force;
		in >> m_TeamFiredBy;
		return true;
	}
	return false;
}

//
// TeapotParams::VInit					- Chapter 19, page 691
//
bool BulletParams::VInit( float radius, int segments, const Vec3& normalDir, float force, int firedBy )
{
	m_Type = AT_Bullet;
	m_Radius = radius;
	m_Segments = segments;
	m_NormalDir = normalDir;
	m_Force = force;
	m_TeamFiredBy = firedBy;

	return true;
}

void BulletParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Radius << " ";
	out << m_Segments << " ";
	out << m_Color.r << " " << m_Color.g << " " << m_Color.b << " " << m_Color.a << " ";
	out << m_NormalDir.x << " " << m_NormalDir.y << " " << m_NormalDir.z << " ";
	out << m_Force << " ";
	out << m_TeamFiredBy << " ";

}

shared_ptr<IActor> BulletParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat.BuildTranslation(m_Pos);
	shared_ptr<IActor> pBullet(new BaseActor(mat, AT_Bullet, shared_ptr<BulletParams>(GCC_NEW BulletParams(*this))));
	logic->VAddActor(pBullet, this);
	logic->VGetGamePhysics()->VAddSphere(m_Radius, &*pBullet, SpecificGravity(PhysDens_Pine), PhysMat_Bouncy);
	logic->VGetGamePhysics()->VApplyForce(m_NormalDir, m_Force, *( m_Id ));
	return pBullet;
}

FloorParams::FloorParams() 
	: ActorParams()
{ 
	m_Type=AT_Floor; 
	m_Length=1000.0f; 
	m_ViewId = VIEWID_NO_VIEW_ATTACHED;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(FloorParams); 
}

bool FloorParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Floor; 
		in >> m_Length;
		in >> m_ViewId;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];

		return true;
	}
	return false;
}

//
// TeapotParams::VInit					- Chapter 19, page 691
//
bool FloorParams::VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color )
{
	m_Type = AT_Floor;
	if( ActorParams::VInit( id, pos, color ) )
	{
		m_Length = length;
		m_ViewId = viewId;
		m_Mat = mat;
	}


	return true;
}

void FloorParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Length << " ";
	out << m_ViewId << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";

}

shared_ptr<IActor> FloorParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pFloor(new BaseActor(mat, AT_Floor, shared_ptr<FloorParams>(GCC_NEW FloorParams(*this))));
	logic->VAddActor(pFloor, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Length, 0.01f, m_Length), &*pFloor, SpecificGravity(PhysDens_Infinite), PhysMat_Slippery);
	return pFloor;
}

CometParams::CometParams() 
	: ActorParams()
{ 
	m_Type=AT_Comet; 
	m_Radius=1.0f; 
	m_Size=sizeof(CometParams);
	m_NormalDir= Vec3( 0.0f, 0.0f, 0.0f );
	m_Force=0.0f;
}

bool CometParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Comet;
		in >> m_Radius;
		in >> m_NormalDir.x >> m_NormalDir.y >> m_NormalDir.y;
		in >> m_Force;
		return true;
	}
	return false;
}

//
// TeapotParams::VInit					- Chapter 19, page 691
//
bool CometParams::VInit( float radius, int segments, const Vec3& normalDir, float force )
{
	m_Type = AT_Comet;
	m_Radius = radius;
	m_NormalDir = normalDir;
	m_Force = force;


	return true;
}

void CometParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Radius << " ";
	out << m_Color.r << " " << m_Color.g << " " << m_Color.b << " " << m_Color.a << " ";
	out << m_NormalDir.x << " " << m_NormalDir.y << " " << m_NormalDir.z << " ";
	out << m_Force << " ";


}

shared_ptr<IActor> CometParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat.BuildTranslation(m_Pos);
	shared_ptr<IActor> pComet(new BaseActor(mat, AT_Comet, shared_ptr<CometParams>(GCC_NEW CometParams(*this))));
	logic->VAddActor(pComet, this);
	logic->VGetGamePhysics()->VAddSphere(m_Radius, &*pComet, SpecificGravity(PhysDens_Granite), PhysMat_Normal);
	logic->VGetGamePhysics()->VApplyForce(m_NormalDir, m_Force, *( m_Id ));
	return pComet;
}


//
// Ring Params
//

RingParams::RingParams() 
	: ActorParams()
{ 
	m_Type=AT_Ring; 
	m_Length=1.0f; 
	m_ViewId = VIEWID_NO_VIEW_ATTACHED;
	m_StartPosition=Mat4x4::g_Identity;
	m_Mat = Mat4x4::g_Identity;
	m_Size=sizeof(RingParams); 
}

bool RingParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Ring; 
		in >> m_Length;
		in >> m_ViewId;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_StartPosition.m[i][j];
		return true;
	}
	return false;
}


bool RingParams::VInit( float length, GameViewId viewId, 
		optional<ActorId> id, const Vec3& pos, const Color& color, Mat4x4 startPosition)
{
	m_Type = AT_Ring;
	if( ActorParams::VInit( id, pos, color ) )
	{
		m_Length = length;
		m_ViewId = viewId;
		m_StartPosition = m_Mat = startPosition;
	}


	return true;
}

void RingParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Length << " ";
	out << m_ViewId << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_StartPosition.m[i][j] << " ";
}


shared_ptr<IActor> RingParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pRing(new BaseActor(mat, AT_Ring, shared_ptr<RingParams>(GCC_NEW RingParams(*this))));
	logic->VAddActor(pRing, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Length, m_Length/5, m_Length), &*pRing, SpecificGravity(PhysDens_Water) / 2, PhysMat_Normal);
	return pRing;
}

GoalParams::GoalParams() 
	: ActorParams()
{ 
	m_Type=AT_Goal; 
	m_Length=1.0f; 
	m_ViewId = VIEWID_NO_VIEW_ATTACHED;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(GoalParams); 
	m_Team = 0;
}

bool GoalParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Goal; 
		in >> m_Length;
		in >> m_ViewId;
		in >> m_Team;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];
		return true;
	}
	return false;
}

//
// TeapotParams::VInit					- Chapter 19, page 691
//
bool GoalParams::VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color, int team )
{
	m_Type = AT_Goal;
	if( ActorParams::VInit( id, pos, color ) )
	{
		m_Length = length;
		m_ViewId = viewId;
		m_Mat = mat;
		m_Team = team;
	}


	return true;
}

void GoalParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Length << " ";
	out << m_ViewId << " ";
	out << m_Team << " ";
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";
}

//
// TeapotParams::VCreate				- Chapter 19, page 693
//
shared_ptr<IActor> GoalParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pGoal(new BaseActor(mat, AT_Goal, shared_ptr<GoalParams>(GCC_NEW GoalParams(*this))));
	logic->VAddActor(pGoal, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Length/5, m_Length, m_Length), &*pGoal, SpecificGravity(PhysDens_Infinite), PhysMat_Normal);
	return pGoal;
}


//GridParams::GridParams() : ActorParams()
//{ 
//	m_Type=AT_Grid; 
//	m_Color=g_Gray40; 
//	strcpy( m_Texture, "grid.dds" );
//	m_Squares=10;
//	m_Mat=Mat4x4::g_Identity;
//	m_Size=sizeof(GridParams); 
//}
//
// bool GridParams::VInit(std::istrstream &in)
//{
//	if (ActorParams::VInit(in))
//	{
//		m_Type=AT_Grid; 
//		in >> m_Texture;
//		in >> m_Squares;
//		for (int i=0; i<4; ++i)
//			for (int j=0; j<4; ++j)
//				in >> m_Mat.m[i][j];
//		return true;
//	}
//	return false;
//}
//
//bool GridParams::VInit(  )
//{
//	//Matrix, Texture, Squares
//
//	m_Type = AT_Grid;
//
//
//	return true;
//}
//
//void GridParams::VSerialize(std::ostrstream &out) const
//{
//	ActorParams::VSerialize(out);
//	out << m_Color.r << " " << m_Color.g << " " << m_Color.b << " " << m_Color.a << " ";
//	out << m_Texture;
//	out << m_Squares;
//	for (int i=0; i<4; ++i)
//		for (int j=0; j<4; ++j)
//			out << m_Mat.m[i][j] << " ";
//}
//
//shared_ptr<IActor> GridParams::VCreate(BaseGameLogic *logic)
//{
//	Mat4x4 mat;
//	mat = m_Mat;
//	shared_ptr<IActor> pGrid(new BaseActor(mat, AT_Grid, shared_ptr<GridParams>(GCC_NEW GridParams(*this))));
//	/*logic->VAddActor(pGrid, this);
//	logic->VGetGamePhysics()->VAddBox(Vec3(m_Squares/2.0f, 0.01f, m_Squares/2.0f), &*pGrid, SpecificGravity(PhysDens_Infinite), PhysMat_Slippery);*/
//	return pGrid;
//}
//
//
//
//shared_ptr<SceneNode> GridParams::VCreateSceneNode(shared_ptr<Scene> pScene)
//{
//	//shared_ptr<SceneNode> grid(GCC_NEW Grid(m_Id, "Grid", *this));
//	//m_pScene->VAddChild(p.m_Id, grid);
//	//grid->VOnRestore(&*m_pScene);
//	return grid;
//}






shared_ptr<SceneNode> ShipParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	if(m_Team == 0)
	{
		shared_ptr<SceneNode> ship(GCC_NEW BlueShipMeshNode(m_Id, "Ship", pScene->m_ResourceCache.getBlueShipMesh(), &m_Mat)); 
		return ship;
	}
	else
	{
		shared_ptr<SceneNode> ship(GCC_NEW RedShipMeshNode(m_Id, "Ship", pScene->m_ResourceCache.getRedShipMesh(), &m_Mat)); 
		return ship;
	}
}

shared_ptr<SceneNode> GoalParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	if(m_Team == 0)
	{
		shared_ptr<SceneNode> ship(GCC_NEW BlueGoalMeshNode(m_Id, "Goal", pScene->m_ResourceCache.getBlueGoalMesh(), &m_Mat)); 
		return ship;
	}
	else
	{
		shared_ptr<SceneNode> ship(GCC_NEW RedGoalMeshNode(m_Id, "Goal", pScene->m_ResourceCache.getRedGoalMesh(), &m_Mat)); 
		return ship;
	}
}

shared_ptr<SceneNode> RingParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> ring(GCC_NEW RingMeshNode(m_Id, "Ring", pScene->m_ResourceCache.getRingMesh(), &m_Mat)); 
	return ring;
}

shared_ptr<SceneNode> WallParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> wall(GCC_NEW WallMeshNode(m_Id, "Wall", pScene->m_ResourceCache.getWallMesh(), &m_Mat)); 
	return wall;
}

shared_ptr<SceneNode> BoundryWallParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> wall(GCC_NEW BoundryWallMeshNode(m_Id, "BoundryWall", pScene->m_ResourceCache.getBoundryWallMesh(), &m_Mat)); 
	return wall;
}

shared_ptr<SceneNode> BulletParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	Mat4x4 trans;
	trans.BuildTranslation(m_Pos);
	shared_ptr<SceneNode> bullet(GCC_NEW BulletMeshNode(m_Id, "Bullet", pScene->m_ResourceCache.getBulletMesh(), &trans)); 
	return bullet;
}

shared_ptr<SceneNode> CometParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	Mat4x4 trans;
	trans.BuildTranslation(m_Pos);
	shared_ptr<SceneNode> bullet(GCC_NEW CometMeshNode(m_Id, "Comet", pScene->m_ResourceCache.getCometMesh(), &trans)); 
	return bullet;
}

shared_ptr<SceneNode> FloorParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> floor(GCC_NEW FloorMeshNode(m_Id, "Floor", pScene->m_ResourceCache.getFloorMesh(), &m_Mat)); 
	return floor;
}