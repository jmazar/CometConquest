#pragma once

#include "GameApp.h"
#include "Network\Network.h"
#include "CometConquest.h"

class CometConquestGameProxy : public CometConquestBaseGame
{
	CometConquestGameProxy(struct GameOptions const &options)
		: CometConquestBaseGame(options) { m_ExpectedPlayers = 1; }
	virtual ~CometConquestGameProxy() { }
};