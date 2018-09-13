/*********************************************************
* Copyright (C) X-Ray Oxygen, 2018. All rights reserved.
* X-Ray Oxygen - open-source X-Ray fork
* Apache License
**********************************************************
* Module Name: Dynamic splash screen
**********************************************************
* DynamicSplash.cpp
* Methods for dynamic splash implementation
*********************************************************/
#include "stdafx.h"
#pragma hdrstop

#include "DynamicSplash.h"
#include <process.h>
#include <CommCtrl.h>
#include <WinUser.h>
#include "../xrPlay/resource.h"
#include "../xrCore/LocatorAPI.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#include <gdiplus.h>
#include <gdiplusinit.h>

ENGINE_API DSplashScreen splashScreen;

ATOM CreateSplashClass(HINSTANCE hInstance, LPCSTR lpClass, WNDPROC wndProc)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDC_APPSTARTING);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDB_BITMAP1);
	wcex.lpszClassName = lpClass;
	wcex.hIconSm = LoadIcon(NULL, IDC_APPSTARTING);

	return RegisterClassEx(&wcex);
}

VOID WINAPI InitSplash(HINSTANCE hInstance, LPCSTR lpClass, WNDPROC wndProc)
{
	CreateSplashClass(hInstance, lpClass, wndProc);

	Gdiplus::GdiplusStartupInput gdiSI;
	Gdiplus::GdiplusStartupOutput gdiSO;
	ULONG_PTR gdiToken;
	ULONG_PTR gdiHookToken;
	gdiSI.SuppressBackgroundThread = TRUE;
	Gdiplus::GdiplusStartup(&gdiToken, &gdiSI, &gdiSO);
	gdiSO.NotificationHook(&gdiHookToken);

	//#VERTVER: PLS REWORK IT
	//////////////////////////////////////
	FS_Path* filePath = FS.get_path("$textures$");
	std::string szPath = std::string(filePath->m_Path);
	std::wstring szWPath = std::wstring(szPath.begin(), szPath.end());
	szWPath += L"\\ui\\Splash.bmp";
	//////////////////////////////////////

	Gdiplus::Image* pImage = Gdiplus::Image::FromFile(szWPath.c_str());
	R_ASSERT(pImage);

	splashScreen.SetBackgroundImage(pImage);
	splashScreen.SetSplashWindowName("Oxy splash");
	delete pImage;

	splashScreen.ShowSplash();
	splashScreen.SetProgressPosition(0, "Engine entry-point");
}

DSplashScreen::DSplashScreen(HWND hwnd)
{
	hThread = nullptr;
	pMainImage = nullptr;
	hwndSplash = NULL;
	threadId = NULL;
	hwndProgress = NULL;
	hEvent = nullptr;
	hwndParent = hwnd;
}

DSplashScreen::DSplashScreen()
{
	hThread = nullptr;
	pMainImage = nullptr;
	hwndSplash = NULL;
	threadId = NULL;
	hwndProgress = NULL;
	hEvent = nullptr;
	hwndParent = NULL;
}

VOID DSplashScreen::SetBackgroundImage(LPVOID pImage)
{
	if (pImage)
	{
		Gdiplus::Image* pCustomImage = reinterpret_cast<Gdiplus::Image*>(pImage);
		pMainImage = pCustomImage->Clone();
	}
}

VOID DSplashScreen::ShowSplash()
{
	if (!hThread)
	{
		// create splash thread
		hEvent = CreateEventA(NULL, FALSE, FALSE, FALSE);	// splash event
		hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)SplashThreadProc, static_cast<LPVOID>(this), NULL, &threadId);
		R_ASSERT(WaitForSingleObject(hEvent, 5000) != WAIT_TIMEOUT);
	}
	else
	{
		PostThreadMessageA(threadId, WM_ACTIVATE, WA_CLICKACTIVE, NULL);
	}
}

VOID DSplashScreen::HideSplash()
{
	if (hThread)
	{
		// destroy splash thread
		PostThreadMessageA(threadId, WM_QUIT, NULL, NULL);
		if (WaitForSingleObject(hThread, 9000) == WAIT_TIMEOUT)
		{
			TerminateThread(hThread, (DWORD)WAIT_TIMEOUT);
		}
		CloseHandle(hThread);
		CloseHandle(hEvent);
	}
	hThread = nullptr;
}

VOID DSplashScreen::SetSplashWindowName(xr_string windowName)
{
	splashWindowName = windowName;
}

VOID DSplashScreen::SetProgressPosition(DWORD percent, xr_string messageString)
{
	if (hThread)
	{
		xr_string* tempMsg = new xr_string(messageString);
		PostThreadMessageA(threadId, PBM_SETPOS, percent, reinterpret_cast<LPARAM>(tempMsg));
	}
}

VOID DSplashScreen::SetProgressPosition(DWORD percent, DWORD resourceId, HMODULE hModule)
{
	LPSTR lpMessage = nullptr;
	int len = ::LoadStringA(hModule, resourceId, reinterpret_cast<LPSTR>(&lpMessage), NULL);

	xr_string* tempMsg = new xr_string(lpMessage, len);
	PostThreadMessageA(threadId, PBM_SETPOS, percent, reinterpret_cast<LPARAM>(tempMsg));
}

VOID DSplashScreen::SetProgressColor(COLORREF refColor)
{
	PostThreadMessageA(threadId, PBM_SETBARCOLOR, NULL, refColor);
}

DWORD WINAPI DSplashScreen::SplashThreadProc(LPVOID pData)
{
	DSplashScreen* pSplash = static_cast<DSplashScreen*>(pData);
	if (!pSplash) { return 0; }

	Gdiplus::Image* pSplashImage = reinterpret_cast<Gdiplus::Image*>(pSplash->pMainImage);

	// create splash window class
	WNDCLASSA windowsClass = { NULL };
	windowsClass.style = CS_HREDRAW | CS_VREDRAW;
	windowsClass.lpfnWndProc = SplashWndProc;
	windowsClass.hInstance = GetModuleHandleA(NULL);	// process module
	windowsClass.hCursor = LoadCursorA(NULL, IDC_APPSTARTING);
	windowsClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowsClass.lpszClassName = "OxySplashScreen";
	windowsClass.hIcon = LoadIcon(windowsClass.hInstance, MAKEINTRESOURCEA(IDI_ICON1));

	R_ASSERT(RegisterClassA(&windowsClass));

	// try to find monitor where mouse was last time
	tagPOINT point = { NULL };
	tagMONITORINFO monitorInfo = { NULL };
	monitorInfo.cbSize = sizeof(MONITORINFOEXA);
	HMONITOR hMonitor = NULL;
	RECT areaRect = { NULL };

	GetCursorPos(&point);
	hMonitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);

	// get window info to rect
	if (GetMonitorInfoA(hMonitor, &monitorInfo))
	{
		areaRect.left = (monitorInfo.rcMonitor.right + monitorInfo.rcMonitor.left - static_cast<LONG>(pSplashImage->GetWidth())) / 2;
		areaRect.top = (monitorInfo.rcMonitor.top + monitorInfo.rcMonitor.bottom - static_cast<LONG>(pSplashImage->GetHeight())) / 2;
	}
	else
	{
		SystemParametersInfoA(SPI_GETWORKAREA, NULL, &areaRect, NULL);
		areaRect.left = (areaRect.right + areaRect.left - pSplashImage->GetWidth()) / 2;
		areaRect.top = (areaRect.top + areaRect.bottom - pSplashImage->GetHeight()) / 2;
	}

	// create splash window
	pSplash->hwndSplash = CreateWindowExA(
		pSplash->splashWindowName.length() ? NULL : WS_EX_TOOLWINDOW,
		"OxySplashScreen",
		pSplash->splashWindowName.c_str(),
		WS_CLIPCHILDREN | WS_POPUP,
		areaRect.left,
		areaRect.top,
		pSplashImage->GetWidth(),
		pSplashImage->GetHeight(),
		pSplash->hwndParent,
		NULL,
		windowsClass.hInstance,
		NULL
	);

	R_ASSERT(pSplash->hwndSplash);

	// get long pointer
	SetWindowLongPtrA(pSplash->hwndSplash, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSplash));
	ShowWindow(pSplash->hwndSplash, SW_SHOWNOACTIVATE);

	MSG msg = { NULL };
	BOOL bRet = FALSE;
	LONG timerCount = 0;

	PeekMessageA(&msg, NULL, 0, 0, 0);
	SetEvent(pSplash->hEvent);

	// while game isn't runned
	while ((bRet = GetMessageA(&msg, NULL, 0, 0) != 0))
	{
		if (msg.message == WM_QUIT) break;
		if (msg.message == PBM_SETPOS)
		{
			KillTimer(NULL, pSplash->timerID);
			SendMessage(pSplash->hwndSplash, PBM_SETPOS, msg.wParam, msg.lParam);
			continue;
		}
		if (msg.message == PBM_SETSTEP)
		{
			SendMessage(pSplash->hwndSplash, PBM_SETPOS, LOWORD(msg.wParam), 0); // initiate progress bar creation
			SendMessage(pSplash->hwndProgress, PBM_SETSTEP, (HIWORD(msg.wParam) - LOWORD(msg.wParam)) / msg.lParam, 0);
			timerCount = static_cast<LONG>(msg.lParam);
			pSplash->timerID = SetTimer(NULL, 0, 1000, NULL);
			continue;
		}
		if (msg.message == WM_TIMER && msg.wParam == pSplash->timerID)
		{
			SendMessage(pSplash->hwndProgress, PBM_STEPIT, 0, 0);
			timerCount--;
			if (timerCount <= 0) 
			{
				timerCount = 0;
				KillTimer(NULL, pSplash->timerID);
				Sleep(0);
			}
			continue;
		}
		if (msg.message == PBM_SETBARCOLOR)
		{
			if (!IsWindow(pSplash->hwndProgress)) 
			{
				SendMessage(pSplash->hwndSplash, PBM_SETPOS, 0, 0); // initiate progress bar creation
			}
			SendMessage(pSplash->hwndProgress, PBM_SETBARCOLOR, msg.wParam, msg.lParam);
			continue;
		}

		if (bRet != -1)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DestroyWindow(pSplash->hwndSplash);
	return NULL;
}

LRESULT CALLBACK DSplashScreen::SplashWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DSplashScreen* pInstance = reinterpret_cast<DSplashScreen*>(GetWindowLongPtr(hwnd, (-21)));

	if (!pInstance) { return DefWindowProcA(hwnd, uMsg, wParam, lParam); }
	
	Gdiplus::Image* pSplashImage = reinterpret_cast<Gdiplus::Image*>(pInstance->pMainImage);

	switch (uMsg)
	{
	case WM_PAINT:
		if (pInstance->pMainImage)
		{
			Gdiplus::Graphics gdip(hwnd);
			gdip.DrawImage(pSplashImage, 0, 0, pSplashImage->GetWidth(), pSplashImage->GetHeight());

			if (pInstance->progressMsg.size() > 0)
			{
				Gdiplus::Font msgFont(L"Arial", 16, 0, Gdiplus::UnitPixel);

				Gdiplus::SolidBrush msgBrush(static_cast<DWORD>(Gdiplus::Color::White));

				//#VERTVER: PLS REWORK IT
				//////////////////////////////////////
				std::string prgress = pInstance->progressMsg.c_str();
				std::wstring progressName(prgress.begin(), prgress.end());
				//////////////////////////////////////

				gdip.DrawString(progressName.c_str(), -1, &msgFont, Gdiplus::PointF(2.0f, pSplashImage->GetHeight() - 43.0f), &msgBrush);
			}
			ValidateRect(hwnd, NULL);
			return 0;
		}
		break;
	case PBM_SETPOS:
		// if window is not be created - create it 
		if (!IsWindow(pInstance->hwndProgress))
		{
			RECT clientRect = { NULL };
			GetClientRect(hwnd, &clientRect);

			pInstance->hwndProgress = CreateWindowA(
				PROGRESS_CLASS,
				NULL,
				WS_CHILD | WS_VISIBLE,
				4,
				clientRect.bottom - 20,
				clientRect.right - 8,
				16,
				hwnd,
				NULL,
				GetModuleHandleA(NULL),
				NULL
			);

			SendMessage(pInstance->hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		}
		SendMessage(pInstance->hwndProgress, PBM_SETPOS, wParam, NULL);

		xr_string msgThread = *reinterpret_cast<xr_string*>(lParam);

		// if our message is not a previos 
		if (msgThread.size() && pInstance->progressMsg != msgThread)
		{
			pInstance->progressMsg = msgThread;
			SendMessage(pInstance->hwndSplash, WM_PAINT, 0, 0);
		}
		return 0;
		break;
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
