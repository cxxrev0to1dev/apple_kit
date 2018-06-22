#include "activation_user.h"
#include <chrono>
#include <thread>
#include <cassert>
#include <functional>
#include <TlHelp32.h>

namespace activation_win{

	std::wstring ActivationUser::GetApp(const wchar_t *name, const wchar_t* module_name){
		wchar_t drive[_MAX_DRIVE] = { 0 };
		wchar_t dir[_MAX_DIR] = { 0 };
		wchar_t fname[_MAX_FNAME] = { 0 };
		wchar_t ext[_MAX_EXT] = { 0 };
		wchar_t buffer[MAX_PATH] = { 0 };
		GetModuleFileNameW(module_name == nullptr ? nullptr : GetModuleHandleW(module_name), buffer, MAX_PATH);
		_wsplitpath_s(buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		return (std::wstring(drive) + std::wstring(dir) + std::wstring(name));
	}

	bool ActivationUser::IsRuningITunes(){
		ActivationUser acts_user;
		HWND hwnd = FindWindowW(acts_user.class_name(), acts_user.window_name());
		return (hwnd != nullptr);
	}

	ActivationUser::ActivationUser(){
		class_name_.append(L"iTunes");
		window_name_.append(L"iTunes");
	}

	ActivationUser::~ActivationUser(){
		class_name_.clear();
		window_name_.clear();
	}
	void ActivationUser::SendLogout(){
		INPUT input;
		input.type = INPUT_MOUSE;
		input.mi.dx = 320;
		input.mi.dy = 320;
		input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP);
		input.mi.mouseData = 0;
		input.mi.dwExtraInfo = NULL;
		input.mi.time = 0;
		SendInput(1, &input, sizeof(INPUT));
		for (size_t i = 0; i < 3; i++){
			keybd_event(VK_DOWN, MapVirtualKey(VK_DOWN, 0), 0, 0);
			Sleep(100);
			keybd_event(VK_DOWN, MapVirtualKey(VK_DOWN, 0), KEYEVENTF_KEYUP, 0);
		}
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);
		Sleep(100);
		keybd_event(VkKeyScanA('O'), MapVirtualKey('O', 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA('O'), MapVirtualKey('O', 0), KEYEVENTF_KEYUP, 0);
	}
	void ActivationUser::SendMaximizeITunes(){
		HWND hwnd = FindWindowW(class_name(), window_name());
		SetForegroundWindow(hwnd);
		SendMessageW(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	void ActivationUser::SendLoginMessage(){
		SendLogout();
		Sleep(2000);
		INPUT input;
		input.type = INPUT_MOUSE;
		input.mi.dx = 320;
		input.mi.dy = 320;
		input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP);
		input.mi.mouseData = 0;
		input.mi.dwExtraInfo = NULL;
		input.mi.time = 0;
		SendInput(1, &input, sizeof(INPUT));
		for (size_t i = 0; i < 3; i++){
			keybd_event(VK_DOWN, MapVirtualKey(VK_DOWN, 0), 0, 0);
			Sleep(100);
			keybd_event(VK_DOWN, MapVirtualKey(VK_DOWN, 0), KEYEVENTF_KEYUP, 0);
		}
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);
		keybd_event(VkKeyScanA('I'), MapVirtualKey('I', 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA('I'), MapVirtualKey('I', 0), KEYEVENTF_KEYUP, 0);
	}
	void ActivationUser::SendUserPassMessage(const char* account_name, const char* password){
		Sleep(3000);
		int length = strlen(account_name);
		for (int index = 0; index < length; index++){
			if (account_name[index] == '@'){
				keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_EXTENDEDKEY | 0, 0),
					keybd_event(LOBYTE(VkKeyScanA('2')), MapVirtualKey('2', 0), 0, 0);
				Sleep(250);
				keybd_event(LOBYTE(VkKeyScanA('2')), MapVirtualKey('2', 0), KEYEVENTF_KEYUP, 0);
				keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				Sleep(250);
				continue;
			}
			keybd_event(LOBYTE(VkKeyScanA(account_name[index])), MapVirtualKey(account_name[index], 0), 0, 0);
			Sleep(250);
			keybd_event(LOBYTE(VkKeyScanA(account_name[index])), MapVirtualKey(account_name[index], 0), KEYEVENTF_KEYUP, 0);
			Sleep(250);
		}
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), 0, 0);
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), KEYEVENTF_KEYUP, 0);
		Sleep(1000);
		length = strlen(password);
		for (int index = 0; index < length; index++){
			int scan = MapVirtualKey(password[index], 0);
			if (isupper(password[index])){
				keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_EXTENDEDKEY | 0, 0),
					keybd_event(LOBYTE(VkKeyScanA(password[index])), scan, 0, 0);
				Sleep(250);
				keybd_event(LOBYTE(VkKeyScanA(password[index])), scan, KEYEVENTF_KEYUP, 0);
				keybd_event(VK_SHIFT, MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				Sleep(250);
			}
			else{
				keybd_event(LOBYTE(VkKeyScanA(password[index])), scan, 0, 0);
				Sleep(250);
				keybd_event(LOBYTE(VkKeyScanA(password[index])), scan, KEYEVENTF_KEYUP, 0);
				Sleep(250);
			}
		}
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), KEYEVENTF_KEYUP, 0);
		Sleep(5000);
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);
	}
	void ActivationUser::EnterKey(){
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), 0, 0);
		Sleep(250);
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);
	}
	void ActivationUser::PostContinueMessage(){
		std::wstring continue_exe = ActivationUser::GetApp(L"continue.exe");
		ShellExecuteW(nullptr, L"open", continue_exe.c_str(), nullptr, nullptr, SW_SHOW);
		Sleep(5000);
		for (int i = 0; i <= 5; i++){
			SetForegroundWindow(FindWindowW(class_name(), window_name()));
			SendMaximizeITunes();
			if (i == 5){
				for (int j = 0; j < 2; j++){
					keybd_event(VkKeyScanA(VK_F10), MapVirtualKey(VK_F10, 0), 0, 0);
					Sleep(100);
					keybd_event(VkKeyScanA(VK_F10), MapVirtualKey(VK_F10, 0), KEYEVENTF_KEYUP, 0);
					Sleep(1000);
				}
				break;
			}
			Sleep(1000);
		}
	}
	void ActivationUser::ClickButton(int x, int y, const wchar_t* button_name){
		RECT rect;
		HWND hwnd = FindWindowW(class_name(), window_name());
		assert(hwnd != nullptr);
		SetForegroundWindow(hwnd);
		GetWindowRect(hwnd, &rect);
		std::function<void(int, int)> Click = [](int x, int y){
			const double XSCALEFACTOR = 65535 / (GetSystemMetrics(SM_CXSCREEN) - 1);
			const double YSCALEFACTOR = 65535 / (GetSystemMetrics(SM_CYSCREEN) - 1);
			double nx = x * XSCALEFACTOR;
			double ny = y * YSCALEFACTOR;
			INPUT Input = { 0 };
			Input.type = INPUT_MOUSE;
			Input.mi.dx = (LONG)nx;
			Input.mi.dy = (LONG)ny;
			Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
			SendInput(1, &Input, sizeof(INPUT));
		};
		Click(rect.left + x, rect.top + y);
	}
	void ActivationUser::SendBillingInformation(const char* unit, const char* room, const char* roadname, const char* city, const char* zip_code, const char* phone_number){
		//std::this_thread::sleep_for(std::chrono::minutes(4));
		for (int index = 0; index < 7; index++){
			if (index != 5){
				Sleep(3000);
				char* src = nullptr;
				switch (index){
				case 0:
					src = const_cast<char*>(unit);
					OutputDebugStringA(unit);
					break;
				case 1:
					src = const_cast<char*>(room);
					OutputDebugStringA(room);
					break;
				case 2:
					src = const_cast<char*>(roadname);
					OutputDebugStringA(roadname);
					break;
				case 3:
					src = const_cast<char*>(city);
					OutputDebugStringA(city);
					break;
				case 4:
					src = const_cast<char*>(zip_code);
					OutputDebugStringA(zip_code);
					break;
				case 6:
					src = const_cast<char*>(phone_number);
					OutputDebugStringA(phone_number);
					break;
				default:
					break;
				}
				if (src!=nullptr){
					int length = strlen(src);
					for (int str_index = 0; str_index < length; str_index++){
						keybd_event(LOBYTE(VkKeyScanA(src[str_index])), MapVirtualKey(src[str_index], 0), 0, 0);
						Sleep(100);
						keybd_event(LOBYTE(VkKeyScanA(src[str_index])), MapVirtualKey(src[str_index], 0), KEYEVENTF_KEYUP, 0);
						Sleep(100);
					}
				}
			}
			keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), 0, 0);
			Sleep(100);
			keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), KEYEVENTF_KEYUP, 0);
		}
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), KEYEVENTF_KEYUP, 0);
		Sleep(1000);
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA(VK_TAB), MapVirtualKey(VK_TAB, 0), KEYEVENTF_KEYUP, 0);
	}
	void ActivationUser::PostCreateMessage(){
		Sleep(5000);
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), 0, 0);
		Sleep(100);
		keybd_event(VkKeyScanA(VK_RETURN), MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);
		Sleep(5000);
		SendMessageW(FindWindowW(nullptr, L"continue"), WM_CLOSE, 0, 0);
		Sleep(5000);
		SetForegroundWindow(FindWindowW(class_name(), window_name()));
		SendMaximizeITunes();
		// 	SendMessageW(FindWindowW(nullptr, L"continue"), WM_CLOSE, 0, 0);
		// 	SendMaximizeITunes();
		// 	Sleep(5000);
		//  	SetForegroundWindow(FindWindowW(class_name(), window_name()));
		// 	INPUT input;
		// 	input.type = INPUT_MOUSE;
		// 	input.mi.dx = 320;
		// 	input.mi.dy = 320;
		// 	input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP);
		// 	input.mi.mouseData = 0;
		// 	input.mi.dwExtraInfo = NULL;
		// 	input.mi.time = 0;
		// 	SendInput(1, &input, sizeof(INPUT));
	}
	void ActivationUser::SendClose(){
		NTSTATUS(*ZwTerminateProcess)(HANDLE ProcessHandle, NTSTATUS ExitStatus);
#ifdef _M_X64
		*(PDWORD64)&ZwTerminateProcess = (DWORD64)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwTerminateProcess");
#else
		*(PDWORD)&ZwTerminateProcess = (DWORD)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwTerminateProcess");
#endif
		*(PDWORD64)&ZwTerminateProcess = (DWORD64)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwTerminateProcess");
		std::function<DWORD(const wchar_t*)> ProcessId = [](const wchar_t* name) ->DWORD{
			PROCESSENTRY32 pe32 = { 0 };
			DWORD process_id = 0;
			HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hProcessSnap != INVALID_HANDLE_VALUE){
				pe32.dwSize = sizeof(PROCESSENTRY32);
				if (Process32First(hProcessSnap, &pe32)){
					do{
						if (!lstrcmpiW(pe32.szExeFile, name)){
							process_id = pe32.th32ProcessID;
							break;
						}
					} while (Process32Next(hProcessSnap, &pe32));
				}
			}
			CloseHandle(hProcessSnap);
			return process_id;
		};
		DWORD process_id = ProcessId(L"iTunes.exe");
		if (process_id){
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
			if (process){
				ZwTerminateProcess(process, 0);
				CloseHandle(process);
			}
		}
	}
}