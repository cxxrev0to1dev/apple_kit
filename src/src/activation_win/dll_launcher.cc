#include "dll_launcher.h"
#pragma comment(lib,"ntdll.lib")
#ifdef _DEBUG
#pragma comment(lib,"uafxcwd.lib")
#else
#pragma comment(lib,"uafxcw.lib")
#endif

namespace activation_win{
	static HANDLE WINAPI ShellcodeBegin(PTHREAD_DATA parameter){
		if (parameter->fnRtlInitUnicodeString != nullptr&&parameter->fnLdrLoadDll != nullptr){
			UNICODE_STRING UnicodeString;
			parameter->fnRtlInitUnicodeString(&UnicodeString, parameter->dllpath);
			HANDLE module_handle;
			parameter->fnLdrLoadDll(nullptr, nullptr, &UnicodeString, &module_handle);
			return module_handle;
		}
		return (HANDLE)-3;
	}
	static DWORD WINAPI ShellcodeEnd(){
		return 0;
	}
	static bool SetProcessPrivilege(DWORD SE_DEBUG_PRIVILEGE = 0x14){
		BOOLEAN bl;
		RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &bl);
		return bl;
	}
	bool Create(const wchar_t* name, bool is_resume){
		static STARTUPINFOW si = {0};
		static PROCESS_INFORMATION pi = {0};
		if (is_resume){
			ResumeThread(pi.hThread);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			memset(&si, 0, sizeof(STARTUPINFOW));
			memset(&pi, 0, sizeof(PROCESS_INFORMATION));
			return false;
		}
		else{
			si.cb = sizeof(STARTUPINFOW);
			if (!CreateProcessW(name,
				nullptr,
				NULL,
				NULL,
				FALSE,
				CREATE_SUSPENDED,
				NULL,
				NULL,
				&si,
				&pi)){
				return true;
			}
			return false;
		}
		//ShellExecuteW(nullptr, L"open", name, nullptr, nullptr, SW_SHOW);
	}
	DWORD SnapshotProcess(const wchar_t* process_name){
		PROCESSENTRY32 pe32;
		DWORD process_id = 0;
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap != INVALID_HANDLE_VALUE){
			pe32.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(hProcessSnap, &pe32)){
				do{
					if (!lstrcmpiW(pe32.szExeFile, process_name)){
						process_id = pe32.th32ProcessID;
						break;
					}
				} while (Process32Next(hProcessSnap, &pe32));
			}
		}
		CloseHandle(hProcessSnap);
		return process_id;
	}
	bool ProcessInternalExecute(PTHREAD_DATA parameter, DWORD process_id, int method){
		HANDLE hProcess = nullptr;
		CLIENT_ID cid = { (HANDLE)process_id, nullptr };
		OBJECT_ATTRIBUTES oa;
		SetProcessPrivilege();
		InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);
		if (!NT_SUCCESS(NtOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &oa, &cid))){
			return false;
		}
		PVOID data = VirtualAllocEx(hProcess, NULL, 0x2000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		PVOID code = VirtualAllocEx(hProcess, NULL, 0x2000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!data || !code){
			NtClose(hProcess);
			return false;
		}
		NtWriteVirtualMemory(hProcess, data, parameter, sizeof(THREAD_DATA), NULL);
		NtWriteVirtualMemory(hProcess, code, (PVOID)ShellcodeBegin, (ULONG)((LPBYTE)ShellcodeEnd - (LPBYTE)ShellcodeBegin), NULL);
		HANDLE hThread = nullptr;
		if (method == 1 && parameter->dllpath[0]){
			void* dll = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			std::wstring dllpath = parameter->dllpath;
			NtWriteVirtualMemory(hProcess, dll, (void*)dllpath.c_str(), dllpath.length()*sizeof(wchar_t), NULL);
			void* load = (void *)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "LoadLibraryW");
			hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)load, dll, 0, NULL);
			if (hThread==NULL){
				NtClose(hProcess);
				return false;
			}
		}
		else if (method == 2 && parameter->dllpath[0]){
			if (!NT_SUCCESS(RtlCreateUserThread(hProcess, NULL, FALSE, 0, 0, 0, code, data, &hThread, NULL))){
				NtClose(hProcess);
				return false;
			}
		}
		NtWaitForSingleObject(hThread, FALSE, NULL);
		DWORD exit_code = -1;
		GetExitCodeThread(hThread, &exit_code);
		NtClose(hThread);
		VirtualFreeEx(hProcess, data, 0, MEM_RELEASE);
		VirtualFreeEx(hProcess, code, 0, MEM_RELEASE);
		NtClose(hProcess);
		return (exit_code==0);
	}
	std::wstring GetAbsolutePath(const std::wstring& name){
		wchar_t fileName[MAX_PATH] = {0};
		GetModuleFileNameW(NULL, fileName, MAX_PATH);
		PathRemoveFileSpec(fileName);
		return std::wstring(fileName).append(name);
	}
	void SetShellcodeLdrModulePath(PTHREAD_DATA parameter,const std::wstring& srcfile){
		wcscpy_s(parameter->dllpath, srcfile.c_str());
	}
}