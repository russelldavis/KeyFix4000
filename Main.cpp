// KeyFix.cpp : main source file for KeyFix.exe
//

#include "stdafx.h"
#include "resource.h"
#include "aboutdlg.h"
#include "KeyFix.h"
#include "SystemTraySDK.h"

#define WM_APP_TRAYMSG WM_APP + 100

CAppModule _Module;

class CKeyFixTray : public CSystemTray
{
protected:
	void OnCommand(int idMenu)
	{
		if (idMenu == ID_TRAYMENU_EXIT)
		{
			PostQuitMessage(0);
		}
	}
};

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);


	CKeyFix::SetHook();

	
	HICON hIcon = LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME));
	CKeyFixTray sysTray;
	sysTray.Create(_Module.GetModuleInstance(), NULL, WM_APP_TRAYMSG, _T("KeyFix"), hIcon, IDR_TRAYMENU);


	int nRet = theLoop.Run();

	CKeyFix::RemoveHook();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
