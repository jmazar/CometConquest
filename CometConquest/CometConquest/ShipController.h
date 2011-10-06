#pragma once

#include "interfaces.h"

class SceneNode;

class ShipController : public IKeyboardHandler
{
protected:
	BYTE					m_bKey[256];			// Which keys are up and down
	shared_ptr<SceneNode>	m_object;
	unsigned int			m_LastShotTime;

public:
	ShipController(shared_ptr<SceneNode> object);

	void OnUpdate(DWORD const elapsedMs);

public:
	bool VOnKeyDown(const BYTE c) { m_bKey[c] = true; return true; }
	bool VOnKeyUp(const BYTE c) { m_bKey[c] = false; return true; }
};