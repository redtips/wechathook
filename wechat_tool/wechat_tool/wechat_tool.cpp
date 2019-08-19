// wechat_tool.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "wechat_tool.h"
#include <Windows.h>
#include "resource.h"
#include <Tlhelp32.h>
#include <stdio.h>
#include <direct.h>
#define WeChatName "WeChat.exe"
#define DLLNAME "recv_hook.dll"

VOID Inject_dll();
DWORD ProcessNameFindPid(LPCSTR ProcessName);
INT_PTR CALLBACK Dlgproc(
	HWND Arg1,
	UINT Arg2,
	WPARAM Arg3,
	LPARAM Arg4
);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{	
	DialogBox(hInstance, MAKEINTRESOURCE(ID_MAIN),NULL,&Dlgproc);
	return 0;
}

INT_PTR CALLBACK Dlgproc(
	HWND Arg1,
	UINT Arg2,
	WPARAM Arg3,
	LPARAM Arg4
)
{
	if (Arg2 == WM_CLOSE) {
		EndDialog(Arg1,0);
	}
	if (Arg2 == WM_COMMAND) {
		if (Arg3 == INJECT_DLL) {
			//注入dll
			Inject_dll();
		}
	}

	return FALSE;
}


VOID Inject_dll() {
	//获取当前工作目录下的dll
	char pathstr[MAX_PATH] = { 0 };
	char* buffer = NULL;
	if ((buffer = _getcwd(NULL, 0)) == NULL)
	{
		MessageBoxA(NULL, "获取当前工作目录失败", "错误", 0);
		return;
	}
	sprintf_s(pathstr, "%s\\%s", buffer, DLLNAME);

	DWORD wechat_pid = ProcessNameFindPid(WeChatName);
	HANDLE wechat_handle = OpenProcess(
		PROCESS_ALL_ACCESS,
		FALSE,
		wechat_pid
	);

	LPVOID dll_lp  = VirtualAllocEx(
		wechat_handle,
		NULL,
		strlen(pathstr),
		MEM_COMMIT,
		PAGE_READWRITE
	);

	WriteProcessMemory(
		wechat_handle,
		dll_lp,
		pathstr,
		strlen(pathstr),
		NULL
	);
	
	HMODULE k32 = GetModuleHandle("kernel32.dll");

	LPVOID load_addr = GetProcAddress(k32, "LoadLibraryA");

	

	HANDLE ret= CreateRemoteThread(
		wechat_handle,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)load_addr,
		dll_lp,
		0,
		NULL
	);

	if (ret == NULL) {
		MessageBox(NULL, "fail", "dll", 0);
	}
	else
	{
		MessageBox(NULL, "scuess", "dll", 0);
	}
}

DWORD ProcessNameFindPid(LPCSTR ProcessName) {
	HANDLE all_handle =  CreateToolhelp32Snapshot(
		TH32CS_SNAPALL,
		0
	);
	if (all_handle == "INVALID_HANDLE_VALUE") {
		MessageBox(NULL, "获取快照失败", "错误", MB_ICONEXCLAMATION);
	}
	//比对进程
	PROCESSENTRY32 processinfo = { 0 };

	processinfo.dwSize = sizeof(PROCESSENTRY32);

	do
	{
		if (strcmp(processinfo.szExeFile,ProcessName) == 0) {
			return processinfo.th32ProcessID;
		}
	} while (Process32Next(all_handle, &processinfo));

}
