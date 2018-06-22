#include "activation_win.h"
#include <time.h>
#include <cstdint>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <thread>
#include <future>
#include <Iphlpapi.h>
#include <ShlObj.h>
#pragma comment(lib,"Shell32.lib")
#pragma comment(lib,"ws2_32.lib")
#include "mhook/mhook-lib/mhook.h"
#include <glog/logging.h>
#include "activation_web.h"

namespace activation_win{
	class AtlDllMain :public ATL::CAtlDllModuleT<AtlDllMain>{
	public:
		AtlDllMain()
			:CAtlDllModuleT< AtlDllMain >(),
			is_initialized_(false) {
		}
		~AtlDllMain() {
		}
		void Initialize(HMODULE module) {
			if (is_initialized_) {
				return;
			}
			is_initialized_ = true;
		}
		bool is_initialized()const{
			return is_initialized_;
		}
		static AtlDllMain* Instance(bool process_exit = false){
			static AtlDllMain* ref_instance;
			if (!ref_instance){
				AtlDllMain* new_instance = new AtlDllMain;
				if (InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&ref_instance), new_instance, NULL)){
					delete new_instance;
				}
			}
			if (process_exit){
				delete ref_instance;
				ref_instance = NULL;
			}
			return ref_instance;
		}
	protected:
		bool is_initialized_;
	};

	std::map<std::string, std::string> GetUsernamePair(const wchar_t* filename, const char filted_tag, const char* filted_str){
		std::function<std::vector<std::string>(const wchar_t* filename)> Read = [](const wchar_t* filename)->std::vector<std::string>{
			std::ifstream is(filename, std::ifstream::binary);
			std::vector<std::string> lines;
			if (is) {
				is.seekg(0, is.beg);
				std::string line;
				while (std::getline(is, line)){
					if (line.empty())
						continue;
					if (line.find('\r') != std::string::npos){
						line.erase(line.find('\r'));
					}
					if (line.find('\n') != std::string::npos){
						line.erase(line.find('\n'));
					}
					lines.push_back(line);
				}
				is.close();
			}
			return lines;
		};
		std::function<std::vector<std::string>(const std::string &s, char delim)> Split = [](
			const std::string &s,
			char delim)->std::vector<std::string>{
			std::vector<std::string> elems;
			std::stringstream ss(s);
			std::string number;
			while (std::getline(ss, number, delim)) {
				elems.push_back(number);
			}
			return elems;
		};
		std::map<std::string, std::string> result;
		std::vector<std::string> files = Read(filename);
		std::vector<std::string>::iterator iter;
		for (iter = files.begin(); iter != files.end(); iter++){
			if (iter->length()){
				std::vector<std::string> array = Split(iter->c_str(), filted_tag);
				if (array.size() == 3 && array[2].find(filted_str) != std::string::npos)
					continue;
				else if (array.size() == 2){
					if (array[1].find('\r') != std::string::npos){
						array[1].erase(array[1].find('\r'));
					}
					result[array[0]] = array[1];
				}

			}
		}
		return result;
	}

	namespace internal{
		bool IsFilterPath(std::wstring name){
			wchar_t buffer[MAX_PATH] = { 0 };
			GetModuleFileNameW(NULL, buffer, MAX_PATH);
			std::transform(name.begin(), name.end(), name.begin(), ::towlower);
			std::wstring process_name(buffer);
			std::transform(process_name.begin(), process_name.end(), process_name.begin(), ::towlower);
			return (wcsstr(process_name.c_str(), name.c_str()) != NULL);
		}
		bool HKLMWriteSystemBios(const wchar_t* bios){
			HKEY h_setting = NULL;
			bool result = false;
			if (!RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System", 0, KEY_WRITE, &h_setting)){
				if (!RegSetValueExW(h_setting,
					L"SystemBiosVersion",
					0,
					REG_SZ,
					(const BYTE*)bios,
					wcslen(bios)*sizeof(wchar_t)))
					result = true;
				RegCloseKey(h_setting);
			}
			return result;
		}
		bool HKLMWriteProcessorName(const wchar_t* processor){
			HKEY h_setting = NULL;
			bool result = false;
			if (!RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_WRITE, &h_setting)){
				if (!RegSetValueExW(h_setting,
					L"ProcessorNameString",
					0,
					REG_SZ,
					(const BYTE*)processor,
					wcslen(processor)*sizeof(wchar_t)))
					result = true;
				RegCloseKey(h_setting);
			}
			return result;
		}
		bool HKLMWriteProductId(const wchar_t* product_id){
			//00330-80000-00000-AA909
			HKEY h_setting = NULL;
			bool result = false;
			wchar_t* reg_name = NULL;
			OSVERSIONINFOEXW version_information = { 0 };
			version_information.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
			version_information.dwMajorVersion = 6;
			version_information.dwMinorVersion = 0;
			unsigned long long condition_mask = VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL);
			if (VerifyVersionInfoW(&version_information, VER_MAJORVERSION | VER_MINORVERSION, condition_mask)){
				reg_name = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
			}
			else{
				reg_name = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
			}
			if (!RegOpenKeyExW(HKEY_LOCAL_MACHINE, reg_name, 0, KEY_WRITE, &h_setting)){
				if (!RegSetValueExW(h_setting,
					L"ProductId",
					0,
					REG_SZ,
					(const BYTE*)product_id,
					wcslen(product_id)*sizeof(wchar_t)))
					result = true;
				RegCloseKey(h_setting);
			}
			return result;
		}
		std::string Read(const wchar_t* filename){
			std::string result;
			std::ifstream is(filename, std::ifstream::binary);
			if (is)
			{
				is.seekg(0, is.end);
				int length = is.tellg();
				is.seekg(0, is.beg);
				if (length){
					char* buffer = new char[length];
					is.read(buffer, length);
					result.append(buffer, length);
					delete[] buffer;
				}
				is.close();
			}
			return result;
		}
		void Write(const wchar_t* filename, const std::string& str, bool is_append = false){
			int flag = std::ofstream::binary | std::ofstream::out;
			if (is_append)
				flag |= std::ofstream::app;
			std::ofstream is(filename, flag);
			if (is){
				is.write(str.c_str(), str.length());
				is.flush();
				is.close();
			}
		}
		namespace configure{
			wchar_t* status_file = L"status_file";
			wchar_t* billing_file = L"billing_file";
			wchar_t* continue_file = L"continue_file";
			wchar_t* itunesid_file = L"itunesid_file";
			std::wstring appleid_file_name;
			std::wstring status_file_name;
			std::wstring billing_file_name;
			std::wstring continue_file_name;
			std::wstring itunesid_file_name;
			void set_appleid_file_name(const std::wstring& str){
				if (appleid_file_name.empty())
					appleid_file_name.append(str);
			}
			void set_status_file_name(const std::wstring& str){
				if (status_file_name.empty())
					status_file_name.append(str);
			}
			void set_billing_file_name(const std::wstring& str){
				if (billing_file_name.empty())
					billing_file_name.append(str);
			}
			void set_continue_file_name(const std::wstring& str){
				if (continue_file_name.empty())
					continue_file_name.append(str);
			}
			void set_itunesid_file_name(const std::wstring& str){
				if (itunesid_file_name.empty())
					itunesid_file_name.append(str);
			}
		}
		wchar_t* computer_name = L"ysyhnihao-pc";
		wchar_t* hw_guid = L"{6bb4f111-2111-22e5-9111-11116f6e1111}";
		wchar_t* bios = L"1.2.3.4";
		wchar_t* processor = L"Mobile Intel® GM965 Express CPU @ 2.77GHz";
		wchar_t* product_id = L"10110-80011-01100-11919";//ME:00330-80000-00000-AA909
		unsigned long volume_serial = 0x87654321;
		unsigned long adapter = 0x10;
		namespace hook{
			typedef BOOL(WINAPI* FN_TYPE_GetVolumeInformationW)(LPCWSTR, LPWSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPWSTR, DWORD);
			typedef BOOL(WINAPI* FN_TYPE_GetComputerNameW)(LPWSTR, LPDWORD);
			typedef BOOL(WINAPI* FN_TYPE_GetCurrentHwProfileW)(LPHW_PROFILE_INFOW);
			typedef DWORD(*FN_TYPE_GetAdaptersInfo)(PIP_ADAPTER_INFO, PULONG);
			typedef ULONG(WINAPI* FN_TYPE_GetAdaptersAddresses)(ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG);
			typedef BSTR(*FN_TYPE_SysAllocString)(const OLECHAR *psz);
			typedef const void* (*FN_TYPE_CFStringMakeConstantString)(char* str);
			typedef const void* (*FN_TYPE_CFStringGetLength)(const void* theString);
			typedef BOOL(*FN_TYPE_CFStringGetCString)(const void*, char *buffer, std::uint32_t, std::uint32_t);

			FN_TYPE_GetVolumeInformationW FN_GetVolumeInformationW;
			FN_TYPE_GetComputerNameW FN_GetComputerNameW;
			FN_TYPE_GetCurrentHwProfileW FN_GetCurrentHwProfileW;
			FN_TYPE_GetAdaptersInfo FN_GetAdaptersInfo;
			FN_TYPE_GetAdaptersAddresses FN_GetAdaptersAddresses;
			FN_TYPE_SysAllocString FN_SysAllocString;
			FN_TYPE_CFStringMakeConstantString FN_CFStringMakeConstantString;
			FN_TYPE_CFStringGetLength FN_CFStringGetLength;
			FN_TYPE_CFStringGetCString FN_CFStringGetCString;

			BOOL WINAPI HookedGetVolumeInformationW(
				LPCWSTR lpRootPathName,
				LPWSTR lpVolumeNameBuffer,
				DWORD nVolumeNameSize,
				LPDWORD lpVolumeSerialNumber,
				LPDWORD lpMaximumComponentLength,
				LPDWORD lpFileSystemFlags,
				LPWSTR lpFileSystemNameBuffer,
				DWORD nFileSystemNameSize
				){
				//OutputDebugStringW(L"HookedGetVolumeInformationW start!!!!!!");
				//OutputDebugStringW(lpRootPathName);
				BOOL result = FN_GetVolumeInformationW(lpRootPathName, lpVolumeNameBuffer,
					nVolumeNameSize, lpVolumeSerialNumber,
					lpMaximumComponentLength, lpFileSystemFlags,
					lpFileSystemNameBuffer, nFileSystemNameSize);
				//OutputDebugStringW(L"HookedGetVolumeInformationW end!!!!!!");
				if (lpVolumeSerialNumber != nullptr){
					*lpVolumeSerialNumber = internal::volume_serial;
				}
				return result;
			}
			BOOL WINAPI HookedGetComputerNameW(LPWSTR lpBuffer, LPDWORD lpnSize){
				BOOL result = FN_GetComputerNameW(lpBuffer, lpnSize);
				if (result){
					//OutputDebugStringW(L"HookedGetComputerNameW!!!!!!");
					lpnSize[0] = wcslen(internal::computer_name);
					wcscpy(lpBuffer, internal::computer_name);
				}
				return result;
			}
			BOOL WINAPI HookedGetCurrentHwProfileW(LPHW_PROFILE_INFOW lpHwProfileInfo){
				BOOL result = FN_GetCurrentHwProfileW(lpHwProfileInfo);
				if (result){
					//OutputDebugStringW(L"HookedGetCurrentHwProfileW!!!!!!");
					wcsncpy(lpHwProfileInfo->szHwProfileGuid, internal::hw_guid, wcslen(internal::hw_guid));
					//OutputDebugStringW(L"HookedGetCurrentHwProfileW!!!!!!");
				}
				return result;
			}
			DWORD HookedGetAdaptersInfo(PIP_ADAPTER_INFO pAdapterInfo,
				PULONG pOutBufLen){
				DWORD result = FN_GetAdaptersInfo(pAdapterInfo, pOutBufLen);
				//OutputDebugStringW(L"HookedGetAdaptersInfo!!!!!!");
				if (result == ERROR_SUCCESS){
					for (unsigned int i = 0; i < 6; i++){
						if (i >= pAdapterInfo->AddressLength){
							break;
						}
						pAdapterInfo->Address[i] += internal::adapter;
					}
				}
				return result;
			}
			ULONG WINAPI HookedGetAdaptersAddresses(
				ULONG Family,
				ULONG Flags,
				PVOID Reserved,
				PIP_ADAPTER_ADDRESSES AdapterAddresses,
				PULONG SizePointer
				){
				ULONG result = FN_GetAdaptersAddresses(Family, Flags, Reserved, AdapterAddresses, SizePointer);
				//OutputDebugStringW(L"HookedGetAdaptersAddresses!!!!!!");
				if (result == ERROR_SUCCESS){
					for (unsigned int i = 0; i < 6; i++){
						if (i >= AdapterAddresses->PhysicalAddressLength){
							break;
						}
						AdapterAddresses->PhysicalAddress[i] += internal::adapter;
					}
				}
				return result;
			}
			const void* HookedCFStringGetLength(const void* theString){
				Mhook_Unhook((PVOID*)&FN_CFStringGetLength);
				FN_CFStringGetLength = (FN_TYPE_CFStringGetLength)::GetProcAddress(::GetModuleHandle(L"CoreFoundation.dll"), "CFStringGetLength");
				if (theString != nullptr){
					try{
						static int failed_count = 0;
						char buffer[MAX_PATH] = { 0 };
						FN_CFStringGetCString(theString, buffer, MAX_PATH, 0);
						//std::wstring filename = ActivationUser::GetApp(internal::configure::status_file, L"itunesid_bind.dll");
						if (!strnicmp(buffer, "5001", 4) || !strnicmp(buffer, "BODY", 4)){
							OutputDebugStringA("FN_CFStringGetCString OK!!!");
							internal::Write(internal::configure::status_file_name.c_str(), std::string("OK"));
						}
						else if (strstr(buffer, "kCFStream") == nullptr&&
							strstr(buffer, "bag.xml") == nullptr&&
							strstr(buffer, "X-Apple-ActionSignature") == nullptr&&
							strstr(buffer, "com.apple.") == nullptr&&
							strstr(buffer, "Sock") == nullptr&&
							strstr(buffer, "iTunes") == nullptr&&
							buffer[0] != 0){
							if ((failed_count + 1) % 2 == 0){
								OutputDebugStringA("FN_CFStringGetCString Failed!!!");
								OutputDebugStringA(buffer);
								internal::Write(internal::configure::status_file_name.c_str(), std::string("Failed"));
							}
							failed_count += 1;
						}
						else{
							OutputDebugStringA("FN_CFStringGetCString ???!!!");
							OutputDebugStringA(buffer);
						}
					}
					catch (std::exception* e){
						OutputDebugStringA(e->what());
					}
				}
				return FN_CFStringGetLength(theString);
			}
			const void* HookedCFStringMakeConstantString(char* str){
				const char tag[] = "cancelButtonAction";
				if (!lstrcmpiA(str, "failureType")){
					FN_CFStringGetLength = (FN_TYPE_CFStringGetLength)::GetProcAddress(::GetModuleHandle(L"CoreFoundation.dll"), "CFStringGetLength");
					Mhook_SetHook((PVOID*)&FN_CFStringGetLength, HookedCFStringGetLength);
				}
				else if (!strnicmp(str, tag, sizeof(tag) - sizeof(char))){
					std::string message("HookedCFStringMakeConstantString:");
					message.append(str);
					OutputDebugStringA(message.c_str());
					//std::wstring filename = ActivationUser::GetApp(internal::configure::status_file, L"itunesid_bind.dll");
					internal::Write(internal::configure::status_file_name.c_str(), std::string("OK"));
				}
				return FN_CFStringMakeConstantString(str);
			}

			BSTR HookedSysAllocString(const OLECHAR *psz){
				const wchar_t tag[] = L"WebProgressFinishedNotification";
				const wchar_t tag_1[] = L"WebProgressStartedNotification";
				const wchar_t tag_2[] = L"WebViewDidChangeSelectionNotification";
				const wchar_t tag_3[] = L"Connections Get Started";
				if (psz){
					if (!wcsnicmp(psz, tag, wcslen(tag))){
						OutputDebugStringA("WebProgressFinishedNotification!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						OutputDebugStringW(psz);
						//std::wstring filename = ActivationUser::GetApp(internal::configure::status_file, L"itunesid_bind.dll");
						//DeleteFileW(filename.c_str());
						internal::Write(internal::configure::status_file_name.c_str(), std::string("OK"));
					}
					else if (!wcsnicmp(psz, tag_1, wcslen(tag_1))){
						OutputDebugStringA("WebProgressStartedNotification!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						OutputDebugStringW(psz);
						//std::wstring filename = ActivationUser::GetApp(internal::configure::status_file, L"itunesid_bind.dll");
						DeleteFileW(internal::configure::status_file_name.c_str());
					}
					else if (!wcsnicmp(psz, tag_2, wcslen(tag_2))){
						OutputDebugStringA("WebViewDidChangeSelectionNotification!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						OutputDebugStringW(psz);
						//std::wstring filename = ActivationUser::GetApp(internal::configure::billing_file, L"itunesid_bind.dll");
						DeleteFileW(internal::configure::billing_file_name.c_str());
						internal::Write(internal::configure::billing_file_name.c_str(), std::string("OK"));
					}
					else if (!wcsnicmp(psz, tag_3, wcslen(tag_3))){
						OutputDebugStringA("Connections Get Started!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						OutputDebugStringW(psz);
						//std::wstring filename = ActivationUser::GetApp(internal::configure::continue_file, L"itunesid_bind.dll");
						DeleteFileW(internal::configure::continue_file_name.c_str());
						internal::Write(internal::configure::continue_file_name.c_str(), std::string("OK"));
					}
					else{
						OutputDebugStringW(psz);
					}
				}
				return FN_SysAllocString(psz);
			}
		}
	}
}
BOOL WINAPI DllMain(HMODULE hDllHandle, DWORD dwReason, LPVOID lpreserved){
	DisableThreadLibraryCalls(hDllHandle);
	using namespace activation_win;
	using namespace activation_win::internal::hook;
	if (dwReason == DLL_PROCESS_ATTACH&&!AtlDllMain::Instance()->is_initialized()){
		internal::configure::set_appleid_file_name(ActivationUser::GetApp(L"appleid.mail", L"activation_win.dll"));
		internal::configure::set_status_file_name(ActivationUser::GetApp(internal::configure::status_file, L"activation_win.dll"));
		internal::configure::set_billing_file_name(ActivationUser::GetApp(internal::configure::billing_file, L"activation_win.dll"));
		internal::configure::set_continue_file_name(ActivationUser::GetApp(internal::configure::continue_file, L"activation_win.dll"));
		internal::configure::set_itunesid_file_name(ActivationUser::GetApp(internal::configure::itunesid_file, L"activation_win.dll"));
		if (internal::IsFilterPath(L"iTunes.exe")){
			HANDLE thread = reinterpret_cast<HANDLE>(CreateThread(nullptr, 0,
				[](void* args) ->DWORD{
				FN_GetVolumeInformationW = (FN_TYPE_GetVolumeInformationW)::GetProcAddress(::LoadLibraryW(L"kernel32.dll"), "GetVolumeInformationW");
				FN_GetComputerNameW = (FN_TYPE_GetComputerNameW)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "GetComputerNameW");
				FN_GetCurrentHwProfileW = (FN_TYPE_GetCurrentHwProfileW)::GetProcAddress(::LoadLibraryW(L"Advapi32.dll"), "GetCurrentHwProfileW");
				FN_GetAdaptersInfo = (FN_TYPE_GetAdaptersInfo)::GetProcAddress(::LoadLibraryW(L"Iphlpapi.dll"), "GetAdaptersInfo");
				FN_GetAdaptersAddresses = (FN_TYPE_GetAdaptersAddresses)::GetProcAddress(::GetModuleHandle(L"Iphlpapi.dll"), "GetAdaptersAddresses");
				FN_SysAllocString = (FN_TYPE_SysAllocString)::GetProcAddress(::LoadLibraryW(L"OleAut32.dll"), "SysAllocString");
				if (FN_SysAllocString)
					Mhook_SetHook((PVOID*)&FN_SysAllocString, HookedSysAllocString);
				if (FN_GetVolumeInformationW)
					Mhook_SetHook((PVOID*)&FN_GetVolumeInformationW, HookedGetVolumeInformationW);
				if (FN_GetComputerNameW)
					Mhook_SetHook((PVOID*)&FN_GetComputerNameW, HookedGetComputerNameW);
				if (FN_GetCurrentHwProfileW)
					Mhook_SetHook((PVOID*)&FN_GetCurrentHwProfileW, HookedGetCurrentHwProfileW);
				if (FN_GetAdaptersInfo)
					Mhook_SetHook((PVOID*)&FN_GetAdaptersInfo, HookedGetAdaptersInfo);
				if (FN_GetAdaptersAddresses)
					Mhook_SetHook((PVOID*)&FN_GetAdaptersAddresses, HookedGetAdaptersAddresses);
				std::this_thread::sleep_for(std::chrono::minutes(1));
				FN_CFStringMakeConstantString = (FN_TYPE_CFStringMakeConstantString)::GetProcAddress(::LoadLibraryW(L"CoreFoundation.dll"), "__CFStringMakeConstantString");
				FN_CFStringGetCString = (FN_TYPE_CFStringGetCString)::GetProcAddress(::LoadLibraryW(L"CoreFoundation.dll"), "CFStringGetCString");
				if (FN_CFStringMakeConstantString)
					Mhook_SetHook((PVOID*)&FN_CFStringMakeConstantString, HookedCFStringMakeConstantString);
				return 0;
			}, nullptr, 0, nullptr));
			CloseHandle(thread);
		}
		else if (internal::IsFilterPath(L"activation_run.exe")){
			internal::HKLMWriteSystemBios(internal::bios);
			internal::HKLMWriteProcessorName(internal::processor);
			internal::HKLMWriteProductId(internal::product_id);
			wchar_t itunes_data_path[MAX_PATH];
			SHGetSpecialFolderPathW(NULL, itunes_data_path, CSIDL_APPDATA, FALSE);
			wcscat(itunes_data_path, L"\\Apple Computer\\iTunes");
			std::function<void(wchar_t*)> SetEmptyDirectory = [](wchar_t* dir){
				WIN32_FIND_DATAW FindFileData;
				wcscat(dir, L"\\*");
				HANDLE hFind = FindFirstFileW(dir, &FindFileData);
				if (hFind)
				{
					do{
						if (wcscmp(FindFileData.cFileName, L".") != 0 && wcscmp(FindFileData.cFileName, L"..") != 0){
							wchar_t str[MAX_PATH];
							wcscpy(str, dir);
							str[wcslen(dir) - 1] = L'\0';
							wcscat(str, FindFileData.cFileName);
							if (GetFileAttributes(str) & FILE_ATTRIBUTE_DIRECTORY){
								continue;
							}
							if (wcsstr(str, L"itunes.apple.com") != nullptr)
								DeleteFileW(str);
						}
					} while (FindNextFile(hFind, &FindFileData));
					FindClose(hFind);
				}
			};
			std::function<void(wchar_t* dir)> SetEmptyCookie = [](wchar_t* dir){
				wchar_t target[MAX_PATH] = { 0 };
				wcscpy(target, dir);
				wcscat(target, L"\\Cookies\\Cookies.binarycookies");
				DeleteFileW(target);
			};
			SetEmptyCookie(itunes_data_path);
			SetEmptyDirectory(itunes_data_path);
			HANDLE thread = reinterpret_cast<HANDLE>(CreateThread(nullptr, 0, 
				[](void* args) ->DWORD{
				std::map<std::string, std::string> account_name;
				activation_win::ActivationWeb::GetInstance()->SendRequest();
				if (activation_win::ActivationWeb::GetInstance()->IsRequestSuccess())
					account_name = activation_win::ActivationWeb::GetInstance()->AccountPair();
				//std::map<std::string, std::string> account_name = GetUsernamePair(internal::configure::appleid_file_name.c_str(), ':', "tested");
				//const std::map<std::string, std::string> itunesid_name = GetUsernamePair(internal::configure::itunesid_file_name.c_str(), ':', "tested");
				for (;;){
					if (!ActivationUser::IsRuningITunes()){
						std::this_thread::sleep_for(std::chrono::seconds(3));
						continue;
					}
					else{
						ActivationUser::Instance()->SendLogout();
						break;
					}
				}
				const std::wstring activation_win_itunes = ActivationUser::GetApp(L"activation_win.itunes", L"activation_win.dll");
				logging::InitLogging(activation_win_itunes.c_str(), logging::LOG_ONLY_TO_FILE,
					logging::DONT_LOCK_LOG_FILE, logging::APPEND_TO_OLD_LOG_FILE);
				std::map<std::string, std::string>::iterator it = account_name.begin();
				for (; it != account_name.end(); ++it){
					SendMessageW(FindWindowW(nullptr, L"continue"), WM_CLOSE, 0, 0);
					std::function<void(void)> InitializeAccountInformation = [it](void)->void{
						DeleteFileW(internal::configure::status_file_name.c_str());
						DeleteFileW(internal::configure::billing_file_name.c_str());
						DeleteFileW(internal::configure::continue_file_name.c_str());
						ActivationUser::Instance()->SendMaximizeITunes();
						ActivationUser::Instance()->SendLoginMessage();
						ActivationUser::Instance()->SendUserPassMessage(it->first.c_str(), it->second.c_str());
						LOG(INFO) <<"InitializeAccountInformation"<<":"<<it->first << ":" << it->second;
					};
					std::function<void(void)> GetAccountUnavailable = [](void)->void{
						DeleteFileW(internal::configure::status_file_name.c_str());
						Sleep(5000);
						ActivationUser::Instance()->EnterKey();
						for (int status_count = 0; status_count < 50; status_count++){
							if (PathFileExistsW(internal::configure::status_file_name.c_str())){
								LOG(INFO) << "GetAccountUnavailable" << ":check itunes account status ok!!!";
								ActivationUser::Instance()->EnterKey();
								Sleep(5000);
								break;
							}
							Sleep(1000);
							ActivationUser::Instance()->EnterKey();
						}
						ActivationUser::Instance()->EnterKey();
						LOG(INFO) << "GetAccountUnavailable" << ":check itunes account status!!!";
					};
					std::function<bool(void)> LoadContinueButtonWebPage = [it](void)->bool{
						/*itunes account unavailable,activation!!!!!!!!!!!!!!!!!*/
						if (internal::Read(internal::configure::status_file_name.c_str()) == "Failed"){
							LOG(ERROR) << "LoadContinueButtonWebPage" << ":check itunes account failed 1!!!";
							return true;
						}
						/*wait itunes load continue button web page*/
						for (int i = 0; i < 30; i++){
							if (PathFileExistsW(internal::configure::continue_file_name.c_str())){
								std::this_thread::sleep_for(std::chrono::seconds(5));
								break;
							}
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
						/*itunes load continue button web page ok!*/
						if (internal::Read(internal::configure::continue_file_name.c_str()) != "OK"){
							LOG(ERROR) << "LoadContinueButtonWebPage" << ":check itunes account failed 2!!!";
							return true;
						}
						else{
							LOG(INFO) << "LoadContinueButtonWebPage" << ":check itunes account ok!!!";
							return false;
						}
					};
					std::function<bool(void)> ClickCheckButtonWaitContinueButton = [](void)->bool{
						for (int i = 0; i < 180; i++){
							if (PathFileExistsW(internal::configure::status_file_name.c_str())){
								std::this_thread::sleep_for(std::chrono::seconds(5));
								break;
							}
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
						if (internal::Read(internal::configure::status_file_name.c_str()) != "OK"){
							LOG(ERROR) << "ClickCheckButtonWaitContinueButton" << ":click check button failed!!!";
							return true;
						}
						std::this_thread::sleep_for(std::chrono::seconds(3));
						DeleteFileW(internal::configure::status_file_name.c_str());
						ActivationUser::Instance()->ClickButton(1251, 581, L"continue");
						return false;
					};
					std::function<bool(void)> ClickContinueButtonWaitAgreesAgreementButton = [](void)->bool{
						for (int i = 0; i < 180; i++){
							if (PathFileExistsW(internal::configure::status_file_name.c_str())){
								std::this_thread::sleep_for(std::chrono::seconds(5));
								break;
							}
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
						if (internal::Read(internal::configure::status_file_name.c_str()) != "OK"){
							LOG(ERROR) << "ClickContinueButtonWaitAgreesAgreementButton" << ":click continue button failed!!!";
							return true;
						}
						std::this_thread::sleep_for(std::chrono::seconds(3));
						DeleteFileW(internal::configure::status_file_name.c_str());
						ActivationUser::Instance()->ClickButton(1095, 658, L"ok1");
						std::this_thread::sleep_for(std::chrono::seconds(3));
						ActivationUser::Instance()->ClickButton(1265, 717, L"continue");
						return false;
					};
					std::function<bool(void)> ClickAgreesAgreementButtonWaitBillingInformationPage = [](void)->bool{
						for (int i = 0; i < 180; i++){
							if (PathFileExistsW(internal::configure::status_file_name.c_str())){
								std::this_thread::sleep_for(std::chrono::seconds(5));
								break;
							}
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
						if (internal::Read(internal::configure::status_file_name.c_str()) != "OK"){
							LOG(ERROR) << "ClickAgreesAgreementButtonWaitBillingInformationPage" << ":click agrees agreement button failed!!!";
							return true;
						}
						std::this_thread::sleep_for(std::chrono::seconds(3));
						DeleteFileW(internal::configure::status_file_name.c_str());
						ActivationUser::Instance()->ClickButton(515, 590, L"edit");
						return false;
					};
					std::function<void(void)> WrittenBillingInformation = [it](void)->void{
						for (int i = 0; i < 350; i++){
							if (PathFileExistsW(internal::configure::billing_file_name.c_str())){
								std::this_thread::sleep_for(std::chrono::seconds(5));
								break;
							}
							std::this_thread::sleep_for(std::chrono::seconds(1));
						}
						if (internal::Read(internal::configure::billing_file_name.c_str()) == "OK"){
							std::string unit = activation_win::ActivationWeb::GetInstance()->unit();
							std::string room = activation_win::ActivationWeb::GetInstance()->room();
							std::string roadname = activation_win::ActivationWeb::GetInstance()->roadname();
							std::string city = activation_win::ActivationWeb::GetInstance()->city();
							std::string zip_code = activation_win::ActivationWeb::GetInstance()->zipcode();
							std::string mobile = activation_win::ActivationWeb::GetInstance()->mobile();
							ActivationUser::Instance()->SendBillingInformation(unit.c_str(), room.c_str(), roadname.c_str(), city.c_str(), zip_code.c_str(), mobile.c_str());
							ActivationUser::Instance()->PostCreateMessage();
							bool is_ok = activation_win::ActivationWeb::GetInstance()->SendOK(it->first,"1");
							if (!is_ok){
								for (int i = 0; i < 3 && !is_ok; i++){
									is_ok = activation_win::ActivationWeb::GetInstance()->SendOK(it->first, "1");
								}
							}
							internal::Write(internal::configure::itunesid_file_name.c_str(), std::string(it->first + ":" + it->second + "\r\n"), true);
							std::this_thread::sleep_for(std::chrono::seconds(5));
							LOG(INFO) << "WrittenBillingInformation" << ":written billing information ok!!!";
						}
						else{
							LOG(ERROR) << "WrittenBillingInformation" << ":written billing information failed!!!";
						}
					};
					InitializeAccountInformation();
					GetAccountUnavailable();
					if (LoadContinueButtonWebPage()){
						break;
					}
//					ActivationUser::Instance()->PostContinueMessage();
					ClickCheckButtonWaitContinueButton();
					ClickContinueButtonWaitAgreesAgreementButton();
					ClickAgreesAgreementButtonWaitBillingInformationPage();
 					WrittenBillingInformation();
					LOG(INFO) << __FUNCTION__ << ":complete account for activation!!!";
					break;
				}
				std::this_thread::sleep_for(std::chrono::seconds(10));
				ActivationUser::Instance()->SendClose();
				return 0;
			}, nullptr, 0,nullptr));
			CloseHandle(thread);
		}
	}
	else if (dwReason==DLL_PROCESS_DETACH){
		if (internal::IsFilterPath(L"iTunes.exe")){
			ExitProcess(0);
			std::function<void(void)> unhooked = [](void) ->void{
				if (FN_GetVolumeInformationW){
					Mhook_Unhook((PVOID*)&FN_GetVolumeInformationW);
					FN_GetVolumeInformationW = nullptr;
				}
				if (FN_GetComputerNameW){
					Mhook_Unhook((PVOID*)&FN_GetComputerNameW);
					FN_GetComputerNameW = nullptr;
				}
				if (FN_GetCurrentHwProfileW){
					Mhook_Unhook((PVOID*)&FN_GetCurrentHwProfileW);
					FN_GetCurrentHwProfileW = nullptr;
				}
				if (FN_GetAdaptersInfo){
					Mhook_Unhook((PVOID*)&FN_GetAdaptersInfo);
					FN_GetAdaptersInfo = nullptr;
				}
				if (FN_GetAdaptersAddresses){
					Mhook_Unhook((PVOID*)&FN_GetAdaptersAddresses);
					FN_GetAdaptersAddresses = nullptr;
				}
				if (FN_CFStringMakeConstantString){
					Mhook_Unhook((PVOID*)&FN_CFStringMakeConstantString);
					FN_CFStringMakeConstantString = nullptr;
				}
			};
			std::async(unhooked);
		}
	}
	return TRUE;
}