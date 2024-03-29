
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

#include <strstream>


class BaseActor : public IActor
{
	friend class BaseGameLogic;
protected:
	ActorId m_id;
	Mat4x4 m_Mat;
	int m_Type;
	shared_ptr<ActorParams> m_Params;


	virtual void VSetID(ActorId id) { m_id = id; } 
	virtual void VSetMat(const Mat4x4 &newMat) { m_Mat = newMat; }

public:
	BaseActor(Mat4x4 mat, int type, shared_ptr<ActorParams> params)
		{ m_Mat=mat; m_Type=type; m_Params=params; }

	virtual Mat4x4 const &VGetMat() { return m_Mat; }			// virtual dumb here?
	virtual const int VGetType() { return m_Type; }				// virtual dumb here?
	virtual ActorId VGetID() { return m_id; }			// virtual dumb here?
	shared_ptr<ActorParams> VGetParams() { return m_Params; }
	virtual bool VIsPhysical() { return true; }
	virtual bool VIsGeometrical() { return true; }
	virtual void VOnUpdate(int deltaMilliseconds) { }
	//virtual void VRotateY(float angleRadians);
};



#define VIEWID_NO_VIEW_ATTACHED	(0)

enum ActorType
{
	AT_Unknown,
	AT_Ship,
	AT_Ring,
	AT_Goal,
	AT_Wall,
	AT_BoundryWall,
	AT_Floor,
	AT_Comet,
	AT_Bullet
};

class BaseGameLogic;
class SceneNode;


struct ActorParams
{
	int m_Size;
	optional<ActorId> m_Id;
	Vec3 m_Pos;
	ActorType m_Type;
	Color m_Color;

	ActorParams(); 

	virtual ~ActorParams() { }

	virtual bool VInit(std::istrstream &in);
	virtual void VSerialize(std::ostrstream &out) const;

	virtual bool VInit( optional<ActorId> id, const Vec3& pos, const Color& color );

	static ActorParams *CreateFromStream(std::istrstream &in);

	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic) { shared_ptr<IActor> p; return p; }
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene) { shared_ptr<SceneNode> p; return p; }


};



struct ShipParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	Mat4x4 m_Mat;
	Mat4x4 m_StartPosition;
	int m_Team;
	ShipParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color, Mat4x4 startPosition, int team );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);

};

struct GoalParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	Mat4x4 m_Mat;
	int m_Team;
	GoalParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color, int team );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);

};

struct RingParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	int m_Team;
	Mat4x4 m_Mat;
	Mat4x4 m_StartPosition;
	RingParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float length, GameViewId viewId, 
		optional<ActorId> id, const Vec3& pos, const Color& color, Mat4x4 startPosition);
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);

};

struct WallParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	Mat4x4 m_Mat;

	WallParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float length, GameViewId viewId, const Mat4x4& mat, 
		optional<ActorId> id, const Vec3& pos, const Color& color );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct BoundryWallParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	Mat4x4 m_Mat;

	BoundryWallParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct FloorParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	Mat4x4 m_Mat;

	FloorParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float length, GameViewId viewId, const Mat4x4& mat, optional<ActorId> id, const Vec3& pos, const Color& color );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct CometParams : public ActorParams
{
	float m_Radius;
	Vec3 m_NormalDir;
	float m_Force;

	CometParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float radius, int segments, const Vec3& normalDir, float force );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct BulletParams : public ActorParams
{
	float m_Radius;
	int m_Segments;
	Vec3 m_NormalDir;
	float m_Force;
	int m_TeamFiredBy;

	BulletParams();

	virtual bool VInit(std::istrstream &in);
	virtual bool VInit( float radius, int segments, const Vec3& normalDir, float force, int firedBy );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};


//struct GridParams : public ActorParams
//{
//	static const int sk_MaxTextureNameLen = _MAX_PATH;
//	char m_Texture[sk_MaxTextureNameLen];
//	unsigned int m_Squares;
//	Mat4x4 m_Mat;
//
//	GridParams();
//
//	virtual bool VInit(std::istrstream &in);
//	virtual bool VInit(  );
//	virtual void VSerialize(std::ostrstream &out) const;
//	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
//	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
//};

