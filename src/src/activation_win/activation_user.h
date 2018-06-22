#ifndef ACTIVATION_USER_H_
#define ACTIVATION_USER_H_

#include <string>
#include <windows.h>

namespace activation_win{
	class ActivationUser
	{
	public:
		ActivationUser();
		virtual ~ActivationUser();
		void SendMaximizeITunes();
		void SendLogout();
		void SendLoginMessage();
		void SendUserPassMessage(const char* account_name, const char* password);
		void EnterKey();
		void PostContinueMessage();
		void ClickButton(int x, int y, const wchar_t* button_name);
		void SendBillingInformation(const char* unit, const char* room, const char* roadname, const char* city, const char* zip_code, const char* phone_number);
		void PostCreateMessage();
		void SendClose();
		static std::wstring GetApp(const wchar_t *name, const wchar_t* module_name = nullptr);
		static bool IsRuningITunes();
		static ActivationUser* Instance(){
			static ActivationUser* info;
			if (!info){
				ActivationUser* new_info = new ActivationUser;
				if (InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&info), new_info, NULL)){
					delete new_info;
				}
			}
			return info;
		}
	private:
		const wchar_t* class_name() const{
			return class_name_.c_str();
		}
		const wchar_t* window_name() const{
			return window_name_.c_str();
		}
		std::wstring class_name_;
		std::wstring window_name_;
	};
}

#endif