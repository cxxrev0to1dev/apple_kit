#ifndef ACTIVATION_WEB_H_
#define ACTIVATION_WEB_H_

#include <map>
#include <string>
#include <windows.h>

namespace activation_win{
	class ActivationWeb
	{
	public:
		ActivationWeb();
		~ActivationWeb();
		void SendRequest();
		bool SendOK(const std::string& name, const std::string& status);
		bool SplitAccountJSON(const std::string& response_json);
		std::map<std::string, std::string> AccountPair();
		bool IsRequestSuccess() const{
			return (account_name_.size() && password_.size() && mobile_.size() && room_.size() && unit_.size() && roadname_.size() && zipcode_.size() && city_.size());
		}
		std::string account_name() const{
			return account_name_;
		}
		std::string password() const{
			return password_;
		}
		std::string mobile() const{
			return mobile_;
		}
		std::string room() const{
			return room_;
		}
		std::string unit() const{
			return unit_;
		}
		std::string roadname() const{
			return roadname_;
		}
		std::string zipcode() const{
			return zipcode_;
		}
		std::string city() const{
			return city_;
		}
		static ActivationWeb* GetInstance(){
			static ActivationWeb* info;
			if (!info){
				ActivationWeb* new_info = new ActivationWeb;
				if (InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&info), new_info, NULL)){
					delete new_info;
				}
			}
			return info;
		}
	private:
		inline void CleanupString(){
			is_mail_server_login_except_ = false;
			account_name_.resize(0);
			password_.resize(0);
			mobile_.resize(0);
			room_.resize(0);
			unit_.resize(0);
			roadname_.resize(0);
			zipcode_.resize(0);
		}
		inline bool is_mail_server_login_except() const{
			return is_mail_server_login_except_;
		}
		std::string account_name_;
		std::string password_;
		std::string mobile_;
		std::string room_;
		std::string unit_;
		std::string roadname_;
		std::string zipcode_;
		std::string city_;
		bool is_mail_server_login_except_;
	};
}

#endif