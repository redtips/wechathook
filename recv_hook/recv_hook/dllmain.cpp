// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <Windows.h>
#include "resource.h"
#include "recv.h"

INT_PTR CALLBACK Dlgproc(
	HWND Arg1,
	UINT Arg2,
	WPARAM Arg3,
	LPARAM Arg4
);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DialogBox(
			hModule,
			MAKEINTRESOURCE(MAIN),
			NULL,
			&Dlgproc
		);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

INT_PTR CALLBACK Dlgproc(
	HWND Arg1,
	UINT Arg2,
	WPARAM Arg3,
	LPARAM Arg4
)
{
	switch (Arg2)
	{
	case WM_CLOSE:EndDialog(
		Arg1,
		0
	);
		break;
	case WM_COMMAND:
		switch (Arg3)
		{
		case RECV_HOOK:
			HookChatRecord();
			EndDialog(
				Arg1,
				0
			);

		default:
			break;
		}
	default:
		break;
	}
	return FALSE;
}
