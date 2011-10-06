#pragma once

#include "Geometry.h"
#include "../Resource Cache/CometConquestResourceCache.h"

class SceneNodeProperties
{
	friend class SceneNode;

protected:
	optional<ActorId>		m_ActorId;
	std::string				m_Name;
	Mat4x4					m_ToWorld, m_FromWorld;
	float					m_Radius;

public:
	optional<ActorId> const &ActorId() const { return m_ActorId; }
	Mat4x4 const &ToWorld() const { return m_ToWorld; }
	Mat4x4 const &FromWorld() const { return m_FromWorld; }
	void Transform(Mat4x4 *toWorld, Mat4x4 *fromWorld) const;

	const char * Name() const { return m_Name.c_str(); }		

	
	float Radius() const { return m_Radius; }

};



typedef std::vector<shared_ptr<ISceneNode> > SceneNodeList;


class SceneNode : public ISceneNode
{
	friend class Scene;

protected:
	SceneNodeList			m_Children;
	SceneNode				*m_pParent;
	SceneNodeProperties		m_Props;

public:
	SceneNode(optional<ActorId> actorId, std::string name, const Mat4x4 *to, const Mat4x4 *from=NULL) 
	{ 
		m_pParent = NULL;
		m_Props.m_ActorId = actorId;
		m_Props.m_Name = name;
		VSetTransform(to, from); 
		SetRadius(0);
	}

	virtual ~SceneNode();

	virtual const SceneNodeProperties * const VGet() const { return &m_Props; }

	virtual HRESULT VPreRender(Scene *pScene);
	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VPostRender(Scene *pScene);

	virtual void VSetTransform(const Mat4x4 *toWorld, const Mat4x4 *fromWorld=NULL);

	virtual bool VAddChild(shared_ptr<ISceneNode> kid);
	virtual bool VRemoveChild(ActorId id);
	virtual HRESULT VRenderChildren(Scene *pScene);
	virtual HRESULT VOnRestore(Scene *pScene);

	virtual HRESULT VOnUpdate(Scene *, DWORD const elapsedMs);

	Vec3 GetPosition() const { return m_Props.m_ToWorld.GetPosition(); }
	void SetPosition(const Vec3 &pos) { m_Props.m_ToWorld.SetPosition(pos); }

	void SetRadius(const float radius) { m_Props.m_Radius = radius; }

	// Most scene nodes will not have actor params, so they return a NULL pointer.
	virtual ActorParams * VGetActorParams( void )
	{
		return NULL;
	}
};

typedef std::map<ActorId, shared_ptr<ISceneNode> > SceneActorMap;

class Scene
{
protected:
	shared_ptr<SceneNode>  	m_Root;
	shared_ptr<CModelViewerCamera> 	m_Camera; 
	ID3DXMatrixStack 		*m_MatrixStack;
	SceneActorMap       m_ActorMap;
	CDXUTDirectionWidget        m_LightControl;
	float                       m_fLightScale;
	int                         m_nNumActiveLights;
	int                         m_nActiveLight;
	

public:

  ID3D11InputLayout*          m_pVertexLayout11;
  ID3D11Buffer*               m_pVertexBuffer;
  ID3D11Buffer*               m_pIndexBuffer;
  ID3D11VertexShader*         m_pVertexShader;
  ID3D11PixelShader*          m_pPixelShader;
  ID3D11SamplerState*         m_pSamLinear;
  CometConquestResourceCache m_ResourceCache;
  struct CB_VS_PER_OBJECT
  {
      D3DXMATRIX m_WorldViewProj;
      D3DXMATRIX m_World;
  };
  UINT                        m_iCBVSPerObjectBind;

  struct CB_PS_PER_OBJECT
  {
      D3DXVECTOR4 m_vObjectColor;
  };
  UINT                        m_iCBPSPerObjectBind;

  struct CB_PS_PER_FRAME
  {
      D3DXVECTOR4 m_vLightDirAmbient;
  };
  UINT                        m_iCBPSPerFrameBind;

  ID3D11Buffer*               m_pcbVSPerObject;
  ID3D11Buffer*               m_pcbPSPerObject;
  ID3D11Buffer*               m_pcbPSPerFrame;

	Scene();
	virtual ~Scene();

	HRESULT OnRender();
	HRESULT OnRestore();
	HRESULT OnUpdate(const int deltaMilliseconds);
	shared_ptr<ISceneNode> FindActor(ActorId id);

	const shared_ptr<CModelViewerCamera> GetCamera() const { return m_Camera; }
  CDXUTDirectionWidget GetLightControl() { return m_LightControl; }

	void PushAndSetMatrix(const D3DXMATRIX &toWorld)
	{
		// Note this code carefully!!!!! It is COMPLETELY different
		// from some DirectX 9 documentation out there....
		// Scene::PushAndSetMatrix - Chapter 14, page 486

		m_MatrixStack->Push();
		m_MatrixStack->MultMatrixLocal(&toWorld);
		DXUTGetD3D9Device()->SetTransform(D3DTS_WORLD, m_MatrixStack->GetTop());
	}

	void PopMatrix() 
	{
		// Scene::PopMatrix - Chapter 14, page 486
		m_MatrixStack->Pop(); 
		DXUTGetD3D9Device()->SetTransform(D3DTS_WORLD, m_MatrixStack->GetTop());	
	}

	const D3DXMATRIX *GetTopMatrix() 
	{ 
		// Scene::GetTopMatrix - Chapter 14, page 486
		return static_cast<const D3DXMATRIX *>(m_MatrixStack->GetTop()); 
	}

  bool AddChild(optional<ActorId> id, shared_ptr<ISceneNode> kid)
	{ 

		// This allows us to search for this later based on actor id
		m_ActorMap[*id] = kid;	

		return m_Root->VAddChild(kid); 
	}

	bool RemoveChild(ActorId id)
	{ 
		m_ActorMap.erase(id);
		return m_Root->VRemoveChild(id); 
	}

  HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

};

class RootNode : public SceneNode
{
public:
	RootNode();

};

class MeshNode : public SceneNode
{
protected:
	CDXUTSDKMesh   *m_pMesh;
	std::wstring m_SDKMeshFileName;


public:
	MeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t); 

	MeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 

	virtual ~MeshNode() { /*m_pMesh->Destroy();*/ }
	HRESULT VRender(Scene *pScene);
	virtual HRESULT VOnRestore(Scene *pScene);

};

class RedShipMeshNode : public MeshNode
{
public:
	RedShipMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	RedShipMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class BlueShipMeshNode : public MeshNode
{
public:
	BlueShipMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	BlueShipMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class WallMeshNode : public MeshNode
{
public:
	WallMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	WallMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class BoundryWallMeshNode : public MeshNode
{
public:
	BoundryWallMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	BoundryWallMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class BulletMeshNode : public MeshNode
{
public:
	BulletMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	BulletMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class CometMeshNode : public MeshNode
{
public:
	CometMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	CometMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class FloorMeshNode : public MeshNode
{
public:
	FloorMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	FloorMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class RingMeshNode : public MeshNode
{
public:
	RingMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	RingMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class RedGoalMeshNode : public MeshNode
{
public:
	RedGoalMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	RedGoalMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

class BlueGoalMeshNode : public MeshNode
{
public:
	BlueGoalMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		CDXUTSDKMesh *mesh, 
    const Mat4x4 *t);

	BlueGoalMeshNode(const optional<ActorId> actorId, 
		std::string name, 
		std::string SDKMeshFileName, 
		const Mat4x4 *t ); 
};

//TODO add more meshes bro