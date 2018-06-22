#ifndef ACTIVATION_WIN_H_
#define ACTIVATION_WIN_H_
//////////////////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN
#include <atlbase.h>
#include <Windows.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include "activation_user.h"
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"Shell32.lib")
//////////////////////////////////////////////////////////////////////////
#ifdef ACTIVATION_WIN_EXPORTS
#define ACTIVATION_WIN_API __declspec(dllexport)
#else
#define ACTIVATION_WIN_API __declspec(dllimport)
#endif
//////////////////////////////////////////////////////////////////////////
#endif