#include "std.h"

#include "CometConquestResourceCache.h"

CometConquestResourceCache::CometConquestResourceCache()
{
}

CometConquestResourceCache::~CometConquestResourceCache()
{
	m_RedShipMesh.Destroy();
	m_BlueShipMesh.Destroy();
	m_RedGoalMesh.Destroy();
	m_BlueGoalMesh.Destroy();
	m_WallMesh.Destroy();
	m_CometMesh.Destroy();
	m_RingMesh.Destroy();
	m_BoundryWallMesh.Destroy();
	m_BulletMesh.Destroy();
	m_FloorMesh.Destroy();
}

void CometConquestResourceCache::Init()
{
	ID3D11Device* pd3dDevice = DXUTGetD3D11Device();

	m_RingMesh.Create(pd3dDevice, L"ring\\ring.sdkmesh",true);
	m_RedShipMesh.Create(pd3dDevice, L"shipred\\ship.sdkmesh",true);
	m_BlueShipMesh.Create(pd3dDevice, L"shipblue\\ship.sdkmesh",true);
	m_RedGoalMesh.Create(pd3dDevice, L"goalred\\goal.sdkmesh",true);
	m_BlueGoalMesh.Create(pd3dDevice, L"goalblue\\goal.sdkmesh",true);
	m_WallMesh.Create(pd3dDevice, L"wall\\wall.sdkmesh", true);
	m_BoundryWallMesh.Create(pd3dDevice, L"boundrywall\\boundrywall.sdkmesh",true);
	m_BulletMesh.Create(pd3dDevice, L"bullet\\bullet.sdkmesh", true);
	m_FloorMesh.Create(pd3dDevice, L"floor\\floor.sdkmesh", true);
	m_CometMesh.Create(pd3dDevice, L"comet\\comet.sdkmesh", true);
}