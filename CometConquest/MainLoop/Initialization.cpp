//========================================================================
// Initialization.cpp : Defines utility functions for game initialization
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
//  CheckHardDisk					- Chapter 5, page 133
//  CheckMemory						- Chapter 5, page 133
//  class GameOptions				- Chapter 5, page 142 
//  IsOnlyInstance					- Chapter 5, page 132
//  GetSaveGameDirectory			- Chapter 5, page 141-142
//========================================================================

#include "std.h"
#include "Initialization.h"



GameOptions::GameOptions(const char* path)
{
	/*m_useHardwareAccel = ::GetPrivateProfileIntA( 
		"DISPLAY", "Hardware_Acceleration", false, path ) ? true : false;

	m_usePageFlipping = ::GetPrivateProfileIntA( 
		"DISPLAY", "PageFlip", true, path ) ? true : false;

	m_useDithering = ::GetPrivateProfileIntA( 
		"DISPLAY", "Dithering", true, path ) ? true : false;

	m_useAntialiasing = ::GetPrivateProfileIntA(
		"DISPLAY", "AntiAliasing", true, path ) ? true : false;

	m_useEgdeAntiAliasing = ::GetPrivateProfileIntA( 
		"DISPLAY", "Edge_AntiAliasing", false, path ) ? true : false;

	m_useVRAM = ::GetPrivateProfileIntA(
		"DISPLAY", "Enable_VRAM", true, path ) ? true : false;

	m_runFullSpeed = ::GetPrivateProfileIntA( 
		"DISPLAY", "Fullspeed", false, path ) ? true : false;

	m_useTexturePerspective = ::GetPrivateProfileIntA( 
		"DISPLAY", "Texture_Perspective", true, path ) ? true : false;

	m_soundEffectsVolume = ::GetPrivateProfileIntA( 
		"SOUND", "Sound_Effects_Volume", false, path ) / 100.0f;

	m_musicVolume = ::GetPrivateProfileIntA( 
		"SOUND", "Music_Volume", false, path ) / 100.0f;*/

	m_expectedPlayers = ::GetPrivateProfileIntA( 
		"MULTIPLAYER", "Expected_Players", 1, path );

	m_listenPort = ::GetPrivateProfileIntA( 
		"MULTIPLAYER", "Listen_Port", -1, path );

	char buffer[256];
	::GetPrivateProfileStringA( 
		"MULTIPLAYER", "Game_Host", "", buffer, 256, path );
	m_gameHost = buffer;

	m_maxPlayers = ::GetPrivateProfileIntA( 
		"MULTIPLAYER", "Max_Players", 6, path );	
}



