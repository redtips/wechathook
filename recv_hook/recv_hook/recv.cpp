#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)
#include <string>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
#include <stdio.h>
#include <Python.h>
#include <Shlwapi.h>
#include <stdlib.h>
#include <iostream> 
#include "recv.h"

using namespace std;


#define ReciveMessage 0x315E98			//接收消息
#define ReciveMessageParam 0x126D7F8		//接收消息的参数偏移
wchar_t tempwxid[50] = { 0 };	//存放微信ID

DWORD r_esp = 0;
DWORD r_eax = 0;

CHAR originalCode[5] = { 0 };
//参数的偏移
DWORD dwParam = (DWORD)GetModuleHandle(L"WeChatWin.dll") + ReciveMessageParam;

//计算需要HOOK的地址
DWORD dwHookAddr = (DWORD)GetModuleHandle(L"WeChatWin.dll") + ReciveMessage - 5;

//返回地址
DWORD RetAddr = dwHookAddr + 5;

//消息结构体
struct Message
{
	wchar_t type[10];		//消息类型
	wchar_t source[20];		//消息来源
	wchar_t wxid[40];		//微信ID/群ID
	wchar_t msgSender[40];	//消息发送者
	wchar_t content[200];	//消息内容
};

PyObject* StringToPy(std::string p_obj)
{
	int wlen = ::MultiByteToWideChar(CP_ACP, NULL, p_obj.c_str(), int(p_obj.size()), NULL, 0);
	wchar_t* wszString = new wchar_t[wlen + 1];
	::MultiByteToWideChar(CP_ACP, NULL, p_obj.c_str(), int(p_obj.size()), wszString, wlen);
	wszString[wlen] = '\0';
	PyObject* pb = PyUnicode_FromUnicode((const Py_UNICODE*)wszString, wlen);
	delete wszString;
	return pb;
}

std::string WstringToString(const std::wstring str)
{// wstring转string
	unsigned len = str.size() * 4;
	setlocale(LC_CTYPE, "");
	char* p = new char[len];
	wcstombs(p, str.c_str(), len);
	std::string str1(p);
	delete[] p;
	return str1;
}

void HookChatRecord()
{
	//组装数据
	BYTE bJmpCode[5] = { 0xE9 };
	*(DWORD*)& bJmpCode[1] = (DWORD)RecieveWxMesage - dwHookAddr - 5;

	//保存当前位置的指令,在unhook的时候使用。
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwHookAddr, originalCode, 5, 0);

	//覆盖指令 B9 E8CF895C //mov ecx,0x5C89CFE8
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwHookAddr, bJmpCode, 5, 0);
}

__declspec(naked) void RecieveWxMesage()
{
	//保存现场
	__asm
	{
		//补充被覆盖的代码
		//5B950573  |.  B9 E8CF895C           mov ecx,WeChatWi.5C89CFE8
		//mov ecx,0x5C89CFE8
		mov ecx, dwParam

		//提取esp寄存器内容，放在一个变量中
		mov r_esp, esp
		mov r_eax, eax

		pushad
		pushfd
	}
	SendWxMessage();

	//恢复现场
	__asm
	{
		popfd
		popad
		//跳回被HOOK指令的下一条指令
		jmp RetAddr
	}
}

void SendWxMessage()
{
	Message* msg = new Message;
	//信息块的位置
	DWORD** msgAddress = (DWORD * *)r_esp;
	//消息类型
	DWORD msgType = *((DWORD*)(**msgAddress + 0x30));
	wstring fullmessgaedata = GetMsgByAddress(**msgAddress + 0x68);

	LPVOID pWxid = *((LPVOID*)(**msgAddress + 0x40));
	swprintf_s(msg->wxid, L"%s", (wchar_t*)pWxid);

	//MessageBox(
	//	NULL,
	//	fullmessgaedata.c_str(),
	//	NULL,
	//	MB_OK
	//);

	if (StrStrW(msg->wxid, L"gh")) {
		//初始化Python环境 
		Py_Initialize();
		PyRun_SimpleString("import sys");
		PyRun_SimpleString("import pymysql");
		//添加Insert模块路径 
		//PyRun_SimpleString(chdir_cmd.c_str());
		PyRun_SimpleString("sys.path.append('./')");
		//导入模块 
		PyObject* pModule = PyImport_ImportModule("demo");
		PyObject* pFunc = NULL;
		pFunc = PyObject_GetAttrString(pModule, "save");
		std::string str = "";
		str = WstringToString(fullmessgaedata);
		const char* p = str.c_str();
		PyObject* pArgs = PyTuple_New(1);
		PyTuple_SetItem(pArgs, 0, StringToPy(str));
		PyObject* pRet = PyObject_CallObject(pFunc, pArgs);
		Py_Finalize();
	}


}


LPCWSTR GetMsgByAddress(DWORD memAddress)
{
	//获取字符串长度
	DWORD msgLength = *(DWORD*)(memAddress + 4);
	if (msgLength == 0)
	{
		WCHAR* msg = new WCHAR[1];
		msg[0] = 0;
		return msg;
	}

	WCHAR* msg = new WCHAR[msgLength + 1];
	ZeroMemory(msg, msgLength + 1);

	//复制内容
	wmemcpy_s(msg, msgLength + 1, (WCHAR*)(*(DWORD*)memAddress), msgLength + 1);
	return msg;
}


