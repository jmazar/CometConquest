#pragma once


class CometConquestResourceCache
{
public:
	CometConquestResourceCache();
	~CometConquestResourceCache();
	void Init();
	CDXUTSDKMesh* getRedShipMesh() { return &m_RedShipMesh; }
	CDXUTSDKMesh* getBlueShipMesh() { return &m_BlueShipMesh; }
	CDXUTSDKMesh* getRedGoalMesh() { return &m_RedGoalMesh; }
	CDXUTSDKMesh* getBlueGoalMesh() { return &m_BlueGoalMesh; }
	CDXUTSDKMesh* getWallMesh() { return &m_WallMesh; }
	CDXUTSDKMesh* getBoundryWallMesh() { return &m_BoundryWallMesh; }
	CDXUTSDKMesh* getBulletMesh() { return &m_BulletMesh; }
	CDXUTSDKMesh* getFloorMesh() { return &m_FloorMesh; }
	CDXUTSDKMesh* getCometMesh() {return &m_CometMesh; }
	CDXUTSDKMesh* getRingMesh() { return &m_RingMesh; }
private:
	CDXUTSDKMesh m_RedShipMesh;
	CDXUTSDKMesh m_BlueShipMesh;
	CDXUTSDKMesh m_RedGoalMesh;
	CDXUTSDKMesh m_BlueGoalMesh;
	CDXUTSDKMesh m_WallMesh;
	CDXUTSDKMesh m_BoundryWallMesh;
	CDXUTSDKMesh m_BulletMesh;
	CDXUTSDKMesh m_FloorMesh;
	CDXUTSDKMesh m_CometMesh;
	CDXUTSDKMesh m_RingMesh;
};