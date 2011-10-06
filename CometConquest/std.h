#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#define BOOST_ALL_NO_LIB
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <crtdbg.h>


#if defined (_MSC_VER) && (_MSC_VER < 1300)
	#include "types.h"
#else
	#ifdef _DEBUG
		#undef _DEBUG
			#include "gcctypes.h"			// for CSize, CPoint and CRect - it's all inline.
		#define _DEBUG
	#else
		#include "gcctypes.h"
	#endif
#endif



#define _VS2005_
#if _MSC_VER < 1400
	#undef _VS2005_
#endif




// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include <assert.h>
#include <mmsystem.h>

#include <algorithm>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <map>


// Game Code Complete - Chapter 12, page 446-447
#if defined(_DEBUG)
#	define GCC_NEW new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#	define GCC_NEW new
#endif

#include "Core\DXUT.h"
#include "Core\DXUTmisc.h"
#include "Optional\SDKmisc.h"
#include "Optional\SDKmesh.h"
#include "Optional\DXUTcamera.h"
#include "Optional\DXUTgui.h"
#include "Optional\DXUTsettingsdlg.h"

#pragma warning( disable : 4201 ) // nonstandard extension used - nameless struct/union - triggered in code inside MMSystem.h
#pragma warning( disable : 4244 ) // conversion from 'int' to 'short', possible loss of data - small number of places
#pragma warning( disable : 4100 ) // unreferenced formal parameter - small number of places
#pragma warning( disable : 4189 ) // local variable initizliaed but not referenced - small number of places
#pragma warning( disable : 4127 ) // conditional expression is constant - small number of places
#pragma warning( disable : 4702 ) // unreachable code - small number of places
#pragma warning( disable : 4996 ) // 'function' declared deprecated - gets rid of all those 2005 warnings....


#include "MiscStuff\templates.h"

#include <boost\config.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\utility.hpp>

using boost::shared_ptr;

#include "SceneGraph\geometry.h"



typedef D3DXCOLOR Color;

extern Color g_White;
extern Color g_Black;
extern Color g_Cyan;
extern Color g_Red;
extern Color g_Green;
extern Color g_Blue;
extern Color g_Yellow;
extern Color g_Gray40;
extern Color g_Gray25;
extern Color g_Gray65;
extern Color g_Transparent;

extern Vec3 g_Up;
extern Vec3 g_Right;
extern Vec3 g_Forward;

extern Vec4 g_Up4;
extern Vec4 g_Right4;
extern Vec4 g_Forward4;



//  AppMsg				- Chapter 9, page 248

struct AppMsg
{
	HWND m_hWnd;
	UINT m_uMsg;
	WPARAM m_wParam;
	LPARAM m_lParam;
};



#include "interfaces.h"

// Useful #defines

#define fOPAQUE (1.0f)
#define iOPAQUE (1)
#define fTRANSPARENT (0.0f)
#define iTRANSPARENT (0)

#define MEGABYTE (1024 * 1024)
#define SIXTY_HERTZ (16.66f)

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#if !defined(SAFE_DELETE)
	#define SAFE_DELETE(x) if(x) delete x; x=NULL;
#endif

#if !defined(SAFE_DELETE_ARRAY)
	#define SAFE_DELETE_ARRAY(x) if (x) delete [] x; x=NULL; 
#endif

#if !defined(SAFE_RELEASE)
	#define SAFE_RELEASE(x) if(x) x->Release(); x=NULL;
#endif

#ifdef UNICODE
	#define _tcssprintf wsprintf
	#define tcsplitpath _wsplitpath
#else
	#define _tcssprintf sprintf
	#define tcsplitpath _splitpath
#endif

extern INT WINAPI GameMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow);
