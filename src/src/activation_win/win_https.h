#ifndef ITUNES_PLUS_HTTPS_H_
#define ITUNES_PLUS_HTTPS_H_

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

namespace activation_win{
	namespace URL{
		static std::string Encode(const std::string &value){
			std::ostringstream escaped;
			escaped.fill('0');
			escaped << std::hex;
			for(std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
				std::string::value_type c = (*i);
				if (isalnum((int)c) || c == '-' || c == '_' || c == '.' || c == '~') {
					escaped << c;
					continue;
				}
				escaped << '%' << std::setw(2) << int((unsigned char) c);
			}
			return escaped.str();
		}
	}
	namespace internal{
		std::string ReadHTTPS(const wchar_t* domain,
			const wchar_t* path,
			const wchar_t* header,
			const wchar_t* referer=NULL,
			const char* port=NULL);
		std::string SendHTTPS(const wchar_t* domain,
			const wchar_t* path,
			const void* src,
			const size_t length,
			const wchar_t* header,
			const wchar_t* referer=NULL,
			const char* post = NULL);
		std::string ReadHTTP(const wchar_t* domain,
			const wchar_t* path,const int port=0);
		std::string SendHTTP(const wchar_t* domain,
			const wchar_t* path,
			const void* src,
			const size_t length,
			const wchar_t* app_header = NULL);
	}
}

#endif