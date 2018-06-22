#include "activation_web.h"
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <atlconv.h>
#include <json/json.h>
#include "win_https.h"
#pragma comment(lib,"Winhttp.lib")

namespace activation_win{
	ActivationWeb::ActivationWeb(){
		CleanupString();
	}
	ActivationWeb::~ActivationWeb(){
		CleanupString();
	}
	void ActivationWeb::SendRequest(){
		do {
			CleanupString();
			std::string response = internal::ReadHTTP(L"103.231.68.4", L"/appchina-ios-backend/admin/appleaccount/automated/info/random.json", 22322);
			if (response.size()){
				SplitAccountJSON(response);
			}
			std::this_thread::sleep_for(std::chrono::seconds(3));
		} while (is_mail_server_login_except()||!IsRequestSuccess());
	}
	bool ActivationWeb::SendOK(const std::string& name, const std::string& status){
		const std::string url = std::string("/appchina-ios-backend/admin/appleaccount/automated/info/modify.json?accountName=") + name + std::string("&activeStatus=") + status;
		USES_CONVERSION;
		std::string response = internal::ReadHTTP(L"103.231.68.4", A2W(url.c_str()), 22322);
		if (response.find("true")!=std::string::npos)
			return true;
		else
			return false;
	}
	bool ActivationWeb::SplitAccountJSON(const std::string& response_json){
		//http://103.231.68.4:22322/appchina-ios-backend/admin/appleaccount/automated/info/random.json
		Json::Value root;
		Json::Reader reader;
		bool parsing_successful = reader.parse(response_json.c_str(), root);
		if (!parsing_successful)
			return false;
		bool status = root.get("status", "1").asBool();
		std::string message = root.get("message", "").asString();
		Json::Value data = root.get("data", "{}");
		if (!data.empty()){
			account_name_ = data.get("accountName", "").asString();
			password_ = data.get("password", "").asString();
		}
		Json::Value address = data.get("address", "{}");
		if (!address.empty()){
			zipcode_ = address.get("zipcode", "").asString();
			roadname_ = address.get("roadname", "").asString();
			unit_ = address.get("unit", "").asString();
			room_ = address.get("room", "").asString();
			mobile_ = address.get("mobile", "").asString();
			city_ = address.get("city", "").asString();
		}
		if (status || message!="ok"){
			CleanupString();
			parsing_successful = false;
		}
		if (data.get("firstName", "").asString()=="mail_server_login_except"){
			is_mail_server_login_except_ = true;
		}
		return parsing_successful;
	}
	std::map<std::string, std::string> ActivationWeb::AccountPair(){
		std::map<std::string, std::string> account_pair;
		if (IsRequestSuccess()){
			account_pair[account_name_] = password_;
		}
		return account_pair;
	}
}
