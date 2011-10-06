#include "std.h"

#include "SceneNodes.h"

////////////////////////////////////////////////////
// SceneNodeProperties Implementation
////////////////////////////////////////////////////

//
// SceneNodeProperties::Transform			- Chapter 14, page 472
//
void SceneNodeProperties::Transform(Mat4x4 *toWorld, Mat4x4 *fromWorld) const
{
	if (toWorld)
		*toWorld = m_ToWorld;
	
	if (fromWorld)
		*fromWorld = m_FromWorld;
}

////////////////////////////////////////////////////
// SceneNode Implementation
////////////////////////////////////////////////////

SceneNode::~SceneNode()
{

}

HRESULT SceneNode::VOnRestore(Scene *pScene)
{
  SceneNodeList::iterator i = m_Children.begin();
  SceneNodeList::iterator end = m_Children.end();

  while (i != end)
	{
		(*i)->VOnRestore(pScene);
		i++;
	}

  return S_OK;
}

void SceneNode::VSetTransform(const Mat4x4 *toWorld, const Mat4x4 *fromWorld)
{
	m_Props.m_ToWorld = *toWorld;
	if (!fromWorld)
	{
		// Good god this is expensive...why bother???? Look in Chapter 14.
		m_Props.m_FromWorld = m_Props.m_ToWorld.Inverse();
	}
	else
	{
		m_Props.m_FromWorld = *fromWorld;
	}
}

HRESULT SceneNode::VPreRender(Scene *pScene)
{
  //pScene->PushAndSetMatrix(m_Props.m_ToWorld);
  return S_OK;
}

HRESULT SceneNode::VPostRender(Scene *pScene)
{
  //pScene->PopMatrix();
  return S_OK;
}

HRESULT SceneNode::VOnUpdate(Scene *pScene, DWORD const elapsedMs)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// VOnUpdate()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		(*i)->VOnUpdate(pScene, elapsedMs);
		i++;
	}
	return S_OK;
}

HRESULT SceneNode::VRender(Scene *pScene)
{
  return S_OK;
}

HRESULT SceneNode::VRenderChildren(Scene *pScene)
{
  SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		if ((*i)->VPreRender(pScene)==S_OK)
		{
			
			(*i)->VRender(pScene);

			(*i)->VRenderChildren(pScene);
		}
		(*i)->VPostRender(pScene);
		i++;
	}

	return S_OK;
}

bool SceneNode::VAddChild(shared_ptr<ISceneNode> kid)
{
  m_Children.push_back(kid);

  return true;
}

bool SceneNode::VRemoveChild(ActorId id)
{
  for(SceneNodeList::iterator i=m_Children.begin(); i!=m_Children.end(); ++i)
	{
		const SceneNodeProperties* pProps = (*i)->VGet();
		if(id==*pProps->ActorId())
		{
			i = m_Children.erase(i);	//this can be expensive for vectors
			return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////
// Scene Implementation
////////////////////////////////////////////////////

Scene::Scene() :
m_pVertexLayout11(NULL),
  m_pVertexBuffer(NULL),
  m_pIndexBuffer(NULL),
  m_pVertexShader(NULL),
  m_pPixelShader(NULL),
  m_pSamLinear(NULL),
  m_iCBVSPerObjectBind(0),
  m_iCBPSPerObjectBind(0),
  m_iCBPSPerFrameBind(1),
  m_pcbVSPerObject(NULL),
  m_pcbPSPerObject(NULL),
  m_pcbPSPerFrame(NULL)
{
  m_Root.reset(new RootNode());
  m_Camera.reset(new CModelViewerCamera());
  D3DXCreateMatrixStack(0, &m_MatrixStack);
}

Scene::~Scene()
{
                  
    SAFE_RELEASE( m_pVertexLayout11 );
    SAFE_RELEASE( m_pVertexBuffer );
    SAFE_RELEASE( m_pIndexBuffer );
    SAFE_RELEASE( m_pVertexShader );
    SAFE_RELEASE( m_pPixelShader );
    SAFE_RELEASE( m_pSamLinear );

    SAFE_RELEASE( m_pcbVSPerObject );
    SAFE_RELEASE( m_pcbPSPerObject );
    SAFE_RELEASE( m_pcbPSPerFrame );
	SAFE_RELEASE(m_MatrixStack);

}

HRESULT Scene::OnRender()
{
  if (m_Root && m_Camera)
	{
		// The scene root could be anything, but it
		// is usually a SceneNode with the identity
		// matrix

		if (m_Root->VPreRender(this)==S_OK)
		{
			m_Root->VRender(this);
			m_Root->VRenderChildren(this);
			m_Root->VPostRender(this);
		}
	}

	return S_OK;
}

HRESULT Scene::OnRestore()
{
  HRESULT hr;
  ID3D11Device* pd3dDevice = DXUTGetD3D11Device();
  m_ResourceCache.Init();
	if (!m_Root)
		return S_OK;
 
  // Compile the shaders using the lowest possible profile for broadest feature level support
  ID3DBlob* pVertexShaderBuffer = NULL;
  V_RETURN( CompileShaderFromFile( L"CometConquestBase_VS.hlsl", "VSMain", "vs_4_0_level_9_1", &pVertexShaderBuffer ) );

  ID3DBlob* pPixelShaderBuffer = NULL;
  V_RETURN( CompileShaderFromFile( L"CometConquestBase_PS.hlsl", "PSMain", "ps_4_0_level_9_1", &pPixelShaderBuffer ) );

  // Create the shaders
  V_RETURN( pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
                                            pVertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader ) );
  DXUT_SetDebugName( m_pVertexShader, "VSMain" );
  V_RETURN( pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
                                            pPixelShaderBuffer->GetBufferSize(), NULL, &m_pPixelShader ) );
  DXUT_SetDebugName( m_pPixelShader, "PSMain" );

  // Create our vertex input layout
  const D3D11_INPUT_ELEMENT_DESC layout[] =
  {
      { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  V_RETURN( pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ), pVertexShaderBuffer->GetBufferPointer(),
                                            pVertexShaderBuffer->GetBufferSize(), &m_pVertexLayout11 ) );
  DXUT_SetDebugName( m_pVertexLayout11, "Primary" );

  SAFE_RELEASE( pVertexShaderBuffer );
  SAFE_RELEASE( pPixelShaderBuffer );

  // Setup constant buffers
  D3D11_BUFFER_DESC Desc;
  Desc.Usage = D3D11_USAGE_DYNAMIC;
  Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  Desc.MiscFlags = 0;

  Desc.ByteWidth = sizeof( CB_VS_PER_OBJECT );
  V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &m_pcbVSPerObject ) );
  DXUT_SetDebugName( m_pcbVSPerObject, "CB_VS_PER_OBJECT" );

  Desc.ByteWidth = sizeof( CB_PS_PER_OBJECT );
  V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &m_pcbPSPerObject ) );
  DXUT_SetDebugName( m_pcbPSPerObject, "CB_PS_PER_OBJECT" );

  Desc.ByteWidth = sizeof( CB_PS_PER_FRAME );
  V_RETURN( pd3dDevice->CreateBuffer( &Desc, NULL, &m_pcbPSPerFrame ) );
  DXUT_SetDebugName( m_pcbPSPerFrame, "CB_PS_PER_FRAME" );

  // Setup the camera's view parameters
  D3DXVECTOR3 vecEye( 0.0f, 300.0f, -5.0f );
  D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
  m_Camera->SetViewParams( &vecEye, &vecAt );
  m_Camera->SetRadius(100.0f, 5.0f, 100.0f );

  D3DXVECTOR3 vLightDir( -1, 1, -1 );
  D3DXVec3Normalize( &vLightDir, &vLightDir );
  m_LightControl.SetLightDirection( vLightDir );

	return m_Root->VOnRestore(this);
}

HRESULT Scene::OnUpdate(const int deltaMilliseconds)
{
	static DWORD lastTime = 0;
	DWORD elapsedTime = 0;
	DWORD now = timeGetTime();

	if (!m_Root)
		return S_OK;

	if (lastTime == 0)
	{
		lastTime = now;
	}

	elapsedTime = now - lastTime;
	lastTime = now;

	return m_Root->VOnUpdate(this, elapsedTime);
}

shared_ptr<ISceneNode> Scene::FindActor(ActorId id)
{
	SceneActorMap::iterator i = m_ActorMap.find(id);
	if (i==m_ActorMap.end())
	{
		shared_ptr<ISceneNode> null;
		return null;
	}

	return (*i).second;
}

HRESULT Scene::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName ) );

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( str, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

////////////////////////////////////////////////////
// RootNode Implementation
////////////////////////////////////////////////////

RootNode::RootNode() : 
SceneNode(optional_empty(), "Root", &Mat4x4::g_Identity)
{
}



////////////////////////////////////////////////////
// MeshNode Implementation
////////////////////////////////////////////////////
MeshNode::MeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : SceneNode(actorId, name, t) 
{ 
  m_pMesh = mesh;
}

//
// MeshNode::MeshNode				- Chapter 14, page 505
//
MeshNode::MeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : SceneNode(actorId, name, t) 
{ 
		m_pMesh = new CDXUTSDKMesh();
		m_SDKMeshFileName.assign(SDKMeshFileName.begin(), SDKMeshFileName.end());
  

}

HRESULT MeshNode::VRender(Scene *pScene)
{
  D3DXMATRIX mWorldViewProjection;
  D3DXVECTOR3 vLightDir;
  D3DXMATRIX mWorld;
  D3DXMATRIX mView;
  D3DXMATRIX mProj;
  HRESULT hr;
  ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
  
  //Get the mesh

// Get the light direction
	vLightDir = pScene->GetLightControl().GetLightDirection();

    // Per frame cb update
    D3D11_MAPPED_SUBRESOURCE MappedResource;
	V( pd3dImmediateContext->Map( pScene->m_pcbPSPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    Scene::CB_PS_PER_FRAME* pPerFrame = ( Scene::CB_PS_PER_FRAME* )MappedResource.pData;
    float fAmbient = 0.1f;
    pPerFrame->m_vLightDirAmbient = D3DXVECTOR4( vLightDir.x, vLightDir.y, vLightDir.z, fAmbient );
    pd3dImmediateContext->Unmap( pScene->m_pcbPSPerFrame, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( pScene->m_iCBPSPerFrameBind, 1, &pScene->m_pcbPSPerFrame );

    //IA setup
    pd3dImmediateContext->IASetInputLayout( pScene->m_pVertexLayout11 );
    UINT Strides[1];
    UINT Offsets[1];
    ID3D11Buffer* pVB[1];
    pVB[0] = m_pMesh->GetVB11( 0, 0 );
    Strides[0] = ( UINT )m_pMesh->GetVertexStride( 0, 0 );
    Offsets[0] = 0;
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
    pd3dImmediateContext->IASetIndexBuffer( m_pMesh->GetIB11( 0 ), m_pMesh->GetIBFormat11( 0 ), 0 );

    // Set the shaders
    pd3dImmediateContext->VSSetShader( pScene->m_pVertexShader, NULL, 0 );
    pd3dImmediateContext->PSSetShader( pScene->m_pPixelShader, NULL, 0 );
    
    // Set the per object constant data
    mWorld = m_Props.ToWorld() * *pScene->GetCamera()->GetWorldMatrix();
    mProj = *pScene->GetCamera()->GetProjMatrix();
    mView = *pScene->GetCamera()->GetViewMatrix();

    mWorldViewProjection = mWorld * mView * mProj;
        
    // VS Per object
    V( pd3dImmediateContext->Map( pScene->m_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    Scene::CB_VS_PER_OBJECT* pVSPerObject = ( Scene::CB_VS_PER_OBJECT* )MappedResource.pData;
    D3DXMatrixTranspose( &pVSPerObject->m_WorldViewProj, &mWorldViewProjection );
    D3DXMatrixTranspose( &pVSPerObject->m_World, &mWorld );
    pd3dImmediateContext->Unmap( pScene->m_pcbVSPerObject, 0 );

    pd3dImmediateContext->VSSetConstantBuffers( pScene->m_iCBVSPerObjectBind, 1, &pScene->m_pcbVSPerObject );

    // PS Per object
    V( pd3dImmediateContext->Map( pScene->m_pcbPSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    Scene::CB_PS_PER_OBJECT* pPSPerObject = ( Scene::CB_PS_PER_OBJECT* )MappedResource.pData;
    pPSPerObject->m_vObjectColor = D3DXVECTOR4( 1, 1, 1, 1 );
    pd3dImmediateContext->Unmap( pScene->m_pcbPSPerObject, 0 );

    pd3dImmediateContext->PSSetConstantBuffers( pScene->m_iCBPSPerObjectBind, 1, &pScene->m_pcbPSPerObject );

    //Render
    SDKMESH_SUBSET* pSubset = NULL;
    D3D11_PRIMITIVE_TOPOLOGY PrimType;

    pd3dImmediateContext->PSSetSamplers( 0, 1, &pScene->m_pSamLinear );

    for( UINT subset = 0; subset < m_pMesh->GetNumSubsets( 0 ); ++subset )
    {
        // Get the subset
        pSubset = m_pMesh->GetSubset( 0, subset );

        PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
        pd3dImmediateContext->IASetPrimitiveTopology( PrimType );

        // TODO: D3D11 - material loading
        ID3D11ShaderResourceView* pDiffuseRV = m_pMesh->GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
        pd3dImmediateContext->PSSetShaderResources( 0, 1, &pDiffuseRV );

        pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
    }

    DXUT_EndPerfEvent();


  return S_OK; 
} 

HRESULT MeshNode::VOnRestore(Scene *pScene)
{
  if (m_SDKMeshFileName.empty())
  {
    return S_OK;
  }

  ID3D11Device* pd3dDevice = DXUTGetD3D11Device();

  // Change post press - release the mesh only if we have a valid mesh file name to load.
  // Otherwise we likely created it on our own, and needs to be kept.
  m_pMesh->Destroy();

  HRESULT hr;
  V_RETURN( m_pMesh->Create( pd3dDevice, m_SDKMeshFileName.c_str(), true ) );


  return S_OK;
}


RedShipMeshNode::RedShipMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

RedShipMeshNode::RedShipMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}

BlueShipMeshNode::BlueShipMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

BlueShipMeshNode::BlueShipMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}

RingMeshNode::RingMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

RingMeshNode::RingMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}

WallMeshNode::WallMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

WallMeshNode::WallMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}

BoundryWallMeshNode::BoundryWallMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

BoundryWallMeshNode::BoundryWallMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}

BulletMeshNode::BulletMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

BulletMeshNode::BulletMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{

}

CometMeshNode::CometMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

CometMeshNode::CometMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}


FloorMeshNode::FloorMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

FloorMeshNode::FloorMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}

RedGoalMeshNode::RedGoalMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

RedGoalMeshNode::RedGoalMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}

BlueGoalMeshNode::BlueGoalMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  CDXUTSDKMesh *mesh, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, mesh, t)
{

}

BlueGoalMeshNode::BlueGoalMeshNode(const optional<ActorId> actorId, 
  std::string name, 
  std::string SDKMeshFileName, 
  const Mat4x4 *t)
  : MeshNode(actorId, name, SDKMeshFileName, t)
{
}